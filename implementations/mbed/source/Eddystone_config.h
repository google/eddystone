/*
 * Copyright (c) 2016, Google, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EDDYSTONE_CONFIG_H_
#define EDDYSTONE_CONFIG_H_

// Version printed out on virtual terminal (independent of logging flag below)
#define BUILD_VERSION_STR "EID Version 1.00 2016-11-19:15:00\r\n"

/**
 * Platform Target (if not set, default is nRF51-DK or nRF51-dongle or nRF52-DK)
 * NOTE1: All targets are assumed to be 32K (in target.json) and S110 (in config.h)
 * NOTE2: Only enable one target below (default is nRF_DK).
 */
// #define MinewTech51
#define MinewTech52
// #define nRF_DK

/**
 * DECLARE YOUR TARGET'S PARAMETERS
 * If you are adding a new target, append to end of elif chain
 * 
 * LED_OFF: value for an LED off state: 1 or 0
 * CONFIG_LED: which LED to blink when in Configuration Mode
 *     On power up will go into configuration mode and eventually time out
 * SHUTDOWN_LED: which LED to blink when shutting down (only used if RESET_BUTTON is defined)
 * RESET_BUTTON: Which button to use. If defined, adds code to handle button presses
 *     Button press will toggle between configuration mode and off
 *     Configuration mode will eventually timeout and broadcast default values
 *     This will shutdown after initial power up! Assumes shipping with a battery in an off state
 * EDDYSTONE_DEFAULT_RADIO_TX_POWER_LEVELS: Which power levels to offer
 * EDDYSTONE_DEFAULT_ADV_TX_POWER_LEVELS: What to advertise these levels (as antennas always loose some power) 
 */
#ifdef MinewTech51
  #define LED_OFF 0
  #define CONFIG_LED p15
  #define SHUTDOWN_LED p16
  #define RESET_BUTTON p18
  #define EDDYSTONE_DEFAULT_RADIO_TX_POWER_LEVELS { -30, -16, -4, 4 }
  #define EDDYSTONE_DEFAULT_ADV_TX_POWER_LEVELS { -42, -30, -25, -13 }  

#elif defined MinewTech52
  #define LED_OFF 0
  #define CONFIG_LED LED3
  #define SHUTDOWN_LED LED2
  #define RESET_BUTTON BUTTON1
  #define EDDYSTONE_DEFAULT_RADIO_TX_POWER_LEVELS { -40, -20, -8, 4 }
  #define EDDYSTONE_DEFAULT_ADV_TX_POWER_LEVELS { -50, -30, -18, -6 }

#else
  // *** nRF_DK or USB Dongle PIN defines ***
  #define LED_OFF 1
  #define CONFIG_LED LED3
  // Uncomment the defines below if you want the DK board to behave like a
  // Beacon target with shutdown on power up, and a mode button
  // #define SHUTDOWN_LED LED2
  // #define RESET_BUTTON BUTTON1
  #define EDDYSTONE_DEFAULT_RADIO_TX_POWER_LEVELS { -30, -16, -4, 4 }
  #define EDDYSTONE_DEFAULT_ADV_TX_POWER_LEVELS { -42, -30, -25, -13 } 
#endif

/** 
 * DEBUG OPTIONS
 * For production: all defines below should be UNCOMMENTED:
 * Key
 *   GEN_BEACON_KEYS_AT_INIT:  Debugging flag to help test entropy source
 *   HARDWARE_RANDOM_NUM_GENERATOR: include if the target supports a hardware RNG
 *   EID_RANDOM_MAC: include if you want to randomize the mac address for each eid rotation
 *   INCLUDE_CONFIG_URL: Includes configuration url when in Configuration Mode
 *   DONT_REMAIN_CONNECTABLE: Debugging flag; remain connectable during beaconing for easy testing
 *   NO_4SEC_START_DELAY: Debugging flag to pause 4s before starting; allow time to connect virtual terminal
 *   NO_EAX_TEST: Debugging flag: when not define, test will check x = EAX_DECRYPT(EAX_ENCRYPT(x)), output in LOG
 *   NO_LOGGING: Debugging flag; controls logging to virtual terminal
 */ 
#define GEN_BEACON_KEYS_AT_INIT
#define HARDWARE_RANDOM_NUM_GENERATOR
#define EID_RANDOM_MAC
#define INCLUDE_CONFIG_URL
#define DONT_REMAIN_CONNECTABLE
#define NO_4SEC_START_DELAY
#define NO_EAX_TEST
#define NO_LOGGING

/* Default enable printf logging, unless explicitly NO_LOGGING */
#ifdef NO_LOGGING
  #define LOG_PRINT 0
#else
  #define LOG_PRINT 1
#endif

#define LOG(x) do { if (LOG_PRINT) printf x; } while (0)

/**
 * GENERIC BEACON BEHAVIORS DEFINED
 * Note: If the CONFIG_URL is enabled (DEFINE above)
 *    The size of the DEVICE_NAME + Encoded Length of the CONFIG_URL
 *    must be LESS THAN OR EQUAL to 23
 */
#define EDDYSTONE_CONFIG_URL "http://c.pw3b.com"
#define EDDYSTONE_CFG_DEFAULT_DEVICE_NAME "Eddystone v3.0"
#define EDDYSTONE_DEFAULT_MAX_ADV_SLOTS 3
#define EDDYSTONE_DEFAULT_CONFIG_ADV_INTERVAL 1000
#define EDDYSTONE_DEFAULT_CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS 60

#define EDDYSTONE_DEFAULT_UNLOCK_KEY { \
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF \
}

#define EDDYSTONE_DEFAULT_SLOT_URLS { \
    "http://c.pw3b.com", \
    "https://www.mbed.com/", \
    "https://www.github.com/" \
}

#define EDDYSTONE_DEFAULT_SLOT_UIDS { \
    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }, \
    { 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0 }, \
    { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF } \
}

#define EDDYSTONE_DEFAULT_SLOT_EID_IDENTITY_KEYS { \
    { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF }, \
    { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF }, \
    { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF } \
}

#define EDDYSTONE_DEFAULT_SLOT_EID_ROTATION_PERIOD_EXPS { 10, 10, 10 }

// The following frame/slot types are supported: URL, UID, TLM, EID. The defaults set URL x2 and EID
#define EDDYSTONE_DEFAULT_SLOT_TYPES { \
    EDDYSTONE_FRAME_URL, \
    EDDYSTONE_FRAME_URL, \
    EDDYSTONE_FRAME_EID \
}

#define EDDYSTONE_DEFAULT_SLOT_INTERVALS { 700, 0, 0 }

#define EDDYSTONE_DEFAULT_SLOT_TX_POWERS { -8, -8, -8 }

/**
 * Lock constants
 */
#define LOCKED 0
#define UNLOCKED 1
#define UNLOCKED_AUTO_RELOCK_DISABLED 2

#define DEFAULT_LOCK_STATE UNLOCKED

/**
 * Set default number of adv slots
 */
const uint8_t MAX_ADV_SLOTS = EDDYSTONE_DEFAULT_MAX_ADV_SLOTS;

/**
 * Slot and Power and Interval Constants
 */
const uint8_t DEFAULT_SLOT = 0;

/**
 * Number of radio power modes supported
 */
const uint8_t NUM_POWER_MODES = 4;

/**
 * Default name for the BLE Device Name characteristic.
 */
const char DEFAULT_DEVICE_NAME[] = EDDYSTONE_CFG_DEFAULT_DEVICE_NAME;

/**
 * ES GATT Capability Constants (6 values)
 */
const uint8_t CAP_HDR_LEN = 6;  // The six constants below
const uint8_t ES_GATT_VERSION = 0;
const uint8_t MAX_EIDS = MAX_ADV_SLOTS;
const uint8_t CAPABILITIES = 0x03; // Per slot variable interval and variable Power
const uint8_t SUPPORTED_FRAMES_H = 0x00;
const uint8_t SUPPORTED_FRAMES_L = 0x0F;

/**
 * ES GATT Capability Constant Array storing the capability constants
 */
const uint8_t CAPABILITIES_DEFAULT[] = {ES_GATT_VERSION, MAX_ADV_SLOTS, MAX_EIDS, CAPABILITIES, \
                                        SUPPORTED_FRAMES_H, SUPPORTED_FRAMES_L};

#endif /* EDDYSTONE_CONFIG_H_ */
