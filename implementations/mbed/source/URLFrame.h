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

#ifndef __URLFRAME_H__
#define __URLFRAME_H__

#include "EddystoneTypes.h"
#include <string.h>

/**
 * Class that encapsulates data that belongs to the Eddystone-URL frame. For
 * more information refer to https://github.com/google/eddystone/tree/master/eddystone-url.
 */
class URLFrame
{
public:
    /**
     * Construct a new instance of this class.
     */
    URLFrame(void);
    
    /**
     * Construct the raw bytes of the Eddystone-URL frame from an unencoded URL
     * (a null terminated string) that will be directly used in the advertising
     * packets.
     *
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] advPowerLevel
     *              Power level value included in the raw frame.
     * @param[in] rawURL
     *              A null terminated string containing the URL
     */
    void setUnencodedUrlData(uint8_t* rawFrame, int8_t advTxPower, const char *rawUrl);
    
    /**
     * Clear frame (intervally indicated by length = 0 )
     */
    void clearFrame(uint8_t* frame);
    
    /**
     * Construct the raw bytes of the Eddystone-URL frame from an encoded URL 
     * plus length information
     *
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     * @param[in] advPowerLevel
     *              Power level value included in the raw frame.
     * @param[in] encodedUrlData
     *              A pointer to the encoded URL bytes.
     * @param[in] encodedUrlLen
     *              The length in bytes of the encoded URL
     */
    void setData(uint8_t* rawFrame, int8_t advPowerLevel, const uint8_t* encodedUrlData, uint8_t encodedUrlLen);
    
    /**
     * Get the URL frame data from the Eddystone-URL frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-URL frame data.
     */
    uint8_t* getData(uint8_t* rawFrame);
    
    /**
     * Get the length of the URL frame data from the Eddystone-UID frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-URL frame.
     */
    uint8_t getDataLength(uint8_t* rawFrame);

    /**
     * Get the URL Adv data from the Eddystone-URLframe.
     * This is the full service data included in the BLE service data params
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the Eddystone-URLAdv frame data.
     */
    uint8_t* getAdvFrame(uint8_t* rawFrame);
    
    /**
     * Get the length of the URLAdv data from the Eddystone-URL frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the Eddystone-URL Adv frame data.
     */
    uint8_t getAdvFrameLength(uint8_t* rawFrame);

    /**
     * Get just the encoded URL data from the Eddystone-URL frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return A pointer to the bytes of the encoded URL in the Eddystone-URL
     * frame.
     */
    uint8_t* getEncodedUrl(uint8_t* rawFrame);
    
    /**
     * Get the length of just the encoded URL data from the Eddystone-URL frame.
     * 
     * @param[in] rawFrame
     *              Pointer to the location where the raw frame will be stored.
     *
     * @return The size in bytes of the encoded URL in the Eddystone-URL frame.
     */
    uint8_t getEncodedUrlLength(uint8_t* rawFrame);
    
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
     * Helper function that encodes a URL null terminated string into the HTTP
     * URL Encoding required in Eddystone-URL frames. Refer to
     * https://github.com/google/eddystone/blob/master/eddystone-url/README.md#eddystone-url-http-url-encoding.
     *
     * @param[in] encodedUrlData
     *              The encoded bytes of the URL
     * @param[in] rawUrl
     *              The null terminated string containing a URL to encode.
     * @return Length of the encodedData in bytes
     */
    static uint8_t encodeURL(uint8_t* encodedUrlData, const char* rawUrl);

    /**
     * The max size (in bytes) of an Eddystone-URL frame.
     */
    static const uint8_t ENCODED_BUF_SIZE = 32;
    /**
     *  The byte ID of an Eddystone-URL frame.
     */
    static const uint8_t FRAME_TYPE_URL     = 0x10;

private:
    static const uint8_t FRAME_LEN_OFFSET = 0;
    static const uint8_t EDDYSTONE_UUID_LEN = 2;
    static const uint8_t URL_DATA_OFFSET = 3;
    static const uint8_t ADV_FRAME_OFFSET = 1;
    static const uint8_t URL_VALUE_OFFSET = 5;
    static const uint8_t URL_HEADER_LEN = 4;
    static const uint8_t URL_TXPOWER_OFFSET = 4;

    /**
     * The minimum size (in bytes) of an Eddystone-URL frame.
     */
    static const uint8_t FRAME_MIN_SIZE_URL = 2;
    
    /**
    * Offset for playload in a rawFrame UID
    */
    static const uint8_t MAX_URL_DATA = 18;
};

#endif /* __URLFRAME_H__ */
