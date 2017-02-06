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

#include "EddystoneService.h"
#include "PersistentStorageHelper/ConfigParamsPersistence.h"
#include "EntropySource/EntropySource.h"

/* Use define zero for production, 1 for testing to allow connection at any time */
#define DEFAULT_REMAIN_CONNECTABLE 0x01

const char * const EddystoneService::slotDefaultUrls[] = EDDYSTONE_DEFAULT_SLOT_URLS;

// Static timer used as time since boot
Timer           EddystoneService::timeSinceBootTimer;

/*
 * CONSTRUCTOR #1 Used on 1st boot (after reflash)
 */
EddystoneService::EddystoneService(BLE                 &bleIn,
                                   const PowerLevels_t &advTxPowerLevelsIn,
                                   const PowerLevels_t &radioTxPowerLevelsIn,
                                   event_queue_t       &evQ,
                                   uint32_t            advConfigIntervalIn) :
    ble(bleIn),
    operationMode(EDDYSTONE_MODE_NONE),
    uidFrame(),
    urlFrame(),
    tlmFrame(),
    eidFrame(),
    tlmBatteryVoltageCallback(NULL),
    tlmBeaconTemperatureCallback(NULL),
    radioManagerCallbackHandle(NULL),
    deviceName(DEFAULT_DEVICE_NAME),
    eventQueue(evQ),
    nextEidSlot(0)
{
    LOG(("1st Boot: ")); 
    LOG((BUILD_VERSION_STR));
    if (advConfigIntervalIn != 0) {
        if (advConfigIntervalIn < ble.gap().getMinAdvertisingInterval()) {
            advConfigInterval = ble.gap().getMinAdvertisingInterval();
        } else if (advConfigIntervalIn > ble.gap().getMaxAdvertisingInterval()) {
            advConfigInterval = ble.gap().getMaxAdvertisingInterval();
        } else {
            advConfigInterval = advConfigIntervalIn;
        }
    }
    memcpy(radioTxPowerLevels, radioTxPowerLevelsIn, sizeof(PowerLevels_t));
    memcpy(advTxPowerLevels,   advTxPowerLevelsIn,   sizeof(PowerLevels_t));

    // 1st Boot so reset everything to factory values
    LOG(("1st BOOT: "));
    doFactoryReset();  // includes genBeaconKeys
    
    LOG(("After FactoryReset: 1st Boot Init: genBeaconKeyRC=%d\r\n", genBeaconKeyRC));

    /* Set the device name at startup */
    ble.gap().setDeviceName(reinterpret_cast<const uint8_t *>(deviceName));
}

/*
 * Constuctor #2:  Used on 2nd+ boot: EddystoneService parameters derived from persistent storage
 */
EddystoneService::EddystoneService(BLE                 &bleIn,
                                   EddystoneParams_t   &paramsIn,
                                   const PowerLevels_t &radioTxPowerLevelsIn,
                                   event_queue_t       &evQ,
                                   uint32_t            advConfigIntervalIn) :
    ble(bleIn),
    operationMode(EDDYSTONE_MODE_NONE),
    uidFrame(),
    urlFrame(),
    tlmFrame(),
    eidFrame(),
    tlmBatteryVoltageCallback(NULL),
    tlmBeaconTemperatureCallback(NULL),
    radioManagerCallbackHandle(NULL),
    deviceName(DEFAULT_DEVICE_NAME),
    eventQueue(evQ),
    nextEidSlot(0)
{
    LOG(("2nd (>=) Boot: "));
    LOG((BUILD_VERSION_STR));
    // Init time Params
    LOG(("Init Params\r\n"));
    timeSinceBootTimer.start();
    memcpy(&timeParams, &(paramsIn.timeParams), sizeof(TimeParams_t));
    LOG(("2nd Boot: Time:"));
    LOG(("PriorBoots=%lu, SinceBoot=%lu\r\n", timeParams.timeInPriorBoots, timeParams.timeSinceLastBoot));
    timeParams.timeInPriorBoots = timeParams.timeInPriorBoots + timeParams.timeSinceLastBoot;
    timeParams.timeSinceLastBoot =  getTimeSinceLastBootMs() / 1000;
    nvmSaveTimeParams();
    
    // Init gneeral params
    memcpy(capabilities, paramsIn.capabilities, sizeof(Capability_t));
    activeSlot          = paramsIn.activeSlot;
    memcpy(radioTxPowerLevels, radioTxPowerLevelsIn, sizeof(PowerLevels_t));
    memcpy(slotRadioTxPowerLevels, paramsIn.slotRadioTxPowerLevels, sizeof(SlotTxPowerLevels_t));
    memcpy(advTxPowerLevels,   paramsIn.advTxPowerLevels,   sizeof(PowerLevels_t));
    memcpy(slotAdvTxPowerLevels, paramsIn.slotAdvTxPowerLevels, sizeof(SlotTxPowerLevels_t));
    memcpy(slotAdvIntervals,   paramsIn.slotAdvIntervals,   sizeof(SlotAdvIntervals_t));
    lockState           = paramsIn.lockState;
    memcpy(unlockKey,   paramsIn.unlockKey,   sizeof(Lock_t));
    memcpy(unlockToken, paramsIn.unlockToken, sizeof(Lock_t));
    memcpy(challenge, paramsIn.challenge, sizeof(Lock_t));
    memset(slotCallbackHandles, 0, sizeof(SlotCallbackHandles_t));
    memcpy(slotStorage, paramsIn.slotStorage, sizeof(SlotStorage_t));
    memcpy(slotFrameTypes, paramsIn.slotFrameTypes, sizeof(SlotFrameTypes_t));
    memcpy(slotEidRotationPeriodExps, paramsIn.slotEidRotationPeriodExps, sizeof(SlotEidRotationPeriodExps_t));
    memcpy(slotEidIdentityKeys, paramsIn.slotEidIdentityKeys, sizeof(SlotEidIdentityKeys_t));
    // Zero next EID slot rotation times to enforce rotation of each slot on restart
    memset(slotEidNextRotationTimes, 0, sizeof(SlotEidNextRotationTimes_t)); 
    remainConnectable   = paramsIn.remainConnectable;

    if (advConfigIntervalIn != 0) {
        if (advConfigIntervalIn < ble.gap().getMinAdvertisingInterval()) {
            advConfigInterval = ble.gap().getMinAdvertisingInterval();
        } else if (advConfigIntervalIn > ble.gap().getMaxAdvertisingInterval()) {
            advConfigInterval = ble.gap().getMaxAdvertisingInterval();
        } else {
            advConfigInterval = advConfigIntervalIn;
        }
    }
    
    // Generate fresh private and public ECDH keys for EID
    genEIDBeaconKeys();

    // Recompute EID Slot Data
    for (int slot = 0; slot < MAX_ADV_SLOTS; slot++) {
        uint8_t* frame = slotToFrame(slot);
        switch (slotFrameTypes[slot]) {
            case EDDYSTONE_FRAME_EID:
               nextEidSlot = slot;
               eidFrame.setData(frame, slotAdvTxPowerLevels[slot], nullEid);
               eidFrame.update(frame, slotEidIdentityKeys[slot], slotEidRotationPeriodExps[slot], getTimeSinceFirstBootSecs());
               break;
        }
    }
    
    /* Set the device name at startup */
    ble.gap().setDeviceName(reinterpret_cast<const uint8_t *>(deviceName));
}

// Regenerate the beacon keys
void EddystoneService::genEIDBeaconKeys(void) {
    genBeaconKeyRC = -1;
#ifdef GEN_BEACON_KEYS_AT_INIT
    memset(privateEcdhKey, 0, 32);
    memset(publicEcdhKey, 0, 32);
    genBeaconKeyRC = eidFrame.genBeaconKeys(privateEcdhKey, publicEcdhKey);
    swapEndianArray(publicEcdhKey, publicEcdhKeyLE, 32);
#endif
}

/**
 * Factory reset all parmeters: used at initial boot, and activated from Char 11
 */
void EddystoneService::doFactoryReset(void)
{    
    // Init Time tracking
    timeSinceBootTimer.start();
    timeParams.timeInPriorBoots = 0;
    timeParams.timeSinceLastBoot = getTimeSinceLastBootMs() / 1000;
    nvmSaveTimeParams();
    // Init callbacks
    memset(slotCallbackHandles, 0, sizeof(SlotCallbackHandles_t));
    radioManagerCallbackHandle = NULL;
    memcpy(capabilities, CAPABILITIES_DEFAULT, CAP_HDR_LEN);
    // Line above leaves powerlevels blank; Line below fills them in
    memcpy(capabilities + CAP_HDR_LEN, radioTxPowerLevels, sizeof(PowerLevels_t));
    activeSlot = DEFAULT_SLOT;
    // Intervals
    uint16_t buf1[] = EDDYSTONE_DEFAULT_SLOT_INTERVALS;
    for (int i = 0; i < MAX_ADV_SLOTS; i++) {
            // Ensure all slot periods are in range
            buf1[i] = correctAdvertisementPeriod(buf1[i]);
    }
    memcpy(slotAdvIntervals, buf1, sizeof(SlotAdvIntervals_t));
    // Radio and Adv TX Power
    int8_t buf2[] = EDDYSTONE_DEFAULT_SLOT_TX_POWERS;
    for (int i = 0; i< MAX_ADV_SLOTS; i++) {
      slotRadioTxPowerLevels[i] = buf2[i];
      slotAdvTxPowerLevels[i] = advTxPowerLevels[radioTxPowerToIndex(buf2[i])];
    }
    // Lock
    lockState      = UNLOCKED;
    uint8_t defKeyBuf[] = EDDYSTONE_DEFAULT_UNLOCK_KEY;
    memcpy(unlockKey,        defKeyBuf,     sizeof(Lock_t));
    memset(unlockToken,      0,     sizeof(Lock_t));
    memset(challenge,        0,     sizeof(Lock_t)); // NOTE: challenge is randomized on first unlockChar read;

    // Generate ECDH Beacon Key Pair (Private/Public)
    genEIDBeaconKeys();
    
    memcpy(slotEidIdentityKeys, slotDefaultEidIdentityKeys, sizeof(SlotEidIdentityKeys_t));
    uint8_t buf4[] = EDDYSTONE_DEFAULT_SLOT_EID_ROTATION_PERIOD_EXPS;
    memcpy(slotEidRotationPeriodExps, buf4, sizeof(SlotEidRotationPeriodExps_t));
    memset(slotEidNextRotationTimes, 0, sizeof(SlotEidNextRotationTimes_t));
    //  Slot Data Type Defaults
    uint8_t buf3[] = EDDYSTONE_DEFAULT_SLOT_TYPES;
    memcpy(slotFrameTypes, buf3, sizeof(SlotFrameTypes_t));
    // Initialize Slot Data Defaults
    int eidSlot;
    for (int slot = 0; slot < MAX_ADV_SLOTS; slot++) {
        uint8_t* frame = slotToFrame(slot);
        switch (slotFrameTypes[slot]) {
            case EDDYSTONE_FRAME_UID:
               uidFrame.setData(frame, slotAdvTxPowerLevels[slot], reinterpret_cast<const uint8_t*>(slotDefaultUids[slot]));
               break;
            case EDDYSTONE_FRAME_URL:
               urlFrame.setUnencodedUrlData(frame, slotAdvTxPowerLevels[slot], slotDefaultUrls[slot]);
               break;
            case EDDYSTONE_FRAME_TLM:
               tlmFrame.setTLMData(TLMFrame::DEFAULT_TLM_VERSION);
               tlmFrame.setData(frame);
               eidSlot = getEidSlot();
               if (eidSlot != NO_EID_SLOT_SET) {
                   LOG(("EID slot Set in FactoryReset\r\n"));
                   tlmFrame.encryptData(frame, slotEidIdentityKeys[eidSlot], slotEidRotationPeriodExps[eidSlot], getTimeSinceFirstBootSecs());
               }
               break;
            case EDDYSTONE_FRAME_EID:
               nextEidSlot = slot;
               eidFrame.setData(frame, slotAdvTxPowerLevels[slot], nullEid);
               eidFrame.update(frame, slotEidIdentityKeys[slot], slotEidRotationPeriodExps[slot], getTimeSinceFirstBootSecs());
               break;
        }
    }

#ifdef DONT_REMAIN_CONNECTABLE
    remainConnectable = REMAIN_CONNECTABLE_UNSET;  
#else
    remainConnectable = REMAIN_CONNECTABLE_SET; 
#endif
    factoryReset = false;
}

/* Setup callback to update BatteryVoltage in TLM frame */
void EddystoneService::onTLMBatteryVoltageUpdate(TlmUpdateCallback_t tlmBatteryVoltageCallbackIn)
{
    tlmBatteryVoltageCallback = tlmBatteryVoltageCallbackIn;
}

/* Setup callback to update BeaconTemperature in TLM frame */
void EddystoneService::onTLMBeaconTemperatureUpdate(TlmUpdateCallback_t tlmBeaconTemperatureCallbackIn)
{
    tlmBeaconTemperatureCallback = tlmBeaconTemperatureCallbackIn;
}

EddystoneService::EddystoneError_t EddystoneService::startEddystoneBeaconAdvertisements(void)
{
    stopEddystoneBeaconAdvertisements();

    bool intervalValidFlag = false;
    for (int i = 0; i < MAX_ADV_SLOTS; i++) {
        if (slotAdvIntervals[i] != 0) {
            intervalValidFlag = true;
        }
    }

    if (!intervalValidFlag) {
        /* Nothing to do, the period is 0 for all frames */
        return EDDYSTONE_ERROR_INVALID_ADVERTISING_INTERVAL;
    }

    // In case left over from Config Adv Mode
    ble.gap().clearScanResponse();

    operationMode = EDDYSTONE_MODE_BEACON;

    /* Configure advertisements initially at power of active slot*/
    ble.gap().setTxPower(slotRadioTxPowerLevels[activeSlot]);

    if (remainConnectable) {
        ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    } else {
         ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_NON_CONNECTABLE_UNDIRECTED);
    }
    ble.gap().setAdvertisingInterval(ble.gap().getMaxAdvertisingInterval());

    /* Make sure the queue is currently empty */
    advFrameQueue.reset();
    /* Setup callbacks to periodically add frames to be advertised to the queue and
     * add initial frame so that we have something to advertise on startup */
    for (int slot = 0; slot < MAX_ADV_SLOTS; slot++) {
        uint8_t* frame = slotToFrame(slot);
        if (slotAdvIntervals[slot] && testValidFrame(frame)) {
            advFrameQueue.push(slot);
            slotCallbackHandles[slot] = eventQueue.post_every(
                &EddystoneService::enqueueFrame, this, slot,
                slotAdvIntervals[slot] /* ms */
            );
        }
    }
    /* Start advertising */
    manageRadio();

    return EDDYSTONE_ERROR_NONE;
}

ble_error_t EddystoneService::setCompleteDeviceName(const char *deviceNameIn)
{
    /* Make sure the device name is safe */
    ble_error_t error = ble.gap().setDeviceName(reinterpret_cast<const uint8_t *>(deviceNameIn));
    if (error == BLE_ERROR_NONE) {
        deviceName = deviceNameIn;
        if (operationMode == EDDYSTONE_MODE_CONFIG) {
            /* Need to update the advertising packets to the new name */
            setupEddystoneConfigScanResponse();
        }
    }

    return error;
}

/* It is not the responsibility of the Eddystone implementation to store
 * the configured parameters in persistent storage since this is
 * platform-specific. So we provide this function that returns the
 * configured values that need to be stored and the main application
 * takes care of storing them.
 */
void EddystoneService::getEddystoneParams(EddystoneParams_t &params)
{
    // Time
    timeParams.timeSinceLastBoot =  getTimeSinceLastBootMs() / 1000;
    memcpy(&(params.timeParams),     &timeParams,         sizeof(TimeParams_t));
    // Capabilities
    memcpy(params.capabilities,     capabilities,           sizeof(Capability_t));
    // Active Slot
    params.activeSlot                = activeSlot;
    // Intervals
    memcpy(params.slotAdvIntervals, slotAdvIntervals,       sizeof(SlotAdvIntervals_t));
    // Power Levels
    memcpy(params.radioTxPowerLevels, radioTxPowerLevels,   sizeof(PowerLevels_t));
    memcpy(params.advTxPowerLevels,   advTxPowerLevels,     sizeof(PowerLevels_t));
    // Slot Power Levels
    memcpy(params.slotRadioTxPowerLevels, slotRadioTxPowerLevels,   sizeof(MAX_ADV_SLOTS));
    memcpy(params.slotAdvTxPowerLevels,   slotAdvTxPowerLevels,     sizeof(MAX_ADV_SLOTS));
    // Lock
    params.lockState                = lockState;
    memcpy(params.unlockKey,        unlockKey,              sizeof(Lock_t));
    memcpy(params.unlockToken,      unlockToken,            sizeof(Lock_t));
    memcpy(params.challenge,        challenge,              sizeof(Lock_t));
    // Slots
    memcpy(params.slotFrameTypes,   slotFrameTypes,         sizeof(SlotFrameTypes_t));
    memcpy(params.slotStorage,      slotStorage,            sizeof(SlotStorage_t));
    memcpy(params.slotEidRotationPeriodExps, slotEidRotationPeriodExps, sizeof(SlotEidRotationPeriodExps_t));
    memcpy(params.slotEidIdentityKeys, slotEidIdentityKeys, sizeof(SlotEidIdentityKeys_t));
    // Testing and Management
    params.remainConnectable        = remainConnectable;
}

void EddystoneService::swapAdvertisedFrame(int slot)
{
    uint8_t* frame = slotToFrame(slot);
    uint8_t frameType = slotFrameTypes[slot];
    uint32_t timeSecs = getTimeSinceFirstBootSecs();
    switch (frameType) {
        case EDDYSTONE_FRAME_UID:
            updateAdvertisementPacket(uidFrame.getAdvFrame(frame), uidFrame.getAdvFrameLength(frame));
            break;
        case EDDYSTONE_FRAME_URL:
            updateAdvertisementPacket(urlFrame.getAdvFrame(frame), urlFrame.getAdvFrameLength(frame));
            break;
        case EDDYSTONE_FRAME_TLM:
            updateRawTLMFrame(frame);
            updateAdvertisementPacket(tlmFrame.getAdvFrame(frame), tlmFrame.getAdvFrameLength(frame));
            break;
        case EDDYSTONE_FRAME_EID:
            // only update the frame if the rotation period is due
            if (timeSecs >= slotEidNextRotationTimes[slot]) {
                eidFrame.update(frame, slotEidIdentityKeys[slot], slotEidRotationPeriodExps[slot], timeSecs);
                slotEidNextRotationTimes[slot] = timeSecs + (1 << slotEidRotationPeriodExps[slot]);
                // select a new random MAC address so the beacon is not trackable 
                setRandomMacAddress(); 
                // Store in NVM in case the beacon loses power
                nvmSaveTimeParams(); 
                LOG(("EID ROTATED: Time=%lu\r\n", timeSecs));
            }
            updateAdvertisementPacket(eidFrame.getAdvFrame(frame), eidFrame.getAdvFrameLength(frame));
            break;
        default:
            //Some error occurred
            error("Frame to swap in does not specify a valid type");
            break;
    }
    ble.gap().setTxPower(slotRadioTxPowerLevels[slot]);
}


/* Helper function that calls user-defined functions to update Battery Voltage and Temperature (if available),
 * then updates the raw frame data and finally updates the actual advertised packet. This operation must be
 * done fairly often because the TLM frame TimeSinceBoot must have a 0.1 secs resolution according to the
 * Eddystone specification.
 */
void EddystoneService::updateRawTLMFrame(uint8_t* frame)
{
    if (tlmBeaconTemperatureCallback != NULL) {
        tlmFrame.updateBeaconTemperature((*tlmBeaconTemperatureCallback)(tlmFrame.getBeaconTemperature()));
    }
    if (tlmBatteryVoltageCallback != NULL) {
        tlmFrame.updateBatteryVoltage((*tlmBatteryVoltageCallback)(tlmFrame.getBatteryVoltage()));
    }
    tlmFrame.updateTimeSinceLastBoot(getTimeSinceLastBootMs());
    tlmFrame.setData(frame);
    int slot = getEidSlot();
    LOG(("TLMHelper Method slot=%d\r\n", slot));
    if (slot != NO_EID_SLOT_SET) {
        LOG(("TLMHelper: Before Encrypting TLM\r\n"));
        tlmFrame.encryptData(frame, slotEidIdentityKeys[slot], slotEidRotationPeriodExps[slot], getTimeSinceFirstBootSecs());
        LOG(("TLMHelper: Before Encrypting TLM\r\n"));
    }
}

void EddystoneService::updateAdvertisementPacket(const uint8_t* rawFrame, size_t rawFrameLength)
{
    ble.gap().clearAdvertisingPayload();
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, EDDYSTONE_UUID, sizeof(EDDYSTONE_UUID));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, rawFrame, rawFrameLength);
}

uint8_t* EddystoneService::slotToFrame(int slot)
{
   return reinterpret_cast<uint8_t *>(&slotStorage[slot * sizeof(Slot_t)]);
}

void EddystoneService::enqueueFrame(int slot)
{
    advFrameQueue.push(slot);
    if (!radioManagerCallbackHandle) {
        /* Advertising stopped and there is not callback posted in the event queue. Just
         * execute the manager to resume advertising */
        manageRadio();
    }
}

void EddystoneService::manageRadio(void)
{
    uint8_t slot;
    uint64_t  startTimeManageRadio = getTimeSinceLastBootMs();

    /* Signal that there is currently no callback posted */
    radioManagerCallbackHandle = NULL;

    if (advFrameQueue.pop(slot)) {
        /* We have something to advertise */
        if (ble.gap().getState().advertising) {
            ble.gap().stopAdvertising();
        }
        swapAdvertisedFrame(slot);
        ble.gap().startAdvertising();

        /* Increase the advertised packet count in TLM frame */
        tlmFrame.updatePduCount();

        /* Post a callback to itself to stop the advertisement or pop the next
         * frame from the queue. However, take into account the time taken to
         * swap in this frame. */
        radioManagerCallbackHandle = eventQueue.post_in(
            &EddystoneService::manageRadio, this,
            ble.gap().getMinNonConnectableAdvertisingInterval() - (getTimeSinceLastBootMs() - startTimeManageRadio) /* ms */
        );
    } else if (ble.gap().getState().advertising) {
        /* Nothing else to advertise, stop advertising and do not schedule any callbacks */
        ble.gap().stopAdvertising();
    }
}

void EddystoneService::startEddystoneConfigService(void)
{
    uint16_t beAdvInterval = swapEndian(slotAdvIntervals[activeSlot]);
    int8_t radioTxPower = slotRadioTxPowerLevels[activeSlot];
    int8_t advTxPower = slotAdvTxPowerLevels[activeSlot];
    uint8_t* slotData = slotToFrame(activeSlot) + 1;
    aes128Encrypt(unlockKey, slotEidIdentityKeys[activeSlot], encryptedEidIdentityKey);

    capabilitiesChar      = new ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(Capability_t)>(UUID_CAPABILITIES_CHAR, capabilities);
    activeSlotChar        = new ReadWriteGattCharacteristic<uint8_t>(UUID_ACTIVE_SLOT_CHAR, &activeSlot);
    advIntervalChar       = new ReadWriteGattCharacteristic<uint16_t>(UUID_ADV_INTERVAL_CHAR, &beAdvInterval);
    radioTxPowerChar      = new ReadWriteGattCharacteristic<int8_t>(UUID_RADIO_TX_POWER_CHAR, &radioTxPower);
    advTxPowerChar        = new ReadWriteGattCharacteristic<int8_t>(UUID_ADV_TX_POWER_CHAR, &advTxPower);
    lockStateChar         = new GattCharacteristic(UUID_LOCK_STATE_CHAR, &lockState, sizeof(uint8_t), sizeof(LockState_t), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE);
    unlockChar            = new ReadWriteArrayGattCharacteristic<uint8_t, sizeof(Lock_t)>(UUID_UNLOCK_CHAR, unlockToken);
    publicEcdhKeyChar     = new GattCharacteristic(UUID_PUBLIC_ECDH_KEY_CHAR, publicEcdhKey, 0, sizeof(PublicEcdhKey_t), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
    eidIdentityKeyChar    = new GattCharacteristic(UUID_EID_IDENTITY_KEY_CHAR, encryptedEidIdentityKey, 0, sizeof(EidIdentityKey_t), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
    advSlotDataChar       = new GattCharacteristic(UUID_ADV_SLOT_DATA_CHAR, slotData, 0, 34, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE);
    factoryResetChar      = new WriteOnlyGattCharacteristic<uint8_t>(UUID_FACTORY_RESET_CHAR, &factoryReset);
    remainConnectableChar = new ReadWriteGattCharacteristic<uint8_t>(UUID_REMAIN_CONNECTABLE_CHAR, &remainConnectable);

    // CHAR-1 capabilities (READ ONLY)
    capabilitiesChar->setReadAuthorizationCallback(this, &EddystoneService::readBasicTestLockAuthorizationCallback);
    // CHAR-2 Active Slot
    activeSlotChar->setReadAuthorizationCallback(this, &EddystoneService::readBasicTestLockAuthorizationCallback);
    activeSlotChar->setWriteAuthorizationCallback(this, &EddystoneService::writeActiveSlotAuthorizationCallback<uint8_t>);
    // CHAR-3 Adv Interval
    advIntervalChar->setReadAuthorizationCallback(this, &EddystoneService::readAdvIntervalAuthorizationCallback);
    advIntervalChar->setWriteAuthorizationCallback(this, &EddystoneService::writeBasicAuthorizationCallback<uint16_t>);
    // CHAR-4  Radio TX Power
    radioTxPowerChar->setReadAuthorizationCallback(this, &EddystoneService::readRadioTxPowerAuthorizationCallback);
    radioTxPowerChar->setWriteAuthorizationCallback(this, &EddystoneService::writeBasicAuthorizationCallback<uint8_t>);
    // CHAR-5
    advTxPowerChar->setReadAuthorizationCallback(this, &EddystoneService::readAdvTxPowerAuthorizationCallback);
    advTxPowerChar->setWriteAuthorizationCallback(this, &EddystoneService::writeBasicAuthorizationCallback<uint8_t>);
    // CHAR-6 Lock State
    lockStateChar->setWriteAuthorizationCallback(this, &EddystoneService::writeLockStateAuthorizationCallback);
    // CHAR-7 Unlock
    unlockChar->setReadAuthorizationCallback(this, &EddystoneService::readUnlockAuthorizationCallback);
    unlockChar->setWriteAuthorizationCallback(this, &EddystoneService::writeUnlockAuthorizationCallback);
    // CHAR-8 Public Ecdh Key (READ ONLY)
    publicEcdhKeyChar->setReadAuthorizationCallback(this, &EddystoneService::readPublicEcdhKeyAuthorizationCallback);
    // CHAR-9 EID Identity Key (READ ONLY)
    eidIdentityKeyChar->setReadAuthorizationCallback(this, &EddystoneService::readEidIdentityAuthorizationCallback);
    // CHAR-10 Adv Slot Data
    advSlotDataChar->setReadAuthorizationCallback(this, &EddystoneService::readDataAuthorizationCallback);  
    advSlotDataChar->setWriteAuthorizationCallback(this, &EddystoneService::writeVarLengthDataAuthorizationCallback);
    // CHAR-11 Factory Reset
    factoryResetChar->setReadAuthorizationCallback(this, &EddystoneService::readBasicTestLockAuthorizationCallback);
    factoryResetChar->setWriteAuthorizationCallback(this, &EddystoneService::writeBasicAuthorizationCallback<bool>);
    // CHAR-12 Remain Connectable
    remainConnectableChar->setReadAuthorizationCallback(this, &EddystoneService::readBasicTestLockAuthorizationCallback);
    remainConnectableChar->setWriteAuthorizationCallback(this, &EddystoneService::writeBasicAuthorizationCallback<bool>);

    // Create pointers to all characteristics in the GATT service
    charTable[0] = capabilitiesChar;
    charTable[1] = activeSlotChar;
    charTable[2] = advIntervalChar;
    charTable[3] = radioTxPowerChar;
    charTable[4] = advTxPowerChar;
    charTable[5] = lockStateChar;
    charTable[6] = unlockChar;
    charTable[7] = publicEcdhKeyChar;
    charTable[8] = eidIdentityKeyChar;
    charTable[9] = advSlotDataChar;
    charTable[10] = factoryResetChar;
    charTable[11] = remainConnectableChar;

    GattService configService(UUID_ES_BEACON_SERVICE, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));

    ble.gattServer().addService(configService);
    ble.gattServer().onDataWritten(this, &EddystoneService::onDataWrittenCallback);
    updateCharacteristicValues();
}


void EddystoneService::freeConfigCharacteristics(void)
{
    delete capabilitiesChar;
    delete activeSlotChar;
    delete advIntervalChar;
    delete radioTxPowerChar;
    delete advTxPowerChar;
    delete lockStateChar;
    delete unlockChar;
    delete publicEcdhKeyChar;
    delete eidIdentityKeyChar;
    delete advSlotDataChar;
    delete factoryResetChar;
    delete remainConnectableChar;
}

void EddystoneService::stopEddystoneBeaconAdvertisements(void)
{
    /* Unschedule callbacks */

    for (int slot = 0; slot < MAX_ADV_SLOTS; slot++) {
        if (slotCallbackHandles[slot]) {
            eventQueue.cancel(slotCallbackHandles[slot]);
            slotCallbackHandles[slot] = NULL;
        }
    }

    if (radioManagerCallbackHandle) {
        eventQueue.cancel(radioManagerCallbackHandle);
        radioManagerCallbackHandle = NULL;
    }

    /* Stop any current Advs (ES Config or Beacon) */
    BLE::Instance().gap().stopAdvertising();
}

/*
 * Internal helper function used to update the GATT database following any
 * change to the internal state of the service object.
 */
void EddystoneService::updateCharacteristicValues(void)
{
    // Init variables for update
    uint16_t beAdvInterval = swapEndian(slotAdvIntervals[activeSlot]);
    int8_t radioTxPower = slotRadioTxPowerLevels[activeSlot];
    int8_t advTxPower = slotAdvTxPowerLevels[activeSlot];
    uint8_t* frame = slotToFrame(activeSlot);
    uint8_t slotLength = 0;
    uint8_t* slotData = NULL;
    memset(encryptedEidIdentityKey, 0, sizeof(encryptedEidIdentityKey));

    switch(slotFrameTypes[activeSlot]) {
        case EDDYSTONE_FRAME_UID:
          slotLength = uidFrame.getDataLength(frame);
          slotData = uidFrame.getData(frame);
          break;
        case EDDYSTONE_FRAME_URL:
          slotLength = urlFrame.getDataLength(frame);
          slotData = urlFrame.getData(frame);
          break;
        case EDDYSTONE_FRAME_TLM:
          updateRawTLMFrame(frame);
          slotLength = tlmFrame.getDataLength(frame);
          slotData = tlmFrame.getData(frame);
          break;
        case EDDYSTONE_FRAME_EID:
          slotLength = eidFrame.getDataLength(frame);
          slotData = eidFrame.getData(frame);
          aes128Encrypt(unlockKey, slotEidIdentityKeys[activeSlot], encryptedEidIdentityKey);
          break;
    }

    ble.gattServer().write(capabilitiesChar->getValueHandle(), reinterpret_cast<uint8_t *>(capabilities), sizeof(Capability_t));
    ble.gattServer().write(activeSlotChar->getValueHandle(), &activeSlot, sizeof(uint8_t));
    ble.gattServer().write(advIntervalChar->getValueHandle(), reinterpret_cast<uint8_t *>(&beAdvInterval), sizeof(uint16_t));
    ble.gattServer().write(radioTxPowerChar->getValueHandle(), reinterpret_cast<uint8_t *>(&radioTxPower), sizeof(int8_t));
    ble.gattServer().write(advTxPowerChar->getValueHandle(), reinterpret_cast<uint8_t *>(&advTxPower), sizeof(int8_t));
    ble.gattServer().write(lockStateChar->getValueHandle(), &lockState, sizeof(uint8_t));
    ble.gattServer().write(unlockChar->getValueHandle(), unlockToken, sizeof(Lock_t));
    ble.gattServer().write(publicEcdhKeyChar->getValueHandle(), reinterpret_cast<uint8_t *>(publicEcdhKey), sizeof(PublicEcdhKey_t));
    ble.gattServer().write(eidIdentityKeyChar->getValueHandle(), reinterpret_cast<uint8_t *>(encryptedEidIdentityKey), sizeof(EidIdentityKey_t));
    ble.gattServer().write(advSlotDataChar->getValueHandle(), slotData, slotLength);
    ble.gattServer().write(factoryResetChar->getValueHandle(), &factoryReset, sizeof(uint8_t));
    ble.gattServer().write(remainConnectableChar->getValueHandle(), &remainConnectable, sizeof(uint8_t));
}

EddystoneService::EddystoneError_t EddystoneService::startEddystoneConfigAdvertisements(void)
{
    stopEddystoneBeaconAdvertisements();

    if (advConfigInterval == 0) {
        // Nothing to do, the advertisement interval is 0
        return EDDYSTONE_ERROR_INVALID_ADVERTISING_INTERVAL;
    }

    operationMode = EDDYSTONE_MODE_CONFIG;

    ble.gap().clearAdvertisingPayload();

    /* Accumulate the new payload */
    // Add the Flags param
    ble.gap().accumulateAdvertisingPayload(
        GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE
    );
#ifdef INCLUDE_CONFIG_URL
    // Add the Eddystone 16-bit Service ID
    ble.gap().accumulateAdvertisingPayload(
        GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, 
        EDDYSTONE_UUID, 
        sizeof(EDDYSTONE_UUID)
    );
#endif
    /* UUID is in different order in the ADV frame (!) */
    uint8_t reversedServiceUUID[sizeof(UUID_ES_BEACON_SERVICE)];
    for (size_t i = 0; i < sizeof(UUID_ES_BEACON_SERVICE); i++) {
        reversedServiceUUID[i] = UUID_ES_BEACON_SERVICE[sizeof(UUID_ES_BEACON_SERVICE) - i - 1];
    }
    ble.gap().accumulateAdvertisingPayload(
        GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
        reversedServiceUUID,
        sizeof(reversedServiceUUID)
    );
    // Add Generic Appearance Tag
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_TAG);
    setupEddystoneConfigScanResponse();

    ble.gap().setTxPower(radioTxPowerLevels[sizeof(PowerLevels_t)-1]); // Max Power for Config
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(advConfigInterval);
    ble.gap().startAdvertising();

    return EDDYSTONE_ERROR_NONE;
}

void EddystoneService::setupEddystoneConfigScanResponse(void)
{
    ble.gap().clearScanResponse();
    // Add LOCAL NAME (indicating the Eddystone Version)
    ble.gap().accumulateScanResponse(
        GapAdvertisingData::COMPLETE_LOCAL_NAME,
        reinterpret_cast<const uint8_t *>(deviceName),
        strlen(deviceName)
    );
#ifdef INCLUDE_CONFIG_URL 
    // Add SERVICE DATA for a PhyWeb Config URL
    uint8_t configFrame[URLFrame::ENCODED_BUF_SIZE];
    int encodedUrlLen = URLFrame::encodeURL(configFrame + CONFIG_FRAME_HDR_LEN, EDDYSTONE_CONFIG_URL);
    uint8_t advPower = advTxPowerLevels[sizeof(PowerLevels_t)-1] & 0xFF;
    uint8_t configFrameHdr[CONFIG_FRAME_HDR_LEN] = {0, 0, URLFrame::FRAME_TYPE_URL, advPower};
    // ++ Fill in the Eddystone Service UUID in the HDR
    memcpy(configFrameHdr, EDDYSTONE_UUID, sizeof(EDDYSTONE_UUID));
    // ++ Copy the HDR to the config frame 
    memcpy(configFrame, configFrameHdr, CONFIG_FRAME_HDR_LEN);
    ble.gap().accumulateScanResponse(
        GapAdvertisingData::SERVICE_DATA,
        configFrame,
        CONFIG_FRAME_HDR_LEN + encodedUrlLen
    );
#else
    // Add TRANSMIT POWER
    ble.gap().accumulateScanResponse(
    GapAdvertisingData::TX_POWER_LEVEL,
    reinterpret_cast<uint8_t *>(&advTxPowerLevels[sizeof(PowerLevels_t)-1]),
    sizeof(uint8_t)
    );
#endif
}

/* WRITE AUTHORIZATION */

void EddystoneService::writeUnlockAuthorizationCallback(GattWriteAuthCallbackParams *authParams)
{
    if (lockState == UNLOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    } else if (authParams->len != sizeof(Lock_t)) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
    } else if (authParams->offset != 0) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
    } else if (memcmp(authParams->data, unlockToken, sizeof(Lock_t)) != 0) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void EddystoneService::writeVarLengthDataAuthorizationCallback(GattWriteAuthCallbackParams *authParams)
{
   if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    } else if (authParams->len > 34) {  
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}


void EddystoneService::writeLockStateAuthorizationCallback(GattWriteAuthCallbackParams *authParams)
{
   if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    } else if ((authParams->len != sizeof(uint8_t)) && (authParams->len != (sizeof(uint8_t) + sizeof(Lock_t)))) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
    } else if (authParams->offset != 0) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

template <typename T>
void EddystoneService::writeBasicAuthorizationCallback(GattWriteAuthCallbackParams *authParams)
{
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    } else if (authParams->len != sizeof(T)) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
    } else if (authParams->offset != 0) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

template <typename T>
void EddystoneService::writeActiveSlotAuthorizationCallback(GattWriteAuthCallbackParams *authParams)
{
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    } else if (authParams->len != sizeof(T)) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
    } else if (*(authParams->data) > MAX_ADV_SLOTS -1) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
    } else if (authParams->offset != 0) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

/* READ AUTHORIZTION */

void EddystoneService::readBasicTestLockAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ BASIC TEST LOCK slot=%d\r\n", activeSlot));
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void EddystoneService::readEidIdentityAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ EID IDENTITY slot=%d\r\n", activeSlot));
    aes128Encrypt(unlockKey, slotEidIdentityKeys[activeSlot], encryptedEidIdentityKey);
    int sum = 0;
    // Test if the IdentityKey is all zeros for this slot
    for (uint8_t i = 0; i < sizeof(EidIdentityKey_t); i++) {
        sum = sum + slotEidIdentityKeys[activeSlot][i];
    }
    ble.gattServer().write(eidIdentityKeyChar->getValueHandle(), encryptedEidIdentityKey, sizeof(EidIdentityKey_t));

    // When the array is all zeros, the key has not been set, so return fault
    if ((lockState == LOCKED) || (sum == 0)) { 
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void EddystoneService::readPublicEcdhKeyAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ BEACON PUBLIC ECDH KEY (LE) slot=%d\r\n", activeSlot));

    ble.gattServer().write(publicEcdhKeyChar->getValueHandle(), publicEcdhKeyLE, sizeof(PublicEcdhKey_t));
    
    // When the array is all zeros, the key has not been set, so return fault
    if (lockState == LOCKED) { 
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
    } else {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void EddystoneService::readDataAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ ADV-DATA : slot=%d\r\n", activeSlot));
    uint8_t frameType = slotFrameTypes[activeSlot];
    uint8_t* frame = slotToFrame(activeSlot);
    uint8_t slotLength = 1;
    uint8_t buf[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t* slotData = buf;
 
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
        return;
    }
    LOG(("IN READ ADV-DATA AFTER LOCK TEST frameType=%d\r\n", frameType));
    if (testValidFrame(frame) ) { // Check the frame has valid data before proceeding
        switch(frameType) {
            case EDDYSTONE_FRAME_UID:
                LOG(("READ ADV-DATA UID SLOT DATA slot=%d\r\n", activeSlot));
                slotLength = uidFrame.getDataLength(frame);
                slotData = uidFrame.getData(frame);
                break;
            case EDDYSTONE_FRAME_URL:
                LOG(("READ ADV-DATA URL SLOT DATA slot=%d\r\n", activeSlot));
                slotLength = urlFrame.getDataLength(frame);
                slotData = urlFrame.getData(frame);
                break;
            case EDDYSTONE_FRAME_TLM:
                LOG(("READ ADV-DATA TLM SLOT DATA slot=%d\r\n", activeSlot));
                updateRawTLMFrame(frame);
                slotLength = tlmFrame.getDataLength(frame);
                slotData = tlmFrame.getData(frame);
                LOG(("READ ADV-DATA AFTER T/E TLM length=%d\r\n", slotLength)); 
                LOG(("Data=")); logPrintHex(slotData, 18);
                break;
            case EDDYSTONE_FRAME_EID:
                LOG(("READ ADV-DATA EID SLOT DATA slot=%d\r\n", activeSlot));
                slotLength = 14;
                buf[0] = EIDFrame::FRAME_TYPE_EID;
                buf[1] = slotEidRotationPeriodExps[activeSlot];
                // Add time as a big endian 32 bit number
                uint32_t timeSecs = getTimeSinceFirstBootSecs();
                buf[2] = (timeSecs  >> 24) & 0xff;
                buf[3] = (timeSecs >> 16) & 0xff;
                buf[4] = (timeSecs >> 8) & 0xff;
                buf[5] = timeSecs & 0xff;
                memcpy(buf + 6, eidFrame.getEid(frame), 8);
                slotData = buf;
                break;
        }
    }
    LOG(("IN READ ADV-DATA AFTER FRAME PROCESSING slot=%d\r\n", activeSlot));
    ble.gattServer().write(advSlotDataChar->getValueHandle(), slotData, slotLength);
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

bool EddystoneService::testValidFrame(uint8_t* frame) {
    return (frame[0] != 0 ) ? true : false; 
}

void EddystoneService::readUnlockAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ UNLOCK slot=%d\r\n", activeSlot));
    if (lockState == UNLOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
        return;
    }
    // Update the challenge ready for the characteristic read
    generateRandom(challenge, sizeof(Lock_t));
    aes128Encrypt(unlockKey, challenge, unlockToken);
    ble.gattServer().write(unlockChar->getValueHandle(), reinterpret_cast<uint8_t *>(challenge), sizeof(Lock_t));     
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

void EddystoneService::readAdvIntervalAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ ADV INTERVAL slot=%d\r\n", activeSlot));
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
        return;
    }
    uint16_t beAdvInterval = swapEndian(slotAdvIntervals[activeSlot]);
    ble.gattServer().write(advIntervalChar->getValueHandle(), reinterpret_cast<uint8_t *>(&beAdvInterval), sizeof(uint16_t));
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

void EddystoneService::readRadioTxPowerAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ RADIO TXPOWER slot=%d\r\n", activeSlot));
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
        return;
    }
    int8_t radioTxPower = slotRadioTxPowerLevels[activeSlot];
    ble.gattServer().write(radioTxPowerChar->getValueHandle(), reinterpret_cast<uint8_t *>(&radioTxPower), sizeof(int8_t));
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

void EddystoneService::readAdvTxPowerAuthorizationCallback(GattReadAuthCallbackParams *authParams)
{
    LOG(("\r\nDO READ ADV TXPOWER slot=%d\r\n", activeSlot));
    if (lockState == LOCKED) {
        authParams->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_READ_NOT_PERMITTED;
        return;
    }
    int8_t advTxPower = slotAdvTxPowerLevels[activeSlot];
    ble.gattServer().write(advTxPowerChar->getValueHandle(), reinterpret_cast<uint8_t *>(&advTxPower), sizeof(int8_t));
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

/*
 * This callback is invoked when a GATT client attempts to modify any of the
 * characteristics of this service. Attempts to do so are also applied to
 * the internal state of this service object.
 */
void EddystoneService::onDataWrittenCallback(const GattWriteCallbackParams *writeParams)
{
    uint16_t handle = writeParams->handle;
    LOG(("\r\nDO WRITE: Handle=%d Len=%d\r\n", handle, writeParams->len));
    // CHAR-1 CAPABILITIES
            /* capabilitySlotChar is READ ONLY */
    // CHAR-2 ACTIVE SLOT
    if (handle == activeSlotChar->getValueHandle()) {
        LOG(("Write: Active Slot Handle=%d\r\n", handle));
        uint8_t slot = *(writeParams->data);
        LOG(("Active Slot=%d\r\n", slot));
        // Ensure slot does not exceed limit, or set highest slot
        if (slot < MAX_ADV_SLOTS) {
            activeSlot = slot;
        }
        ble.gattServer().write(activeSlotChar->getValueHandle(), &activeSlot, sizeof(uint8_t));
    // CHAR-3 ADV INTERVAL
    } else if (handle == advIntervalChar->getValueHandle()) {
        LOG(("Write: Interval Handle=%d\r\n", handle));
        uint16_t interval = correctAdvertisementPeriod(swapEndian(*((uint16_t *)(writeParams->data))));
        slotAdvIntervals[activeSlot] = interval; // Store this value for reading
        uint16_t beAdvInterval = swapEndian(slotAdvIntervals[activeSlot]);
        ble.gattServer().write(advIntervalChar->getValueHandle(), reinterpret_cast<uint8_t *>(&beAdvInterval), sizeof(uint16_t));
    // CHAR-4 RADIO TX POWER
    } else if (handle == radioTxPowerChar->getValueHandle()) {
        LOG(("Write: RADIO Power Handle=%d\r\n", handle));
        int8_t radioTxPower = *(writeParams->data);
        uint8_t index = radioTxPowerToIndex(radioTxPower);
        radioTxPower = radioTxPowerLevels[index]; // Power now corrected to nearest allowed power
        slotRadioTxPowerLevels[activeSlot] = radioTxPower; // Store by slot number
        int8_t advTxPower = advTxPowerLevels[index]; // Determine adv power equivalent
        slotAdvTxPowerLevels[activeSlot] = advTxPower;
        setFrameTxPower(activeSlot, advTxPower); // Set the actual frame radio TxPower for this slot
        ble.gattServer().write(radioTxPowerChar->getValueHandle(), reinterpret_cast<uint8_t *>(&radioTxPower), sizeof(int8_t));
    // CHAR-5 ADV TX POWER
    } else if (handle == advTxPowerChar->getValueHandle()) {
        LOG(("Write: ADV Power Handle=%d\r\n", handle));
        int8_t advTxPower = *(writeParams->data);
        slotAdvTxPowerLevels[activeSlot] = advTxPower;
        setFrameTxPower(activeSlot, advTxPower); // Update the actual frame Adv TxPower for this slot
        ble.gattServer().write(advTxPowerChar->getValueHandle(), reinterpret_cast<uint8_t *>(&advTxPower), sizeof(int8_t));
    // CHAR-6 LOCK STATE
    } else if (handle == lockStateChar->getValueHandle()) {
        LOG(("Write: Lock State Handle=%d\r\n", handle));
        uint8_t newLockState = *(writeParams->data);
        if ((writeParams->len == sizeof(uint8_t)) || (writeParams->len == sizeof(uint8_t) + sizeof(Lock_t))) {
            if ((newLockState == LOCKED) || (newLockState == UNLOCKED) || (newLockState == UNLOCKED_AUTO_RELOCK_DISABLED)) {
                lockState = newLockState;
            }
        }
        if ((newLockState == LOCKED) && (writeParams->len == (sizeof(uint8_t) + sizeof(Lock_t))) ) {
            // And sets the new secret lock code if present
            uint8_t encryptedNewKey[sizeof(Lock_t)];
            uint8_t newKey[sizeof(Lock_t)];
            memcpy(encryptedNewKey, (writeParams->data)+1, sizeof(Lock_t));
            // Decrypt the new key
            aes128Decrypt(unlockKey, encryptedNewKey, newKey);
            memcpy(unlockKey, newKey, sizeof(Lock_t));
        }
        ble.gattServer().write(lockStateChar->getValueHandle(), reinterpret_cast<uint8_t *>(&lockState), sizeof(uint8_t));
    // CHAR-7 UNLOCK
    } else if (handle == unlockChar->getValueHandle()) {
       LOG(("Write: Unlock Handle=%d\r\n", handle));
       // NOTE: Actual comparison with unlock code is done in:
       // writeUnlockAuthorizationCallback(...)  which is executed before this method call.
       lockState = UNLOCKED;
       // Regenerate challenge and expected unlockToken for Next unlock operation
       generateRandom(challenge, sizeof(Lock_t));
       aes128Encrypt(unlockKey, challenge, unlockToken);
       // Update Chars
       ble.gattServer().write(unlockChar->getValueHandle(), reinterpret_cast<uint8_t *>(challenge), sizeof(Lock_t));      // Update the challenge
       ble.gattServer().write(lockStateChar->getValueHandle(), reinterpret_cast<uint8_t *>(&lockState), sizeof(uint8_t)); // Update the lock
    // CHAR-8 PUBLIC ECDH KEY
        /* PublicEchdChar is READ ONLY */
    // CHAR-9 EID INDENTITY KEY
        /* EidIdentityChar is READ ONLY */
    // CHAR-10 ADV DATA
    } else if (handle == advSlotDataChar->getValueHandle()) {
        LOG(("Write: Adv Slot DATA Handle=%d\r\n", handle));
        uint8_t* frame = slotToFrame(activeSlot);
        int8_t advTxPower = slotAdvTxPowerLevels[activeSlot];
        uint8_t writeFrameFormat = *(writeParams->data);
        uint8_t writeFrameLen = (writeParams->len);
        uint8_t writeData[34];
        uint8_t serverPublicEcdhKey[32];
        
        if (writeFrameLen != 0) {
            writeFrameLen--; // Remove the Format byte from the count
        } else {
            writeFrameFormat = UNDEFINED_FRAME_FORMAT; // Undefined format
        }
        
        memcpy(writeData, (writeParams->data) + 1, writeFrameLen);
        LOG(("ADV Data Write=%d,%d\r\n", writeFrameFormat, writeFrameLen));
        switch(writeFrameFormat) {
            case UIDFrame::FRAME_TYPE_UID:
                if (writeFrameLen == 16) {
                    uidFrame.setData(frame, advTxPower,reinterpret_cast<const uint8_t *>((writeParams->data) + 1));
                    slotFrameTypes[activeSlot] = EDDYSTONE_FRAME_UID;
                } else if (writeFrameLen == 0) {
                    uidFrame.clearFrame(frame);
                }
                break;
            case URLFrame::FRAME_TYPE_URL:
               if (writeFrameLen <= 18) {
                    urlFrame.setData(frame, advTxPower, reinterpret_cast<const uint8_t*>((writeParams->data) + 1), writeFrameLen );
                    slotFrameTypes[activeSlot] = EDDYSTONE_FRAME_URL;
                } else if (writeFrameLen == 0) {
                    urlFrame.clearFrame(frame);
                }
                break;
            case TLMFrame::FRAME_TYPE_TLM:
                if (writeFrameLen == 0) {
                    updateRawTLMFrame(frame);
                    tlmFrame.setData(frame);
                    int slot = getEidSlot();
                    LOG(("WRITE: Testing if TLM or ETLM=%d\r\n", slot));
                    if (slot != NO_EID_SLOT_SET) {
                        LOG(("WRITE: Configuring ETLM Slot time(S)=%lu\r\n", getTimeSinceFirstBootSecs() ));
                        tlmFrame.encryptData(frame, slotEidIdentityKeys[slot], slotEidRotationPeriodExps[slot], getTimeSinceFirstBootSecs() );
                    }
                    slotFrameTypes[activeSlot] = EDDYSTONE_FRAME_TLM;
                }
                break;
            case EIDFrame::FRAME_TYPE_EID:
                LOG(("EID Len=%d\r\n", writeFrameLen));
                if (writeFrameLen == 17) {
                    // Least secure
                    LOG(("EID Insecure branch\r\n"));
                    aes128Decrypt(unlockKey, writeData, slotEidIdentityKeys[activeSlot]);
                    slotEidRotationPeriodExps[activeSlot] = writeData[16]; // index 16 is the exponent
                    ble.gattServer().write(eidIdentityKeyChar->getValueHandle(), reinterpret_cast<uint8_t *>(&writeData), sizeof(EidIdentityKey_t));
                } else if (writeFrameLen == 33 ) {  
                    // Most secure
                    memcpy(serverPublicEcdhKey, writeData, 32);
                    ble.gattServer().write(publicEcdhKeyChar->getValueHandle(), reinterpret_cast<uint8_t *>(&serverPublicEcdhKey), sizeof(PublicEcdhKey_t));
                    LOG(("ServerPublicEcdhKey=")); logPrintHex(serverPublicEcdhKey, 32);
                    slotEidRotationPeriodExps[activeSlot] = writeData[32]; // index 32 is the exponent
                    LOG(("Exponent=%i\r\n", writeData[32]));
                    LOG(("genBeaconKeyRC=%x\r\n", genBeaconKeyRC));
                    LOG(("BeaconPrivateEcdhKey=")); logPrintHex(privateEcdhKey, 32);
                    LOG(("BeaconPublicEcdhKey=")); logPrintHex(publicEcdhKey, 32);
                    LOG(("genECDHShareKey\r\n"));
                    int rc = eidFrame.genEcdhSharedKey(privateEcdhKey, publicEcdhKey, serverPublicEcdhKey, slotEidIdentityKeys[activeSlot]);
                    LOG(("Gen Keys RC = %x\r\n", rc));
                    LOG(("Generated eidIdentityKey=")); logPrintHex(slotEidIdentityKeys[activeSlot], 16);
                    aes128Encrypt(unlockKey, slotEidIdentityKeys[activeSlot], encryptedEidIdentityKey);
                    LOG(("encryptedEidIdentityKey=")); logPrintHex(encryptedEidIdentityKey, 16);      
                    ble.gattServer().write(eidIdentityKeyChar->getValueHandle(), reinterpret_cast<uint8_t *>(&encryptedEidIdentityKey), sizeof(EidIdentityKey_t));
                } else if (writeFrameLen == 0) {
                    // Reset eidFrame
                    eidFrame.clearFrame(frame);
                    break;
                } else {
                    break; // Do nothing, this is not a recognized Frame length
                }
                // Establish the new frame type
                slotFrameTypes[activeSlot] = EDDYSTONE_FRAME_EID;
                nextEidSlot = activeSlot; // This was the last one updated
                LOG(("update Eid Frame\r\n"));
                // Generate EID ADV frame packet 
                eidFrame.setData(frame, advTxPower, nullEid);
                // Fill in the correct EID Value from the Identity Key/exp/clock
                eidFrame.update(frame, slotEidIdentityKeys[activeSlot], slotEidRotationPeriodExps[activeSlot], getTimeSinceFirstBootSecs() );
                LOG(("END update Eid Frame\r\n"));
                break;
            default:
                frame[0] = 0; // Frame format unknown so clear the entire frame by writing 0 to its length
                break;
        }
        // Read takes care of setting the Characteristic  Value
    // CHAR-11 FACTORY RESET
    } else if (handle == factoryResetChar->getValueHandle() && (*((uint8_t *)writeParams->data) != 0)) {
        LOG(("Write: Factory Reset: Handle=%d\r\n", handle));
        // Reset params to default values
        doFactoryReset();
        // Update all characteristics based on params
        updateCharacteristicValues();
    // CHAR-12 REMAIN CONNECTABLE
    } else if (handle == remainConnectableChar->getValueHandle()) {
        LOG(("Write: Remain Connectable Handle=%d\r\n", handle));
        remainConnectable = *(writeParams->data);
        ble.gattServer().write(remainConnectableChar->getValueHandle(), &remainConnectable, sizeof(uint8_t));
    }

}

void EddystoneService::setFrameTxPower(uint8_t slot, int8_t advTxPower) {
    uint8_t* frame = slotToFrame(slot);
    uint8_t frameType = slotFrameTypes[slot] << 4; // Converting the enum to an actual frame type
    switch (frameType) {
        case UIDFrame::FRAME_TYPE_UID:
           uidFrame.setAdvTxPower(frame, advTxPower);
           break;
        case URLFrame::FRAME_TYPE_URL:
           urlFrame.setAdvTxPower(frame, advTxPower);
           break;
        case EIDFrame::FRAME_TYPE_EID:
           eidFrame.setAdvTxPower(frame, advTxPower);
           break;
    }
}

uint8_t EddystoneService::radioTxPowerToIndex(int8_t txPower) {
    // NOTE: txPower is an 8-bit signed number
    uint8_t size = sizeof(PowerLevels_t);
    // Look for the value in range (or next biggest value)
    for (uint8_t i = 0; i < size; i++) {
      if (txPower <= radioTxPowerLevels[i]) {
          return i;
      }
    }
    return size - 1;
}

/** AES128 encrypts a 16-byte input array with a key, resulting in a 16-byte output array */
void EddystoneService::aes128Encrypt(uint8_t key[], uint8_t input[], uint8_t output[]) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key, 8 * sizeof(Lock_t));
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, input, output);
    mbedtls_aes_free(&ctx);
}

/** AES128 decrypts a 16-byte input array with a key, resulting in a 16-byte output array */
void EddystoneService::aes128Decrypt(uint8_t key[], uint8_t input[], uint8_t output[]) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_dec(&ctx, key, 8 * sizeof(Lock_t));
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, input, output);
    mbedtls_aes_free(&ctx);
}



#ifdef HARDWARE_RANDOM_NUM_GENERATOR
// Generates a set of random values in byte array[size] based on hardware source
void EddystoneService::generateRandom(uint8_t ain[], int size) {
    mbedtls_entropy_context entropy;
    mbedtls_entropy_init(&entropy);
    // init entropy source
    eddystoneRegisterEntropySource(&entropy);
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
    mbedtls_ctr_drbg_random(&ctr_drbg, ain, size);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return;
}
#else
// Generates a set of random values in byte array[size] seeded by the clock(ms)
void EddystoneService::generateRandom(uint8_t ain[], int size) {
    int i;
    // Random seed based on boot time in milliseconds
    srand(getTimeSinceLastBootMs());
    for (i = 0; i < size; i++) {
        ain[i] = rand() % 256;
    }
    return;
}
#endif

/** Reverse Even sized Array endianess: Big to Little or Little to Big */
void EddystoneService::swapEndianArray(uint8_t ptrIn[], uint8_t ptrOut[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        ptrOut[i] = ptrIn[size - i - 1];
    }
    return;
}

/** Reverse endianess: Big to Little or Little to Big */
uint16_t EddystoneService::swapEndian(uint16_t arg) {
    return (arg / 256) + (arg % 256) * 256;
}

uint16_t EddystoneService::correctAdvertisementPeriod(uint16_t beaconPeriodIn) const
{
    /* Re-map beaconPeriod to within permissible bounds if necessary. */
    if (beaconPeriodIn != 0) {
        if (beaconPeriodIn < ble.gap().getMinNonConnectableAdvertisingInterval()) {
            return ble.gap().getMinNonConnectableAdvertisingInterval();
        } else if (beaconPeriodIn > ble.gap().getMaxAdvertisingInterval()) {
            return ble.gap().getMaxAdvertisingInterval();
        }
    }
    return beaconPeriodIn;
}

void EddystoneService::logPrintHex(uint8_t* a, int len) {
    for (int i = 0; i < len; i++) {
        LOG(("%x%x", a[i] >> 4, a[i] & 0x0f ));
    }
    LOG(("\r\n"));
}

void EddystoneService::setRandomMacAddress(void) {
#ifdef EID_RANDOM_MAC
    uint8_t macAddress[6]; // 48 bit Mac Address
    generateRandom(macAddress, 6);
    macAddress[5] |= 0xc0; // Ensure upper two bits are 11's for Random Add
    ble.setAddress(BLEProtocol::AddressType::RANDOM_STATIC, macAddress);
#endif
}

int EddystoneService::getEidSlot(void) {
    int eidSlot = NO_EID_SLOT_SET; // by default;
    for (int i = 0; i < MAX_ADV_SLOTS; i++) {
        if (slotFrameTypes[nextEidSlot] == EDDYSTONE_FRAME_EID) {
             eidSlot = nextEidSlot;
             nextEidSlot = (nextEidSlot-1) % MAX_ADV_SLOTS;
             break;
        }
        nextEidSlot = (nextEidSlot-1) % MAX_ADV_SLOTS; // ensure the slot numbers wrap
    }
    return eidSlot;
}

bool EddystoneService::isLocked(void) {
    if (lockState == LOCKED) {
	return true;
    } else {
	return false;
    }
}

/**
 * Time : Stable Storage
 */

/**
 * Returns the time since FIRST Boot (Time in Prior Boots + Time since Last Boot) in SECONDS
 */
uint32_t EddystoneService::getTimeSinceFirstBootSecs(void) {
    timeParams.timeSinceLastBoot = getTimeSinceLastBootMs() / 1000;
    uint32_t totalTimeSinceFirstBoot = timeParams.timeSinceLastBoot + timeParams.timeInPriorBoots;
    // Timer Overflow condition = 136 years (32 bits in seconds) so no need for wrap check
    return totalTimeSinceFirstBoot;
}

/**
 * Returns the time since last boot in MILLISECONDS
 * NOTE: This solution is needed as a stopgap until the Timer API is updated to 64-bit
 */
uint64_t EddystoneService::getTimeSinceLastBootMs(void) {
    static uint64_t time64bit = 0;
    time64bit += timeSinceBootTimer.read_ms();
    timeSinceBootTimer.reset();
    return time64bit;
}

/**
 * Store only the time params in Pstorage(e.g. NVM), to maintain time between boots
 * NOTE: Platform-specific implementation for persistence on the nRF5x. Based on the
 * pstorage module provided by the Nordic SDK. 
 */
void EddystoneService::nvmSaveTimeParams(void) {
    LOG(("Time NVM: "));
    LOG(("PriorBoots=%lu, SinceBoot=%lu\r\n", timeParams.timeInPriorBoots, timeParams.timeSinceLastBoot));
    saveEddystoneTimeParams(&timeParams);
}

/*
 * Establish constant arrays
 */
const uint8_t EddystoneService::slotDefaultUids[MAX_ADV_SLOTS][16] = EDDYSTONE_DEFAULT_SLOT_UIDS;

const uint8_t EddystoneService::slotDefaultEidIdentityKeys[MAX_ADV_SLOTS][16] = EDDYSTONE_DEFAULT_SLOT_EID_IDENTITY_KEYS;

const uint8_t EddystoneService::nullEid[8] = {0,0,0,0,0,0,0,0};



