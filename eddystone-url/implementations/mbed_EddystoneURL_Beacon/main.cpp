/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/services/EddystoneURLConfigService.h"
#include "ble/services/DFUService.h"
#include "ble/services/DeviceInformationService.h"
#include "ConfigParamsPersistence.h"

BLE ble;
EddystoneURLConfigService *eddystoneUrlConfig;

/**
 * Eddystone-URL beacons can operate in two modes: a configuration mode which
 * allows a user to update settings over a connection; and normal Eddystone URL Beacon mode
 * which involves advertising a URI. Constructing an object from the EddystoneURLConfigService
 * sets up advertisements for the configuration mode. It is then up to
 * the application to switch to the Eddystone-URL beacon mode based on some timeout.
 *
 * The following help with this switch.
 */
static const int CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS = 30;  // Duration after power-on that config service is available.
Ticker configAdvertisementTimeoutTicker;

/**
 * Stop advertising the Eddystone URL Config Service after a delay; and switch to normal Eddystone-URL advertisements.
 */
void timeout(void)
{
    Gap::GapState_t state;
    state = ble.getGapState();
    if (!state.connected) { /* don't switch if we're in a connected state. */
        configAdvertisementTimeoutTicker.detach(); /* disable the callback from the timeout Ticker. */
        eddystoneUrlConfig->setupEddystoneURLAdvertisements();
        ble.startAdvertising();
    }
}

/**
 * Callback triggered upon a disconnection event. Needs to re-enable advertisements.
 */
void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    configAdvertisementTimeoutTicker.detach(); /* disable the callback from the timeout Ticker. */
    eddystoneUrlConfig->setupEddystoneURLAdvertisements();
    ble.startAdvertising();
}

int main(void)
{
    ble.init();
    ble.onDisconnection(disconnectionCallback);

    /*
     * Load parameters from (platform specific) persistent storage. Parameters
     * can be set to non-default values while the Eddystone-URL beacon is in configuration
     * mode (within the first 60 seconds of power-up). Thereafter, parameters
     * get copied out to persistent storage before switching to normal EddystoneURL
     * operation.
     */
    EddystoneURLConfigService::Params_t params;
    bool fetchedFromPersistentStorage = loadEddystoneURLConfigParams(&params);

    /* Initialize a Eddystone URL Config service providing config params, default URI, and power levels. */
    static EddystoneURLConfigService::PowerLevels_t defaultAdvPowerLevels = {-20, -4, 0, 10}; // Values for ADV packets related to firmware levels
    eddystoneUrlConfig= new EddystoneURLConfigService(ble, params, !fetchedFromPersistentStorage, "http://physical-web.org", defaultAdvPowerLevels);
    if (!eddystoneUrlConfig->configuredSuccessfully()) {
        error("failed to accommodate URI");
    }
    configAdvertisementTimeoutTicker.attach(timeout, CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS);

    // Setup auxiliary services to allow over-the-air firmware updates, etc
    DFUService dfu(ble);
    DeviceInformationService deviceInfo(ble, "ARM", "Eddystone-URL", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");

    ble.startAdvertising(); /* Set the whole thing in motion. After this call a GAP central can scan the EddystoneURLConfig
                             * service. This can then be switched to the normal Eddystone-URL beacon functionality after a timeout. */

    while (true) {
        ble.waitForEvent();
    }
}
