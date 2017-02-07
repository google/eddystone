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

#ifndef __UIDFRAME_H__
#define __UIDFRAME_H__

#include <string.h>
#include "EddystoneTypes.h"

/**
 * Class that encapsulates data that belongs to the Eddystone-UID frame. For
 * more information refer to https://github.com/google/eddystone/tree/master/eddystone-uid.
 */
class UIDFrame
{
public:
    static const uint8_t UID_LENGTH = 16;

    /**
     * Construct a new instance of this class.
     */
    UIDFrame(void);
    
    /**
     * Clear frame (intervally indicated by length = 0 )
     */
    void clearFrame(uint8_t* frame);
    
    /**
     * Construct the raw bytes of the Eddystone-UID frame that will be directly
     * used in the advertising packets.
     *
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] advPowerLevel
     *              Power level value included in the raw frame.
     * @param[in] uidData
     *              The actual 16-byte UID data in the raw frame.
     */
    void setData(uint8_t* rawFrame, int8_t advTxPower, const uint8_t* uidData);
    
    /**
     * Get the UID frame data from the Eddystone-UID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-UID frame data.
     */
    uint8_t* getData(uint8_t* rawFrame);
    
    /**
     * Get the length of the UID frame data from the Eddystone-UID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-UID frame.
     */
    uint8_t  getDataLength(uint8_t* rawFrame);
    
    /**
     * Get the UID Adv data from the Eddystone-UID frame.
     * This is the full service data included in the BLE service data params
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-UID Adv frame data.
     */
    uint8_t* getAdvFrame(uint8_t* rawFrame);
    
    /**
     * Get the length of the UID Adv data from the Eddystone-UID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-UID Adv frame data.
     */
    uint8_t getAdvFrameLength(uint8_t* rawFrame);

    /**
     * Get just the UID data from the Eddystone-UID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the UID in the Eddystone-UID frame.
     */
    uint8_t* getUid(uint8_t* rawFrame);
    
    /**
     * Get the length of just the UID data from the Eddystone-UID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the UID in the Eddystone-UID frame.
     */
    uint8_t getUidLength(uint8_t* rawFrame);
    
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
     *  The byte ID of an Eddystone-UID frame.
     */
    static const uint8_t FRAME_TYPE_UID = 0x00;

private:
    static const uint8_t UID_FRAME_LEN = 20; 
    static const uint8_t FRAME_LEN_OFFSET = 0;
    static const uint8_t EDDYSTONE_UUID_LEN = 2;
    static const uint8_t UID_DATA_OFFSET = 3;
    static const uint8_t ADV_FRAME_OFFSET = 1;
    static const uint8_t UID_VALUE_OFFSET = 5;
    static const uint8_t UID_HEADER_LEN = 4;
    static const uint8_t UID_TXPOWER_OFFSET = 4;
    /**
     * The size (in bytes) of an Eddystone-UID frame.
     * This is the some of the Eddystone UUID(2 bytes), FrameType, AdvTxPower,
     * UID Name Length, and UID Instance Length
     */
    static const uint8_t FRAME_SIZE_UID = 20;
    /**
     * The size (in bytes) of an Eddystone-UID frame.
     */
    static const uint8_t UID_NAMESPACEID_LENGTH = 10;
    static const uint8_t UID_INSTANCEID_LENGTH = 6;
 
};

#endif  /* __UIDFRAME_H__ */
