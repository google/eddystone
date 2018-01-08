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

#include "URLFrame.h"

/* CONSTRUCTOR */ 
URLFrame::URLFrame(void)
{
}
    
void URLFrame::setUnencodedUrlData(uint8_t* rawFrame, int8_t advTxPower, const char *rawUrl)
{    
    uint8_t encodedUrl[ENCODED_BUF_SIZE];
    int encodedUrlLen = encodeURL(encodedUrl, rawUrl);
    encodedUrlLen = (encodedUrlLen > MAX_URL_DATA) ? MAX_URL_DATA : encodedUrlLen;
    setData(rawFrame, advTxPower, reinterpret_cast<const uint8_t*>(encodedUrl), encodedUrlLen);
}

void URLFrame::clearFrame(uint8_t* frame) {
    frame[FRAME_LEN_OFFSET] = 0; // Set frame length to zero to clear it
}

void URLFrame::setData(uint8_t* rawFrame, int8_t advTxPower, const uint8_t* encodedUrlData, uint8_t encodedUrlLen)
{
    uint8_t index = 0;
    rawFrame[index++] = URL_HEADER_LEN + encodedUrlLen; // INDEX=0 = Frame Length = encodedURL size + 4 bytes of header below
    rawFrame[index++] = EDDYSTONE_UUID[0];              // FRAME 16-bit Eddystone UUID Low Byte (little endian)
    rawFrame[index++] = EDDYSTONE_UUID[1];              // FRAME 16-bit Eddystone UUID High Byte
    rawFrame[index++] = FRAME_TYPE_URL;                 // URL Frame Type
    rawFrame[index++] = advTxPower;                     // Power @ 0meter 
    
    memcpy(rawFrame + index, encodedUrlData, encodedUrlLen); 
}

uint8_t*  URLFrame::getData(uint8_t* rawFrame) {
    return &(rawFrame[URL_DATA_OFFSET]);
}


uint8_t  URLFrame::getDataLength(uint8_t* rawFrame) {
    return rawFrame[FRAME_LEN_OFFSET] - EDDYSTONE_UUID_LEN;
}

uint8_t* URLFrame::getAdvFrame(uint8_t* rawFrame) 
{
    return &(rawFrame[ADV_FRAME_OFFSET]);
}

uint8_t URLFrame::getAdvFrameLength(uint8_t* rawFrame) 
{
    return rawFrame[FRAME_LEN_OFFSET];
}

uint8_t* URLFrame::getEncodedUrl(uint8_t* rawFrame)
{
    return &(rawFrame[URL_VALUE_OFFSET]);
}

uint8_t URLFrame::getEncodedUrlLength(uint8_t* rawFrame) 
{
    return rawFrame[ADV_FRAME_OFFSET] - URL_HEADER_LEN;
}


uint8_t URLFrame::encodeURL(uint8_t* encodedUrl, const char *rawUrl)
{
    uint8_t urlDataLength = 0;
    
    const char  *prefixes[] = {
        "http://www.",
        "https://www.",
        "http://",
        "https://",
    };
    const size_t NUM_PREFIXES = sizeof(prefixes) / sizeof(char *);
    const char  *suffixes[]   = {
        ".com/",
        ".org/",
        ".edu/",
        ".net/",
        ".info/",
        ".biz/",
        ".gov/",
        ".com",
        ".org",
        ".edu",
        ".net",
        ".info",
        ".biz",
        ".gov"
    };
    const size_t NUM_SUFFIXES = sizeof(suffixes) / sizeof(char *);

    /*
     * Fill with one more 0 than max url data size to ensure its null terminated
     * And can be printed out for debug purposes
     */ 
    memset(encodedUrl, 0, MAX_URL_DATA + 1);

    if ((rawUrl == NULL) || (strlen(rawUrl) == 0)) {
        return urlDataLength;
    }

    /*
     * handle prefix
     */
    for (size_t i = 0; i < NUM_PREFIXES; i++) {
        size_t prefixLen = strlen(prefixes[i]);
        if (strncmp(rawUrl, prefixes[i], prefixLen) == 0) {
            encodedUrl[urlDataLength++]  = i;
            rawUrl                      += prefixLen;
            break;
        }
    }

    /*
     * handle suffixes
     */
    while (*rawUrl && (urlDataLength <= MAX_URL_DATA)) {
        /* check for suffix match */
        size_t i;
        for (i = 0; i < NUM_SUFFIXES; i++) {
            size_t suffixLen = strlen(suffixes[i]);
            if (strncmp(rawUrl, suffixes[i], suffixLen) == 0) {
                encodedUrl[urlDataLength++]  = i;
                rawUrl                      += suffixLen;
                break; /* from the for loop for checking against suffixes */
            }
        }
        /* This is the default case where we've got an ordinary character which doesn't match a suffix. */
        if (i == NUM_SUFFIXES) {
            encodedUrl[urlDataLength++] = *rawUrl;
            ++rawUrl;
        }
    }
    return urlDataLength;
}

void URLFrame::setAdvTxPower(uint8_t* rawFrame, int8_t advTxPower)
{
    rawFrame[URL_TXPOWER_OFFSET] = advTxPower;
}
