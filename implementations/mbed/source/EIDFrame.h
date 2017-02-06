/*
 * Copyright (c) 2016, Google Inc, All Rights Reserved
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

#ifndef __EIDFRAME_H__
#define __EIDFRAME_H__

#include <string.h>
#include "EddystoneTypes.h"
#include "mbedtls/aes.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/md.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "aes_eax.h"

/**
 * Class that encapsulates data that belongs to the Eddystone-EID frame. For
 * more information refer to https://github.com/google/eddystone/tree/master/eddystone-EID.
 */
class EIDFrame
{
public:
    static const uint8_t SALT = 0xff;
    static const uint8_t EID_LENGTH = 8;
    static const int EID_SUCCESS = 0;
    static const int EID_RC_SS_IS_ZERO = -1;
    static const int EID_RND_FAIL = -2;
    static const int EID_GRP_FAIL = -3;
    static const int EID_GENKEY_FAIL = -4;

    /**
     * Construct a new instance of this class.
     */
    EIDFrame();
    
    /**
     * Clear frame (internally represented by length = 0 )
     */
    void clearFrame(uint8_t* frame);
    
    /**
     * Construct the raw bytes of the Eddystone-EID frame that will be directly
     * used in the advertising packets.
     *
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] advPowerLevel
     *              Power level value included in the raw frame.
     * @param[in] eidData
     *              The actual 16-byte EID data in the raw frame.
     */
    void setData(uint8_t* rawFrame, int8_t advTxPower, const uint8_t* eidData);
    
    /**
     * Get the EID frame data from the Eddystone-EID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-EID frame data.
     */
    uint8_t* getData(uint8_t* rawFrame);
    
    /**
     * Get the length of the EID frame data from the Eddystone-EID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-EID frame.
     */
    uint8_t  getDataLength(uint8_t* rawFrame);
    
    /**
     * Get the EID Adv data from the Eddystone-EID frame.
     * This is the full service data included in the BLE service data params
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-EID Adv frame data.
     */
    uint8_t* getAdvFrame(uint8_t* rawFrame);
    
    /**
     * Get the length of the EID Adv data from the Eddystone-EID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-EID Adv frame data.
     */
    uint8_t getAdvFrameLength(uint8_t* rawFrame);

    /**
     * Get just the EID data from the Eddystone-EID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the EID in the Eddystone-EID frame.
     */
    uint8_t* getEid(uint8_t* rawFrame);
    
    /**
     * Get the length of just the EID data from the Eddystone-EID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the EID in the Eddystone-EID frame.
     */
    uint8_t getEidLength(uint8_t* rawFrame);
    
    /**
     * Set the Adv TX Power in the frame. This is necessary because the adv
     * Tx Power might be updated independent of the data bytes
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] advPowerLevel
     *              Power level value included in the raw frame.
     *
     */
    void setAdvTxPower(uint8_t* rawFrame, int8_t advTxPower);
    

    /**
     * Generate the beacon private and public keys. This should be called on
     * every restart of the beacon.
     * 
     * @param[out] beaconPrivateEcdhKey
     *              Pointer to the beacon private key array.
     * @param[out] beaconPublicEcdhKey
     *              Pointer to the beacon public key array.
     *
     */
    int genBeaconKeys(PrivateEcdhKey_t beaconPrivateEcdhKey, PublicEcdhKey_t beaconPublicEcdhKey);

    /**
     * Update the EID frame. Tests if its time to rotate the EID payload, and if due, calculates and establishes the new value
     * 
     * @param[in] *rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] *eidIdentityKey
     *              Eid key used to regenerate the EID id.
     * @param[in] rotationPeriodExp
     *              EID rotation time as an exponent k : 2^k seconds
     * @param[in] timeSecs
     *              time in seconds
     *
     */
    void update(uint8_t* rawFrame, uint8_t* eidIdentityKey, uint8_t rotationPeriodExp,  uint32_t timeSecs);
    
    /**
     * genEcdhSharedKey generates the eik value for inclusion in the EID ADV packet
     *
     * @param[in] beaconPrivateEcdhKey
     *              The beacon's private ECDH key, generated by genBeaconKeys()
     * @param[in] beaconPublicEcdhKey
     *              The beacon's public ECDH key, generated by genBeaconKeys()
     * @param[in] serverPublicEcdhKey
     *              The server's public ECDH key
     * @param[out] eidIdentityKey
     *              Identity key for this beacon and server combination
     */
    int genEcdhSharedKey(PrivateEcdhKey_t beaconPrivateEcdhKey, PublicEcdhKey_t beaconPublicEcdhKey, PublicEcdhKey_t serverPublicEcdhKey, EidIdentityKey_t eidIdentityKey);
    
    /**
     *  The byte ID of an Eddystone-EID frame.
     */
    static const uint8_t FRAME_TYPE_EID = 0x30;

private:
    
    // Declare context for crypto functions
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ecdh_context ecdh_ctx;
    mbedtls_md_context_t md_ctx;

    /**
     * The size (in bytes) of an Eddystone-EID frame.
     * This is the some of the Eddystone UUID(2 bytes), FrameType, AdvTxPower,
     * EID Value
     */
    static const uint8_t EID_FRAME_LEN = 18;
    static const uint8_t FRAME_LEN_OFFSET = 0;
    static const uint8_t EDDYSTONE_UUID_LEN = 2;
    static const uint8_t EID_DATA_OFFSET = 3;
    static const uint8_t ADV_FRAME_OFFSET = 1;
    static const uint8_t EID_VALUE_OFFSET = 5;
    static const uint8_t EID_HEADER_LEN = 4;
    static const uint8_t EID_TXPOWER_OFFSET = 4;
    
    /**
     * AES128 ECB Encrypts a 16-byte input array with a key, to an output array
     *
     * @param[in] *key
     *              The encryption key
     * @param[in] *input
     *              The input array
     * @param[in] *output
     *              The output array (contains the encrypted data)
     */
    void aes128Encrypt(uint8_t *key, uint8_t *input, uint8_t *output);
 
};

#endif  /* __EIDFRAME_H__ */
