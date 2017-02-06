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

#include "TLMFrame.h"
#include "EddystoneService.h"

TLMFrame::TLMFrame(uint8_t  tlmVersionIn,
                   uint16_t tlmBatteryVoltageIn,
                   uint16_t tlmBeaconTemperatureIn,
                   uint32_t tlmPduCountIn,
                   uint32_t tlmTimeSinceBootIn) :
    tlmVersion(tlmVersionIn),
    lastTimeSinceBootRead(0),
    tlmBatteryVoltage(tlmBatteryVoltageIn),
    tlmBeaconTemperature(tlmBeaconTemperatureIn),
    tlmPduCount(tlmPduCountIn),
    tlmTimeSinceBoot(tlmTimeSinceBootIn)
{
}

void TLMFrame::setTLMData(uint8_t tlmVersionIn)
{
    /* According to the Eddystone spec BatteryVoltage is 0 and
     * BeaconTemperature is 0x8000 if not supported
     */
    tlmVersion           = tlmVersionIn;
    tlmBatteryVoltage    = 0;
    tlmBeaconTemperature = 0x8000;
    tlmPduCount          = 0;
    tlmTimeSinceBoot     = 0;
}

void TLMFrame::setData(uint8_t *rawFrame)  // add eidTime - a 4 byte quantity
{
    size_t index = 0;
    rawFrame[index++] = EDDYSTONE_UUID_SIZE + FRAME_SIZE_TLM; // Length of Frame
    rawFrame[index++] = EDDYSTONE_UUID[0];                    // 16-bit Eddystone UUID
    rawFrame[index++] = EDDYSTONE_UUID[1];
    rawFrame[index++] = FRAME_TYPE_TLM;                       // Eddystone frame type = Telemetry
    rawFrame[index++] = tlmVersion;                           // TLM Version Number
    rawFrame[index++] = (uint8_t)(tlmBatteryVoltage >> 8);    // Battery Voltage[0]
    rawFrame[index++] = (uint8_t)(tlmBatteryVoltage >> 0);    // Battery Voltage[1]
    rawFrame[index++] = (uint8_t)(tlmBeaconTemperature >> 8); // Beacon Temp[0]
    rawFrame[index++] = (uint8_t)(tlmBeaconTemperature >> 0); // Beacon Temp[1]
    rawFrame[index++] = (uint8_t)(tlmPduCount >> 24);         // PDU Count [0]
    rawFrame[index++] = (uint8_t)(tlmPduCount >> 16);         // PDU Count [1]
    rawFrame[index++] = (uint8_t)(tlmPduCount >> 8);          // PDU Count [2]
    rawFrame[index++] = (uint8_t)(tlmPduCount >> 0);          // PDU Count [3]
    rawFrame[index++] = (uint8_t)(tlmTimeSinceBoot >> 24);    // Time Since Boot [0]
    rawFrame[index++] = (uint8_t)(tlmTimeSinceBoot >> 16);    // Time Since Boot [1]
    rawFrame[index++] = (uint8_t)(tlmTimeSinceBoot >> 8);     // Time Since Boot [2]
    rawFrame[index++] = (uint8_t)(tlmTimeSinceBoot >> 0);     // Time Since Boot [3]
}

void TLMFrame::encryptData(uint8_t* rawFrame, uint8_t* eidIdentityKey, uint8_t rotationPeriodExp, uint32_t beaconTimeSecs) {
    // Initialize AES data
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx); 
    mbedtls_aes_setkey_enc(&ctx, eidIdentityKey, sizeof(EidIdentityKey_t) *8 );
    // Change the TLM version number to the encrypted version
    rawFrame[VERSION_OFFSET] = ETLM_VERSION; // Encrypted TLM Version number
    // Create EAX Params
    uint8_t nonce[ETLM_NONCE_LEN];
    // Calculate the 48-bit nonce
    generateEtlmNonce(nonce, rotationPeriodExp, beaconTimeSecs);
 
    uint8_t* input = rawFrame + DATA_OFFSET;  // array size 12
    uint8_t output[ETLM_DATA_LEN]; // array size 16 (4 bytes are added: SALT[2], MIC[2])
    memset(output, 0, ETLM_DATA_LEN);
    uint8_t emptyHeader[1]; // Empty header
    LOG(("EIDIdentityKey=\r\n")); EddystoneService::logPrintHex(eidIdentityKey, 16);
    LOG(("ETLM Encoder INPUT=\r\n")); EddystoneService::logPrintHex(input, 12);
    LOG(("ETLM SALT=\r\n")); EddystoneService::logPrintHex(nonce+4, 2);
    LOG(("ETLM Nonce=\r\n")); EddystoneService::logPrintHex(nonce, 6);
    // Encrypt the TLM to ETLM
    eddy_aes_authcrypt_eax(&ctx, MBEDTLS_AES_ENCRYPT, nonce, sizeof(nonce), emptyHeader, 0, TLM_DATA_LEN, input, output, output + MIC_OFFSET, MIC_LEN);

#ifndef NO_EAX_TEST
    // Part of test code to confirm x == EAX_DECRYPT( EAX_ENCRYPT(x) )
    uint8_t newinput[ETLM_DATA_LEN];
    memcpy(newinput, output, ETLM_DATA_LEN);
#endif

    // Only use first 2 bytes of Nonce
    output[SALT_OFFSET] = nonce[4]; // Nonce MSB
    output[SALT_OFFSET+1] = nonce[5]; // Nonce LSB
    LOG(("ETLM output+SALT=\r\n")); EddystoneService::logPrintHex(output, 16);
    // copy the encrypted payload to the output
    memcpy((rawFrame + DATA_OFFSET), output, ETLM_DATA_LEN);
        
#ifndef NO_EAX_TEST
    // Perform test to confirm x == EAX_DECRYPT( EAX_ENCRYPT(x) )
    uint8_t buf[ETLM_DATA_LEN];
    memset(buf, 0, ETLM_DATA_LEN);
    int ret = eddy_aes_authcrypt_eax(&ctx, MBEDTLS_AES_DECRYPT, nonce, sizeof(nonce), emptyHeader, 0, TLM_DATA_LEN, newinput, buf, newinput + MIC_OFFSET, MIC_LEN);
    LOG(("ETLM Decoder OUTPUT ret=%d buf=\r\n", ret)); EddystoneService::logPrintHex(buf, 12);
#endif
        
    // fix the frame length to the encrypted length
    rawFrame[FRAME_LEN_OFFSET] = FRAME_SIZE_ETLM + EDDYSTONE_UUID_SIZE; 
    // Free the AES data struture
    mbedtls_aes_free(&ctx);
}
    

size_t TLMFrame::getRawFrameSize(uint8_t* rawFrame)
{
    return rawFrame[FRAME_LEN_OFFSET];
}

uint8_t* TLMFrame::getData(uint8_t* rawFrame) 
{
    if (rawFrame[VERSION_OFFSET] == TLM_VERSION) {
        setData(rawFrame);
    }
    return &(rawFrame[TLM_DATA_OFFSET]);
}

uint8_t TLMFrame::getDataLength(uint8_t* rawFrame)
{
    return rawFrame[FRAME_LEN_OFFSET] - EDDYSTONE_UUID_LEN;
}

uint8_t* TLMFrame::getAdvFrame(uint8_t* rawFrame){
    return &(rawFrame[ADV_FRAME_OFFSET]);
}

uint8_t TLMFrame::getAdvFrameLength(uint8_t* rawFrame){
    return rawFrame[FRAME_LEN_OFFSET];
}

void TLMFrame::updateTimeSinceLastBoot(uint32_t nowInMillis)
{
    // Measured in tenths of a second
    tlmTimeSinceBoot      += (nowInMillis - lastTimeSinceBootRead) / 100;
    lastTimeSinceBootRead  = nowInMillis;
}

void TLMFrame::updateBatteryVoltage(uint16_t tlmBatteryVoltageIn)
{
    tlmBatteryVoltage = tlmBatteryVoltageIn;
}

void TLMFrame::updateBeaconTemperature(uint16_t tlmBeaconTemperatureIn)
{
    tlmBeaconTemperature = tlmBeaconTemperatureIn;
}

void TLMFrame::updatePduCount(void)
{
    tlmPduCount++;
}

uint16_t TLMFrame::getBatteryVoltage(void) const
{
    return tlmBatteryVoltage;
}

uint16_t TLMFrame::getBeaconTemperature(void) const
{
    return tlmBeaconTemperature;
}

uint8_t TLMFrame::getTLMVersion(void) const
{
    return tlmVersion;
}

int TLMFrame::generateEtlmNonce(uint8_t* nonce, uint8_t rotationPeriodExp, uint32_t beaconTimeSecs) {
    int rc = 0;
    if (sizeof(nonce) != ETLM_NONCE_LEN) {
      rc = ETLM_NONCE_INVALID_LEN; 
    }
    uint32_t scaledTime = (beaconTimeSecs >> rotationPeriodExp) << rotationPeriodExp;
    int index = 0;
    nonce[index++] = (scaledTime  >> 24) & 0xff;
    nonce[index++] = (scaledTime >> 16) & 0xff;
    nonce[index++] = (scaledTime >> 8) & 0xff;
    nonce[index++] = scaledTime & 0xff;
    EddystoneService::generateRandom(nonce + index, SALT_LEN);
    return rc;
}

