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
 *
 * FILE
 *     battery_service.c
 *
 * DESCRIPTION
 *     This file defines routines for using the Battery Service.
 *
 
 ****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *===========================================================================*/

#include <gatt.h>           /* GATT application interface */
#include <battery.h>        /* Read the battery voltage */
#include <buf_utils.h>      /* Buffer functions */

/*============================================================================*
 *  Local Header Files
 *===========================================================================*/

#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */
#include "battery_service.h"/* Interface to this file */
#include "nvm_access.h"     /* Non-volatile memory access */
#include "app_gatt_db.h"    /* GATT database definitions */

/*============================================================================*
 *  Private Data Types
 *===========================================================================*/

/* Battery Service data type */
typedef struct _BATT_DATA_T
{
    /* Battery Level in percent */
    uint8   level;

    /* Client configuration descriptor for Battery Level characteristic */
    gatt_client_config level_client_config;

    /* NVM Offset at which Battery data is stored */
    uint16 nvm_offset;

} BATT_DATA_T;

/*============================================================================*
 *  Private Data
 *===========================================================================*/

/* Battery Service data instance */
static BATT_DATA_T g_batt_data;

/*============================================================================*
 *  Private Definitions
 *===========================================================================*/

/* Battery full level as a percentage */
#define BATTERY_LEVEL_FULL                            (100)

/* Battery critical level as a percentage */
#define BATTERY_CRITICAL_LEVEL                        (10)

/* Battery minimum and maximum voltages in mV */
#define BATTERY_FULL_BATTERY_VOLTAGE                  (3000)          /* 3.0V */
#define BATTERY_FLAT_BATTERY_VOLTAGE                  (1800)          /* 1.8V */

/* Number of words of NVM memory used by Battery Service */
#define BATTERY_SERVICE_NVM_MEMORY_WORDS              (1)

/* The offset of data being stored in NVM for the Battery Service. This offset
 * is added to the Battery Service offset in the NVM region (see
 * g_batt_data.nvm_offset) to get the absolute offset at which this data is
 * stored in NVM.
 */
#define BATTERY_NVM_LEVEL_CLIENT_CONFIG_OFFSET        (0)

/*============================================================================*
 *  Private Function Prototypes
 *===========================================================================*/

/* Read the battery level */
static uint8 readBatteryLevel(void);

/*============================================================================*
 *  Private Function Implementations
 *===========================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      readBatteryLevel
 *
 *  DESCRIPTION
 *      This function reads the battery level.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Battery level in percent
 *----------------------------------------------------------------------------*/
static uint8 readBatteryLevel(void)
{
    uint32 bat_voltage;                 /* Battery voltage in mV */
    uint32 bat_level;                   /* Battery level in percent */

    /* Read battery voltage and level it with minimum voltage */
    bat_voltage = BatteryReadVoltage();

    /* Level the read battery voltage to the minimum value */
    if(bat_voltage < BATTERY_FLAT_BATTERY_VOLTAGE)
    {
        bat_voltage = BATTERY_FLAT_BATTERY_VOLTAGE;
    }

    bat_voltage -= BATTERY_FLAT_BATTERY_VOLTAGE;

    /* Get battery level in percent */
    bat_level = (bat_voltage * 100) / (BATTERY_FULL_BATTERY_VOLTAGE - 
                                                  BATTERY_FLAT_BATTERY_VOLTAGE);

    /* Check the precision errors */
    if(bat_level > 100)
    {
        bat_level = 100;
    }

    /* Return the battery level (as a percentage of full) */
    return (uint8)bat_level;
}

/*============================================================================*
 *  Public Function Implementations
 *===========================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise the Battery Service data structure.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryDataInit(void)
{
    if(!IsDeviceBonded())
    {
        /* Initialise battery level client configuration characterisitic
         * descriptor value only if device is not bonded
         */
        g_batt_data.level_client_config = gatt_client_config_none;
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryInitChipReset
 *
 *  DESCRIPTION
 *      This function is used to initialise the Battery Service data structure
 *      at chip reset.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryInitChipReset(void)
{
    /* Initialise battery level to 0 percent so that the battery level 
     * notification (if configured) is sent when the value is read for 
     * the first time after power cycle.
     */
    g_batt_data.level = 0;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on Battery Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;                  /* Length of attribute data, octets */
    uint8  value[2];                    /* Attribute value */
    uint8 *p_val = NULL;                /* Pointer to attribute value */
    sys_status rc = sys_status_success; /* Function status */

    switch(p_ind->handle)
    {

        case HANDLE_BATT_LEVEL:
        {
            /* Read the battery level */
            length = 1; /* One Octet */

            g_batt_data.level = readBatteryLevel();

            value[0] = g_batt_data.level;
        }
        break;

        case HANDLE_BATT_LEVEL_C_CFG:
        {
            /* Read the client configuration descriptor for the battery level
             * characteristic.
             */
            length = 2; /* Two Octets */
            p_val = value;

            BufWriteUint16((uint8 **)&p_val, g_batt_data.level_client_config);
        }
        break;

        default:
            /* No more IRQ characteristics */
            rc = gatt_status_read_not_permitted;
        break;

    }

    /* Send ACCESS RESPONSE */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, length, value);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on Battery Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    uint8 *p_value = p_ind->value;      /* New attribute value */
    uint16 client_config;               /* Client configuration descriptor */
    sys_status rc = sys_status_success; /* Function status */

    switch(p_ind->handle)
    {
        case HANDLE_BATT_LEVEL_C_CFG:
        {
            /* Write the client configuration descriptor for the battery level
             * characteristic.
             */
            client_config = BufReadUint16(&p_value);

            /* Only notifications are allowed for this client configuration 
             * descriptor.
             */
            if((client_config == gatt_client_config_notification) ||
               (client_config == gatt_client_config_none))
            {
                g_batt_data.level_client_config = client_config;

                /* Write battery level client configuration to NVM if the 
                 * device is bonded.
                 */
                if(IsDeviceBonded())
                {
                     Nvm_Write(&client_config,
                              sizeof(client_config),
                              g_batt_data.nvm_offset + 
                              BATTERY_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
                }
            }
            else
            {
                /* INDICATION or RESERVED */

                /* Return error as only notifications are supported */
                rc = gatt_status_app_mask;
            }

        }
        break;


        default:
            rc = gatt_status_write_not_permitted;
        break;

    }

    /* Send ACCESS RESPONSE */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);

    /* Send an update as soon as notifications are configured */
    if(g_batt_data.level_client_config == gatt_client_config_notification)
    {
        /* Reset current battery level to an invalid value so that it 
         * triggers notifications on reading the current battery level 
         */
        g_batt_data.level = 0xFF; /* 0 to 100: Valid value range */

        /* Update the battery level and send notification. */
        BatteryUpdateLevel(p_ind->cid);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryUpdateLevel
 *
 *  DESCRIPTION
 *      This function is to monitor the battery level and trigger notifications
 *      (if configured) to the connected host.
 *
 *  PARAMETERS
 *      ucid [in]               Connection ID of the host
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryUpdateLevel(uint16 ucid)
{
    uint8 old_vbat;                     /* Previous battery level, percent */
    uint8 cur_bat_level;                /* Current battery level, percent */

    /* Read the battery level */
    cur_bat_level = readBatteryLevel();

    old_vbat = g_batt_data.level;

    /* If the current and old battery level are not same, update the  connected
     * host if notifications are configured.
     */
    if(old_vbat != cur_bat_level)
    {

        if((ucid != GATT_INVALID_UCID) &&
           (g_batt_data.level_client_config == gatt_client_config_notification))
        {

            GattCharValueNotification(ucid, 
                                      HANDLE_BATT_LEVEL, 
                                      1, &cur_bat_level);

            /* Update Battery Level characteristic in database */
            g_batt_data.level = cur_bat_level;

        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryReadDataFromNVM
 *
 *  DESCRIPTION
 *      This function is used to read Battery Service specific data stored in 
 *      NVM.
 *
 *  PARAMETERS
 *      p_offset [in]           Offset to Battery Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryReadDataFromNVM(uint16 *p_offset)
{

    g_batt_data.nvm_offset = *p_offset;

    /* Read NVM only if devices are bonded */
    if(IsDeviceBonded())
    {
        /* Read battery level client configuration descriptor */
        Nvm_Read((uint16*)&g_batt_data.level_client_config,
                sizeof(g_batt_data.level_client_config),
                *p_offset + 
                BATTERY_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
    }

    /* Increment the offset by the number of words of NVM memory required 
     * by the Battery Service 
     */
    *p_offset += BATTERY_SERVICE_NVM_MEMORY_WORDS;

}
/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryWriteDataToNVM
 *
 *  DESCRIPTION
 *      This function is used to writes Battery Service specific data to 
 *      NVM.
 *
 *  PARAMETERS
 *      p_offset [in]           Offset to Beacon Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryWriteDataToNVM(uint16 *p_offset)
{

    g_batt_data.nvm_offset = *p_offset;
    
    /* Increment the offset by the number of words of NVM memory required 
     * by the Beacon Service 
     */
    *p_offset += BATTERY_SERVICE_NVM_MEMORY_WORDS;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the Battery 
 *      Service.
 *
 *  PARAMETERS
 *      handle [in]             Handle to check
 *
 *  RETURNS
 *      TRUE if handle belongs to the Battery Service, FALSE otherwise
 *----------------------------------------------------------------------------*/
extern bool BatteryCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_BATTERY_SERVICE) &&
            (handle <= HANDLE_BATTERY_SERVICE_END))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryBondingNotify
 *
 *  DESCRIPTION
 *      This function is used by application to notify bonding status to the
 *      Battery Service.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BatteryBondingNotify(void)
{

    /* Write data to NVM if bond is established */
    if(IsDeviceBonded())
    {
        /* Write to NVM the client configuration value of battery level 
         * that was configured prior to bonding 
         */
        Nvm_Write((uint16*)&g_batt_data.level_client_config, 
                  sizeof(g_batt_data.level_client_config), 
                  g_batt_data.nvm_offset + 
                  BATTERY_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
    }

}
