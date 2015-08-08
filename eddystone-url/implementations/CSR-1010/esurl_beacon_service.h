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
 
 * Copyright Google 2014
 *
 *  FILE
 *      esurl_beacon_service.h
 *
 *  DESCRIPTION
 *      Header definitions for the Esurl Beacon Service
 *
 
 *
 *****************************************************************************/

#ifndef __ESURL_BEACON_SERVICE_H__
#define __ESURL_BEACON_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */
#include <gatt.h>           /* GATT application interface */


/* Maximum payload data that can be fit in a single packet exchange
 * It is currently set to ATT_MTU (23 octets) - 3.
 */
#define MAX_CHARACTERISTIC_LENGTH                        (23 - 3)

/* Data lenght of params in an ADV packet */
#define DATA_LENGTH_SIZE                                  (1)   

/*============================================================================*
 *  ESURL BEACON Specific Header Files
 *============================================================================*/

/* Offsets in ADV ESURL BEACON HDR Packet */
#define SERVICE_DATA_LENGTH_OFFSET  (4)   
#define ESURL_BEACON_FLAGS_PKT_OFFSET (8)
#define ESURL_BEACON_TX_POWER_PKT_OFFSET (9)
#define ESURL_BEACON_URI_PKT_OFFSET (10)

/* Size of fields in the ADV packet */

/* length of service data hdr before URI */
#define BEACON_DATA_HDR_SIZE (10)
#define ESURL_BEACON_DATA_MAX (18)
#define SERVICE_DATA_PRE_URI_SIZE (5) 
#define ESURL_BEACON_FLAGS_SIZE (1) 

/* Size of array definitions for uribeacon data structure */
#define ESURL_BEACON_LOCK_CODE_SIZE  (16)
#define ESURL_BEACON_ADV_TX_POWER_LEVELS_SIZE (4)
#define ESURL_BEACON_RADIO_TX_POWER_LEVELS_SIZE (4)
#define ESURL_BEACON_PERIOD_SIZE (2)
#define ESURL_BEACON_RESET_SIZE (1)

/* TX Power mode values */
#define TX_POWER_MODE_LOWEST   (0)
/* DEFAULT */
#define TX_POWER_MODE_LOW      (1)  
#define TX_POWER_MODE_MEDIUM   (2)
#define TX_POWER_MODE_HIGH     (3)

/* TX Power ADV values */
/* This is at 0 meters by spec (If 1m is required, subtract 41dBm from each) */
#define ADV_TX_POWER_FOR_NEG_18   (-22)
/* DEFAULT */
#define ADV_TX_POWER_FOR_NEG_10   (-14)
#define ADV_TX_POWER_FOR_NEG_2    (-6)
#define ADV_TX_POWER_FOR_POS_6    (2)

/* CSR CHIP TX Reg Table */
/* Actual Power (dBm) vs value in register */
#define RADIO_TX_POWER_NEG_18 (0)
#define RADIO_TX_POWER_NEG_14 (1)
#define RADIO_TX_POWER_NEG_10 (2) 
#define RADIO_TX_POWER_NEG_6  (3)
#define RADIO_TX_POWER_NEG_2  (4)
#define RADIO_TX_POWER_POS_2  (5)
#define RADIO_TX_POWER_POS_6  (6)
#define RADIO_TX_POWER_POS_8  (7)

/* Default values */
#define FLAGS_DEFAULT   (0x10)
#define ADV_TX_POWER_DEFAULT ADV_TX_POWER_FOR_NEG_10
#define TX_POWER_MODE_DEFAULT TX_POWER_MODE_LOW

/* Configuration Power */
#define RADIO_TX_POWER_CONFIG RADIO_TX_POWER_NEG_2
#define ADV_TX_POWER_CONFIG ADV_TX_POWER_FOR_NEG_2

/* Time in milliseconds */
#define BEACON_PERIOD_MIN (100)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the Beacon Service data structure.*/
extern void EsurlBeaconDataInit(void);

/* Initialise the Beacon Service data structure at chip reset */
extern void EsurlBeaconInitChipReset(void);

/* Handle read operations on Beacon Service attributes maintained by the
 * application
 */
extern void EsurlBeaconHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* Handle write operations on Beacon Service attributes maintained by the
 * application
 */
extern void EsurlBeaconHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Returns the current value of the beacon data */
extern void EsurlBeaconGetData(uint8** data, uint8* data_size);

/* Returns the current value of the beacon period */
extern uint32 EsurlBeaconGetPeriodMillis(void);

/* Read the Esurl Beacon Service specific data stored in NVM */
extern void EsurlBeaconReadDataFromNVM(uint16 *p_offset);

/* Read the Esurl Beacon Service specific data stored in NVM */
extern void OldEsurlBeaconReadDataFromNVM(uint16 *p_offset);

/* Write the Esurl Beacon Sevice specific data to NVM */ 
extern void EsurlBeaconWriteDataToNVM(uint16 *p_offset);

/* Check if the handle belongs to the Beacon Service */
extern bool EsurlBeaconCheckHandleRange(uint16 handle);

/* Notify bonding status to the Beacon Service */
extern void EsurlBeaconBondingNotify(void);

/* Set the both TX parameters in the RADIO and ADV pkt */
extern void EsurlBeaconUpdateTxPowerFromMode(uint8 tx_power_mode);

/* Get the Esurl Beacon Tx Power Mode */
extern uint8 EsurlBeaconGetTxPowerMode(void);

#endif /* __ESURL_BEACON_SERVICE_H__ */
