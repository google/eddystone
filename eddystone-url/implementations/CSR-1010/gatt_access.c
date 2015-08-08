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
 *  FILE
 *      gatt_access.c
 *
 *  DESCRIPTION
 *      GATT-related routine implementations
 *
 
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <ls_app_if.h>      /* Link Supervisor application interface */
#include <gap_app_if.h>     /* GAP application interface */
#include <gap_types.h>      /* GAP definitions */
#include <ls_err.h>         /* Upper Stack Link Supervisor error codes */
#include <ls_types.h>       /* Link Supervisor definitions */
#include <panic.h>          /* Support for applications to panic */
#include <gatt.h>           /* GATT application interface */
#include <gatt_uuid.h>      /* Common Bluetooth UUIDs and macros */
#include <timer.h>          /* Chip timer functions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */
#include "constants.h"        /* Constant data shared with services */
#include "app_gatt_db.h"      /* GATT database definitions */
#include "gatt_access.h"      /* Interface to this file */
#include "appearance.h"       /* Macros for commonly used appearance values */
#include "gap_service.h"      /* GAP Service interface */
#include "battery_service.h"  /* Battery Service interface */
#include "esurl_beacon_service.h"/* Beacon2 Service interface */
#include "esurl_beacon_uuids.h"  /* Battery Service UUIDs */
#include "battery_uuids.h"    /* Battery Service UUIDs */
#include "dev_info_uuids.h"   /* Device Information Service UUIDs */
#include "dev_info_service.h" /* Device Information Service interface */
#include "debug_interface.h"  /* Debug serial-port for adding print statements */

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* This constant defines an array that is large enough to hold the advertisement
 * data.
 */
#define MAX_ADV_DATA_LEN                                  (31)

/* Acceptable shortened device name length that can be sent in advertisement 
 * data 
 */
#define SHORTENED_DEV_NAME_LEN                            (8)

/* Length of Tx Power prefixed with 'Tx Power' AD Type */
#define TX_POWER_VALUE_LENGTH                             (2)

/*============================================================================*
 *  Private Data types
 *============================================================================*/

/* GATT data structure */
typedef struct _APP_GATT_DATA_T
{
    /* Value for which advertisement timer needs to be started. 
     *
     * For bonded devices, the timer is initially started for 10 seconds to 
     * enable fast connection by bonded device to the sensor. If bonded device 
     * doesn't connect within this time, another 20 second timer is started 
     * to enable fast connections from any collector device in the vicinity. 
     * This is then followed by reduced power advertisements.
     *
     * For non-bonded devices, the timer is initially started for 30 seconds 
     * to enable fast connections from any collector device in the vicinity.
     * This is then followed by reduced power advertisements.
     */
    uint32                     advert_timer_value;

} APP_GATT_DATA_T;

/*============================================================================*
 *  Private Data 
 *============================================================================*/

/* Application GATT data instance */
static APP_GATT_DATA_T g_gatt_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Add device name to advertisement or scan response */
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len);

/* Set advertisement parameters */
static void gattSetAdvertParams(TYPED_BD_ADDR_T* p_addr, bool fast_connection);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      addDeviceNameToAdvData
 *
 *  DESCRIPTION
 *      This function is used to add a device name to advertisement or scan 
 *      response data. It follows these steps:
 *      a. Try to add the complete device name to the advertisement packet
 *      b. Try to add the complete device name to the scan response packet
 *      c. Try to add the shortened device name to the advertisement packet
 *      d. Try to add the shortened (max possible) device name to the scan 
 *         response packet.
 *
 *  PARAMETERS
 *      adv_data_len [in]       Length of advertisement packet
 *      scan_data_len [in]      Length of scan response packet
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len)
{

    uint8 *p_device_name = NULL;        /* Device name */
    uint16 device_name_adtype_len;      /* Device name length including AD Type
                                         */

    /* Read device name along with AD Type and its length */
    p_device_name = GapGetNameAndLength(&device_name_adtype_len);

    /* Add complete device name to Advertisement data */
    p_device_name[0] = AD_TYPE_LOCAL_NAME_COMPLETE;

    /* Increment device_name_adtype_len by one to account for length field
     * which will be added by the GAP layer. 
     */

    /* Check if Complete Device Name can fit in remaining advertisement 
     * data space 
     */
    if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - adv_data_len))
    {
        /* Add Complete Device Name to Advertisement Data */
        if (LsStoreAdvScanData(device_name_adtype_len, p_device_name, 
                      ad_src_advertise) != ls_err_none)
        {
            ReportPanic(app_panic_set_advert_data);
        }

    }
    /* Check if Complete Device Name can fit in Scan response message */
    else if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - scan_data_len)) 
    {
        /* Add Complete Device Name to Scan Response Data */
        if (LsStoreAdvScanData(device_name_adtype_len, p_device_name, 
                      ad_src_scan_rsp) != ls_err_none)
        {
            ReportPanic(app_panic_set_scan_rsp_data);
        }

    }
    /* Check if Shortened Device Name can fit in remaining advertisement 
     * data space 
     */
    else if((MAX_ADV_DATA_LEN - adv_data_len) >=
            (SHORTENED_DEV_NAME_LEN + 2)) /* Added 2 for Length and AD Type 
                                           * added by GAP layer
                                           */
    {
        /* Add shortened device name to Advertisement data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (LsStoreAdvScanData(SHORTENED_DEV_NAME_LEN, p_device_name, 
                      ad_src_advertise) != ls_err_none)
        {
            ReportPanic(app_panic_set_advert_data);
        }

    }
    else /* Add device name to remaining Scan response data space */
    {
        /* Add as much as can be stored in Scan Response data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (LsStoreAdvScanData(MAX_ADV_DATA_LEN - scan_data_len, 
                                    p_device_name, 
                                    ad_src_scan_rsp) != ls_err_none)
        {
            ReportPanic(app_panic_set_scan_rsp_data);
        }

    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      gattSetAdvertParams
 *
 *  DESCRIPTION
 *      This function is used to set advertisement parameters.
 *
 *  PARAMETERS
 *      p_addr [in]             Bonded host address
 *      fast_connection [in]    TRUE:  Fast advertisements
 *                              FALSE: Slow advertisements
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void gattSetAdvertParams(TYPED_BD_ADDR_T *p_addr, bool fast_connection)
{
    uint8 advert_data[MAX_ADV_DATA_LEN];/* Advertisement packet */
    uint16 length;                      /* Length of advertisement packet */
    /* Advertisement interval, microseconds */
    uint32 adv_interval_min;
    uint32 adv_interval_max;

    /* Tx power level value prefixed with 'Tx Power' AD Type */
    /* Refer to BT4.0 specification, Vol3-part-C-Section-11.1.5 */ 
    uint8 device_tx_power[TX_POWER_VALUE_LENGTH] = {
                AD_TYPE_TX_POWER
                };

    /* Device appearance */
    uint8 device_appearance[ATTR_LEN_DEVICE_APPEARANCE + 1] = {
                AD_TYPE_APPEARANCE,
                WORD_LSB(APPEARANCE_APPLICATION_VALUE),
                WORD_MSB(APPEARANCE_APPLICATION_VALUE)
                };

    /* A variable to keep track of the data added to advert_data. The limit is 
     * MAX_ADV_DATA_LEN. GAP layer will add AD Flags to advert_data which is 3
     * bytes. Refer BT Spec 4.0, Vol 3, Part C, Sec 11.1.3:
     *
     * First byte is length
     * second byte is AD TYPE = 0x1
     * Third byte is Flags description 
     */
    uint16 length_added_to_adv = 3;

    if(fast_connection)
    {
        adv_interval_min = FC_ADVERTISING_INTERVAL_MIN;
        adv_interval_max = FC_ADVERTISING_INTERVAL_MAX;
    }

    if((GapSetMode(gap_role_peripheral, gap_mode_discover_general,
                        gap_mode_connect_undirected, 
                        gap_mode_bond_yes,
                        gap_mode_security_unauthenticate) != ls_err_none) ||
       (GapSetAdvInterval(adv_interval_min, adv_interval_max) 
                        != ls_err_none))
    {
        ReportPanic(app_panic_set_advert_params);
    }

    /* Add bonded device to white list.*/
    if(IsWhiteListEnabled())
    {
        /* Initial case when advertisements are started for Bonded host for 
         * 10 seconds. White list is configured with the Bonded host address
         */
        if(LsAddWhiteListDevice(p_addr)!= ls_err_none)
        {
            ReportPanic(app_panic_add_whitelist);
        }
    }

    /* Reset existing advertising data */
    if(LsStoreAdvScanData(0, NULL, ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* Reset existing scan response data */
    if(LsStoreAdvScanData(0, NULL, ad_src_scan_rsp) != ls_err_none)
    {
        ReportPanic(app_panic_set_scan_rsp_data);
    }

    /* Setup ADVERTISEMENT DATA */

    /* Add UUID list of the services supported by the device */
    length = GetSupportedUUIDServiceList(advert_data);

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (length + 1);

    if (LsStoreAdvScanData(length, advert_data, 
                        ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (sizeof(device_appearance) + 1);

    /* Add device appearance to the advertisements */
    if (LsStoreAdvScanData(ATTR_LEN_DEVICE_APPEARANCE + 1, 
        device_appearance, ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* Change the Radio and Adv tx params for CONFIG mode */
    
    /* Update the radio tx power level here */
    LsSetTransmitPowerLevel(RADIO_TX_POWER_CONFIG); 

    /* Update the adv tx power level here */
    device_tx_power[TX_POWER_VALUE_LENGTH - 1] = ADV_TX_POWER_CONFIG;
    
    /* NOTE: The 2 tx params above are reset to the tx_power_mode on disconnection */

    /* One added for Length field, it will be added to Adv Data by GAP layer */
    length_added_to_adv += (TX_POWER_VALUE_LENGTH + 1);

    /* Add tx power value of device to the advertising data */
    if (LsStoreAdvScanData(TX_POWER_VALUE_LENGTH, device_tx_power, 
                          ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    addDeviceNameToAdvData(length_added_to_adv, 0);

}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitGattData
 *
 *  DESCRIPTION
 *      This function initialises the application GATT data.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void InitGattData(void)
{
    g_gatt_data.advert_timer_value = TIMER_INVALID;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application and responds
 *      with the GATT_ACCESS_RSP message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    /* For the received attribute handle, check all the services that support 
     * attribute 'Read' operation handled by application.
     */
    /* More services may be added here to support their read operations */
    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to the GAP service */
        GapHandleAccessRead(p_ind);
    }
    else if(DeviceInfoCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to the DEVICE INFORMATION service */
        DeviceInfoHandleAccessRead(p_ind);
    }
    else if(BatteryCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to BATTERY service */
        BatteryHandleAccessRead(p_ind);
    }
    else if(EsurlBeaconCheckHandleRange (p_ind->handle))
    {
        /* Attribute handle belongs to Beacon service */
        EsurlBeaconHandleAccessRead(p_ind);
    }    
    else
    {
        /* Application doesn't support 'Read' operation on received attribute
         * handle, so return 'gatt_status_read_not_permitted' status.
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_read_not_permitted,
                      0, NULL);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application and responds
 *      with the GATT_ACCESS_RSP message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    /* For the received attribute handle, check all the services that support 
     * attribute 'Write' operation handled by application.
     */
    /* More services may be added here to support their write operations */
    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to GAP service */
        GapHandleAccessWrite(p_ind);
    }
    else if(BatteryCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to BATTERY service */
        BatteryHandleAccessWrite(p_ind);
    }
    else if(EsurlBeaconCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Beacon service */
        EsurlBeaconHandleAccessWrite(p_ind);
    }    
    else
    {
        /* Application doesn't support 'Write' operation on received  attribute
         * handle, so return 'gatt_status_write_not_permitted' status
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_write_not_permitted,
                      0, NULL);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStartAdverts
 *
 *  DESCRIPTION
 *      This function is used to start undirected advertisements and moves to 
 *      ADVERTISING state.
 *
 *  PARAMETERS
 *      p_addr [in]             Bonded host address
 *      fast_connection [in]    TRUE:  Fast advertisements
 *                              FALSE: Slow advertisements
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GattStartAdverts(TYPED_BD_ADDR_T *p_addr, bool fast_connection)
{
    /* Variable 'connect_flags' needs to be updated to have peer address type 
     * if Directed advertisements are supported as peer address type will 
     * only be used in that case. We don't support directed advertisements for 
     * this application.
     */
#ifdef USE_STATIC_RANDOM_ADDRESS
    uint16 connect_flags = L2CAP_CONNECTION_SLAVE_UNDIRECTED | 
                           L2CAP_OWN_ADDR_TYPE_RANDOM;
#else
    uint16 connect_flags = L2CAP_CONNECTION_SLAVE_UNDIRECTED | 
                           L2CAP_OWN_ADDR_TYPE_PUBLIC;
#endif /* USE_STATIC_RANDOM_ADDRESS */

    /* Set advertisement parameters */
    gattSetAdvertParams(p_addr, fast_connection);

    /* If white list is enabled, set the controller's advertising filter policy 
     * to "process scan and connection requests only from devices in the White 
     * List"
     */
    if(IsWhiteListEnabled() && !GattIsAddressResolvableRandom(p_addr))
    {
#ifdef USE_STATIC_RANDOM_ADDRESS
        connect_flags = L2CAP_CONNECTION_SLAVE_WHITELIST |
                        L2CAP_OWN_ADDR_TYPE_RANDOM;
#else
        connect_flags = L2CAP_CONNECTION_SLAVE_WHITELIST |
                        L2CAP_OWN_ADDR_TYPE_PUBLIC;
#endif /* USE_STATIC_RANDOM_ADDRESS */
    }

    /* Start GATT connection in Slave role */
    GattConnectReq(NULL, connect_flags);

     /* Start advertisement timer */
    if(g_gatt_data.advert_timer_value)
    {
        StartAdvertTimer(g_gatt_data.advert_timer_value);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetSupportedUUIDServiceList
 *
 *  DESCRIPTION
 *      This function prepares the list of supported service UUIDs to be 
 *      added to Advertisement data. It also adds the relevant AD Type to the 
 *      start of AD array.
 *
 *  PARAMETERS
 *      p_service_uuid_ad [out] AD Service UUID list
 *
 *  RETURNS
 *      Size of AD Service UUID list
 *----------------------------------------------------------------------------*/
extern uint16 GetSupportedUUIDServiceList(uint8 *p_service_uuid_ad)
{
    uint8   size_data = 0;              /* Size of AD Service UUID list */
    int     i;                          /* iterator index */

    /* Add 128-bit UUID for supported main service */
    p_service_uuid_ad[size_data++] = AD_TYPE_SERVICE_UUID_128BIT_LIST;
    
    for (i=0; i<16; i++) {
      p_service_uuid_ad[size_data++] = uribeacon_v2_service[15-i];
    }     
    
    /* Add all the supported UUIDs in this function*/

    /* Return the size of AD service data. */
    return ((uint16)size_data);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattIsAddressResolvableRandom
 *
 *  DESCRIPTION
 *      This function checks if the address is resolvable random or not.
 *
 *  PARAMETERS
 *      p_addr [in]             Address to check
 *
 *  RETURNS
 *      TRUE if supplied address is a resolvable private address
 *      FALSE if supplied address is non-resolvable private address
 *----------------------------------------------------------------------------*/
extern bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *p_addr)
{
    if(p_addr->type != L2CA_RANDOM_ADDR_TYPE || 
      (p_addr->addr.nap & BD_ADDR_NAP_RANDOM_TYPE_MASK)
                                      != BD_ADDR_NAP_RANDOM_TYPE_RESOLVABLE)
    {
        /* This is not a resolvable private address  */
        return FALSE;
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattTriggerFastAdverts
 *
 *  DESCRIPTION
 *      This function is used to trigger fast advertisements.
 *
 *  PARAMETERS
 *      p_addr [in]             Bonded host address
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GattTriggerFastAdverts(TYPED_BD_ADDR_T *p_addr)
{
    if(IsDeviceBonded())
    {
        g_gatt_data.advert_timer_value = BONDED_DEVICE_ADVERT_TIMEOUT_VALUE;
    }
    else
    {
        g_gatt_data.advert_timer_value = FAST_CONNECTION_ADVERT_TIMEOUT_VALUE;
    }

    /* Trigger fast connections */
    GattStartAdverts(p_addr, TRUE);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStopAdverts
 *
 *  DESCRIPTION
 *      This function is used to stop advertisements.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GattStopAdverts(void)
{
    switch(GetState())
    {
        case app_state_fast_advertising:
        {
            if(IsWhiteListEnabled())
            {
                /* Set advertisement timer for remaining 20 seconds for fast
                 * connections without any device in the white list.
                 */
                g_gatt_data.advert_timer_value = 
                                FAST_CONNECTION_ADVERT_TIMEOUT_VALUE - 
                                BONDED_DEVICE_ADVERT_TIMEOUT_VALUE;
            }

            /* Stop on-going advertisements */
            GattCancelConnectReq();
        }
        /* FALLTHROUGH*/

        default:
            /* Ignore timer in remaining states */
        break;
    }
}
