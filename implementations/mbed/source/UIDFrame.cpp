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

#include "UIDFrame.h"

UIDFrame::UIDFrame(void) {
}

void UIDFrame::clearFrame(uint8_t* frame) {
    frame[FRAME_LEN_OFFSET] = 0; // Set frame length to zero to clear it
}

void UIDFrame::setData(uint8_t *rawFrame, int8_t advTxPower, const uint8_t* uidData) {
    size_t index = 0;
    rawFrame[index++] = UID_HEADER_LEN + UID_LENGTH;            // UID length + overhead of four bytes below
    rawFrame[index++] = EDDYSTONE_UUID[0];                      // LSB 16-bit Eddystone UUID (little endian)
    rawFrame[index++] = EDDYSTONE_UUID[1];                      // MSB
    rawFrame[index++] = FRAME_TYPE_UID;                         // 1B  Type
    rawFrame[index++] = advTxPower;                             // 1B  Power @ 0meter

    memcpy(rawFrame + index, uidData, UID_LENGTH);              // UID = 10B NamespaceID + 6B InstanceID
}

uint8_t* UIDFrame::getData(uint8_t* rawFrame) {
    return &(rawFrame[UID_DATA_OFFSET]);
}

uint8_t  UIDFrame::getDataLength(uint8_t* rawFrame)
{
    return rawFrame[FRAME_LEN_OFFSET] - EDDYSTONE_UUID_LEN;
}

uint8_t* UIDFrame::getAdvFrame(uint8_t* rawFrame)
{
    return &(rawFrame[ADV_FRAME_OFFSET]);
}

uint8_t UIDFrame::getAdvFrameLength(uint8_t* rawFrame)
{
    return rawFrame[FRAME_LEN_OFFSET];
}

uint8_t* UIDFrame::getUid(uint8_t* rawFrame)
{
    return &(rawFrame[UID_VALUE_OFFSET]);
}

uint8_t UIDFrame::getUidLength(uint8_t* rawFrame)
{
    return rawFrame[FRAME_LEN_OFFSET] - UID_HEADER_LEN;
}

void UIDFrame::setAdvTxPower(uint8_t* rawFrame, int8_t advTxPower)
{
    rawFrame[UID_TXPOWER_OFFSET] = advTxPower;
}