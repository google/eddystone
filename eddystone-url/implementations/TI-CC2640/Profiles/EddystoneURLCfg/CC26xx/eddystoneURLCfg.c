/**************************************************************************************************
  Filename:       eddystoneURLCfg.c

  Description:    This file contains the Eddystone URL Configuration service 
                  profile for use with the BLE sample application.

* Copyright (c) 2015, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* *  Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* *  Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* *  Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "ll.h"
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "eddystoneURLCfg.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        28

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// URL Configuration Service UUID
static const uint8_t urlCfgSvcUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_SVC_UUID)
};

// Lock State UUID
static const uint8_t urlCfgCharLockStateUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_LOCK_STATE_UUID)
};

// Lock UUID - Optional
static const uint8_t urlCfgCharLockUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_LOCK_UUID)
};

// Unlock UUID - Optional
static const uint8_t urlCfgCharUnlockUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_UNLOCK_UUID)
};

// URI Data UUID
static const uint8_t urlCfgCharURIDataUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_URI_DATA_UUID)
};
   
// Flags UUID
static const uint8_t urlCfgCharFlagsUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_FLAGS_UUID)
};

// Advertised TX Power Level UUID
static const uint8_t urlCfgCharAdvTXPwrLvlsUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_ADV_TX_PWR_LVLS_UUID)
};

// TX Power Mode UUID
static const uint8_t urlCfgCharTXPowerModeUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_TX_POWER_MODE_UUID)
};
   
// Beacon Period UUID
static const uint8_t urlCfgCharBeaconPeriodUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_BEACON_PERIOD_UUID)
};

// Reset UUID
static const uint8_t urlCfgCharResetUUID[ATT_UUID_SIZE] =
{
  EDDYSTONE_BASE_UUID_128(URLCFGSVC_RESET_UUID)
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

uint8    URIDataLen;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static urlCfgSvcCBs_t *urlCfgSvc_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// URL Configuration Service attribute
static CONST gattAttrType_t urlCfgService = { ATT_UUID_SIZE, urlCfgSvcUUID };

// Lock State Characteristic Properties
static uint8 urlCfgCharLockStateProps = GATT_PROP_READ;
// Lock State Characteristic Value
static uint8 urlCfgCharLockState = 0; // unlocked
// Lock State Characteristic User Description
static uint8 urlCfgCharLockStateUserDesc[] = "Lock State";

// Lock Characteristic Properties
static uint8 urlCfgCharLockProps = GATT_PROP_WRITE;
// Lock Characteristic Value
static uint8 urlCfgCharLock[16];  // should be initialized
// Lock Characteristic User Description
static uint8 urlCfgCharLockUserDesc[] = "Lock";

// Unlock Characteristic Properties
static uint8 urlCfgCharUnlockProps = GATT_PROP_WRITE;
// Lock Characteristic User Description
static uint8 urlCfgCharUnlockUserDesc[] = "UnLock";

// URI Data Characteristic Properties
static uint8 urlCfgCharURIDataProps = GATT_PROP_READ | GATT_PROP_WRITE;
// URI Data Characteristic Value
static uint8 urlCfgCharURIData[19];     // should be initialized
// URI Data Characteristic User Description
static uint8 urlCfgCharURIDataUserDesc[] = "URI Data";

// Flags Characteristic Properties
static uint8 urlCfgCharFlagsProps = GATT_PROP_READ | GATT_PROP_WRITE;
// Flags Characteristic Value
static uint8 urlCfgCharFlags = URLCFG_CHAR_FLAGS_DEFAULT;
// Flags Characteristic User Description
static uint8 urlCfgCharFlagsUserDesc[] = "Flags";

// Advertised TX Power Levels Characteristic Properties
static uint8 urlCfgCharAdvTXPwrLvlsProps = GATT_PROP_READ | GATT_PROP_WRITE;
// Advertised TX Power Levels Characteristic Value
static int8 urlCfgCharAdvTXPwrLvls[4] = {-20, -10, -2, 0};
// Advertised TX Power Levels Characteristic User Description
static uint8 urlCfgCharAdvTXPwrLvlsUserDesc[] = "Adv TX Pwr Lvls";

// TX Power Mode Characteristic Properties
static uint8 urlCfgCharTXPowerModeProps = GATT_PROP_READ | GATT_PROP_WRITE;
// TX Power Mode Characteristic Value
static uint8 urlCfgCharTXPowerMode = URLCFG_CHAR_TX_POWER_MODE_DEFAULT;
// TX Power Mode Characteristic User Description
static uint8 urlCfgCharTXPowerModeUserDesc[] = "TX Power Mode";

// Beacon Period Characteristic Properties
static uint8 urlCfgCharBeaconPeriodProps = GATT_PROP_READ | GATT_PROP_WRITE;
// Beacon Period Characteristic Value
static uint16 urlCfgCharBeaconPeriod = URLCFG_CHAR_BEACON_PERIOD_DEFAULT;
// Beacon Period Characteristic User Description
static uint8 urlCfgCharBeaconPeriodUserDesc[] = "Beacon Period";

// Reset Characteristic Properties
static uint8 urlCfgCharResetProps = GATT_PROP_WRITE;
// Reset Characteristic Value
static uint8 urlCfgCharReset = 0;
// Reset Characteristic User Description
static uint8 urlCfgCharResetUserDesc[] = "Reset";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t urlCfgSvcAttrTbl[] = 
{
  // Simple Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&urlCfgService            /* pValue */
  },

    // Lock State Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharLockStateProps
    },

      // Lock State Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharLockStateUUID },
        GATT_PERMIT_READ,
        0,
        &urlCfgCharLockState
      },

      // Lock State Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharLockStateUserDesc
      },

    // Lock Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharLockProps
    },
    
      // Lock Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharLockUUID },
        GATT_PERMIT_WRITE,
        0,
        urlCfgCharLock
      },
    
      // Lock Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharLockUserDesc
      },
    
    // Unlock Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharUnlockProps
    },
    
      // Unlock Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharUnlockUUID },
        GATT_PERMIT_WRITE,
        0,
        NULL,
      },
    
      // Unlock Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharUnlockUserDesc,
      },
    
    // URI Data Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharURIDataProps
    },

      // URI Data Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharURIDataUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        urlCfgCharURIData
      },

      // URI Data Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharURIDataUserDesc
      },           

    // Flags Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharFlagsProps
    },
    
      // Flags Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharFlagsUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        &urlCfgCharFlags
      },
    
      // Flags Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharFlagsUserDesc
      },           
    
    // Advertised TX Power Levels Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharAdvTXPwrLvlsProps
    },
    
      // Advertised TX Power Levels Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharAdvTXPwrLvlsUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *) urlCfgCharAdvTXPwrLvls
      },
    
      // Advertised TX Power Levels Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharAdvTXPwrLvlsUserDesc
      },           
    
    // TX Power Mode Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharTXPowerModeProps
    },
    
      // TX Power Mode Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharTXPowerModeUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        &urlCfgCharTXPowerMode
      },
    
      // TX Power Mode Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharTXPowerModeUserDesc
      },           
    
    // Beacon Period Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharBeaconPeriodProps
    },
    
      // Beacon Period Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharBeaconPeriodUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *) &urlCfgCharBeaconPeriod
      },
    
      // Beacon Period Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharBeaconPeriodUserDesc
      },           
    
    // Reset Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &urlCfgCharResetProps
    },
    
      // Reset Characteristic Value
      { 
        { ATT_UUID_SIZE, urlCfgCharResetUUID },
        GATT_PERMIT_WRITE,
        0,
        &urlCfgCharReset
      },
    
      // Reset Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        urlCfgCharResetUserDesc
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t urlCfgSvc_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr, 
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method);
static bStatus_t urlCfgSvc_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// URL Configuration Service Callbacks
CONST gattServiceCBs_t urlCfgSvcCBs =
{
  urlCfgSvc_ReadAttrCB,  // Read callback function pointer
  urlCfgSvc_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      URLCfgSvc_AddService
 *
 * @brief   Initializes the URL Configuration service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t URLCfgSvc_AddService( void )
{
  // Register GATT attribute list and CBs with GATT Server App
  return GATTServApp_RegisterService( urlCfgSvcAttrTbl, 
                                      GATT_NUM_ATTRS( urlCfgSvcAttrTbl ),
                                      GATT_MAX_ENCRYPT_KEY_SIZE,
                                      &urlCfgSvcCBs );
}

/*********************************************************************
 * @fn      URLCfgSvc_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t URLCfgSvc_RegisterAppCBs( urlCfgSvcCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    urlCfgSvc_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      URLCfgSvc_SetParameter
 *
 * @brief   Set a URL Configuration Service parameter.
 *
 * @param   param - Characteristic ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t URLCfgSvc_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case URLCFGSVC_LOCK_STATE:
      if ( len == sizeof( uint8 ) ) 
      {
        urlCfgCharLockState = *((uint8*) value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case URLCFGSVC_LOCK:
      if ( len == 16 ) 
      {
        VOID memcpy( urlCfgCharLock, value, len );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case URLCFGSVC_URI_DATA:
      if ( len <= URLCFGSVC_CHAR_URI_DATA_LEN ) 
      {
        VOID memcpy( urlCfgCharURIData, value, len );
        URIDataLen = len;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case URLCFGSVC_FLAGS:
      if ( len == sizeof(uint8)) 
      {
        urlCfgCharFlags = *((uint8*) value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case URLCFGSVC_ADV_TX_PWR_LVLS:
      if ( len == sizeof(urlCfgCharAdvTXPwrLvls) )
      {
        memcpy(urlCfgCharAdvTXPwrLvls, value, sizeof(urlCfgCharAdvTXPwrLvls));
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case URLCFGSVC_TX_POWER_MODE:
      if ( len == sizeof(uint8) &&
          *((uint8*) value) < sizeof(urlCfgCharAdvTXPwrLvls) ) 
      {
        urlCfgCharTXPowerMode = *((uint8*) value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case URLCFGSVC_BEACON_PERIOD:
      if ( len == sizeof(uint16) ) 
      {
        urlCfgCharBeaconPeriod = *((uint16*) value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t URLCfgSvc_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case URLCFGSVC_LOCK_STATE:
      *((uint8*) value) = urlCfgCharLockState;
      break;

    case URLCFGSVC_LOCK:
      VOID memcpy(value, urlCfgCharLock, 16);
      break;

    case URLCFGSVC_URI_DATA:
      VOID memcpy(value, urlCfgCharURIData, URIDataLen);
      break;

    case URLCFGSVC_URI_DATA_LEN:
      *((uint8*) value) = URIDataLen;
      break;

    case URLCFGSVC_FLAGS:
      *((uint8*) value) = urlCfgCharFlags;
      break;      

    case URLCFGSVC_ADV_TX_PWR_LVLS:
      VOID memcpy(value, urlCfgCharAdvTXPwrLvls, 4);
      break;  

    case URLCFGSVC_TX_POWER_MODE:
      *((uint8*) value) = urlCfgCharTXPowerMode;
      break;

    case URLCFGSVC_BEACON_PERIOD:
      *((uint16*) value) = urlCfgCharBeaconPeriod;
      break;      
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          urlCfgSvc_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t urlCfgSvc_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method)
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_UUID_SIZE )
  {
    // 128-bit UUID
    if (!memcmp(pAttr->type.uuid, urlCfgCharLockStateUUID, ATT_UUID_SIZE) ||
        !memcmp(pAttr->type.uuid, urlCfgCharFlagsUUID, ATT_UUID_SIZE) ||
        !memcmp(pAttr->type.uuid, urlCfgCharTXPowerModeUUID, ATT_UUID_SIZE))
    {
      *pLen = 1;
      pValue[0] = *pAttr->pValue;
    }
    else if (!memcmp(pAttr->type.uuid, urlCfgCharURIDataUUID, ATT_UUID_SIZE))
    {
      *pLen = URIDataLen;
      memcpy(pValue, pAttr->pValue, URIDataLen);
    }
    else if (!memcmp(pAttr->type.uuid, urlCfgCharAdvTXPwrLvlsUUID, ATT_UUID_SIZE))
    {
      *pLen = 4;
      memcpy(pValue, pAttr->pValue, 4);
    }
    else if (!memcmp(pAttr->type.uuid, urlCfgCharBeaconPeriodUUID, ATT_UUID_SIZE))
    {
      *pLen = 2;
      pValue[0] = LO_UINT16(urlCfgCharBeaconPeriod);
      pValue[1] = HI_UINT16(urlCfgCharBeaconPeriod);
    }
    else
    {
      // Should never get here!
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
 * @fn      urlCfgSvc_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t urlCfgSvc_WriteAttrCB(uint16_t connHandle,
                                        gattAttribute_t *pAttr,
                                        uint8_t *pValue, uint16_t len,
                                        uint16_t offset, uint8_t method)
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    status = ATT_ERR_ATTR_NOT_FOUND; // Should never get here!
  }
  else
  {
    // 128-bit UUID
    if (!memcmp(pAttr->type.uuid, urlCfgCharLockUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len != 16)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        memcpy(urlCfgCharLock, pValue, len);  // save the lock code
        urlCfgCharLockState = 1;  // Now locked
      }
    }
    else if (!memcmp(pAttr->type.uuid, urlCfgCharURIDataUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len > URLCFGSVC_CHAR_URI_DATA_LEN)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        URIDataLen = len;

        memcpy(urlCfgCharURIData, pValue, len);
      }
    }
    else if (!memcmp(pAttr->type.uuid, urlCfgCharFlagsUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len != 1)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        urlCfgCharFlags = *pValue;
      }
    }
    else if (!memcmp(pAttr->type.uuid,
                     urlCfgCharAdvTXPwrLvlsUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len != 4)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        memcpy(urlCfgCharAdvTXPwrLvls, pValue, 4);
      }
    }
    else if (!memcmp(pAttr->type.uuid,
                     urlCfgCharTXPowerModeUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len != 1)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else if (*pValue > 3)
      {
        status = ATT_ERR_WRITE_NOT_PERMITTED;
      }
      else
      {
        urlCfgCharTXPowerMode = *pValue;
      }
    }
    else if (!memcmp(pAttr->type.uuid,
                     urlCfgCharBeaconPeriodUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len != 2)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        uint16 tempPeriod;
        
        urlCfgCharBeaconPeriod = BUILD_UINT16(pValue[0], pValue[1]);

        // convert into multiple of 0.625us
        tempPeriod = (uint16) (urlCfgCharBeaconPeriod * 8L / 5);
        if (0 < tempPeriod && tempPeriod < LL_ADV_NONCONN_INTERVAL_MIN)
        {
          urlCfgCharBeaconPeriod =
            (uint16) (LL_ADV_NONCONN_INTERVAL_MIN * 5 / 8);
        }
      }
    }
    else if (!memcmp(pAttr->type.uuid, urlCfgCharResetUUID, ATT_UUID_SIZE))
    {
      if (urlCfgCharLockState)
      {
        status = ATT_ERR_INSUFFICIENT_AUTHOR;
      }
      else if (len != 1)
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        urlCfgCharReset = *pValue;
        if (urlCfgCharReset != 0)
        {
          notifyApp = URLCFGSVC_RESET;
        }
      }
    }
    else
    {
      status = ATT_ERR_ATTR_NOT_FOUND;
    }
  }

  // If a characteristic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && urlCfgSvc_AppCBs && urlCfgSvc_AppCBs->pfnURLCfgSvcChange )
  {
    urlCfgSvc_AppCBs->pfnURLCfgSvcChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
*********************************************************************/
