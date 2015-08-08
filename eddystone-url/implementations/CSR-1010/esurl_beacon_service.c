/******************************************************************************
 *    Copyright (c) 2015 Cambridge Silicon Radio Limited 
 *    All rights reserved.
 * 
 *    Redistribution and use in source and binary forms, with or without modification, 
 *    are permitted (subject to the limitations in the disclaimer below) provided that the
 *    following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this list of 
 *    conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 *    and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * 
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *  Copyright Google 2014
 *
 * FILE
 *     esurl_beacon_service.c
 *
 * DESCRIPTION
 *     This file defines routines for using the Beacon Configuration Service.
 *
  *
 ****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *===========================================================================*/

#include <gatt.h>           /* GATT application interface */
#include <buf_utils.h>      /* Buffer functions */
#include <mem.h>            /* Memory routines */
#include <ls_app_if.h>      /* Link supervisor interface e.g. TX Power */

/*============================================================================*
 *  Local Header Files
 *===========================================================================*/

#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */
#include "esurl_beacon_service.h" /* Interface to this file */
#include "beaconing.h"      /* Beaconing routines */
#include "nvm_access.h"     /* Non-volatile memory access */
#include "app_gatt_db.h"    /* GATT database definitions */

/*============================================================================*
 *  Constants Arrays  
 *============================================================================*/ 
/* Note AD Types referenced below can be found at the site below */
/* www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile */

/* Esurl Beacon ADV header */
unsigned char adv_service_hdr[] =
{
    0x03, // Length of Service List
            0x03, // AD Type: Service List 
            0xAA, // Esurl Beacon Service Data UUID LSB
            0xFE // Esurl Beacon Service Data UUID MSB      
        };

/* Esurl Beacon Service Data Param and Bluetooth SIG assigned 16-bit UUID */
unsigned char adv_service_data_hdr[] = 
{
    0x16, // AD Type: Service Data
            0xAA, // Esurl Beacon Service Data UUID LSB
            0xFE  // Esurl Beacon Service Data UUID MSB
        };

/* Initialise Uri to http://eddystone.org for a new beacon (using compression) */
unsigned char initial_uri[] =
{
    0x02, 'p', 'h', 'y', 's', 'i', 'c', 'a', 'l', '-', 'w', 'e', 'b', 0x08 
        };
    
/* Esurl Beacon Adv TX calibration for packets Low to high */
unsigned char adv_tx_power_levels[] =
{    ADV_TX_POWER_FOR_NEG_18,  // 0 LOWEST
     ADV_TX_POWER_FOR_NEG_10,  // 1 LOW (DEFAULT)
     ADV_TX_POWER_FOR_NEG_2,   // 2 MEDIUM
     ADV_TX_POWER_FOR_POS_6    // 3 HIGH
};

/* Esurl Beacon Radio TX calibration  range low to high */
unsigned char radio_tx_power_levels[] =
{    RADIO_TX_POWER_NEG_18,    // 0 LOWEST
     RADIO_TX_POWER_NEG_10,    // 1 LOW (DEFAULT)
     RADIO_TX_POWER_NEG_2,     // 2 MEDIUM 
     RADIO_TX_POWER_POS_6      // 3 HIGH 
 };     

/*============================================================================*
 *  Private Data Types
 *===========================================================================*/

/* Total size 28 bytes */
typedef struct _ESURL_BEACON_ADV_T
{
    /* Current beacon data value */
    uint8 service_hdr[sizeof(adv_service_hdr)];
    
    uint8 service_data_length;
    
    uint8 service_data_hdr[sizeof(adv_service_data_hdr)];
    
    uint8 flags;
    
    uint8 tx_power;
    
    uint8 uri_data[ESURL_BEACON_DATA_MAX];
    
} ESURL_BEACON_ADV_T;

/* Beacon data type */
typedef struct _ESURL_BEACON_DATA_T
{
    /* Current adv beacon size (does not include AD Flags)*/
    uint8 adv_length;
    
    /* Adv data */
    ESURL_BEACON_ADV_T adv;
    
    /* A boolean TRUE/FALSE value determining if the beacon is locked */
    uint8 lock_state;
    
    /* A 128-bit (16 byte) code for locking/unlocking the beacon */
    uint8 lock_code[ESURL_BEACON_LOCK_CODE_SIZE];
    
    /* A value 0-3 that is mapped to a pkt and radio val via the cal tables */
    uint8 tx_power_mode;
    
    /* A calibration table mapping tx_level_code to the packet code */
    uint8 adv_tx_power_levels[ESURL_BEACON_ADV_TX_POWER_LEVELS_SIZE];
    
    /* A calibration table mapping tx_level_code to the radio code */
    uint8 radio_tx_power_levels[ESURL_BEACON_RADIO_TX_POWER_LEVELS_SIZE];   
    
    /* Beacon period in milliseconds 0-65536ms */
    uint16 period;
    
} ESURL_BEACON_DATA_T;

/*============================================================================*
 *  Private Data
 *===========================================================================*/

/* Esurl Beacon Service data instance */
static ESURL_BEACON_DATA_T g_esurl_beacon_data;

/* Esurl Beacon nvm write flag indicates if the esurl_beacon_data is dirty */
static uint8 g_esurl_beacon_nvm_write_flag = FALSE;

/* Temporary buffer used for read/write characteristics */
static uint8 g_esurl_beacon_buf[ESURL_BEACON_PERIOD_SIZE];

/* NVM Offset at which ESURL BEACON data is stored */
static uint16 g_esurl_beacon_nvm_offset;


/*============================================================================*
 *  Public Function Implementations
 *===========================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise the Beacon Service data structure.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconDataInit(void)
{
    /* Data initialized from NVM during readPersistentStore */
    
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconInitChipReset
 *
 *  DESCRIPTION
 *      This function is used to initialise the Beacon Service data structure
 *      at chip reset.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconInitChipReset(void)
{
    /* Initialize memory at reset before first NVM write */
    /* set the adv size to be the minimum (Just the header) */
    g_esurl_beacon_data.adv_length = BEACON_DATA_HDR_SIZE;    
    
    /* Init the ADV data */
    
    /* Init Beacon data hdr: 10 bytes*/
    MemCopy(g_esurl_beacon_data.adv.service_hdr, adv_service_hdr, sizeof(adv_service_hdr));
    g_esurl_beacon_data.adv.service_data_length = SERVICE_DATA_PRE_URI_SIZE;
    MemCopy(g_esurl_beacon_data.adv.service_data_hdr, adv_service_data_hdr, sizeof(adv_service_data_hdr));    
    g_esurl_beacon_data.adv.flags = FLAGS_DEFAULT;  
    g_esurl_beacon_data.adv.tx_power = ADV_TX_POWER_DEFAULT;
    
    /* Initialize uri_data memory in adv: 0 - 18 bytes */
    MemSet(g_esurl_beacon_data.adv.uri_data, 0, ESURL_BEACON_DATA_MAX); 
    /* Initialize uri_data with a URI:  http://physical-web.org */
    MemCopy(g_esurl_beacon_data.adv.uri_data, initial_uri, sizeof(initial_uri)); 
    g_esurl_beacon_data.adv.service_data_length = SERVICE_DATA_PRE_URI_SIZE + sizeof(initial_uri);
    g_esurl_beacon_data.adv_length = BEACON_DATA_HDR_SIZE + sizeof(initial_uri);    
    
    /* Managagement Data: Not included in transmitted ADV packet */
    
    /* Inialized lock to be unlocked */
    g_esurl_beacon_data.lock_state = FALSE;       
    
    /* Init memory in esurl beacon lock_code */
    MemSet(g_esurl_beacon_data.lock_code, 0, ESURL_BEACON_LOCK_CODE_SIZE);  
    
    /* tx mode values 0-3: this is looked up in the calibration tabs below */    
    g_esurl_beacon_data.tx_power_mode = TX_POWER_MODE_DEFAULT;  
    
    MemCopy(g_esurl_beacon_data.adv_tx_power_levels, adv_tx_power_levels, ESURL_BEACON_ADV_TX_POWER_LEVELS_SIZE);
    
    MemCopy(g_esurl_beacon_data.radio_tx_power_levels, radio_tx_power_levels, ESURL_BEACON_RADIO_TX_POWER_LEVELS_SIZE); 
    
    /* Update ADV pkt and RADIO based on the TX power mode default */
    EsurlBeaconUpdateTxPowerFromMode(TX_POWER_MODE_DEFAULT);    
    
    /* Set default period = 1000 milliseconds */
    g_esurl_beacon_data.period = 1000;
    
    /* Flag data structure needs writing to NVM */
    g_esurl_beacon_nvm_write_flag = TRUE;    
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on Beacon Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;                  /* Length of attribute data, octets */
    uint8 *p_val = NULL;                /* Pointer to attribute value */
    sys_status rc = sys_status_success; /* Function status */
    uint8 uri_data_size = 0;            /* Size of uri data */
    
    switch(p_ind->handle)
    {  
    case HANDLE_ESURL_BEACON_LOCK_STATE:
        length = sizeof(g_esurl_beacon_data.lock_state); 
        p_val = &g_esurl_beacon_data.lock_state; 
        break;
        
    case HANDLE_ESURL_BEACON_FLAGS:
        length = ESURL_BEACON_FLAGS_SIZE;
        p_val = &g_esurl_beacon_data.adv.flags;
        break; 
        
    case HANDLE_ESURL_BEACON_TX_POWER_MODE:
        length = sizeof(g_esurl_beacon_data.tx_power_mode); 
        p_val = &g_esurl_beacon_data.tx_power_mode;
        break;        
        
    case HANDLE_ESURL_BEACON_URI_DATA:
        
        uri_data_size = g_esurl_beacon_data.adv_length - BEACON_DATA_HDR_SIZE; 
        /* Return the beacon data & protect against overflow */
        if (uri_data_size > (ESURL_BEACON_DATA_MAX))
        {
            length = ESURL_BEACON_DATA_MAX;
        } 
        else
        {              
            length = uri_data_size;  
        }
        p_val = g_esurl_beacon_data.adv.uri_data;
        
        break;    
        
    case HANDLE_ESURL_BEACON_ADV_TX_POWER_LEVELS:
        length = sizeof(g_esurl_beacon_data.adv_tx_power_levels);
        p_val = g_esurl_beacon_data.adv_tx_power_levels;
        break; 
        
    case HANDLE_ESURL_BEACON_RADIO_TX_POWER_LEVELS:
        length = sizeof(g_esurl_beacon_data.radio_tx_power_levels);
        p_val = g_esurl_beacon_data.radio_tx_power_levels;
        break;     
        
    case HANDLE_ESURL_BEACON_PERIOD:          
        length = ESURL_BEACON_PERIOD_SIZE;
        g_esurl_beacon_buf[0] = g_esurl_beacon_data.period & 0xFF;        
        g_esurl_beacon_buf[1] = (g_esurl_beacon_data.period >> 8) & 0xFF;            
        p_val = g_esurl_beacon_buf;            
        break;         
        
        /* NO MATCH */
        
     default:
        /* No more IRQ characteristics */
        rc = gatt_status_read_not_permitted;
    }
    
    /* Send ACCESS RESPONSE */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, length, p_val);
    
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on Beacon Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    uint8 *p_value = p_ind->value;      /* New attribute value */
    uint8 p_size = p_ind->size_value;     /* New value length */    
    sys_status rc = sys_status_success; /* Function status */
    
    switch(p_ind->handle)
    {    
    case HANDLE_ESURL_BEACON_LOCK:
        /* Sanity check for the data size */
        if ((p_size != sizeof(g_esurl_beacon_data.lock_code)) &&
           (g_esurl_beacon_data.lock_state == FALSE))           
        {                
            rc = gatt_status_invalid_length;
        }
        else if ( g_esurl_beacon_data.lock_state == FALSE) 
        {
            MemCopy(g_esurl_beacon_data.lock_code, 
                    p_value,
                    sizeof(g_esurl_beacon_data.lock_code));
            
            /* Flag the lock is set */
            g_esurl_beacon_data.lock_state = TRUE;
            /* Flag state needs writing to NVM */
            g_esurl_beacon_nvm_write_flag = TRUE;
        } 
        else
        {
            rc = gatt_status_insufficient_authorization;            
        }
        break;
        
    case HANDLE_ESURL_BEACON_UNLOCK:
        /* Sanity check for the data size */
        if (p_size != sizeof(g_esurl_beacon_data.lock_code))
        {                
            rc = gatt_status_invalid_length;
        }
        /* if locked then process the unlock request */
        else if ( g_esurl_beacon_data.lock_state) 
        {
            if (MemCmp(p_value, 
                       g_esurl_beacon_data.lock_code,
                       sizeof(g_esurl_beacon_data.lock_code)) == 0)
            {
                /* SUCCESS: so unlock beacoon */
                g_esurl_beacon_data.lock_state = FALSE; 
                
                /* Flag state needs writing to NVM */
                g_esurl_beacon_nvm_write_flag = TRUE;                       
            } 
            else
            { /* UNLOCK FAILED */
                rc = gatt_status_insufficient_authorization;    
            }       
        }
        break;
        
    case HANDLE_ESURL_BEACON_URI_DATA:
        if (g_esurl_beacon_data.lock_state) {
            rc = gatt_status_insufficient_authorization;
        } 
        /* Sanity check for URI will fit in the Beacon */            
        else if (p_size > (ESURL_BEACON_DATA_MAX))
        {               
            rc = gatt_status_invalid_length;
        }
        /* Process the characteristic Write */
        else
        {                    
            int uri_data_size = p_size;
            
            /* Updated the URL in the beacon structure */
            MemCopy(g_esurl_beacon_data.adv.uri_data, p_value, uri_data_size);
            g_esurl_beacon_data.adv_length = uri_data_size + BEACON_DATA_HDR_SIZE;
            
            /* Write the new data service size into the ADV header */
            g_esurl_beacon_data.adv.service_data_length =
                    uri_data_size + SERVICE_DATA_PRE_URI_SIZE;
            
            /* Flag state needs writing to NVM */
            g_esurl_beacon_nvm_write_flag = TRUE;                  
        }
        break;     
        
    case HANDLE_ESURL_BEACON_FLAGS:
        if (g_esurl_beacon_data.lock_state)
        {
            rc = gatt_status_insufficient_authorization;
        }
        /* Sanity check for the data size */
        else if (p_size != sizeof(g_esurl_beacon_data.adv.flags))
        {                
            /* invalid length */
            rc = gatt_status_invalid_length;
        }
        /* Write the flags */
        else
        {
            g_esurl_beacon_data.adv.flags = p_value[0];
            
            /* Flag state needs writing to NVM */
            g_esurl_beacon_nvm_write_flag = TRUE;                   
        }
        break;        
        
        
    case HANDLE_ESURL_BEACON_TX_POWER_MODE:
        if (g_esurl_beacon_data.lock_state)
        {
            rc = gatt_status_insufficient_authorization;
        }
        /* Sanity check for the data size */
        else if(p_size != sizeof(g_esurl_beacon_data.tx_power_mode))
        {                
            rc = gatt_status_invalid_length;
        }
        /* Write the tx power value */
        else
        {       
            int tx_power_mode = p_value[0];
            if (( tx_power_mode >= TX_POWER_MODE_LOWEST) && 
                (tx_power_mode <= TX_POWER_MODE_HIGH))
            {
                g_esurl_beacon_data.tx_power_mode =  tx_power_mode; 
                
                /* NOTE: The effects of updating tx_power_mode here are turned
                 * into ADV and RADIO power updates on esurl beacon service disconnect 
                 * in the file gatt_access.c
                 */

                /* Flag state needs writing to NVM */
                g_esurl_beacon_nvm_write_flag = TRUE;
            } 
            else
            {
                rc = gatt_status_write_not_permitted;
            }  
        }
        break;
        
    case HANDLE_ESURL_BEACON_ADV_TX_POWER_LEVELS:
        if (g_esurl_beacon_data.lock_state)
        {
            rc = gatt_status_insufficient_authorization;
        }
        /* Sanity check for the data size */
        else if(p_size != ESURL_BEACON_ADV_TX_POWER_LEVELS_SIZE)
        {                
            /* invalid length */
            rc = gatt_status_invalid_length;
        }
        /* Do the calibration table write */
        else 
        {
            /* Updated the tx power calibration table for the pkt */
            MemCopy(g_esurl_beacon_data.adv_tx_power_levels, p_value, ESURL_BEACON_ADV_TX_POWER_LEVELS_SIZE);
            
            /* Flag state needs writing to NVM */
            g_esurl_beacon_nvm_write_flag = TRUE;               
        }     
        break;
        
    case HANDLE_ESURL_BEACON_RADIO_TX_POWER_LEVELS:
        if (g_esurl_beacon_data.lock_state)
        {
            rc = gatt_status_insufficient_authorization;
        }
        /* Sanity check for the data size */
        else if (p_size != ESURL_BEACON_RADIO_TX_POWER_LEVELS_SIZE)
        {                
            rc = gatt_status_invalid_length;
        }
        /* Update the adv tx power levels */
        else 
        {
            /* Updated the tx power calibration table for the pkt */
            MemCopy(g_esurl_beacon_data.adv_tx_power_levels, p_value, ESURL_BEACON_ADV_TX_POWER_LEVELS_SIZE);
            
            /* Flag state needs writing to NVM */
            g_esurl_beacon_nvm_write_flag = TRUE;                 
        }     
        break;        
        
    case HANDLE_ESURL_BEACON_PERIOD:
        if (g_esurl_beacon_data.lock_state)
        {
            rc = gatt_status_insufficient_authorization;
        }
        else if (p_size != ESURL_BEACON_PERIOD_SIZE)
        {
            rc = gatt_status_invalid_length;       
        }
        else
        {
            /* Write the period (little endian 16-bits in p_value) */;   
            g_esurl_beacon_data.period = p_value[0] + (p_value[1] << 8);  
            
            if ((g_esurl_beacon_data.period < BEACON_PERIOD_MIN) && (g_esurl_beacon_data.period != 0))
            { /* minimum beacon period is 100ms; zero turns off beaconing */
                g_esurl_beacon_data.period = BEACON_PERIOD_MIN;
            }
            /* Flag state needs writing to NVM */
            g_esurl_beacon_nvm_write_flag = TRUE;           
        }
        break;      
        
    case HANDLE_ESURL_BEACON_RESET:
        if (g_esurl_beacon_data.lock_state)
        {
            rc = gatt_status_insufficient_authorization;
            break;
        } 
        else if (p_size != ESURL_BEACON_RESET_SIZE)
        {
            rc = gatt_status_invalid_length;       
        }   
        else
        {
            /* Reset local data and NVM memory */
            EsurlBeaconInitChipReset();
            
            /* Update NVM from g_esurl_beacon_data */       
            EsurlBeaconWriteDataToNVM(NULL); 
        }
        break;
        
        default:
        rc = gatt_status_write_not_permitted;
    }
    
    /* Send ACCESS RESPONSE */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);
}
/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconGetData
 *
 *  DESCRIPTION
 *      This function returns the current value of the beacon data
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconGetData(uint8** data, uint8* data_size)
{
    /* return current values */
    *data = (uint8*) &g_esurl_beacon_data.adv;
    *data_size = g_esurl_beacon_data.adv_length;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconGetPeriod
 *
 *  DESCRIPTION
 *      This function returns the current value of the beacon period (1-65kms)
 *
 *  RETURNS
 *      Beacon Period (unit32)
 *----------------------------------------------------------------------------*/
extern uint32 EsurlBeaconGetPeriodMillis(void)
{
    /* return current value */
    return (uint32) g_esurl_beacon_data.period * (SECOND / 1000); 
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconReadDataFromNVM
 *
 *  DESCRIPTION
 *      This function is used to read Beacon Service specific data stored in 
 *      NVM.
 *
 *  PARAMETERS
 *      p_offset [in]           Offset to Beacon Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconReadDataFromNVM(uint16 *p_offset)
{
    g_esurl_beacon_nvm_offset = *p_offset;
    
    /* Read beacon data size */
    Nvm_Read((uint16*)&g_esurl_beacon_data, sizeof(g_esurl_beacon_data),
             g_esurl_beacon_nvm_offset);
    
    *p_offset += sizeof(g_esurl_beacon_data);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconWriteDataToNVM
 *
 *  DESCRIPTION
 *      This function is used to write Beacon Service specific data in memory to 
 *      NVM.
 *
 *  PARAMETERS
*      p_offset  [in]           Offset to EsurlBeacon Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void EsurlBeaconWriteDataToNVM(uint16 *p_offset)
{
    if (*p_offset != NULL) 
    { /* Update the stored nvm offset with the pointer arg */
        g_esurl_beacon_nvm_offset = *p_offset;    
    } else {
        /* Null arg, so the .nvm_offset is already set correctly */      
        *p_offset = g_esurl_beacon_nvm_offset;
    }
    
    /* Only write out the esurl beacon data if it is flagged dirty */
    if (g_esurl_beacon_nvm_write_flag) 
    {
        /* Write all esurl beacon service data into NVM */
        Nvm_Write((uint16*)&g_esurl_beacon_data, sizeof(g_esurl_beacon_data),
                  g_esurl_beacon_nvm_offset); 
        g_esurl_beacon_nvm_write_flag = FALSE;
    }
    
    *p_offset += sizeof(g_esurl_beacon_data);   
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EsurlBeaconCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the Beacon 
 *      Service.
 *
 *  PARAMETERS
 *      handle [in]             Handle to check
 *
 *  RETURNS
 *      TRUE if handle belongs to the Battery Service, FALSE otherwise
 *----------------------------------------------------------------------------*/
extern bool EsurlBeaconCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_ESURL_BEACON_SERVICE) &&
            (handle <= HANDLE_ESURL_BEACON_SERVICE_END));
}

/*----------------------------------------------------------------------------*
  *  NAME
  *      EsurlBeaconBondingNotify
  *
  *  DESCRIPTION
  *      This function is used by application to notify bonding status to the
  *      Beacon Service.
  *
  *  PARAMETERS
  *      None
  *
  *  RETURNS
  *      Nothing
  *----------------------------------------------------------------------------*/
extern void EsurlBeaconBondingNotify(void)
{
    /* Empty */
}

/*----------------------------------------------------------------------------*
  *  NAME
  *      EsurlBeaconUpdateTxFromMode
  *
  *  DESCRIPTION
  *      This function is used to set both the ADV and RADIO TX param in the
  *      Esurl Beacon packet
  *
  *  PARAMETERS
  *      None
  *
  *  RETURNS
  *      Nothing
  *----------------------------------------------------------------------------*/
extern void EsurlBeaconUpdateTxPowerFromMode(uint8 tx_power_mode)
{
    /* Update the pkt tx level here */
    g_esurl_beacon_data.adv.tx_power =
            g_esurl_beacon_data.adv_tx_power_levels[tx_power_mode];
    /* Update the radio tx level here */
    LsSetTransmitPowerLevel(
            g_esurl_beacon_data.radio_tx_power_levels[tx_power_mode]);
}

/*----------------------------------------------------------------------------*
  *  NAME
  *      EsurlBeaconGetTxPowerMode
  *
  *  DESCRIPTION
  *      This function is used find the last Tx Power Mode set by a client
  *
  *  PARAMETERS
  *      None
  *
  *  RETURNS
  *      uint8 : a power mode 0 - 3
  *----------------------------------------------------------------------------*/
extern uint8 EsurlBeaconGetTxPowerMode(void) 
{
    return g_esurl_beacon_data.tx_power_mode;
}