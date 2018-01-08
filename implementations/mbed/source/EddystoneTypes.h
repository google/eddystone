/* 
 * Copyright (c) 2006-2016 Google Inc, All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __EDDYSTONETYPES_H__
#define __EDDYSTONETYPES_H__

#include <stdint.h>
#include <stddef.h>
#include "Eddystone_config.h"

/**
 * Macro to expand a 16-bit Eddystone UUID to 128-bit UUID.
 */
#define UUID_ES_BEACON(FIRST, SECOND) {                          \
        0xa3, 0x0c8, FIRST, SECOND, 0x8e, 0xd3, 0x4b, 0xdf,      \
        0x8a, 0x39, 0xa0, 0x1b, 0xeb, 0xed, 0xe2, 0x95,          \
}

/**
 * Eddystone 16-bit UUID.
 */
const uint8_t EDDYSTONE_UUID[] = {0xAA, 0xFE};

/**
 * Size of Eddystone UID. Needed to construct all frames raw bytes.
 */
const uint16_t EDDYSTONE_UUID_SIZE = sizeof(EDDYSTONE_UUID);

/** BEGINING OF CHARACTERISTICS */

/**
 * 128-bit UUID for Eddystone-GATT Configuration Service.
 */
const uint8_t UUID_ES_BEACON_SERVICE[]         = UUID_ES_BEACON(0x75, 0x00);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Capabilities
 * characteristic.
 */
const uint8_t UUID_CAPABILITIES_CHAR[]          = UUID_ES_BEACON(0x75, 0x01);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Active Slot
 * characteristic.
 */
const uint8_t UUID_ACTIVE_SLOT_CHAR[]           = UUID_ES_BEACON(0x75, 0x02);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Advertising Interval
 * characteristic.
 */
const uint8_t UUID_ADV_INTERVAL_CHAR[]          = UUID_ES_BEACON(0x75, 0x03);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Radio Tx Power
 * characteristic.
 */
const uint8_t UUID_RADIO_TX_POWER_CHAR[]        = UUID_ES_BEACON(0x75, 0x04);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Adv Tx Power
 * characteristic.
 */
const uint8_t UUID_ADV_TX_POWER_CHAR[]          = UUID_ES_BEACON(0x75, 0x05);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Lock State
 * characteristic.
 */
const uint8_t UUID_LOCK_STATE_CHAR[]            = UUID_ES_BEACON(0x75, 0x06);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Unlock
 * characteristic.
 */
const uint8_t UUID_UNLOCK_CHAR[]                = UUID_ES_BEACON(0x75, 0x07);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Public ECDH Key
 * characteristic.
 */
const uint8_t UUID_PUBLIC_ECDH_KEY_CHAR[]       = UUID_ES_BEACON(0x75, 0x08);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service EID Identity Key
 * characteristic.
 */
const uint8_t UUID_EID_IDENTITY_KEY_CHAR[]      = UUID_ES_BEACON(0x75, 0x09);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Adv Slot Data
 * characteristic.
 */
const uint8_t UUID_ADV_SLOT_DATA_CHAR[]         = UUID_ES_BEACON(0x75, 0x0a);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Reset
 * characteristic.
 */
const uint8_t UUID_FACTORY_RESET_CHAR[]         = UUID_ES_BEACON(0x75, 0x0b);

/**
 * 128-bit UUID for Eddystone-URL Configuration Service Remain Connectable
 * characteristic.
 */
const uint8_t UUID_REMAIN_CONNECTABLE_CHAR[]    = UUID_ES_BEACON(0x75, 0x0c);

/** END OF CHARACTERISTICS  */

/**
 * Default Lock State used  by EddystoneService.
 */
const uint8_t DEFAULT_LOCK_STATE_DATA[] = {DEFAULT_LOCK_STATE};

/**
 * A type defining the size of the READ ONLY capability characteristic
 */
typedef uint8_t Capability_t[CAP_HDR_LEN + NUM_POWER_MODES];

/**
 * Type for the 128-bit for Eddystone-URL Configuration Service Lock and Unlock
 * characteristic value.
 */
typedef uint8_t Lock_t[16];

/**
 * Type for the 128-bit for Eddystone-URL Configuration Service Advertised TX
 * Power Levels characteristic value.
 */
typedef int8_t PowerLevels_t[NUM_POWER_MODES];

/**
 * Type representing the power level set for each slot
 */
typedef int8_t SlotTxPowerLevels_t[MAX_ADV_SLOTS];

/**
 * Type representing the adv interval set for each slot
 */
typedef uint16_t SlotAdvIntervals_t[MAX_ADV_SLOTS];

/**
 * Type representing the buffer used to represent the LockState and potentially
 * an updated key
 */
typedef uint8_t LockState_t[17];

/**
 * Type representing the EID private ECDH Key
 */
typedef uint8_t PrivateEcdhKey_t[32];

/**
 * Type representing the EID public ECDH Key
 */
typedef uint8_t PublicEcdhKey_t[32];

/**
 * Type representing the EID Identity Key
 */
typedef uint8_t EidIdentityKey_t[16];

/**
 * Type representing the storage for a single slot
 */
typedef uint8_t Slot_t[32]; 

/**
 * Type representing the storage for all slots given MAX_ADV_SLOTS
 */
typedef uint8_t SlotStorage_t[MAX_ADV_SLOTS * sizeof(Slot_t)];

/**
 * Type representing the current frame types if each slot
 */
typedef uint8_t SlotFrameTypes_t[MAX_ADV_SLOTS];

/**
 * Type representing the EID rotation period exp for each slot
 */
typedef uint8_t SlotEidRotationPeriodExps_t[MAX_ADV_SLOTS];

/**
 * Type representing the EID next rotation time for each slot
 */
typedef uint32_t SlotEidNextRotationTimes_t[MAX_ADV_SLOTS];

/**
 * Type representing the EID identity keys for each slot
 */
typedef EidIdentityKey_t SlotEidIdentityKeys_t[MAX_ADV_SLOTS];

/**
 * Size in bytes of UID namespace ID.
 */
const size_t UID_NAMESPACEID_SIZE = 10;

/**
 * Type for the UID namespace ID.
 */
typedef uint8_t UIDNamespaceID_t[UID_NAMESPACEID_SIZE];

/**
 * Size in bytes of UID instance ID.
 */
const size_t UID_INSTANCEID_SIZE = 6;

/**
 * Type for the UID instance ID.
 */
typedef uint8_t UIDInstanceID_t[UID_INSTANCEID_SIZE];

/**
 * Type for callbacks to update Eddystone-TLM frame Batery Voltage and Beacon
 * Temperature.
 */
typedef uint16_t (*TlmUpdateCallback_t) (uint16_t);

// END OF PROTOTYPES

typedef struct {
    uint32_t timeInPriorBoots;
    uint32_t timeSinceLastBoot;
} TimeParams_t;

#endif /* __EDDYSTONETYPES_H__ */
