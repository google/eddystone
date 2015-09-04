/**************************************************************************************************
  Filename:       simpleEddystoneBeacon.c

  Description:    This file contains the Simple Eddystone Beacon sample
                  application for use with the CC2650 Bluetooth Low Energy
                  Protocol Stack.

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

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include "hci.h"
#include "hci_tl.h"
#include "gatt.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "eddystoneURLCfg.h"

#if defined(SENSORTAG_HW)
#include "bsp_spi.h"
#endif // SENSORTAG_HW

#if defined(FEATURE_OAD) || defined(IMAGE_INVALIDATE)
#include "oad_target.h"
#include "oad.h"
#endif //FEATURE_OAD || IMAGE_INVALIDATE

#include "peripheral.h"
#include "gapbondmgr.h"

#include "osal_snv.h"
#include "ICallBleAPIMSG.h"

#include "util.h"
#include "board_lcd.h"
#include "board_key.h"
#include "Board.h"
   
#include "eddystoneURLCfg.h"
#include "simpleEddystoneBeacon.h"

#include "UTC_clock.h"

#include <ti/drivers/lcd/LCDDogm1286.h>
// DriverLib
#include <driverlib/aon_batmon.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

#ifndef FEATURE_OAD
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800
#else
// Minimum connection interval (units of 1.25ms, 8=10ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     8

// Maximum connection interval (units of 1.25ms, 8=10ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     8
#endif // FEATURE_OAD

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter
// update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is
// formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

#ifdef FEATURE_OAD
// The size of an OAD packet.
#define OAD_PACKET_SIZE                       ((OAD_BLOCK_SIZE) + 2)
#endif // FEATURE_OAD

// Task configuration
#define SEB_TASK_PRIORITY                     1

#ifndef SEB_TASK_STACK_SIZE
#define SEB_TASK_STACK_SIZE                   644
#endif
  
// Internal Events for RTOS application
#define SEB_STATE_CHANGE_EVT                  0x0001
#define SEB_KEY_CHANGE_EVT                    0x0002
#define SEB_CONN_EVT_END_EVT                  0x0008
#define SEB_CHAR_CHANGE_EVT                   0x0010
#define SEB_ADV_COMPLETE_EVT                  0x0020

// Eddystone definitions
#define EDDYSTONE_SERVICE_UUID                  0xFEAA
   
#define EDDYSTONE_FRAME_TYPE_UID                0x00
#define EDDYSTONE_FRAME_TYPE_URL                0x10
#define EDDYSTONE_FRAME_TYPE_TLM                0x20
  
#define EDDYSTONE_FRAME_OVERHEAD_LEN            8
#define EDDYSTONE_SVC_DATA_OVERHEAD_LEN         3
#define EDDYSTONE_MAX_URL_LEN                   18

// # of URL Scheme Prefix types
#define EDDYSTONE_URL_PREFIX_MAX        4
// # of encodable URL words
#define EDDYSTONE_URL_ENCODING_MAX      14
  
/*********************************************************************
 * TYPEDEFS
 */

// App event passed from profiles.
typedef struct
{
  appEvtHdr_t hdr; // Event header.
} sebEvt_t;

// Eddystone UID frame
typedef struct
{
  uint8_t   frameType;      // UID
  int8_t    rangingData;
  uint8_t   namespaceID[10];
  uint8_t   instanceID[6];
  uint8_t   reserved[2];
} eddystoneUID_t;

// Eddystone URL frame
typedef struct
{
  uint8_t   frameType;      // URL | Flags
  int8_t    txPower;
  uint8_t   encodedURL[EDDYSTONE_MAX_URL_LEN];  // the 1st byte is prefix
} eddystoneURL_t;

// Eddystone TLM frame
typedef struct
{
  uint8_t   frameType;      // TLM
  uint8_t   version;        // 0x00 for now
  uint8_t   vBatt[2];       // Battery Voltage, 1mV/bit, Big Endian
  uint8_t   temp[2];        // Temperature. Signed 8.8 fixed point
  uint8_t   advCnt[4];      // Adv count since power-up/reboot
  uint8_t   secCnt[4];      // Time since power-up/reboot
                          // in 0.1 second resolution
} eddystoneTLM_t;

typedef union
{
  eddystoneUID_t        uid;
  eddystoneURL_t        url;
  eddystoneTLM_t        tlm;
} eddystoneFrame_t;

typedef struct
{
  uint8_t               length1;        // 2
  uint8_t               dataType1;      // for Flags data type (0x01)
  uint8_t               data1;          // for Flags data (0x04)
  uint8_t               length2;        // 3
  uint8_t               dataType2;      // for 16-bit Svc UUID list data type (0x03)
  uint8_t               data2;          // for Eddystone UUID LSB (0xAA)
  uint8_t               data3;          // for Eddystone UUID MSB (0xFE)
  uint8_t               length;         // Eddystone service data length
  uint8_t               dataType3;      // for Svc Data data type (0x16)
  uint8_t               data4;          // for Eddystone UUID LSB (0xAA)
  uint8_t               data5;          // for Eddystone UUID MSB (0xFE)
  eddystoneFrame_t      frame;
} eddystoneAdvData_t;

typedef struct
{
  uint8_t               length1;        // 2
  uint8_t               dataType1;      // for Flags data type (0x01)
  uint8_t               data1;          // for Flags data (0x06)
  uint8_t               length2;        // 17
  uint8_t               dataType2;      // for 128-bit Svc UUID list data type (0x07)
  uint8_t               data2[16];      // for Eddystone Cfg service UUID
  uint8_t               length3;        // 2
  uint8_t               dataType3;      // for Power Level data type (0x0a)
  int8_t                powerLevel;     // for Eddystone UUID LSB (0xAA)
} eddystoneCfgAdvData_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Semaphore globally used to post events to the application thread
static ICall_Semaphore sem;

// Queue object used for app messages
static Queue_Struct appMsg;
static Queue_Handle appMsgQueue;

#if defined(FEATURE_OAD)
// Event data from OAD profile.
static Queue_Struct oadQ;
static Queue_Handle hOadQ;
#endif //FEATURE_OAD

// Task configuration
Task_Struct sebTask;
Char sebTaskStack[SEB_TASK_STACK_SIZE];

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static eddystoneAdvData_t eddystoneAdv = 
{ 
  // Flags; this sets the device to use general discoverable mode
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  
  // Complete list of 16-bit Service UUIDs
  0x03,   // length of this data including the data type byte
  GAP_ADTYPE_16BIT_COMPLETE,
  LO_UINT16(EDDYSTONE_SERVICE_UUID),
  HI_UINT16(EDDYSTONE_SERVICE_UUID),
  
  // Service Data
  0x03, // to be set properly later
  GAP_ADTYPE_SERVICE_DATA,
  LO_UINT16(EDDYSTONE_SERVICE_UUID),
  HI_UINT16(EDDYSTONE_SERVICE_UUID)
};

eddystoneUID_t   eddystoneUID;
eddystoneURL_t   eddystoneURL;
eddystoneTLM_t   eddystoneTLM;

static eddystoneCfgAdvData_t eddystoneCfgAdv = 
{ 
  // Flags; this sets the device to use general discoverable mode
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  
  // Complete list of 128-bit Service UUIDs
  0x11,   // length of this data
  GAP_ADTYPE_128BIT_COMPLETE,
  {EDDYSTONE_BASE_UUID_128(URLCFGSVC_SVC_UUID)},
  
  // Power Level
  0x02, // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  -2  // To be set properly later
};

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x16,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'S',
  'i',
  'm',
  'p',
  'l',
  'e',
  'E',
  'd',
  'd',
  'y',
  's',
  't',
  'o',
  'n',
  'e',
  'B',
  'e',
  'a',
  'c',
  'o',
  'n',

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),   // 100ms
  HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
  LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),   // 1s
  HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
};

// Array of URL Scheme Prefices
static char* eddystoneURLPrefix[EDDYSTONE_URL_PREFIX_MAX] =
{
  "http://www.",
  "https://www.",
  "http://",
  "https://"
};

// Array of URLs to be encoded
static char* eddystoneURLEncoding[EDDYSTONE_URL_ENCODING_MAX] =
{
  ".com/",
  ".org/",
  ".edu/",
  ".net/",
  ".info/",
  ".biz/",
  ".gov/",
  ".com/",
  ".org/",
  ".edu/",
  ".net/",
  ".info/",
  ".biz/",
  ".gov/"
};

static uint32 advCount = 0;

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple ES Beacon";

// Globals used for ATT Response retransmission
static gattMsgEvent_t *pAttRsp = NULL;
static uint8_t rspTxRetry = 0;

// Eddystone frame type currently used
static uint8 currentFrameType = EDDYSTONE_FRAME_TYPE_UID;

// URL Configuration mode
static uint8 URLCfgMode = FALSE;

// Connection status
static uint8 ConnectedInCfgMode = FALSE;

#ifdef SENSORTAG_HW
// Pins that are used on SensorTag
static PIN_Config SensortagAppPinTable[] =
{
    Board_LED1       | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,     /* LED initially off             */
    Board_LED2       | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,     /* LED initially off             */
    Board_KEY_LEFT   | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_BOTHEDGES | PIN_HYSTERESIS,        /* Button is active low          */
    Board_KEY_RIGHT  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_BOTHEDGES | PIN_HYSTERESIS,        /* Button is active low          */

    PIN_TERMINATE
};

// Global pin resources
PIN_State pinGpioState;
PIN_Handle hGpioPin;

// Value of keys pressed
static uint8_t keysPressed_st;

// Key debounce clock
static Clock_Struct keyChangeClock_st;

// Pointer to application callback
keysPressedCB_t appKeyChangeHandler_st = NULL;
#endif  // SENSORTAG_HW

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SimpleEddystoneBeacon_init(void);
static void SimpleEddystoneBeacon_taskFxn(UArg a0, UArg a1);

static uint8_t SimpleEddystoneBeacon_processStackMsg(ICall_Hdr *pMsg);
static uint8_t SimpleEddystoneBeacon_processGATTMsg(gattMsgEvent_t *pMsg);
static void SimpleEddystoneBeacon_processAppMsg(sebEvt_t *pMsg);
static void SimpleEddystoneBeacon_processStateChangeEvt(gaprole_States_t newState);
static void SimpleEddystoneBeacon_processCharValueChangeEvt(uint8_t paramID);
static void SimpleEddystoneBeacon_processAdvCompleteEvt(void);

static void SimpleEddystoneBeacon_sendAttRsp(void);
static void SimpleEddystoneBeacon_freeAttRsp(uint8_t status);

static void SimpleEddystoneBeacon_stateChangeCB(gaprole_States_t newState);
static void SimpleEddystoneBeacon_charValueChangeCB(uint8_t paramID);

static void SimpleEddystoneBeacon_updateTLM(void);
static void SimpleEddystoneBeacon_initUID(void);
static void SimpleEddystoneBeacon_initConfiguration(void);
static void SimpleEddystoneBeacon_applyConfiguration(void);
static void SimpleEddystoneBeacon_selectFrame(uint8 frameType);
static void SimpleEddystoneBeacon_startRegularAdv(void);

static void SimpleEddystoneBeacon_keyChangeHandler(uint8 keys);

#ifdef SENSORTAG_HW
static void SensorTag_keyChangeHandler(UArg a0);
static void SensorTag_callback(PIN_Handle handle, PIN_Id pinId);
#endif  // SENSORTAG_HW
static void SimpleEddystoneBeacon_enqueueMsg(uint8_t event, uint8_t state);

#ifdef FEATURE_OAD
void SimpleEddystoneBeacon_processOadWriteCB(uint8_t event, uint16_t connHandle,
                                           uint8_t *pData);
#endif //FEATURE_OAD

void SimpleEddystoneBeacon_initLCD(void);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t SimpleEddystoneBeacon_gapRoleCBs =
{
  SimpleEddystoneBeacon_stateChangeCB     // Profile State Change Callbacks
};

// GAP Bond Manager Callbacks
static gapBondCBs_t SimpleEddystoneBeacon_BondMgrCBs =
{
  NULL, // Passcode callback (not used by application)
  NULL  // Pairing / Bonding state Callback (not used by application)
};

// Eddystone URL Configuration Service Callbacks
static urlCfgSvcCBs_t SimpleEddystoneBeacon_urlCfgCBs =
{
  SimpleEddystoneBeacon_charValueChangeCB // Characteristic value change callback
};

#ifdef FEATURE_OAD
static oadTargetCBs_t SimpleEddystoneBeacon_oadCBs =
{
  SimpleEddystoneBeacon_processOadWriteCB // Write Callback.
};
#endif //FEATURE_OAD

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_createTask
 *
 * @brief   Task creation function for the Simple Eddystone Beacon.
 *
 * @param   None.
 *
 * @return  None.
 */
void SimpleEddystoneBeacon_createTask(void)
{
  Task_Params taskParams;
    
  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = sebTaskStack;
  taskParams.stackSize = SEB_TASK_STACK_SIZE;
  taskParams.priority = SEB_TASK_PRIORITY;
  
  Task_construct(&sebTask, SimpleEddystoneBeacon_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_init
 *
 * @brief   Initialization function for the Simple Eddystone Beacon App
 *          Task. This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notification ...).
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_init(void)
{
	// ******************************************************************
  // N0 STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
  // ******************************************************************
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &sem);
    
  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

#ifndef SENSORTAG_HW
  Board_initKeys(SimpleEddystoneBeacon_keyChangeHandler);
  Board_openLCD();
#endif // !SENSORTAG_HW
  
#if SENSORTAG_HW
  // Setup SPI bus for serial flash and Devpack interface
  bspSpiOpen();

  // Handling of buttons and LEDs
  hGpioPin = PIN_open(&pinGpioState, SensortagAppPinTable);
  PIN_registerIntCb(hGpioPin, SensorTag_callback);

  // Setup keycallback for keys
  Util_constructClock(&keyChangeClock_st, SensorTag_keyChangeHandler,
                      KEY_DEBOUNCE_TIMEOUT, 0, false, 0);

  // Set the application callback
  appKeyChangeHandler_st = SimpleEddystoneBeacon_keyChangeHandler;
#endif //SENSORTAG_HW
  
  // Setup the GAP
  GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);

  // Setup the GAP Peripheral Role Profile
  {
    // Don't start advertising upon initialization
    uint8_t initialAdvertEnable = FALSE;
    uint8_t initialNonConnAdvEnable = FALSE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16_t advertOffTime = 0;
      
    uint8_t enableUpdateRequest = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16_t desiredMinInterval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16_t desiredMaxInterval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16_t desiredSlaveLatency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16_t desiredConnTimeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Initialize UID frame
    SimpleEddystoneBeacon_initUID();

    // Start Clock for TLM
    UTC_init();
    
    // Set the GAP Role Parameters
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), 
                         &initialAdvertEnable);
    GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8_t), 
                         &initialNonConnAdvEnable);

    GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME, sizeof(uint16_t), 
                         &advertOffTime);
    
    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof (scanRspData), 
                         scanRspData);
    
    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(uint8_t),
                         &enableUpdateRequest);
    GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMinInterval);
    GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMaxInterval);
    GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(uint16_t),
                         &desiredSlaveLatency);
    GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(uint16_t),
                         &desiredConnTimeout);
  }

  // Set the GAP Characteristics
  GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

  // Register an event for adv completion
  HCI_EXT_AdvEventNoticeCmd(selfEntity, SEB_ADV_COMPLETE_EVT);
  
  // Setup the GAP Bond Manager
  {
    uint32_t passkey = 0; // passkey "000000"
    uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8_t mitm = TRUE;
    uint8_t ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8_t bonding = TRUE;

    GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE, sizeof(uint32_t),
                            &passkey);
    GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
    GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION, sizeof(uint8_t), &mitm);
    GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
    GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED, sizeof(uint8_t), &bonding);
  }

   // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);           // GAP
  GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes
  DevInfo_AddService();                        // Device Information Service

  VOID URLCfgSvc_AddService();               // URL Configuration Service

#ifdef FEATURE_OAD
  VOID OAD_addService();                    // OAD Profile
  OAD_register((oadTargetCBs_t *)&SimpleEddystoneBeacon_oadCBs);
  hOadQ = Util_constructQueue(&oadQ);
#endif

#ifdef IMAGE_INVALIDATE
  Reset_addService();
#endif //IMAGE_INVALIDATE
  
  // Setup the URL Configuration Characteristic Values
  SimpleEddystoneBeacon_initConfiguration();

  // Register callback with SimpleGATTprofile
  URLCfgSvc_RegisterAppCBs(&SimpleEddystoneBeacon_urlCfgCBs);
  
  // Start the Device
  VOID GAPRole_StartDevice(&SimpleEddystoneBeacon_gapRoleCBs);
  
  // Start Bond Manager
  VOID GAPBondMgr_Register(&SimpleEddystoneBeacon_BondMgrCBs);

  // Register with GAP for HCI/Host messages
  GAP_RegisterForMsgs(selfEntity);
  
  // Register for GATT local events and ATT Responses pending for transmission
  GATT_RegisterForMsgs(selfEntity);
  
  LCD_WRITE_STRING("Eddystone Beacon", LCD_PAGE0);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_initUID
 *
 * @brief   initialize UID frame
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_initUID(void)
{
  // Set Eddystone UID frame with meaningless numbers for example.
  // This need to be replaced with some algorithm-based formula
  // for production.
  eddystoneUID.namespaceID[0] = 0x00;
  eddystoneUID.namespaceID[1] = 0x01;
  eddystoneUID.namespaceID[2] = 0x02;
  eddystoneUID.namespaceID[3] = 0x03;
  eddystoneUID.namespaceID[4] = 0x04;
  eddystoneUID.namespaceID[5] = 0x05;
  eddystoneUID.namespaceID[6] = 0x06;
  eddystoneUID.namespaceID[7] = 0x07;
  eddystoneUID.namespaceID[8] = 0x08;
  eddystoneUID.namespaceID[9] = 0x09;
  
  eddystoneUID.instanceID[0] = 0x04;
  eddystoneUID.instanceID[1] = 0x51;
  eddystoneUID.instanceID[2] = 0x40;
  eddystoneUID.instanceID[3] = 0x00;
  eddystoneUID.instanceID[4] = 0xB0;
  eddystoneUID.instanceID[5] = 0x00;
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_encodeURL
 *
 * @brief   Encodes URL in accordance with Eddystone URL frame spec
 *
 * @param   urlOrg - Plain-string URL to be encoded
 *          urlEnc - Encoded URL. Should be URLCFGSVC_CHAR_URI_DATA_LEN-long.
 *
 * @return  0 if the prefix is invalid
 *          The length of the encoded URL including prefix otherwise
 */
uint8 SimpleEddystoneBeacon_encodeURL(char* urlOrg, uint8* urlEnc)
{
  uint8 i, j;
  uint8 urlLen;
  uint8 tokenLen;
  
  urlLen = (uint8) strlen(urlOrg);
  
  // search for a matching prefix
  for (i = 0; i < EDDYSTONE_URL_PREFIX_MAX; i++)
  {
    tokenLen = strlen(eddystoneURLPrefix[i]);
    if (strncmp(eddystoneURLPrefix[i], urlOrg, tokenLen) == 0)
    {
      break;
    }
  }
  
  if (i == EDDYSTONE_URL_PREFIX_MAX)
  {
    return 0;       // wrong prefix
  }
        
  // use the matching prefix number
  urlEnc[0] = i;
  urlOrg += tokenLen;
  urlLen -= tokenLen;
  
  // search for a token to be encoded
  for (i = 0; i < urlLen; i++)
  {
    for (j = 0; j < EDDYSTONE_URL_ENCODING_MAX; j++)
    {
      tokenLen = strlen(eddystoneURLEncoding[j]);
      if (strncmp(eddystoneURLEncoding[j], urlOrg + i, tokenLen) == 0)
      {
        // matching part found
        break;
      }
    }
    
    if (j < EDDYSTONE_URL_ENCODING_MAX)
    {
      memcpy(&urlEnc[1], urlOrg, i);
      // use the encoded byte
      urlEnc[i + 1] = j;
      break;
    }
  }
  
  if (i < urlLen)
  {
    memcpy(&urlEnc[i + 2],
           urlOrg + i + tokenLen, urlLen - i - tokenLen);
    return urlLen - tokenLen + 2;
  }

  memcpy(&urlEnc[1], urlOrg, urlLen);
  return urlLen + 1;
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_updateTLM
 *
 * @brief   Update TLM elements
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_updateTLM(void)
{
  uint32 time100MiliSec;
  uint32 batt;
  
  // Battery voltage (bit 10:8 - integer, but 7:0 fraction)
  batt = AONBatMonBatteryVoltageGet();
  batt = (batt * 125) >> 5; // convert V to mV
  eddystoneTLM.vBatt[0] = HI_UINT16(batt);
  eddystoneTLM.vBatt[1] = LO_UINT16(batt);
  // Temperature - 19.5 (Celcius) for example
  eddystoneTLM.temp[0] = 19;
  eddystoneTLM.temp[1] = 256 / 2;
  // advertise packet cnt;
  eddystoneTLM.advCnt[0] = BREAK_UINT32(advCount, 3);
  eddystoneTLM.advCnt[1] = BREAK_UINT32(advCount, 2);
  eddystoneTLM.advCnt[2] = BREAK_UINT32(advCount, 1);
  eddystoneTLM.advCnt[3] = BREAK_UINT32(advCount, 0);
  // running time
  time100MiliSec = UTC_getClock() * 10; // 1-second resolution for now
  eddystoneTLM.secCnt[0] = BREAK_UINT32(time100MiliSec, 3);
  eddystoneTLM.secCnt[1] = BREAK_UINT32(time100MiliSec, 2);
  eddystoneTLM.secCnt[2] = BREAK_UINT32(time100MiliSec, 1);
  eddystoneTLM.secCnt[3] = BREAK_UINT32(time100MiliSec, 0);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_initConfiguration
 *
 * @brief   set all URL Configuration characteristics to default values
 *
 * @param   none
 *
 * @return  none
 */
void SimpleEddystoneBeacon_initConfiguration(void)
{
  uint8 tempURLEnc[URLCFGSVC_CHAR_URI_DATA_LEN];
  uint8 temp8;
  uint16 temp16;
  uint8 tempLock[16] = URLCFG_CHAR_LOCK_DEFAULT;
  
  // set URI Data
  temp8 = SimpleEddystoneBeacon_encodeURL(URLCFG_CHAR_URI_DATA_DEFAULT,
                                          tempURLEnc);
  URLCfgSvc_SetParameter(URLCFGSVC_URI_DATA, temp8, tempURLEnc);

  // set Flags
  temp8 = URLCFG_CHAR_FLAGS_DEFAULT;
  URLCfgSvc_SetParameter(URLCFGSVC_FLAGS, 1, &temp8);

  // set TX Power Mode
  temp8 = URLCFG_CHAR_TX_POWER_MODE_DEFAULT;
  URLCfgSvc_SetParameter(URLCFGSVC_TX_POWER_MODE, 1, &temp8);

  // set Beacon Period
  temp16 = URLCFG_CHAR_BEACON_PERIOD_DEFAULT;
  URLCfgSvc_SetParameter(URLCFGSVC_BEACON_PERIOD, 2, &temp16);

  // set Lock Code
  URLCfgSvc_SetParameter(URLCFGSVC_LOCK, 16, tempLock);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_applyConfiguration
 *
 * @brief   Apply the changes maded in URL Configuration mode
 *
 * @param   none
 *
 * @return  none
 */
void SimpleEddystoneBeacon_applyConfiguration(void)
{
  int8 tempPwrLvls[4];
  uint8 tempPowerMode;
  int8 tempPower;
  uint16 tempPeriod;
  
  // update URL frame
  URLCfgSvc_GetParameter(URLCFGSVC_URI_DATA, eddystoneURL.encodedURL);

  // update TX power
  URLCfgSvc_GetParameter(URLCFGSVC_ADV_TX_PWR_LVLS, tempPwrLvls);
  URLCfgSvc_GetParameter(URLCFGSVC_TX_POWER_MODE, &tempPowerMode);
  tempPower = tempPwrLvls[tempPowerMode];
  HCI_EXT_SetTxPowerCmd(tempPower);
  eddystoneUID.rangingData = tempPower;
  eddystoneURL.txPower = tempPower;

  // update adv period
  URLCfgSvc_GetParameter(URLCFGSVC_BEACON_PERIOD, &tempPeriod);
  if (tempPeriod != 0)
  {
    // convert into multiple of 0.625us
    tempPeriod = (uint16) (tempPeriod * 8L / 5);
    
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, tempPeriod);
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, tempPeriod);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, tempPeriod);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, tempPeriod);
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_selectFrame
 *
 * @brief   Selecting the type of frame to be put in the service data
 *
 * @param   frameType - Eddystone frame type
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_selectFrame(uint8 frameType)
{
  if (frameType == EDDYSTONE_FRAME_TYPE_UID ||
      frameType == EDDYSTONE_FRAME_TYPE_URL ||
      frameType == EDDYSTONE_FRAME_TYPE_TLM)
  {
    eddystoneFrame_t*   pFrame;
    uint8               frameSize;
    uint8               temp;

    eddystoneAdv.length = EDDYSTONE_SVC_DATA_OVERHEAD_LEN;
    // Fill with 0s first
    memset((uint8*) &eddystoneAdv.frame, 0x00, sizeof(eddystoneFrame_t));
    
    switch (frameType)
    {
    case EDDYSTONE_FRAME_TYPE_UID:
      eddystoneUID.frameType = EDDYSTONE_FRAME_TYPE_UID;
      frameSize = sizeof(eddystoneUID_t);
      pFrame = (eddystoneFrame_t *) &eddystoneUID;
      break;

    case EDDYSTONE_FRAME_TYPE_URL:
      eddystoneURL.frameType = EDDYSTONE_FRAME_TYPE_URL;
      URLCfgSvc_GetParameter(URLCFGSVC_URI_DATA_LEN, &temp);
      frameSize = sizeof(eddystoneURL_t) - EDDYSTONE_MAX_URL_LEN + temp;
      pFrame = (eddystoneFrame_t *) &eddystoneURL;
      break;

    case EDDYSTONE_FRAME_TYPE_TLM:
      eddystoneTLM.frameType = EDDYSTONE_FRAME_TYPE_TLM;
      frameSize = sizeof(eddystoneTLM_t);
      SimpleEddystoneBeacon_updateTLM();
      pFrame = (eddystoneFrame_t *) &eddystoneTLM;
      break;
    }
    
    memcpy((uint8 *) &eddystoneAdv.frame, (uint8 *) pFrame, frameSize);
    eddystoneAdv.length += frameSize;
    
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA,
                         EDDYSTONE_FRAME_OVERHEAD_LEN + eddystoneAdv.length,
                         &eddystoneAdv);
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_startRegularAdv
 *
 * @brief   Start regular advertise.
 *          If configuration mode was on going, stop it.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_startRegularAdv(void)
{
  uint8 advertEnabled = FALSE;
  uint8 advType = GAP_ADTYPE_ADV_NONCONN_IND;
  uint8 tempPeriod;
  
  SimpleEddystoneBeacon_applyConfiguration();

  // Stop connectable advertising
  GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                       &advertEnabled);

  // if BeaconPeriod is 0, don't advertise.
  URLCfgSvc_GetParameter(URLCFGSVC_BEACON_PERIOD, &tempPeriod);
  if (tempPeriod != 0)
  {
    advertEnabled = TRUE;
  }
  
  GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &advType);
  
  // Select UID or URL frame as adv data initially
  SimpleEddystoneBeacon_selectFrame(currentFrameType);

  GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8_t),
                       &advertEnabled);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_startConfigAdv
 *
 * @brief   Start advertising in configuration mode
 *          If regular advertising was on going, stop it.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_startConfigAdv(void)
{
  uint8 advertEnabled;
  uint8 advType;
  uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;
  uint8 pwrLvls[4];
  
  advertEnabled = FALSE;
  GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8_t), 
                       &advertEnabled);

  advType = GAP_ADTYPE_ADV_IND;
  GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &advType);

  GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
  GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
  GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
  GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);

  // update TX power
  URLCfgSvc_GetParameter(URLCFGSVC_ADV_TX_PWR_LVLS, pwrLvls);
  HCI_EXT_SetTxPowerCmd(pwrLvls[TX_POWER_MODE_MEDIUM]);

  GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(eddystoneCfgAdv),
                       &eddystoneCfgAdv);

  advertEnabled = TRUE;
  GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), 
                       &advertEnabled);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processEvent
 *
 * @brief   Application task entry point for the Simple Eddystone Beacon.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_taskFxn(UArg a0, UArg a1)
{
  // Initialize application
  SimpleEddystoneBeacon_init();
  
  // Application main loop
  for (;;)
  {
    // Waits for a signal to the semaphore associated with the calling thread.
    // Note that the semaphore associated with a thread is signaled when a
    // message is queued to the message receive queue of the thread or when
    // ICall_signal() function is called onto the semaphore.
    ICall_Errno errno = ICall_wait(ICALL_TIMEOUT_FOREVER);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;
      
      if (ICall_fetchServiceMsg(&src, &dest, 
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {
        uint8 safeToDealloc = TRUE;
        
        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          ICall_Event *pEvt = (ICall_Event *)pMsg;
          
          // Check for BLE stack events first
          if (pEvt->signature == 0xffff)
          {
            if (pEvt->event_flag & SEB_ADV_COMPLETE_EVT)
            {
              SimpleEddystoneBeacon_processAdvCompleteEvt();
            }

            if (pEvt->event_flag & SEB_CONN_EVT_END_EVT)
            {
              // Try to retransmit pending ATT Response (if any)
              SimpleEddystoneBeacon_sendAttRsp();
            }
          }
          else
          {
            // Process inter-task message
            safeToDealloc = SimpleEddystoneBeacon_processStackMsg((ICall_Hdr *)pMsg);
          }
        }

        if (pMsg && safeToDealloc)
        {
          ICall_freeMsg(pMsg);
        }
      }

      // If RTOS queue is not empty, process app message.
      while (!Queue_empty(appMsgQueue))
      {
        sebEvt_t *pMsg = (sebEvt_t *)Util_dequeueMsg(appMsgQueue);
        if (pMsg)
        {
          // Process message.
          SimpleEddystoneBeacon_processAppMsg(pMsg);
          
          // Free the space from the message.
          ICall_free(pMsg);
        }
      }
    }

#ifdef FEATURE_OAD
    while (!Queue_empty(hOadQ))
    {
      oadTargetWrite_t *oadWriteEvt = Queue_dequeue(hOadQ);

      // Identify new image.
      if (oadWriteEvt->event == OAD_WRITE_IDENTIFY_REQ)
      {
        OAD_imgIdentifyWrite(oadWriteEvt->connHandle, oadWriteEvt->pData);
      }
      // Write a next block request.
      else if (oadWriteEvt->event == OAD_WRITE_BLOCK_REQ)
      {
        OAD_imgBlockWrite(oadWriteEvt->connHandle, oadWriteEvt->pData);
      }

      // Free buffer.
      ICall_free(oadWriteEvt);
    }
#endif //FEATURE_OAD
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_handleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_handleKeys(uint8_t shift, uint8_t keys)
{
  (void)shift;  // Intentionally unreferenced parameter

  if (keys & KEY_UP)
  {
    return;
  }

  if (keys & KEY_LEFT)
  {
    uint8 advertEnabled = FALSE;
    
    if (URLCfgMode)
    {
      if (ConnectedInCfgMode)
      {
        GAPRole_TerminateConnection();
      }
      else
      {
        SimpleEddystoneBeacon_startRegularAdv();
        URLCfgMode = FALSE;
      }
    }
    else if (currentFrameType == EDDYSTONE_FRAME_TYPE_URL)
    {
      // If in regular mode, stop non-connectable advertising.
      GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8_t), 
                           &advertEnabled);
      SimpleEddystoneBeacon_startConfigAdv();
      URLCfgMode = TRUE;
    }

    return;
  }

  if (keys & KEY_DOWN)
  {
    return;
  }

  if (keys & KEY_RIGHT)
  {
    if (URLCfgMode == FALSE)
    {
      // Toggle frame type
      if (currentFrameType == EDDYSTONE_FRAME_TYPE_UID)
      {
        currentFrameType = EDDYSTONE_FRAME_TYPE_URL;
        LCD_WRITE_STRING("Advertising URL", LCD_PAGE2);
      }
      else
      {
        currentFrameType = EDDYSTONE_FRAME_TYPE_UID;
        LCD_WRITE_STRING("Advertising UID", LCD_PAGE2);
      }
    }

    return;
  }

  if (keys & KEY_SELECT)
  {
    return;
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processStackMsg
 *
 * @brief   Process an incoming stack message.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleEddystoneBeacon_processStackMsg(ICall_Hdr *pMsg)
{
  uint8_t safeToDealloc = TRUE;
    
  switch (pMsg->event)
  {
    case GATT_MSG_EVENT:
      // Process GATT message
      safeToDealloc = SimpleEddystoneBeacon_processGATTMsg((gattMsgEvent_t *)pMsg);
      break;

    case HCI_GAP_EVENT_EVENT:
      {
        // Process HCI message
        switch(pMsg->status)
        {
          case HCI_COMMAND_COMPLETE_EVENT_CODE:
            // Process HCI Command Complete Event
//            SimpleEddystoneBeacon_processCmdCompleteEvt((hciEvt_CmdComplete_t *)pMsg);
            break;
            
          default:
            break;
        }
      }
      break;
      
    default:
      // do nothing
      break;
  }
  
  return (safeToDealloc);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processGATTMsg
 *
 * @brief   Process GATT messages and events.
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleEddystoneBeacon_processGATTMsg(gattMsgEvent_t *pMsg)
{
  // See if GATT server was unable to transmit an ATT response
  if (pMsg->hdr.status == blePending)
  {
    // No HCI buffer was available. Let's try to retransmit the response
    // on the next connection event.
    if (HCI_EXT_ConnEventNoticeCmd(pMsg->connHandle, selfEntity,
                                   SEB_CONN_EVT_END_EVT) == SUCCESS)
    {
      // First free any pending response
      SimpleEddystoneBeacon_freeAttRsp(FAILURE);
      
      // Hold on to the response message for retransmission
      pAttRsp = pMsg;
      
      // Don't free the response message yet
      return (FALSE);
    }
  }
  else if (pMsg->method == ATT_FLOW_CTRL_VIOLATED_EVENT)
  {
    // ATT request-response or indication-confirmation flow control is
    // violated. All subsequent ATT requests or indications will be dropped.
    // The app is informed in case it wants to drop the connection.
    
    // Display the opcode of the message that caused the violation.
    LCD_WRITE_STRING_VALUE("FC Violated:", pMsg->msg.flowCtrlEvt.opcode,
                           10, LCD_PAGE5);
  }    
  else if (pMsg->method == ATT_MTU_UPDATED_EVENT)
  {
    // MTU size updated
    LCD_WRITE_STRING_VALUE("MTU Size:", pMsg->msg.mtuEvt.MTU, 10, LCD_PAGE5);
  }
  
  // Free message payload. Needed only for ATT Protocol messages
  GATT_bm_free(&pMsg->msg, pMsg->method);
  
  // It's safe to free the incoming message
  return (TRUE);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_sendAttRsp
 *
 * @brief   Send a pending ATT response message.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_sendAttRsp(void)
{
  // See if there's a pending ATT Response to be transmitted
  if (pAttRsp != NULL)
  {
    uint8_t status;
    
    // Increment retransmission count
    rspTxRetry++;
    
    // Try to retransmit ATT response till either we're successful or
    // the ATT Client times out (after 30s) and drops the connection.
    status = GATT_SendRsp(pAttRsp->connHandle, pAttRsp->method, &(pAttRsp->msg));
    if ((status != blePending) && (status != MSG_BUFFER_NOT_AVAIL))
    {
      // Disable connection event end notice
      HCI_EXT_ConnEventNoticeCmd(pAttRsp->connHandle, selfEntity, 0);
      
      // We're done with the response message
      SimpleEddystoneBeacon_freeAttRsp(status);
    }
    else
    {
      // Continue retrying
      LCD_WRITE_STRING_VALUE("Rsp send retry:", rspTxRetry, 10, LCD_PAGE5);
    }
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_freeAttRsp
 *
 * @brief   Free ATT response message.
 *
 * @param   status - response transmit status
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_freeAttRsp(uint8_t status)
{
  // See if there's a pending ATT response message
  if (pAttRsp != NULL)
  {
    // See if the response was sent out successfully
    if (status == SUCCESS)
    {
      LCD_WRITE_STRING_VALUE("Rsp sent, retry:", rspTxRetry, 10, LCD_PAGE5);
    }
    else
    {
      // Free response payload
      GATT_bm_free(&pAttRsp->msg, pAttRsp->method);
      
      LCD_WRITE_STRING_VALUE("Rsp retry failed:", rspTxRetry, 10, LCD_PAGE5);
    }
    
    // Free response message
    ICall_freeMsg(pAttRsp);
    
    // Reset our globals
    pAttRsp = NULL;
    rspTxRetry = 0;
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processAdvCompleteEvt
 *
 * @brief   Notification of a compleletion of advertise packet transmission.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleEddystoneBeacon_processAdvCompleteEvt(void)
{
  advCount++;
    
  if (URLCfgMode != TRUE)
  {
    if ((advCount % 10) == 0)
    {
      // Send TLM frame every 100 advertise packets
      SimpleEddystoneBeacon_selectFrame(EDDYSTONE_FRAME_TYPE_TLM);
    }
    else
    {
      // Send UID or URL
      SimpleEddystoneBeacon_selectFrame(currentFrameType);
    }
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processAppMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void SimpleEddystoneBeacon_processAppMsg(sebEvt_t *pMsg)
{
  switch (pMsg->hdr.event)
  {
    case SEB_STATE_CHANGE_EVT:
      SimpleEddystoneBeacon_processStateChangeEvt((gaprole_States_t)pMsg->
                                                hdr.state);
      break;

    case SEB_KEY_CHANGE_EVT:
      SimpleEddystoneBeacon_handleKeys(0, pMsg->hdr.state); 
      break;
      
    case SEB_CHAR_CHANGE_EVT:
      SimpleEddystoneBeacon_processCharValueChangeEvt(pMsg->hdr.state);
      break;

    default:
      // Do nothing.
      break;
  }
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_stateChangeCB
 *
 * @brief   Callback from GAP Role indicating a role state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void SimpleEddystoneBeacon_stateChangeCB(gaprole_States_t newState)
{
  SimpleEddystoneBeacon_enqueueMsg(SEB_STATE_CHANGE_EVT, newState);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processStateChangeEvt
 *
 * @brief   Process a pending GAP Role state change event.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void SimpleEddystoneBeacon_processStateChangeEvt(gaprole_States_t newState)
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8_t ownAddress[B_ADDR_LEN];
        uint8_t systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        // Display device address
        LCD_WRITE_STRING(Util_convertBdAddr2Str(ownAddress), LCD_PAGE1);
        LCD_WRITE_STRING("Initialized", LCD_PAGE2);

        // Start advertising
        SimpleEddystoneBeacon_startRegularAdv();
      }
      break;

    case GAPROLE_ADVERTISING:
      LCD_WRITE_STRING("Config Mode", LCD_PAGE2);
      break;

    case GAPROLE_ADVERTISING_NONCONN:
      SimpleEddystoneBeacon_freeAttRsp(bleNotConnected);

      if (currentFrameType == EDDYSTONE_FRAME_TYPE_UID)
      {
        LCD_WRITE_STRING("Advertising UID", LCD_PAGE2);
      }
      else
      {
        LCD_WRITE_STRING("Advertising URL", LCD_PAGE2);
      }
      LCD_WRITE_STRING("", LCD_PAGE3);
      break;

    case GAPROLE_CONNECTED:
      {
        uint8_t peerAddress[B_ADDR_LEN];

        GAPRole_GetParameter(GAPROLE_CONN_BD_ADDR, peerAddress);

        ConnectedInCfgMode = TRUE;
        
        LCD_WRITE_STRING("Connected", LCD_PAGE2);
        LCD_WRITE_STRING(Util_convertBdAddr2Str(peerAddress), LCD_PAGE3);
      }
      break;

    case GAPROLE_WAITING:
    case GAPROLE_WAITING_AFTER_TIMEOUT:
      SimpleEddystoneBeacon_freeAttRsp(bleNotConnected);
      if (ConnectedInCfgMode == TRUE)
      {
        ConnectedInCfgMode = FALSE;
        URLCfgMode = FALSE;
        SimpleEddystoneBeacon_startRegularAdv();
      }
      break;

    case GAPROLE_ERROR:
      LCD_WRITE_STRING("Error", LCD_PAGE2);
      break;

    default:
      LCD_WRITE_STRING("", LCD_PAGE2);
      break;
  }

  // Update the state
  //gapProfileState = newState;
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_charValueChangeCB
 *
 * @brief   Callback from Simple Profile indicating a characteristic
 *          value change.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimpleEddystoneBeacon_charValueChangeCB(uint8_t paramID)
{
  SimpleEddystoneBeacon_enqueueMsg(SEB_CHAR_CHANGE_EVT, paramID);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processCharValueChangeEvt
 *
 * @brief   Process a pending Simple Profile characteristic value change
 *          event.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimpleEddystoneBeacon_processCharValueChangeEvt(uint8_t paramID)
{
  switch(paramID)
  {
    case URLCFGSVC_RESET:
      SimpleEddystoneBeacon_initConfiguration();
      break;

    default:
      // should not reach here!
      break;
  }
}

#if defined(FEATURE_OAD)
/*********************************************************************
 * @fn      SimpleEddystoneBeacon_processOadWriteCB
 *
 * @brief   Process a write request to the OAD profile.
 *
 * @param   event      - event type:
 *                       OAD_WRITE_IDENTIFY_REQ
 *                       OAD_WRITE_BLOCK_REQ
 * @param   connHandle - the connection Handle this request is from.
 * @param   pData      - pointer to data for processing and/or storing.
 *
 * @return  None.
 */
void SimpleEddystoneBeacon_processOadWriteCB(uint8_t event, uint16_t connHandle,
                                           uint8_t *pData)
{
  oadTargetWrite_t *oadWriteEvt = ICall_malloc( sizeof(oadTargetWrite_t) + \
                                             sizeof(uint8_t) * OAD_PACKET_SIZE);
  
  if ( oadWriteEvt != NULL )
  {
    oadWriteEvt->event = event;
    oadWriteEvt->connHandle = connHandle;
    
    oadWriteEvt->pData = (uint8_t *)(&oadWriteEvt->pData + 1);
    memcpy(oadWriteEvt->pData, pData, OAD_PACKET_SIZE);

    Queue_enqueue(hOadQ, (Queue_Elem *)oadWriteEvt);
    
    // Post the application's semaphore.
    Semaphore_post(sem);
  }
  else
  {
    // Fail silently.
  }
}
#endif //FEATURE_OAD

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_keyChangeHandler
 *
 * @brief   Key event handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
void SimpleEddystoneBeacon_keyChangeHandler(uint8 keys)
{
  SimpleEddystoneBeacon_enqueueMsg(SEB_KEY_CHANGE_EVT, keys);
}

/*********************************************************************
 * @fn      SimpleEddystoneBeacon_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   state - message state.
 *
 * @return  None.
 */
static void SimpleEddystoneBeacon_enqueueMsg(uint8_t event, uint8_t state)
{
  sebEvt_t *pMsg;

  // Create dynamic pointer to message.
  if ((pMsg = ICall_malloc(sizeof(sebEvt_t))))
  {
    pMsg->hdr.event = event;
    pMsg->hdr.state = state;

    // Enqueue the message.
    Util_enqueueMsg(appMsgQueue, sem, (uint8*)pMsg);
  }
}

#ifdef SENSORTAG_HW
/*********************************************************************
 * @fn      SensorTag_keyChangeHandler
 *
 * @brief   Handler for key change
 *
 * @param   UArg a0 - ignored
 *
 * @return  none
 */
static void SensorTag_keyChangeHandler(UArg a0)
{
  if (appKeyChangeHandler_st != NULL)
  {
    // Notify the application
    (*appKeyChangeHandler_st)(keysPressed_st);
  }
}

/*!*****************************************************************************
 *  @fn         SensorTag_callback
 *
 *  Interrupt service routine for buttons
 *
 *  @param      handle PIN_Handle connected to the callback
 *
 *  @param      pinId  PIN_Id of the DIO triggering the callback
 *
 *  @return     none
 ******************************************************************************/
static void SensorTag_callback(PIN_Handle handle, PIN_Id pinId)
{
  keysPressed_st = 0;

  if ( PIN_getInputValue(Board_KEY_LEFT) == 0 )
  {
    keysPressed_st |= KEY_LEFT;
  }

  if ( PIN_getInputValue(Board_KEY_RIGHT) == 0 )
  {
    keysPressed_st |= KEY_RIGHT;
  }

  Util_startClock(&keyChangeClock_st);
}
#endif  // SENSORTAG_HW

/*********************************************************************
*********************************************************************/
