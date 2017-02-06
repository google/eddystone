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

#ifndef __TLMFRAME_H__
#define __TLMFRAME_H__

#include "EddystoneTypes.h"
#include "aes_eax.h"

/**
 * Class that encapsulates data that belongs to the Eddystone-TLM frame. For
 * more information refer to https://github.com/google/eddystone/tree/master/eddystone-tlm.
 */
class TLMFrame
{
public:
    /**
     * Construct a new instance of this class.
     *
     * @param[in] tlmVersionIn
     *              Eddystone-TLM version number to use.
     * @param[in] tlmBatteryVoltageIn
     *              Initial value for the Eddystone-TLM Battery Voltage.
     * @param[in] tlmBeaconTemperatureIn
     *              Initial value for the Eddystone-TLM Beacon Temperature.
     * @param[in] tlmPduCountIn
     *              Initial value for the Eddystone-TLM Advertising PDU Count.
     * @param[in] tlmTimeSinceBootIn
     *              Intitial value for the Eddystone-TLM time since boot timer.
     8              This timer has a 0.1 second resolution.
     */
    TLMFrame(uint8_t  tlmVersionIn           = 0,
             uint16_t tlmBatteryVoltageIn    = 0,
             uint16_t tlmBeaconTemperatureIn = 0x8000,
             uint32_t tlmPduCountIn          = 0,
             uint32_t tlmTimeSinceBootIn     = 0);

    /**
     * Set the Eddystone-TLM version number.
     */
    void setTLMData(uint8_t tlmVersionIn = 0);

    /**
     * Construct the raw bytes of the Eddystone-TLM frame that will be directly
     * used in the advertising packets.
     *
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     */
    void setData(uint8_t *rawFrame);
    
    /**
     * Construct the encrypted bytes of the Eddystone-ETLM frame that will be directly
     * used in the advertising packets.
     *
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] eidIdentityKey
     *              Pointer to the eidIdentityKey in use
     * @param[in] rotationPeriodExp
     *              Rotation exponent for EID
     * @param[in] beaconTimeSecs
     *              Time in seconds since beacon boot.
     */
    void encryptData(uint8_t* rawFrame, uint8_t* eidIdentityKey, uint8_t rotationPeriodExp, uint32_t beaconTimeSecs);

    /**
     * Get the size of the Eddystone-TLM frame constructed with the
     * current state of the TLMFrame object.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-TLM frame.
     */
    size_t getRawFrameSize(uint8_t* rawFrame);
    
    
    /**
     * Get the TLM frame data from the Eddystone-TLM frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-TLM frame data.
     */
    uint8_t* getData(uint8_t* rawFrame);
    
    /**
     * Get the length of the TLM frame data from the Eddystone-TLM frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-TLM frame.
     */
    uint8_t  getDataLength(uint8_t* rawFrame);
    
    /**
     * Get the TLM Adv data from the Eddystone-TLMframe.
     * This is the full service data included in the BLE service data params
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-TLM Adv frame data.
     */
    uint8_t* getAdvFrame(uint8_t* rawFrame);
    
    /**
     * Get the length of the TLM Adv data from the Eddystone-TLMframe.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-TLM Adv frame data.
     */
    uint8_t getAdvFrameLength(uint8_t* rawFrame);

    /**
     * Update the time since last boot.
     *
     * @param[in] nowInMillis
     *              The time since boot in milliseconds.
     */
    void updateTimeSinceLastBoot(uint32_t nowInMillis);

    /**
     * Update the Battery Voltage.
     *
     * @param[in] tlmBatteryVoltageIn
     *              The new Battery Voltage value.
     */
    void updateBatteryVoltage(uint16_t tlmBatteryVoltageIn);

    /**
     * Update the Beacon Temperature.
     *
     * @param[in] tlmBeaconTemperatureIn
     *              The new Beacon Temperature value.
     */
    void updateBeaconTemperature(uint16_t tlmBeaconTemperatureIn);

    /**
     * Increment the current PDU counter by 1.
     */
    void updatePduCount(void);

    /**
     * Get the current Battery Voltage.
     *
     * @return The Battery Voltage.
     */
    uint16_t getBatteryVoltage(void) const;

    /**
     * Get the current Beacon Temperature.
     *
     * @return The Beacon Temperature.
     */
    uint16_t getBeaconTemperature(void) const;

    /**
     * Get the current TLM Version number.
     *
     * @return The TLM Version number.
     */
    uint8_t getTLMVersion(void) const;
    
    /**
     * The byte ID of an Eddystone-TLM frame.
     */
    static const uint8_t FRAME_TYPE_TLM = 0x20;
    
    /**
    * The verison number of the Telemetry packets being used
    */
    static const uint8_t DEFAULT_TLM_VERSION = 0;

    /**
     * The size of an Eddystone-TLM frame.
     */
    static const uint8_t FRAME_SIZE_TLM = 14;
    /**
     * The size of an Eddystone-ETLM frame.
     */
    static const uint8_t FRAME_SIZE_ETLM = (FRAME_SIZE_TLM + 4);

    // Nonce
    static const uint8_t ETLM_NONCE_LEN = 6;
    // Version
    static const uint8_t VERSION_OFFSET = 4;
    static const uint8_t TLM_VERSION = 0x00;
    static const uint8_t ETLM_VERSION = 0x01;
    // Data
    static const uint8_t DATA_OFFSET = 5;
    static const uint8_t TLM_DATA_LEN = 12;
    static const uint8_t ETLM_DATA_LEN = 16;
    // Salt
    static const uint8_t SALT_OFFSET = 12;
    static const uint8_t SALT_LEN = 2;
    // Message Integrity Check
    static const uint8_t MIC_OFFSET = 14;
    static const uint8_t MIC_LEN = 2;
    // Return codes
    static const int ETLM_NONCE_INVALID_LEN = -1;

    /**
     * Constructs 6 byte (48-bit) Nonce from an empty array, rotationExp and beacon time (secs) 
     *
     * @param[in] nonce
     *              the input and target nonce[] array
     * @param[in] rotationPeriodExp
     *              Rotation exponent for EID
     * @param[in] beaconTimeSecs
     *              Time in seconds since beacon boot.
     * @return[out] return code (success = 0)
     */
    int generateEtlmNonce(uint8_t* nonce, uint8_t rotatePeriodExp, uint32_t beaconTimeSecs);


private:

    /**
     * The size (in bytes) of an Eddystone-EID frame.
     * This is the some of the Eddystone UUID(2 bytes), FrameType, AdvTxPower,
     * EID Value
     */
    // static const uint8_t TLM_FRAME_LEN = 16;
    // static const uint8_t ETLM_FRAME_LEN = 20;
    static const uint8_t FRAME_LEN_OFFSET = 0;
    static const uint8_t EDDYSTONE_UUID_LEN = 2; 
    static const uint8_t TLM_DATA_OFFSET = 3;
    static const uint8_t ADV_FRAME_OFFSET = 1;

    /**
     * Eddystone-TLM version value.
     */
    uint8_t              tlmVersion;
    /**
     * Time since boot in milliseconds.
     */
    uint32_t             lastTimeSinceBootRead;
    /**
     * Eddystone-TLM Battery Voltage value.
     */
    uint16_t             tlmBatteryVoltage;
    /**
     * Eddystone-TLM Beacon temperature value.
     */
    uint16_t             tlmBeaconTemperature;
    /**
     * Eddystone-TLM Advertising PDU Count.
     */
    uint32_t             tlmPduCount;
    /**
     * Eddystone-TLM time since boot with 0.1 second resolution.
     */
    uint32_t             tlmTimeSinceBoot;


};
#endif  /* __TLMFRAME_H__ */
