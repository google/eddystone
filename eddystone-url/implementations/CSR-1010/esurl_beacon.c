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
 *      esurl_beacon.c
 *
 *  DESCRIPTION
 *      This file implements the core Physical Web Esurl Beacon functionality.
 *
 *  NOTICE
 *      Use of beta release software is at your own risk.  
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <main.h>           /* Functions relating to powering up the device */
#include <types.h>          /* Commonly used type definitions */
#include <timer.h>          /* Chip timer functions */
#include <mem.h>            /* Memory library */
#include <config_store.h>   /* Interface to the Configuration Store */

/* Upper Stack API */
#include <gatt.h>           /* GATT application interface */
#include <ls_app_if.h>      /* Link Supervisor application interface */
#include <gap_app_if.h>     /* GAP application interface */
#include <buf_utils.h>      /* Buffer functions */
#include <security.h>       /* Security Manager application interface */
#include <panic.h>          /* Support for applications to panic */
#include <nvm.h>            /* Access to Non-Volatile Memory */
#include <random.h>         /* Generators for pseudo-random data sequences */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
                                
#include "gatt_access.h"    /* GATT-related routines */
#include "app_gatt_db.h"    /* GATT database definitions */
#include "buzzer.h"         /* Buzzer functions */
#include "led.h"            /* LED functions */
#include "nvm_access.h"     /* Non-volatile memory access */
#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */
#include "hw_access.h"      /* Hardware access */
#include "debug_interface.h"/* Application debug routines */
#include "gap_service.h"    /* GAP service interface */
#include "battery_service.h"/* Battery service interface */
#include "esurl_beacon_service.h" /* Beacon service interface */
#include "beaconing.h"      /* Beacon routines */

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Maximum number of timers. Up to five timers are required by this application:
 *  
 *  buzzer.c:       buzzer_tid
 *  This file:      con_param_update_tid
 *  This file:      app_tid
 *  This file:      bonding_reattempt_tid (if PAIRING_SUPPORT defined)
 *  hw_access.c:    button_press_tid
 *  This file:      connectable_advert_tid
 */
#define MAX_APP_TIMERS                 (6)

/* Number of Identity Resolving Keys (IRKs) that application can store */
#define MAX_NUMBER_IRK_STORED          (1)

/* Magic value to check the sanity of Non-Volatile Memory (NVM) region used by
 * the application. This value is unique for each application.
 */
#define NVM_SANITY_MAGIC               (0x6006)

/* NVM offset for NVM sanity word */
#define NVM_OFFSET_SANITY_WORD         (0)

/* NVM offset for bonded flag */
#define NVM_OFFSET_BONDED_FLAG         (NVM_OFFSET_SANITY_WORD + 1)

/* NVM offset for bonded device Bluetooth address */
#define NVM_OFFSET_BONDED_ADDR         (NVM_OFFSET_BONDED_FLAG + \
                                        sizeof(g_app_data.bonded))

/* NVM offset for diversifier */
#define NVM_OFFSET_SM_DIV              (NVM_OFFSET_BONDED_ADDR + \
                                        sizeof(g_app_data.bonded_bd_addr))

/* NVM offset for IRK */
#define NVM_OFFSET_SM_IRK              (NVM_OFFSET_SM_DIV + \
                                        sizeof(g_app_data.diversifier))

/* Number of words of NVM used by application. Memory used by supported 
 * services is not taken into consideration here.
 */
#define NVM_MAX_APP_MEMORY_WORDS       (NVM_OFFSET_SM_IRK + \
                                        MAX_WORDS_IRK)

/* Slave device is not allowed to transmit another Connection Parameter 
 * Update request till time TGAP(conn_param_timeout). Refer to section 9.3.9.2,
 * Vol 3, Part C of the Core 4.0 BT spec. The application should retry the 
 * 'Connection Parameter Update' procedure after time TGAP(conn_param_timeout)
 * which is 30 seconds.
 */
#define GAP_CONN_PARAM_TIMEOUT          (30 * SECOND)

/*============================================================================*
 *  Private Data types
 *============================================================================*/

/* Application data structure */
typedef struct _APP_DATA_T
{
    /* Current state of application */
    app_state                  state;

    /* TYPED_BD_ADDR_T of the host to which device is connected */
    TYPED_BD_ADDR_T            con_bd_addr;

    /* Track the Connection Identifier (UCID) as Clients connect and
     * disconnect
     */
    uint16                     st_ucid;

    /* Boolean flag to indicate whether the device is bonded */
    bool                       bonded;

    /* TYPED_BD_ADDR_T of the host to which device is bonded */
    TYPED_BD_ADDR_T            bonded_bd_addr;

    /* Diversifier associated with the Long Term Key (LTK) of the bonded
     * device
     */
    uint16                     diversifier;

    /* Timer ID for Connection Parameter Update timer in Connected state */
    timer_id                   con_param_update_tid;
    
    /* Central Private Address Resolution IRK. Will only be used when
     * central device used resolvable random address.
     */
    uint16                     irk[MAX_WORDS_IRK];

    /* Number of connection parameter update requests made */
    uint8                      num_conn_update_req;

    /* Boolean flag to indicate pairing button press */
    bool                       pairing_button_pressed;

    /* Timer ID for 'UNDIRECTED ADVERTS' and activity on the sensor device like
     * measurements or user intervention in CONNECTED state.
     */
    timer_id                   app_tid;

    /* Boolean flag to indicate whether to set white list with the bonded
     * device. This flag is used in an interim basis while configuring 
     * advertisements.
     */
    bool                       enable_white_list;

#ifdef PAIRING_SUPPORT
    /* Boolean flag to indicate whether encryption is enabled with the bonded
     * host
     */
    bool                       encrypt_enabled;

    /* This timer will be used if the application is already bonded to the 
     * remote host address but the remote device wanted to rebond which we had 
     * declined. In this scenario, we give ample time to the remote device to 
     * encrypt the link using old keys. If remote device doesn't encrypt the 
     * link, we will disconnect the link when this timer expires.
     */
    timer_id                   bonding_reattempt_tid;
#endif /* PAIRING_SUPPORT */

    /* Current connection interval */
    uint16                     conn_interval;

    /* Current slave latency */
    uint16                     conn_latency;

    /* Current connection timeout value */
    uint16                     conn_timeout;

} APP_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare space for application timers */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/* Application data instance */
static APP_DATA_T g_app_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Initialise application data structure */
static void appDataInit(void);

/* Initialise and read NVM data */
static void readPersistentStore(void);

/* Enable whitelist based advertising */
static void enableWhiteList(void);

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
    /* Handle Idle timer expiry in connected states */
    static void appIdleTimerHandler(timer_id tid);

    /* Reset the time for which the application was idle */
    static void resetIdleTimer(void);
#endif /* CONNECTED_IDLE_TIMEOUT_VALUE */

/* Start the Connection update timer */
static void appStartConnUpdateTimer(void);

#ifdef PAIRING_SUPPORT
    /* Handle the expiry of the bonding chance timer */
    static void handleBondingChanceTimerExpiry(timer_id tid);
#endif /* PAIRING_SUPPORT */

/* Send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST to the remote device */
static void requestConnParamUpdate(timer_id tid);

/* Exit the advertising states */
static void appExitAdvertising(void);

/* Exit the initialisation state */
static void appInitExit(void);

/* Handle advertising timer expiry */
static void appAdvertTimerHandler(timer_id tid);

/* LM_EV_CONNECTION_COMPLETE signal handler */
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data);

/* GATT_ADD_DB_CFM signal handler */
static void handleSignalGattAddDbCfm(GATT_ADD_DB_CFM_T *p_event_data);

/* GATT_CANCEL_CONNECT_CFM signal handler */
static void handleSignalGattCancelConnectCfm(void);

/* GATT_CONNECT_CFM signal handler */
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T *p_event_data);

/* SM_KEYS_IND signal handler */
static void handleSignalSmKeysInd(SM_KEYS_IND_T *p_event_data);

#ifdef PAIRING_SUPPORT
    /* SM_PAIRING_AUTH_IND signal handler */
    static void handleSignalSmPairingAuthInd(
                    SM_PAIRING_AUTH_IND_T *p_event_data);
    
    /* LM_EV_ENCRYPTION_CHANGE signal handler */
    static void handleSignalLMEncryptionChange(
                    HCI_EV_DATA_ENCRYPTION_CHANGE_T *p_event_data);
#endif /* PAIRING_SUPPORT */

/* SM_SIMPLE_PAIRING_COMPLETE_IND signal handler */
static void handleSignalSmSimplePairingCompleteInd(
                    SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data);

/* SM_DIV_APPROVE_IND signal handler */
static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data);

/* LS_CONNECTION_PARAM_UPDATE_CFM signal handler */
static void handleSignalLsConnParamUpdateCfm(
                    LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data);

/* LS_CONNECTION_PARAM_UPDATE_IND signal handler */
static void handleSignalLsConnParamUpdateInd(
                    LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data);

/* GATT_ACCESS_IND signal handler */
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data);

/* LM_EV_DISCONNECT_COMPLETE signal handler */
static void handleSignalLmDisconnectComplete(
                    HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      appDataInit
 *
 *  DESCRIPTION
 *      This function is called to initialise the application data structure.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appDataInit(void)
{
    /* Initialise general application timer */
    if (g_app_data.app_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.app_tid);
        g_app_data.app_tid = TIMER_INVALID;
    }

    /* Reset the pairing button press flag */
    g_app_data.pairing_button_pressed = FALSE;

    /* Initialise the connection parameter update timer */
    if (g_app_data.con_param_update_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.con_param_update_tid);
        g_app_data.con_param_update_tid = TIMER_INVALID;
    }

    /* Initialise the connected client ID */
    g_app_data.st_ucid = GATT_INVALID_UCID;

    /* Initialise white list flag */
    g_app_data.enable_white_list = FALSE;

#ifdef PAIRING_SUPPORT
    /* Initialise link encryption flag */
    g_app_data.encrypt_enabled = FALSE;

    /* Initialise the bonding reattempt timer */
    if (g_app_data.bonding_reattempt_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.bonding_reattempt_tid);
        g_app_data.bonding_reattempt_tid = TIMER_INVALID;
    }
#endif /* PAIRING_SUPPORT */

    /* Reset the connection parameter variables */
    g_app_data.conn_interval = 0;
    g_app_data.conn_latency = 0;
    g_app_data.conn_timeout = 0;

    /* Initialise the application GATT data */
    InitGattData();
    
    /* Reset application hardware data */
    HwDataReset();

    /* Initialise GAP data structure */
    GapDataInit();

    /* Battery Service data initialisation */
    BatteryDataInit();

    /* Beacon Service data initialisation */
    EsurlBeaconDataInit();
    
    /* Call the required service data initialisation APIs from here */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readPersistentStore
 *
 *  DESCRIPTION
 *      This function is used to initialise and read NVM data.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void readPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = NVM_MAX_APP_MEMORY_WORDS;
    uint16 nvm_sanity = 0xffff;
    bool nvm_start_fresh = FALSE;

    /* Read persistent storage to find if the device was last bonded to another
     * device. If the device was bonded, trigger fast undirected advertisements
     * by setting the white list for bonded host. If the device was not bonded,
     * trigger undirected advertisements for any host to connect.
     */
    
    Nvm_Read(&nvm_sanity, 
             sizeof(nvm_sanity), 
             NVM_OFFSET_SANITY_WORD);

    if(nvm_sanity == NVM_SANITY_MAGIC)
    {

        /* Read Bonded Flag from NVM */
        Nvm_Read((uint16*)&g_app_data.bonded,
                  sizeof(g_app_data.bonded),
                  NVM_OFFSET_BONDED_FLAG);

        if(g_app_data.bonded)
        {
            /* Bonded Host Typed BD Address will only be stored if bonded flag
             * is set to TRUE. Read last bonded device address.
             */
            Nvm_Read((uint16*)&g_app_data.bonded_bd_addr, 
                       sizeof(TYPED_BD_ADDR_T),
                       NVM_OFFSET_BONDED_ADDR);

            /* If device is bonded and bonded address is resolvable then read 
             * the bonded device's IRK
             */
            if(GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
            {
                Nvm_Read(g_app_data.irk, 
                         MAX_WORDS_IRK,
                         NVM_OFFSET_SM_IRK);
            }

        }
        else /* Case when we have only written NVM_SANITY_MAGIC to NVM but 
              * didn't get bonded to any host in the last powered session
              */
        {
            /* Any initialisation can be done here for non-bonded devices */
            
        }

        /* Read the diversifier associated with the presently bonded/last 
         * bonded device.
         */
        Nvm_Read(&g_app_data.diversifier, 
                 sizeof(g_app_data.diversifier),
                 NVM_OFFSET_SM_DIV);

        /* If NVM in use, read device name and length from NVM */
        GapReadDataFromNVM(&nvm_offset);

    }
    else /* NVM Sanity check failed means either the device is being brought up 
          * for the first time or memory has got corrupted in which case 
          * discard the data and start fresh.
          */
    {

        nvm_start_fresh = TRUE;

        nvm_sanity = NVM_SANITY_MAGIC;

        /* Write NVM Sanity word to the NVM */
        Nvm_Write(&nvm_sanity, 
                  sizeof(nvm_sanity), 
                  NVM_OFFSET_SANITY_WORD);

        /* The device will not be bonded as it is coming up for the first 
         * time 
         */
        g_app_data.bonded = FALSE;

        /* Write bonded status to NVM */
        Nvm_Write((uint16*)&g_app_data.bonded, 
                   sizeof(g_app_data.bonded), 
                  NVM_OFFSET_BONDED_FLAG);

        /* When the application is coming up for the first time after flashing 
         * the image to it, it will not have bonded to any device. So, no LTK 
         * will be associated with it. Hence, set the diversifier to 0.
         */
        g_app_data.diversifier = 0;

        /* Write the same to NVM. */
        Nvm_Write(&g_app_data.diversifier, 
                  sizeof(g_app_data.diversifier),
                  NVM_OFFSET_SM_DIV);

        /* If fresh NVM, write device name and length to NVM for the 
         * first time.
         */
        GapInitWriteDataToNVM(&nvm_offset);
        BatteryWriteDataToNVM(&nvm_offset);       
        EsurlBeaconWriteDataToNVM(&nvm_offset);
    }

    /* Read Battery service data from NVM if the devices are bonded and  
     * update the offset with the number of words of NVM required by 
     * this service
     */
    nvm_offset = NVM_MAX_APP_MEMORY_WORDS;    
    GapReadDataFromNVM(&nvm_offset);   
    BatteryReadDataFromNVM(&nvm_offset);
    EsurlBeaconReadDataFromNVM(&nvm_offset);
    
    
    /* Add the 'read Service data from NVM' API call here, to initialise the
     * service data, if the device is already bonded. One must take care of the
     * offset being used for storing the data.
     */

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      enableWhiteList
 *
 *  DESCRIPTION
 *      This function enables white list based advertising.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void enableWhiteList(void)
{
    if(IsDeviceBonded())
    {
        if(!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
        {
            /* Enable white list if the device is bonded and the bonded host 
             * is not using resolvable random address.
             */
            g_app_data.enable_white_list = TRUE;
        }
    }
}

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
/*----------------------------------------------------------------------------*
 *  NAME
 *      appIdleTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to handle Idle timer expiry in connected states.
 *      At the expiry of this timer, application shall disconnect with the 
 *      host and shall move to 'APP_DISCONNECTING' state.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appIdleTimerHandler(timer_id tid)
{
    if(tid == g_app_data.app_tid)
    {
        /* Timer has just expired, so mark it as invalid */
        g_app_data.app_tid = TIMER_INVALID;

        /* Handle signal as per current state */
        switch(g_app_data.state)
        {
            case app_state_connected:
            {
                /* Trigger Disconnect and move to app_state_disconnecting 
                 * state 
                 */
                SetState(app_state_disconnecting);
            }
            break;

            default:
                /* Ignore timer in any other state */
            break;
        }

    } /* Else ignore the timer */
}
#endif /* CONNECTED_IDLE_TIMEOUT_VALUE */

/*----------------------------------------------------------------------------*
 *  NAME
 *      appStartConnUpdateTimer
 *
 *  DESCRIPTION
 *      This function starts the connection parameter update timer.
 *      g_app_data.con_param_update_tid must be TIMER_INVALID on entry to this
 *      function.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appStartConnUpdateTimer(void)
{
    if(g_app_data.conn_interval < PREFERRED_MIN_CON_INTERVAL ||
       g_app_data.conn_interval > PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
       || g_app_data.conn_latency < PREFERRED_SLAVE_LATENCY
#endif
      )
    {
        /* Set the number of connection parameter update attempts to zero */
        g_app_data.num_conn_update_req = 0;

        /* Start timer to trigger connection parameter update procedure */
        g_app_data.con_param_update_tid = TimerCreate(
                            GAP_CONN_PARAM_TIMEOUT,
                            TRUE, requestConnParamUpdate);

    }
}

#ifdef PAIRING_SUPPORT
/*----------------------------------------------------------------------------*
 *  NAME
 *      handleBondingChanceTimerExpiry
 *
 *  DESCRIPTION
 *      This function is handle the expiry of bonding chance timer.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleBondingChanceTimerExpiry(timer_id tid)
{
    if(g_app_data.bonding_reattempt_tid == tid)
    {
        /* The timer has just expired, so mark it as invalid */
        g_app_data.bonding_reattempt_tid = TIMER_INVALID;
        /* The bonding chance timer has expired. This means the remote has not
         * encrypted the link using old keys. Disconnect the link.
         */
        SetState(app_state_disconnecting);
    }/* Else it may be due to some race condition. Ignore it. */
}
#endif /* PAIRING_SUPPORT */

/*----------------------------------------------------------------------------*
 *  NAME
 *      requestConnParamUpdate
 *
 *  DESCRIPTION
 *      This function is used to send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST 
 *      to the remote device.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void requestConnParamUpdate(timer_id tid)
{
    /* Application specific preferred parameters */
     ble_con_params app_pref_conn_param = 
                {
                    PREFERRED_MIN_CON_INTERVAL,
                    PREFERRED_MAX_CON_INTERVAL,
                    PREFERRED_SLAVE_LATENCY,
                    PREFERRED_SUPERVISION_TIMEOUT
                };

    if(g_app_data.con_param_update_tid == tid)
    {
        /* Timer has just expired, so mark it as being invalid */
        g_app_data.con_param_update_tid = TIMER_INVALID;

        /* Handle signal as per current state */
        switch(g_app_data.state)
        {

            case app_state_connected:
            {
                /* Send Connection Parameter Update request using application 
                 * specific preferred connection parameters
                 */

                if(LsConnectionParamUpdateReq(&g_app_data.con_bd_addr, 
                                &app_pref_conn_param) != ls_err_none)
                {
                    ReportPanic(app_panic_con_param_update);
                }

                /* Increment the count for connection parameter update 
                 * requests 
                 */
                ++ g_app_data.num_conn_update_req;

            }
            break;

            default:
                /* Ignore in other states */
            break;
        }

    } /* Else ignore the timer */

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appExitAdvertising
 *
 *  DESCRIPTION
 *      This function is called while exiting app_state_fast_advertising and
 *      app_state_slow_advertising states.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appExitAdvertising(void)
{
    /* Cancel advertisement timer. Must be valid because timer is active
     * during app_state_fast_advertising and app_state_slow_advertising states.
     */
    TimerDelete(g_app_data.app_tid);
    g_app_data.app_tid = TIMER_INVALID;
    
    /* stop LED indication */
    LedEnable(FALSE);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appAdvertTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to handle Advertisement timer expiry.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appAdvertTimerHandler(timer_id tid)
{
    /* Based upon the timer id, stop on-going advertisements */
    if(g_app_data.app_tid == tid)
    {
        /* Timer has just expired so mark it as invalid */
        g_app_data.app_tid = TIMER_INVALID;

        GattStopAdverts();
    }/* Else ignore timer expiry, could be because of 
      * some race condition */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appInitExit
 *
 *  DESCRIPTION
 *      This function is called upon exiting from app_state_init state. The 
 *      application starts advertising after exiting this state.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appInitExit(void)
{
    if(g_app_data.bonded && 
       !GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
    {
        /* If the device is bonded and the bonded device address is not
         * resolvable random, configure the white list with the bonded 
         * host address.
         */
        if(LsAddWhiteListDevice(&g_app_data.bonded_bd_addr) != ls_err_none)
        {
            ReportPanic(app_panic_add_whitelist);
        }
    }
}

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
/*----------------------------------------------------------------------------*
 *  NAME
 *      resetIdleTimer
 *
 *  DESCRIPTION
 *      This function is used to reset the time for which the application was
 *      idle during the connected state.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void resetIdleTimer(void)
{
    /* Delete the Idle timer, if already running */
    if (g_app_data.app_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.app_tid);
    }

    /* Start the Idle timer again.*/
    g_app_data.app_tid  = TimerCreate(CONNECTED_IDLE_TIMEOUT_VALUE, 
                                    TRUE, appIdleTimerHandler);
}
#endif /* CONNECTED_IDLE_TIMEOUT_VALUE */

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAddDbCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_ADD_DB_CFM.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by GATT_ADD_DB_CFM signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattAddDbCfm(GATT_ADD_DB_CFM_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_init:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Start beaconing */
                SetState(app_state_beaconing);
            }
            else
            {
                /* This should never happen */
                ReportPanic(app_panic_db_registration);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmEvConnectionComplete
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_COMPLETE.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LM_EV_CONNECTION_COMPLETE
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data)
{
    /* Store the connection parameters. */
    g_app_data.conn_interval = p_event_data->data.conn_interval;
    g_app_data.conn_latency = p_event_data->data.conn_latency;
    g_app_data.conn_timeout = p_event_data->data.supervision_timeout;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattCancelConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CANCEL_CONNECT_CFM.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattCancelConnectCfm(void)
{
    if(g_app_data.pairing_button_pressed)
    {
        /* Pairing removal has been initiated by the user */
        g_app_data.pairing_button_pressed = FALSE;

        /* Disable white list */
        g_app_data.enable_white_list = FALSE;

        /* Reset and clear the white list */
        LsResetWhiteList();

        /* Trigger fast advertisements */
        if(g_app_data.state == app_state_fast_advertising)
        {
            GattTriggerFastAdverts(&g_app_data.bonded_bd_addr);
        }
        else
        {
            SetState(app_state_fast_advertising);
        }
    }
    else
    {
        /* Handle signal as per current state.
         *
         * The application follows this sequence in advertising state:
         *
         *    Fast advertising for FAST_CONNECTION_ADVERT_TIMEOUT_VALUE seconds:
         *    If the application is bonded to a remote device then during this
         *    period it will use the white list for the first
         *    BONDED_DEVICE_ADVERT_TIMEOUT_VALUE seconds, and for the remaining
         *    time (FAST_CONNECTION_ADVERT_TIMEOUT_VALUE - 
         *    BONDED_DEVICE_ADVERT_TIMEOUT_VALUE seconds), it will do fast
         *    advertising without any white list.
         */
        switch(g_app_data.state)
        {
            case app_state_fast_advertising:
            {
                bool fastConns = FALSE;

                if(g_app_data.enable_white_list)
                {
                    /* Bonded device advertisements stopped. Reset the white
                     * list
                     */
                    if(LsDeleteWhiteListDevice(&g_app_data.bonded_bd_addr)
                        != ls_err_none)
                    {
                        ReportPanic(app_panic_delete_whitelist);
                    }

                    /* Case of stopping advertisements for the bonded device 
                     * at the expiry of BONDED_DEVICE_ADVERT_TIMEOUT_VALUE timer
                     */
                    g_app_data.enable_white_list = FALSE;

                    fastConns = TRUE;
                }

                if(fastConns)
                {
                    GattStartAdverts(&g_app_data.bonded_bd_addr, TRUE);

                    /* Remain in same state */
                }
                else
                {
                    SetState(app_state_beaconing);
                }
            }
            break;
            
            case app_state_beaconing:
            {
                /* nothing to do */
            }
            break;

            default:
                /* Control should never come here */
                ReportPanic(app_panic_invalid_state);
            break;
        }

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CONNECT_CFM.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by GATT_CONNECT_CFM signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_fast_advertising:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Store received UCID */
                g_app_data.st_ucid = p_event_data->cid;

                /* Store connected BD Address */
                g_app_data.con_bd_addr = p_event_data->bd_addr;

                if(g_app_data.bonded && 
                   GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr) &&
                   (SMPrivacyMatchAddress(&p_event_data->bd_addr,
                                          g_app_data.irk,
                                          MAX_NUMBER_IRK_STORED, 
                                          MAX_WORDS_IRK) < 0))
                {
                    /* Application was bonded to a remote device using 
                     * resolvable random address and application has failed 
                     * to resolve the remote device address to which we just 
                     * connected, so disconnect and start advertising again
                     */
                    SetState(app_state_disconnecting);
                }
                else
                {
                    /* Enter connected state 
                     * - If the device is not bonded OR
                     * - If the device is bonded and the connected host doesn't 
                     *   support Resolvable Random address OR
                     * - If the device is bonded and connected host supports 
                     *   Resolvable Random address and the address gets resolved
                     *   using the stored IRK key
                     */
                    SetState(app_state_connected);

#ifndef PAIRING_SUPPORT
                    /* if the application does not mandate encryption
                     * requirement on its characteristics, the remote master may
                     * or may not encrypt the link. Start a timer here to give
                     * remote master some time to encrypt the link and on expiry
                     * of that timer, send a connection parameter update request
                     * to remote device.
                     */

                    /* If the current connection parameters being used don't 
                     * comply with the application's preferred connection 
                     * parameters and the timer is not running, start a timer
                     * to trigger the Connection Parameter Update procedure.
                     */

                    if(g_app_data.con_param_update_tid == TIMER_INVALID)
                    {
                        appStartConnUpdateTimer();
                    } /* Else at the expiry of the timer the connection
                       * parameter update procedure will be triggered
                       */

#endif /* !PAIRING_SUPPORT */
                }
            }
            else
            {
                /* Connection failure - Trigger fast advertisements */
                GattStartAdverts(&g_app_data.bonded_bd_addr, TRUE);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmKeysInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_KEYS_IND and copies the IRK from it.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_KEYS_IND signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmKeysInd(SM_KEYS_IND_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {

            /* Store the diversifier which will be used for accepting/rejecting
             * the encryption requests.
             */
            g_app_data.diversifier = (p_event_data->keys)->div;

            /* Write the new diversifier to NVM */
            Nvm_Write(&g_app_data.diversifier,
                      sizeof(g_app_data.diversifier), 
                      NVM_OFFSET_SM_DIV);

            /* Store IRK if the connected host is using random resolvable 
             * address. IRK is used afterwards to validate the identity of 
             * connected host 
             */
            if(GattIsAddressResolvableRandom(&g_app_data.con_bd_addr)) 
            {
                MemCopy(g_app_data.irk, 
                        (p_event_data->keys)->irk,
                        MAX_WORDS_IRK);

                /* If bonded device address is resolvable random
                 * then store IRK to NVM 
                 */
                Nvm_Write(g_app_data.irk, 
                          MAX_WORDS_IRK, 
                          NVM_OFFSET_SM_IRK);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

#ifdef PAIRING_SUPPORT
/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmPairingAuthInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_PAIRING_AUTH_IND. This message will
 *      only be received when the peer device is initiating 'Just Works' 
 *      pairing.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_PAIRING_AUTH_IND signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmPairingAuthInd(SM_PAIRING_AUTH_IND_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            /* Authorise the pairing request if the application is NOT bonded */
            if(!g_app_data.bonded)
            {
                SMPairingAuthRsp(p_event_data->data, TRUE);
            }
            else /* Otherwise Reject the pairing request */
            {
                SMPairingAuthRsp(p_event_data->data, FALSE);
            }
        }
        break;

        default:
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLMEncryptionChange
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_ENCRYPTION_CHANGE.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LM_EV_ENCRYPTION_CHANGE signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLMEncryptionChange(
                    HCI_EV_DATA_ENCRYPTION_CHANGE_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            if(p_event_data->status == sys_status_success)
            {
                g_app_data.encrypt_enabled = p_event_data->enc_enable;

                if(g_app_data.encrypt_enabled)
                {
                    /* Delete the bonding chance timer, if running */
                    if (g_app_data.bonding_reattempt_tid != TIMER_INVALID)
                    {
                        TimerDelete(g_app_data.bonding_reattempt_tid);
                        g_app_data.bonding_reattempt_tid = TIMER_INVALID;
                    }

                    /* Update battery status at every connection instance. It 
                     * may not be worth updating timer this often, but this will 
                     * depend upon application requirements.
                     */
                    BatteryUpdateLevel(g_app_data.st_ucid);

                    /* If the current connection parameters being used don't 
                     * comply with the application's preferred connection 
                     * parameters and the timer is not running, start a timer
                     * to trigger the Connection Parameter Update procedure.
                     */
                    
                    if(g_app_data.con_param_update_tid == TIMER_INVALID)
                    {
                        appStartConnUpdateTimer();
                    } /* Else at the expiry of the timer the Connection
                       * Parameter Update procedure will be triggered
                       */
                }
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}
#endif /* #ifdef PAIRING_SUPPORT */

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmSimplePairingCompleteInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_SIMPLE_PAIRING_COMPLETE_IND.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_SIMPLE_PAIRING_COMPLETE_IND
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmSimplePairingCompleteInd(
                                 SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            if(p_event_data->status == sys_status_success)
            {
                /* Store bonded host information to NVM. This includes
                 * application and service specific information.
                 */
                g_app_data.bonded = TRUE;
                g_app_data.bonded_bd_addr = p_event_data->bd_addr;

                /* Store bonded host typed bd address to NVM */

                /* Write one word bonded flag */
                Nvm_Write((uint16*)&g_app_data.bonded, 
                          sizeof(g_app_data.bonded), 
                          NVM_OFFSET_BONDED_FLAG);

                /* Write typed bd address of bonded host */
                Nvm_Write((uint16*)&g_app_data.bonded_bd_addr, 
                          sizeof(TYPED_BD_ADDR_T), 
                          NVM_OFFSET_BONDED_ADDR);

                /* Configure white list with the Bonded host address only 
                 * if the connected host doesn't support random resolvable
                 * addresses
                 */
                if(!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
                {
                    /* It is important to note that this application does not
                     * support Reconnection Address. In future, if the
                     * application is enhanced to support Reconnection Address,
                     * make sure that we don't add Reconnection Address to the
                     * white list
                     */
                    if(LsAddWhiteListDevice(&g_app_data.bonded_bd_addr) !=
                        ls_err_none)
                    {
                        ReportPanic(app_panic_add_whitelist);
                    }

                }

                /* If the devices are bonded then send notification to all 
                 * registered services for the same so that they can store
                 * required data to NVM.
                 */
                BatteryBondingNotify();
                EsurlBeaconBondingNotify();         
                
                /* Add the Service Bonding Notify API here */
            }
            else
            {
#ifdef PAIRING_SUPPORT
                /* Pairing has failed.
                 * 1. If pairing has failed due to repeated attempts, the 
                 *    application should immediately disconnect the link.
                 * 2. If the application was bonded and pairing has failed, then
                 *    since the application was using a white list the remote 
                 *    device has the same address as our bonded device address.
                 *    The remote connected device may be a genuine one but
                 *    instead of using old keys, wanted to use new keys. We do
                 *    not allow bonding again if we are already bonded but we
                 *    will give the connected device some time to encrypt the
                 *    link using the old keys. If the remote device fails to
                 *    encrypt the link in that time we will disconnect the link.
                 */
                 if(p_event_data->status == sm_status_repeated_attempts)
                 {
                    SetState(app_state_disconnecting);
                 }
                 else if(g_app_data.bonded)
                 {
                    g_app_data.encrypt_enabled = FALSE;
                    g_app_data.bonding_reattempt_tid = 
                                          TimerCreate(
                                               BONDING_CHANCE_TIMER,
                                               TRUE, 
                                               handleBondingChanceTimerExpiry);
                 }
#else /* !PAIRING_SUPPORT */
                /* If application is already bonded to this host and pairing 
                 * fails, remove device from the white list.
                 */
                if(g_app_data.bonded)
                {
                    if(LsDeleteWhiteListDevice(&g_app_data.bonded_bd_addr) != 
                                        ls_err_none)
                    {
                        ReportPanic(app_panic_delete_whitelist);
                    }

                    g_app_data.bonded = FALSE;
                }

                /* The case when pairing has failed. The connection may still be
                 * there if the remote device hasn't disconnected it. The
                 * remote device may retry pairing after a time defined by its
                 * own application. So reset all other variables except the
                 * connection specific ones.
                 */

                /* Update bonded status to NVM */
                Nvm_Write((uint16*)&g_app_data.bonded,
                          sizeof(g_app_data.bonded),
                          NVM_OFFSET_BONDED_FLAG);

                /* Initialise the data of used services as the device is no 
                 * longer bonded to the remote host.
                 */
                GapDataInit();
                BatteryDataInit();
                EsurlBeaconDataInit();
                
                /* Call all the APIs which initailise the required service(s)
                 * supported by the application.
                 */
#endif /* PAIRING_SUPPORT */

            }
        }
        break;

        default:
            /* Firmware may send this signal after disconnection. So don't 
             * panic but ignore this signal.
             */
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmDivApproveInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_DIV_APPROVE_IND.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_DIV_APPROVE_IND signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        
        /* Request for approval from application comes only when pairing is not
         * in progress
         */
        case app_state_connected:
        {
            sm_div_verdict approve_div = SM_DIV_REVOKED;
            
            /* Check whether the application is still bonded (bonded flag gets
             * reset upon 'connect' button press by the user). Then check 
             * whether the diversifier is the same as the one stored by the 
             * application
             */
            if(g_app_data.bonded)
            {
                if(g_app_data.diversifier == p_event_data->div)
                {
                    approve_div = SM_DIV_APPROVED;
                }
            }

            SMDivApproval(p_event_data->cid, approve_div);
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateCfm
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_CFM.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LS_CONNECTION_PARAM_UPDATE_CFM
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateCfm(
                            LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            /* Received in response to the L2CAP_CONNECTION_PARAMETER_UPDATE 
             * request sent from the slave after encryption is enabled. If 
             * the request has failed, the device should send the same request
             * again only after Tgap(conn_param_timeout). Refer Bluetooth 4.0
             * spec Vol 3 Part C, Section 9.3.9 and profile spec.
             */
            if ((p_event_data->status != ls_err_none) &&
                    (g_app_data.num_conn_update_req < 
                    MAX_NUM_CONN_PARAM_UPDATE_REQS))
            {
                /* Delete timer if running */
                if (g_app_data.con_param_update_tid != TIMER_INVALID)
                {
                    TimerDelete(g_app_data.con_param_update_tid);
                }

                g_app_data.con_param_update_tid = TimerCreate(
                                             GAP_CONN_PARAM_TIMEOUT,
                                             TRUE, requestConnParamUpdate);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateInd
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_IND.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LS_CONNECTION_PARAM_UPDATE_IND
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateInd(
                                 LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {

        case app_state_connected:
        {
            /* Delete timer if running */
            if (g_app_data.con_param_update_tid != TIMER_INVALID)
            {
                TimerDelete(g_app_data.con_param_update_tid);
                g_app_data.con_param_update_tid = TIMER_INVALID;
            }

            /* Store the new connection parameters. */
            g_app_data.conn_interval = p_event_data->conn_interval;
            g_app_data.conn_latency = p_event_data->conn_latency;
            g_app_data.conn_timeout = p_event_data->supervision_timeout;
            
            /* Connection parameters have been updated. Check if new parameters 
             * comply with application preferred parameters. If not, application
             * shall trigger Connection parameter update procedure.
             */
            appStartConnUpdateTimer();
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAccessInd
 *
 *  DESCRIPTION
 *      This function handles GATT_ACCESS_IND messages for attributes maintained
 *      by the application.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by GATT_ACCESS_IND message
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            /* Received GATT ACCESS IND with write access */
            if(p_event_data->flags == 
                (ATT_ACCESS_WRITE | 
                 ATT_ACCESS_PERMISSION | 
                 ATT_ACCESS_WRITE_COMPLETE))
            {
                HandleAccessWrite(p_event_data);
            }
            /* Received GATT ACCESS IND with read access */
            else if(p_event_data->flags == 
                (ATT_ACCESS_READ | 
                ATT_ACCESS_PERMISSION))
            {
                HandleAccessRead(p_event_data);
            }
            else
            {
                /* No other request is supported */
                GattAccessRsp(p_event_data->cid, p_event_data->handle, 
                              gatt_status_request_not_supported,
                              0, NULL);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmDisconnectComplete
 *
 *  DESCRIPTION
 *      This function handles LM Disconnect Complete event which is received
 *      at the completion of disconnect procedure triggered either by the 
 *      device or remote host or because of link loss.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LM_EV_DISCONNECT_COMPLETE
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data)
{

    /* Set UCID to INVALID_UCID */
    g_app_data.st_ucid = GATT_INVALID_UCID;

    /* Reset the connection parameter variables. */
    g_app_data.conn_interval = 0;
    g_app_data.conn_latency = 0;
    g_app_data.conn_timeout = 0;
    
    /* LM_EV_DISCONNECT_COMPLETE event can have following disconnect reasons:
     *
     * HCI_ERROR_CONN_TIMEOUT - Link Loss case
     * HCI_ERROR_CONN_TERM_LOCAL_HOST - Disconnect triggered by device
     * HCI_ERROR_OETC_* - Other end (i.e., remote host) terminated connection
     */

    /* ON DISCONNECT: Update the TX Power in *both* RADIO and ADV the tx_level_mode */
    EsurlBeaconUpdateTxPowerFromMode(EsurlBeaconGetTxPowerMode());
    /* ON DISCONNECT: Write out NVM state for Esurl Beacon Service (DONE LAST) */
    EsurlBeaconWriteDataToNVM(NULL); // Null argument assumes the prior nvm_offset 
    
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
            /* Initialise Application data instance */
            appDataInit();

            /* FALLTHROUGH */

        case app_state_disconnecting:
        {
            /* Start beaconing */
            SetState(app_state_beaconing);
        }
        break;
        
        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReportPanic
 *
 *  DESCRIPTION
 *      This function calls firmware panic routine and gives a single point 
 *      of debugging any application level panics.
 *
 *  PARAMETERS
 *      panic_code [in]         Code to supply to firmware Panic function.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void ReportPanic(app_panic_code panic_code)
{
    /* Raise panic */
    Panic(panic_code);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleShortButtonPress
 *
 *  DESCRIPTION
 *      This function contains handling of short button press. If connected,
 *      the device disconnects from the connected host else it triggers
 *      advertisements.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandleShortButtonPress(void)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
            /* Disconnect from the connected host */
            SetState(app_state_disconnecting);
            
            /* As per the specification Vendor may choose to initiate the 
             * idle timer which will eventually initiate the disconnect.
             */
             
        break;

        case app_state_idle:
        case app_state_beaconing:
            /* Trigger fast advertisements */
            SetState(app_state_fast_advertising);
        break;

        default:
            /* Ignore in remaining states */
        break;

    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SetState
 *
 *  DESCRIPTION
 *      This function is used to set the state of the application.
 *
 *  PARAMETERS
 *      new_state [in]          State to move to
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void SetState(app_state new_state)
{
    /* Check that the new state is not the same as the current state */
    app_state old_state = g_app_data.state;
    
    if (old_state != new_state)
    {
        /* Exit current state */
        switch (old_state)
        {
            case app_state_init:
                appInitExit();
            break;
            
            case app_state_beaconing:
                /* Stop beaconing */
                BeaconStart(FALSE);
            break;

            case app_state_disconnecting:
                /* Common things to do whenever application exits
                 * app_state_disconnecting state.
                 */

                /* Initialise application and used services data structure 
                 * while exiting Disconnecting state
                 */
                appDataInit();
            break;

            case app_state_fast_advertising:
                /* Common things to do whenever application exits
                 * APP_*_ADVERTISING state.
                 */
                appExitAdvertising();
            break;

            case app_state_connected:
                /* The application may need to maintain the values of some
                 * profile specific data across connections and power cycles.
                 * These values would have changed in 'connected' state. So,
                 * update the values of this data stored in the NVM.
                 */
            break;

            case app_state_idle:
                /* Nothing to do */
            break;

            default:
                /* Nothing to do */
            break;
        }

        /* Set new state */
        g_app_data.state = new_state;

        /* Enter new state */
        switch (new_state)
        {
            case app_state_fast_advertising:
            {
                /* Enable white list if application is bonded to some remote 
                 * device and that device is not using resolvable random 
                 * address.
                 */
                enableWhiteList();
                /* Trigger fast advertisements. */
                GattTriggerFastAdverts(&g_app_data.bonded_bd_addr);

                /* Indicate advertising mode by sounding two short beeps */
                SoundBuzzer(buzzer_beep_twice);
                
                /* start LED indication */
                LedEnable(TRUE);
            }
            break;
            
            case app_state_beaconing:
                /* Sound long beep to indicate non-connectable mode */
                SoundBuzzer(buzzer_beep_long);

                /* Start beaconing */
                BeaconStart(TRUE);
            break;

            case app_state_idle:
                /* Sound long beep to indicate non-connectable mode */
                SoundBuzzer(buzzer_beep_long);
            break;

            case app_state_connected:
            {
                /* Common things to do whenever application enters
                 * app_state_connected state.
                 */
#ifdef PAIRING_SUPPORT                
                /* Trigger SM Slave Security request only if the remote 
                 * host is not using resolvable random address
                 */
                if(!GattIsAddressResolvableRandom(&g_app_data.con_bd_addr))
                {
                    SMRequestSecurityLevel(&g_app_data.con_bd_addr);
                }
#else /* !PAIRING_SUPPORT */
                /* Update battery status at every connection instance. It may 
                 * not be worth updating timer this often, but this will 
                 * depend upon application requirements 
                 */
                BatteryUpdateLevel(g_app_data.st_ucid);
#endif /* PAIRING_SUPPORT */

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
                resetIdleTimer();
#endif
            }
            break;

            case app_state_disconnecting:
                /* Disconnect the link */
                GattDisconnectReq(g_app_data.st_ucid);
            break;

            default:
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetState
 *
 *  DESCRIPTION
 *      This function returns the current state of the application.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Current application state
 *----------------------------------------------------------------------------*/
extern app_state GetState(void)
{
    return g_app_data.state;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      IsWhiteListEnabled
 *
 *  DESCRIPTION
 *      This function returns whether white list is enabled or not.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      TRUE if white list is enabled, FALSE otherwise.
 *----------------------------------------------------------------------------*/
extern bool IsWhiteListEnabled(void)
{
    return g_app_data.enable_white_list;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePairingRemoval
 *
 *  DESCRIPTION
 *      This function contains pairing removal handling.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandlePairingRemoval(void)
{

    /* Remove bonding information */

    /* The device will no longer be bonded */
    g_app_data.bonded = FALSE;

    /* Write bonded status to NVM */
    Nvm_Write((uint16*)&g_app_data.bonded, 
              sizeof(g_app_data.bonded), 
              NVM_OFFSET_BONDED_FLAG);


    switch(g_app_data.state)
    {

        case app_state_connected:
        {
            /* Disconnect from the connected host before triggering 
             * advertisements again for any host to connect. Application
             * and services data related to bonding status will get 
             * updated while exiting disconnecting state.
             */
            SetState(app_state_disconnecting);

            /* Reset and clear the white list */
            LsResetWhiteList();
        }
        break;

        case app_state_fast_advertising:
        {
            /* Initialise application and services data related to 
             * bonding status
             */
            appDataInit();

            /* Set flag for pairing / bonding removal */
            g_app_data.pairing_button_pressed = TRUE;

            /* Stop advertisements first as it may be making use of the white 
             * list. Once advertisements are stopped, reset the white list
             * and trigger advertisements again for any host to connect.
             */
            GattStopAdverts();
        }
        break;

        case app_state_disconnecting:
        {
            /* Disconnect procedure is on-going, so just reset the white list 
             * and wait for the procedure to complete before triggering 
             * advertisements again for any host to connect. Application
             * and services data related to bonding status will get 
             * updated while exiting disconnecting state.
             */
            LsResetWhiteList();
        }
        break;

        default: /* app_state_init / app_state_idle handling */
        {
            /* Initialise application and services data related to 
             * bonding status
             */
            appDataInit();

            /* Reset and clear the white list */
            LsResetWhiteList();

            /* Start fast undirected advertisements */
            SetState(app_state_fast_advertising);
        }
        break;

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      StartAdvertTimer
 *
 *  DESCRIPTION
 *      This function starts the advertisement timer.
 *
 *  PARAMETERS
 *      interval [in]           Timer duration, microseconds
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void StartAdvertTimer(uint32 interval)
{
    /* Cancel existing timer, if valid */
    if (g_app_data.app_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.app_tid);
    }

    /* Start advertisement timer  */
    g_app_data.app_tid = TimerCreate(interval, TRUE, appAdvertTimerHandler);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      IsDeviceBonded
 *
 *  DESCRIPTION
 *      This function returns the status whether the connected device is 
 *      bonded or not.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      TRUE if device is bonded, FALSE if not.
 *----------------------------------------------------------------------------*/
extern bool IsDeviceBonded(void)
{
    return g_app_data.bonded;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetConnectionID
 *
 *  DESCRIPTION
 *      This function returns the connection identifier.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Connection identifier.
 *----------------------------------------------------------------------------*/
extern uint16 GetConnectionID(void)
{
    return g_app_data.st_ucid;
}

/*============================================================================*
 *  System Callback Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerOnReset
 *
 *  DESCRIPTION
 *      This user application function is called just after a power-on reset
 *      (including after a firmware panic), or after a wakeup from Hibernate or
 *      Dormant sleep states.
 *
 *      At the time this function is called, the last sleep state is not yet
 *      known.
 *
 *      NOTE: this function should only contain code to be executed after a
 *      power-on reset or panic. Code that should also be executed after an
 *      HCI_RESET should instead be placed in the AppInit() function.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void AppPowerOnReset(void)
{
    /* Code that is only executed after a power-on reset or firmware panic
     * should be implemented here - e.g. configuring application constants
     */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This user application function is called after a power-on reset
 *      (including after a firmware panic), after a wakeup from Hibernate or
 *      Dormant sleep states, or after an HCI Reset has been requested.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after AppPowerOnReset().
 *
 *  PARAMETERS
 *      last_sleep_state [in]   Last sleep state
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void AppInit(sleep_state last_sleep_state)
{
    uint16 gatt_db_length = 0;  /* GATT database size */
    uint16 *p_gatt_db = NULL;   /* GATT database */
    
#if defined(USE_STATIC_RANDOM_ADDRESS) && !defined(PAIRING_SUPPORT)
    BD_ADDR_T ra;
#endif
   
    /* Initialise application debug */
    DebugIfInit();
    
    /* Start the debug messages on the UART */
    DebugIfWriteString("\r\n\r\n**************************\r\n");
    DebugIfWriteString("GATT server GAP peripheral\r\n\r\n");

    /* Use static random address for the application */
#if defined(USE_STATIC_RANDOM_ADDRESS) && !defined(PAIRING_SUPPORT)
     GapSetStaticAddress();
#endif
    

    /* Initialise the GATT Server application state */
    g_app_data.state = app_state_init;

    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);
    
    /* Initialise local timers */
    g_app_data.con_param_update_tid = TIMER_INVALID;
    g_app_data.app_tid = TIMER_INVALID;
#ifdef PAIRING_SUPPORT
    g_app_data.bonding_reattempt_tid = TIMER_INVALID;
#endif

    /* Initialise GATT entity */
    GattInit();

    /* Initialise GATT Server H/W */
    InitHardware();

    /* Install GATT Server support for the optional Write procedure.
     * This is mandatory only if the control point characteristic is supported. 
     */
    GattInstallServerWrite();

    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(FALSE);

#ifdef NVM_TYPE_EEPROM
    /* Configure the NVM manager to use I2C EEPROM for NVM store */
    NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
    /* Configure the NVM Manager to use SPI flash for NVM store. */
    NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    Nvm_Disable();
    
    /* Initialize the GAP data. Needs to be done before readPersistentStore */
    GapDataInit();    

    /* Battery initialisation on chip reset */
    BatteryInitChipReset();

    /* Beacon initialisation on chip reset */
    EsurlBeaconInitChipReset();    

    /* Read persistent storage */
    readPersistentStore();  
    
#if defined(USE_STATIC_RANDOM_ADDRESS) && !defined(PAIRING_SUPPORT)

    /* Generate static random device address. By default, the static random
     * is derived from IDENTITY_ROOT CsKey. For simplicity, we manually
     * generate random address and pass it to the firmware
     */
    ra.nap = BD_ADDR_NAP_RANDOM_TYPE_STATIC | Random16();
    ra.lap = ((((uint24)Random16()) << 16) | (Random16() >> 8));
    ra.uap = ((uint8)(Random16() >> 8));
    
    /* set random address */
    GapSetRandomAddress(&ra);
#endif

    /* Tell Security Manager module what value it needs to initialise its
     * diversifier to.
     */
    SMInit(g_app_data.diversifier);
    
    /* Initialise hardware data */
    HwDataInit();

    /* Initialise application data structure */
    appDataInit();

    /* Tell GATT about our database. We will get a GATT_ADD_DB_CFM event when
     * this has completed.
     */
    p_gatt_db = GattGetDatabase(&gatt_db_length);

    GattAddDatabaseReq(gatt_db_length, p_gatt_db);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcesSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  PARAMETERS
 *      id   [in]               System event ID
 *      data [in]               Event data
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch(id)
    {
        case sys_event_battery_low:
        {
            /* Battery low event received - notify the connected host. If 
             * not connected, the battery level will get notified when 
             * device gets connected again
             */
            if(g_app_data.state == app_state_connected)
            {
                BatteryUpdateLevel(g_app_data.st_ucid);
            }
        }
        break;

        case sys_event_pio_changed:
        {
             /* Handle the PIO changed event. */
             HandlePIOChangedEvent((pio_changed_data*)data);
        }
        break;
            
        default:
            /* Ignore anything else */
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event
 *      is received by the system.
 *
 *  PARAMETERS
 *      event_code [in]         LM event ID
 *      event_data [in]         LM event data
 *
 *  RETURNS
 *      TRUE if the app has finished with the event data; the control layer
 *      will free the buffer.
 *----------------------------------------------------------------------------*/
bool AppProcessLmEvent(lm_event_code event_code, LM_EVENT_T *p_event_data)
{
    switch (event_code)
    {

        /* Handle events received from Firmware */

        case GATT_ADD_DB_CFM:
            /* Attribute database registration confirmation */
            handleSignalGattAddDbCfm((GATT_ADD_DB_CFM_T*)p_event_data);
        break;

        case LM_EV_CONNECTION_COMPLETE:
            /* Handle the LM connection complete event. */
            handleSignalLmEvConnectionComplete(
                                (LM_EV_CONNECTION_COMPLETE_T*)p_event_data);
        break;

        case GATT_CANCEL_CONNECT_CFM:
            /* Confirmation for the completion of GattCancelConnectReq()
             * procedure 
             */
            handleSignalGattCancelConnectCfm();
        break;

        case GATT_CONNECT_CFM:
            /* Confirmation for the completion of GattConnectReq() 
             * procedure
             */
            handleSignalGattConnectCfm((GATT_CONNECT_CFM_T*)p_event_data);
        break;

        case SM_KEYS_IND:
            /* Indication for the keys and associated security information
             * on a connection that has completed Short Term Key Generation 
             * or Transport Specific Key Distribution
             */
            handleSignalSmKeysInd((SM_KEYS_IND_T *)p_event_data);
        break;

#ifdef PAIRING_SUPPORT
        case SM_PAIRING_AUTH_IND:
            /* Authorise or Reject the pairing request */
            handleSignalSmPairingAuthInd((SM_PAIRING_AUTH_IND_T*)p_event_data);
        break;
#endif /* PAIRING_SUPPORT */


        case SM_SIMPLE_PAIRING_COMPLETE_IND:
            /* Indication for completion of Pairing procedure */
            handleSignalSmSimplePairingCompleteInd(
                (SM_SIMPLE_PAIRING_COMPLETE_IND_T *)p_event_data);
        break;

        case LM_EV_ENCRYPTION_CHANGE:
            /* Indication for encryption change event */
#ifdef PAIRING_SUPPORT
            handleSignalLMEncryptionChange(
            (HCI_EV_DATA_ENCRYPTION_CHANGE_T *)&p_event_data->enc_change.data);
#else /* !PAIRING_SUPPORT */
            /* Nothing to do */
#endif /* PAIRING_SUPPORT */

        break;


         case SM_DIV_APPROVE_IND:
            /* Indication for SM Diversifier approval requested by F/W when 
             * the last bonded host exchange keys. Application may or may not
             * approve the diversifier depending upon whether the application 
             * is still bonded to the same host
             */
            handleSignalSmDivApproveInd((SM_DIV_APPROVE_IND_T *)p_event_data);
        break;


        /* Received in response to the LsConnectionParamUpdateReq() 
         * request sent from the slave after encryption is enabled. If 
         * the request has failed, the device should send the same 
         * request again only after Tgap(conn_param_timeout). Refer Bluetooth 4.0 
         * spec Vol 3 Part C, Section 9.3.9 and HID over GATT profile spec 
         * section 5.1.2.
         */
        case LS_CONNECTION_PARAM_UPDATE_CFM:
            handleSignalLsConnParamUpdateCfm(
                            (LS_CONNECTION_PARAM_UPDATE_CFM_T*)p_event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_IND:
            /* Indicates completion of remotely triggered Connection 
             * Parameter Update procedure
             */
            handleSignalLsConnParamUpdateInd(
                            (LS_CONNECTION_PARAM_UPDATE_IND_T *)p_event_data);
        break;

        case GATT_ACCESS_IND:
            /* Indicates that an attribute controlled directly by the
             * application (ATT_ATTR_IRQ attribute flag is set) is being 
             * read from or written to.
             */
            handleSignalGattAccessInd((GATT_ACCESS_IND_T *)p_event_data);
        break;

        case GATT_DISCONNECT_IND:
            /* Disconnect procedure triggered by remote host or due to 
             * link loss is considered complete on reception of 
             * LM_EV_DISCONNECT_COMPLETE event. So, it gets handled on 
             * reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case GATT_DISCONNECT_CFM:
            /* Confirmation for the completion of GattDisconnectReq()
             * procedure is ignored as the procedure is considered complete 
             * on reception of LM_EV_DISCONNECT_COMPLETE event. So, it gets 
             * handled on reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case LM_EV_DISCONNECT_COMPLETE:
        {
            /* Disconnect procedures either triggered by application or remote
             * host or link loss case are considered completed on reception 
             * of LM_EV_DISCONNECT_COMPLETE event
             */
             handleSignalLmDisconnectComplete(
                    &((LM_EV_DISCONNECT_COMPLETE_T *)p_event_data)->data);
        }
        break;

        default:
            /* Ignore any other event */ 
        break;

    }

    return TRUE;
}





