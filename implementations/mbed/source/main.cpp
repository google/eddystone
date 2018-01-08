/* 
 * Copyright (c) 2006-2016 Google Inc, All Rights Reserved
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

#ifdef YOTTA_CFG_MBED_OS  // use minar on mbed OS
#   include "mbed-drivers/mbed.h"
#else
#   include "mbed.h"
#endif

#include "ble/BLE.h"
#include "EddystoneService.h"

#include "PersistentStorageHelper/ConfigParamsPersistence.h"
#include "stdio.h"

#if (defined(NRF51) || defined(NRF52))
    #include "nrf_soc.h"
#endif

// Instantiation of the main event loop for this program

#ifdef YOTTA_CFG_MBED_OS  // use minar on mbed OS
#   include "EventQueue/EventQueueMinar.h"
    typedef eq::EventQueueMinar event_queue_t;

#else      // otherwise use the event classic queue
#   include "EventQueue/EventQueueClassic.h"
    typedef eq::EventQueueClassic<
        /* event count */ 10
    > event_queue_t;

#endif

static event_queue_t eventQueue;

EddystoneService *eddyServicePtr;

/* Duration after power-on that config service is available. */
static const int CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS = EDDYSTONE_DEFAULT_CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS;

/* Values for ADV packets related to firmware levels, calibrated based on measured values at 1m */
static const PowerLevels_t advTxPowerLevels = EDDYSTONE_DEFAULT_ADV_TX_POWER_LEVELS;
/* Values for radio power levels, provided by manufacturer. */
static const PowerLevels_t radioTxPowerLevels = EDDYSTONE_DEFAULT_RADIO_TX_POWER_LEVELS;

DigitalOut configLED(CONFIG_LED, LED_OFF);

static const int BLINKY_MSEC = 500;                       // How long to cycle config LED on/off
static event_queue_t::event_handle_t handle = 0;         // For the config mode timeout
static event_queue_t::event_handle_t BlinkyHandle = 0;   // For the blinking LED when in config mode

static void blinky(void)  { configLED = !configLED; }

static void configLED_on(void) {
    configLED = !LED_OFF;
    BlinkyHandle = eventQueue.post_every(blinky, BLINKY_MSEC);
}

static void configLED_off(void) {
    configLED = LED_OFF;
    if (BlinkyHandle) {
        eventQueue.cancel(BlinkyHandle);
        BlinkyHandle = NULL;
    }
}

/**
 * Callback triggered some time after application started to switch to beacon mode.
 */
static void timeoutToStartEddystoneBeaconAdvertisements(void)
{
    Gap::GapState_t state;
    state = BLE::Instance().gap().getState();
    if (!state.connected) { /* don't switch if we're in a connected state. */
        eddyServicePtr->startEddystoneBeaconAdvertisements();
        configLED_off();
    }
}

/**
 * Callback triggered for a connection event.
 */
static void connectionCallback(const Gap::ConnectionCallbackParams_t *cbParams)
{
    (void) cbParams;
    // Stop advertising whatever the current mode
    eddyServicePtr->stopEddystoneBeaconAdvertisements();
}

/**
 * Callback triggered for a disconnection event.
 */
static void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *cbParams)
{
    (void) cbParams;
    BLE::Instance().gap().startAdvertising();
    // Save params in persistent storage
    EddystoneService::EddystoneParams_t params;
    eddyServicePtr->getEddystoneParams(params);
    saveEddystoneServiceConfigParams(&params);
    // Ensure LED is off at the end of Config Mode or during a connection
    configLED_off();
    // 0.5 Second callback to rapidly re-establish Beaconing Service
    // (because it needs to be executed outside of disconnect callback)
    eventQueue.post_in(timeoutToStartEddystoneBeaconAdvertisements, 500 /* ms */);
}

// This section defines a simple push button handler to enter config or shutdown the beacon
// Only compiles if "reset_button" is set in config.json in the "platform" section
//
#ifdef RESET_BUTTON

InterruptIn button(RESET_BUTTON);
DigitalOut shutdownLED(SHUTDOWN_LED, LED_OFF);

static void shutdownLED_on(void) { shutdownLED = !LED_OFF; }
static void shutdownLED_off(void) { shutdownLED = LED_OFF; }

static bool beaconIsOn = true;   // Button handler boolean to switch on or off
static bool buttonBusy;          // semaphore to make prevent switch bounce problems

static void freeButtonBusy(void) { buttonBusy = false; }

// Callback used to handle button presses from thread mode (not IRQ)
static void button_task(void) {
    bool locked = eddyServicePtr->isLocked();

    // only shutdown if ON and unlocked
    if (beaconIsOn && !locked) {
	eventQueue.cancel(handle);   // kill any pending callback tasks
	beaconIsOn = false;
	eddyServicePtr->stopEddystoneBeaconAdvertisements();
	configLED_off();    // just in case it's still running...
	shutdownLED_on();   // Flash shutdownLED to let user know we're turning off
	eventQueue.post_in(shutdownLED_off, 1000);
    // only go into configMode if OFF or locked and not in configMode
    } else if (!beaconIsOn || (locked && BlinkyHandle == NULL)) {
	eventQueue.cancel(handle); // kill any pending callback tasks
        beaconIsOn = true;
        eddyServicePtr->startEddystoneConfigAdvertisements();
        configLED_on();
        handle = eventQueue.post_in(
            timeoutToStartEddystoneBeaconAdvertisements,
            CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS * 1000 /* ms */
        );
    }
    eventQueue.post_in(freeButtonBusy, 750 /* ms */);
}

/**
 * Raw IRQ handler for the reset button. We don't want to actually do any work here.
 * Instead, we queue work to happen later using an event queue, by posting a callback.
 * This has the added avantage of serialising actions, so if the button press happens
 * during the config->beacon mode transition timeout, the button_task won't happen
 * until the previous task has finished.
 *
 * If your buttons aren't debounced, you should do this in software, or button_task
 * might get queued multiple times.
 */
static void reset_rise(void)
{
    if (!buttonBusy) {
        buttonBusy = true;
        eventQueue.post(button_task);
    }
}
#endif

static void onBleInitError(BLE::InitializationCompleteCallbackContext* initContext)
{
    /* Initialization error handling goes here... */
    (void) initContext;
}


static void bleInitComplete(BLE::InitializationCompleteCallbackContext* initContext)
{
    BLE         &ble  = initContext->ble;
    ble_error_t error = initContext->error;

    if (error != BLE_ERROR_NONE) {
        onBleInitError(initContext);
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);

    ble.gap().onConnection(connectionCallback);

    EddystoneService::EddystoneParams_t params;

    wait_ms(35); // Allow the RNG number generator to collect data

    // Determine if booting directly after re-Flash or not
    if (loadEddystoneServiceConfigParams(&params)) {
        // 2+ Boot after reflash, so get parms from Persistent Storage
        eddyServicePtr = new EddystoneService(ble, params, radioTxPowerLevels, eventQueue);
    } else {
        // 1st Boot after reflash, so reset everything to defaults
        /* NOTE: slots are initialized in the constructor from the config.json file */
        eddyServicePtr = new EddystoneService(ble, advTxPowerLevels, radioTxPowerLevels, eventQueue);
    }

    // Save Default params in persistent storage ready for next boot event
    eddyServicePtr->getEddystoneParams(params);
    saveEddystoneServiceConfigParams(&params);
    // Start the Eddystone Config service - This will never stop (only connectability will change)
    eddyServicePtr->startEddystoneConfigService();

    /* Start Eddystone config Advertizements (to initialize everything properly) */
    configLED_on();
    eddyServicePtr->startEddystoneConfigAdvertisements();
    handle = eventQueue.post_in(
        timeoutToStartEddystoneBeaconAdvertisements,
        CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS * 1000 /* ms */
    );

#if (defined(NRF51) || defined(NRF52))
	sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);	// set the DCDC mode for the Nordic chip to lower power consumption
#endif
	
   // now shut everything off (used for final beacon that ships w/ battery)
#ifdef RESET_BUTTON
   eventQueue.post_in(button_task, 2000 /* ms */);
#endif
}

void app_start(int, char *[])
{

#ifdef NO_LOGGING
    /* Tell standard C library to not allocate large buffers for these streams */
    // setbuf(stdout, NULL);
    // setbuf(stderr, NULL);
    // setbuf(stdin, NULL);
#endif

#ifndef NO_4SEC_START_DELAY
    // delay ~4secs before starting to allow time the nRF51 hardware to settle
    // Also allows time to attach a virtual terimal to read logging output during init
    wait_ms(4000);
#endif
    
#ifdef RESET_BUTTON
    beaconIsOn = true;             // Booting up, initialize for button handler
    buttonBusy = false;         // software debouncing of the reset button
    button.rise(&reset_rise);   // setup reset button
#endif

    BLE &ble = BLE::Instance();
    ble.init(bleInitComplete);
}

#if !defined(YOTTA_CFG_MBED_OS)

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    eventQueue.post(&BLE::processEvents, &context->ble);
    }

int main() {

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);

    app_start(0, NULL);

    while (true) {
       eventQueue.dispatch();
       sleep();
    }

    return 0;
}


#endif
