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
 *      gatt_access.h
 *
 *  DESCRIPTION
 *      Header definitions for common application attributes
 *

 *
******************************************************************************/

#ifndef __GATT_ACCESS_H__
#define __GATT_ACCESS_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */
#include <time.h>           /* Application interface to System Time */
#include <gatt.h>           /* GATT application interface */

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Timeout values for advertisements */
#define BONDED_DEVICE_ADVERT_TIMEOUT_VALUE       (5 * SECOND)
#define FAST_CONNECTION_ADVERT_TIMEOUT_VALUE     (10 * SECOND)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/

/* Application states */
typedef enum 
{
    /* Application initial state */
    app_state_init = 0,
            
    /* Application is beaconing */
    app_state_beaconing,

    /* Application is performing fast undirected advertisements */
    app_state_fast_advertising,

    /* Connection has been established with the host */
    app_state_connected,

    /* Disconnection initiated by the application */
    app_state_disconnecting,

    /* Application is neither advertising nor connected to a host */
    app_state_idle

} app_state;

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Invalid UCID indicating we are not currently connected */
#define GATT_INVALID_UCID                    (0xFFFF)

/* Invalid Attribute Handle */
#define INVALID_ATT_HANDLE                   (0x0000)

/* AD Type for Appearance */
#define AD_TYPE_APPEARANCE                   (0x19)

/* Maximum Length of Device Name 
 * Note: Do not increase device name length beyond (DEFAULT_ATT_MTU - 3 = 20) 
 * octets as the GAP service doesn't support handling of Prepare write and
 * Execute write procedures.
 */
#define DEVICE_NAME_MAX_LENGTH               (20)

/* The following macro definition should be included only if a user wants the
 * application to have a static random address.
 */
#define USE_STATIC_RANDOM_ADDRESS

/* Timer value for remote device to re-encrypt the link using old keys */
#define BONDING_CHANCE_TIMER            (30*SECOND)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/

/* GATT Client Characteristic Configuration Value [Ref GATT spec, 3.3.3.3] */
typedef enum
{
    gatt_client_config_none            = 0x0000,
    gatt_client_config_notification    = 0x0001,
    gatt_client_config_indication      = 0x0002,
    gatt_client_config_reserved        = 0xFFF4

} gatt_client_config;

/*  Application defined panic codes */
typedef enum
{
    /* Failure while setting advertisement parameters */
    app_panic_set_advert_params,

    /* Failure while setting advertisement data */
    app_panic_set_advert_data,

    /* Failure while setting scan response data */
    app_panic_set_scan_rsp_data,

    /* Failure while registering GATT DB with firmware */
    app_panic_db_registration,

    /* Failure while reading NVM */
    app_panic_nvm_read,

    /* Failure while writing NVM */
    app_panic_nvm_write,

    /* Failure while reading Tx Power Level */
    app_panic_read_tx_pwr_level,

    /* Failure while deleting device from whitelist */
    app_panic_delete_whitelist,

    /* Failure while adding device to whitelist */
    app_panic_add_whitelist,

    /* Failure while triggering connection parameter update procedure */
    app_panic_con_param_update,

    /* Event received in an unexpected application state */
    app_panic_invalid_state,

    /* Unexpected beep type */
    app_panic_unexpected_beep_type

} app_panic_code;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Handle read operations on attributes maintained by the application */
extern void HandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* Handle write operations on attributes maintained by the application. */
extern void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Start undirected advertisements and move to ADVERTISING state */
extern void GattStartAdverts(TYPED_BD_ADDR_T *p_addr, bool fast_connection);

/* Stop on-going advertisements */
extern void GattStopAdverts(void);

/* Prepare the list of supported service UUIDs to be added to
 * Advertisement data
 */
extern uint16 GetSupportedUUIDServiceList(uint8 *p_service_uuid_ad);

/* Check if the address is resolvable random or not */
extern bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *p_addr);

/* Trigger fast advertisements */
extern void GattTriggerFastAdverts(TYPED_BD_ADDR_T *p_addr);

/* Initialise the application GATT data. */
extern void InitGattData(void);

#endif /* __GATT_ACCESS_H__ */
