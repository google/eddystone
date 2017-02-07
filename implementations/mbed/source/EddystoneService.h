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

#ifndef __EDDYSTONESERVICE_H__
#define __EDDYSTONESERVICE_H__
//
// 2016-03 Eddystone Unified GATT
//
#include "EventQueue/EventQueue.h"
#include "ble/BLE.h"
#include "EddystoneTypes.h"
#include "UIDFrame.h"
#include "URLFrame.h"
#include "TLMFrame.h"
#include "EIDFrame.h"
#include <string.h>
#include "mbedtls/aes.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
// #include "PersistentStorageHelper/ConfigParamsPersistence.h"

#ifdef YOTTA_CFG_MBED_OS
    #include "mbed-drivers/mbed.h"
    #include "mbed-drivers/CircularBuffer.h"
#else
    #include "mbed.h"
    #include "CircularBuffer.h"
#endif

#include "stdio.h"
#include "Eddystone_config.h"
#include "pstorage_platform.h"

/**
 * This class implements the Eddystone-URL Config Service and the Eddystone
 * Protocol Specification as defined in the publicly available specification at
 * https://github.com/google/eddystone/blob/master/protocol-specification.md.
 */
class EddystoneService
{
public:
    /**
     * Total number of GATT Characteristics in the Eddystonei-URL Configuration
     * Service.
     */
    static const uint16_t TOTAL_CHARACTERISTICS = 12;
    
    /**
     * Max data that can be written to the data characteristic
     */
    static const uint8_t MAX_DATA_WRITE = 34; // FrameType+32B(IdentityKey)+Exp

    /**
     * Default interval for advertising packets for the Eddystone-URL
     * Configuration Service.
     */
    static const uint32_t DEFAULT_CONFIG_PERIOD_MSEC    = EDDYSTONE_DEFAULT_CONFIG_ADV_INTERVAL;

    /**
     * Enumeration that defines the various operation modes of the
     * EddystoneService.
     *
     * @note The main app can change the mode of EddystoneService at any point
     *       of time by calling startConfigService() or startBeaconService().
     *       Resources from the previous mode will be freed.
     *
     * @note It is currently NOT possible to force EddystoneService back into
     *       EDDYSTONE_MODE_NONE.
     */
    enum OperationModes {
        /**
         * NONE: EddystoneService has been initialized but no memory has been
         * dynamically allocated. Additionally, no services are running
         * nothing is being advertised.
         */
        EDDYSTONE_MODE_NONE,
        /**
         * CONFIG: EddystoneService has been initialized, the configuration
         *         service started and memory has been allocated for BLE
         *         characteristics. Memory consumption peaks during CONFIG
         *         mode.
         */
        EDDYSTONE_MODE_CONFIG,
        /**
         * BEACON: Eddystone service is running as a beacon advertising URL,
         *         UID and/or TLM frames depending on how it is configured.
         */
        EDDYSTONE_MODE_BEACON
    };

    /**
     * Structure that encapsulates the Eddystone configuration parameters. This
     * structure is particularly useful when storing the parameters to
     * persistent storage.
     */
    struct EddystoneParams_t {
        /**
         * 
         */
        TimeParams_t            timeParams;
        /**
         * A buffer describing the capabilities of the beacon
         */
        Capability_t            capabilities;

         /**
         * Defines the slot that advInterval, radioPower, advPower, advSlotData operate on
         */
        uint8_t                 activeSlot;

        /**
         * The Beacon interval for each beacon slot
         *
         * @note A value of zero disables Eddystone-URL frame trasmissions.
         */
        SlotAdvIntervals_t      slotAdvIntervals;

         /**
         * The Radio TX Powers supported by this beacon
         */
        PowerLevels_t           radioTxPowerLevels;

         /**
         * The Radio TX Power set for each slot
         */
        SlotTxPowerLevels_t     slotRadioTxPowerLevels;

        /**
         * The Calibrated Adv TX Powers supported by this beacon (one for each radio power)
         */
        PowerLevels_t           advTxPowerLevels;

        /**
         * The Adv TX Power set for each slot
         */
        SlotTxPowerLevels_t     slotAdvTxPowerLevels;

        /**
         * The value of the Eddystone-URL Configuration Service Lock State
         * characteristic.
         */
        uint8_t                 lockState;

        /**
         * The value of the Eddystone-URL Configuration Service Unlock
         * characteristic that can be used to unlock the beacon and clear the
         * single-use lock-code.
         */
        Lock_t                  unlockToken;

        /**
         * An array holding the 128-bit unlockKey (big endian)
         */
        Lock_t                  unlockKey;

        /**
         * An array holding the 128-bit challenge (big endian) in the
         * challenge/response unlock protocol
         */
        Lock_t                  challenge;

        /**
         * EID: An array holding the slot rotation period exponents
         */
        SlotEidRotationPeriodExps_t     slotEidRotationPeriodExps;

        /**
         * EID: An array holding the slot 128-bit EID Identity Key (big endian)
         */
        SlotEidIdentityKeys_t           slotEidIdentityKeys;

        /**
         * Specifies the type of each frame indexed by slot
         */
        SlotFrameTypes_t    slotFrameTypes;

        /**
         * A buffer that contains all slot frames, 32-bytes allocated to each frame
         */
        SlotStorage_t       slotStorage;

         /**
         * The state of the recently invoked Factory Reset characteristic
         */
        uint8_t          factoryReset;

        /**
         * The state of the recently invoked Remain Connectable characteristic
         */
        uint8_t          remainConnectable;
    };

    /**
     * Enumeration that defines the various error codes for EddystoneService.
     */
    enum EddystoneError_t {
        /**
         * No error occurred.
         */
        EDDYSTONE_ERROR_NONE,
        /**
         * The supplied advertising interval is invalid. The interval may be
         * too short/long for the type of advertising packets being broadcast.
         *
         * @note For the acceptable range of advertising interval refer to the
         *       following functions in mbed BLE API:
         *       - Gap::getMinNonConnectableAdvertisingInterval()
         *       - Gap::getMinAdvertisingInterval()
         *       - Gap::getMaxAdvertisingInterval()
         */
        EDDYSTONE_ERROR_INVALID_ADVERTISING_INTERVAL,
        /**
         * The result of executing a call when the the EddystoneService is in
         * the incorrect operation mode.
         */
        EDDYSTONE_ERROR_INVALID_STATE
    };

    /**
     * Enumeration that defines the available frame types within Eddystone
     * advertising packets.
     */
    enum FrameType {
        /**
         * The Eddystone-UID frame. Refer to
         * https://github.com/google/eddystone/tree/master/eddystone-uid.
         */
        EDDYSTONE_FRAME_UID,
        /**
         * The Eddystone-URL frame. Refer to
         * https://github.com/google/eddystone/tree/master/eddystone-url.
         */
        EDDYSTONE_FRAME_URL,
        /**
         * The Eddystone-TLM frame. Refer to
         * https://github.com/google/eddystone/tree/master/eddystone-tlm.
         */
        EDDYSTONE_FRAME_TLM,
        /**
         * The Eddystone-EID frame. Refer to
         * https://github.com/google/eddystone/tree/master/eddystone-eid.
         */
        EDDYSTONE_FRAME_EID,
        /**
         * The total number Eddystone frame types.
         */
        NUM_EDDYSTONE_FRAMES
    };

    typedef eq::EventQueue event_queue_t;

    /**
     * Constructor that Initializes the EddystoneService using parameters from
     * the supplied EddystoneParams_t. This constructor is particularly useful
     * for configuring the EddystoneService with parameters fetched from
     * persistent storage.
     *
     * @param[in] bleIn
     *              The BLE instance.
     * @param[in] paramIn
     *              The input Eddystone configuration parameters.
     * @param[in] radioPowerLevelsIn
     *              The value set internally into the radion tx power.
     * @param[in] eventQueue
     *              The event queue used by the service to schedule tasks.
     * @param[in] advConfigIntervalIn
     *              The advertising interval for advertising packets of the
     *              Eddystone-URL Configuration Service.
     */
    EddystoneService(BLE                 &bleIn,
                     EddystoneParams_t   &paramsIn,
                     const PowerLevels_t &radioPowerLevelsIn,
                     event_queue_t       &eventQueue,
                     uint32_t            advConfigIntervalIn = DEFAULT_CONFIG_PERIOD_MSEC);

    /**
     * Constructor to initialize the EddystoneService to default values.
     *
     * @param[in] bleIn
     *              The BLE instance.
     * @param[in] advPowerLevelsIn
     *              The value of the Eddystone-URL Configuration Service TX
     *              Power Mode characteristic.
     * @param[in] radioPowerLevelsIn
     *              The value set internally into the radion tx power.
     * @param[in] eventQueue
     *              The event queue used by the service to schedule tasks.
     * @param[in] advConfigIntervalIn
     *              The advertising interval for advertising packets of the
     *              Eddystone-URL Configuration Service.
     *
     * @note When using this constructor the setURLData(), setTMLData() and
     *       setUIDData() and setEIDData() functions must be called to initialize
     *       EddystoneService manually.
     */
    EddystoneService(BLE                 &bleIn,
                     const PowerLevels_t &advPowerLevelsIn,
                     const PowerLevels_t &radioPowerLevelsIn,
                     event_queue_t       &eventQueue,
                     uint32_t            advConfigIntervalIn = DEFAULT_CONFIG_PERIOD_MSEC);
                     
          
    /**
     * Generate the EID Beacon Random ECHD Keys (private and Public)
     */                  
    void genEIDBeaconKeys(void);                

    /**
     * Factory Reset all parameters in the beacon
     */
    void doFactoryReset(void);

    /**
     * Setup callback to update BatteryVoltage in Eddystone-TLM frames
     *
     * @param[in] tlmBatteryVoltageCallbackIn
     *              The callback being registered.
     */
    void onTLMBatteryVoltageUpdate(TlmUpdateCallback_t tlmBatteryVoltageCallbackIn);

    /**
     * Setup callback to update BeaconTemperature in Eddystone-TLM frames
     *
     * @param[in] tlmBeaconTemperatureCallbackIn
     *              The callback being registered.
     */
    void onTLMBeaconTemperatureUpdate(TlmUpdateCallback_t tlmBeaconTemperatureCallbackIn);

    /**
     * Change the EddystoneService OperationMode to EDDYSTONE_MODE_CONFIG.
     *
     * @retval EDDYSTONE_ERROR_NONE if the operation succeeded.
     * @retval EDDYSONE_ERROR_INVALID_ADVERTISING_INTERVAL if the configured
     *         advertising interval is zero.
     *
     * @note If EddystoneService was previously in EDDYSTONE_MODE_BEACON, then
     *       the resources allocated to that mode of operation such as memory
     *       are freed and the BLE instance shutdown before the new operation
     *       mode is configured.
     */
    EddystoneError_t startConfigService(void);

    /**
     * Change the EddystoneService to start transmitting Eddystone beacons
     * operationMode = EDDYSTONE_MODE_BEACON
     *
     * @retval EDDYSTONE_ERROR_NONE if the operation succeeded.
     * @retval EDDYSONE_ERROR_INVALID_ADVERTISING_INTERVAL if the configured
     *         advertising interval is zero.
     *
     * @note If EddystoneService was previously in EDDYSTONE_MODE_CONFIG, then
     *       the resources allocated to that mode of operation such as memory
     *       are freed and the BLE instance shutdown before the new operation
     *       mode is configured.
     */
    EddystoneError_t startEddystoneBeaconAdvertisements(void);

    /**
     * Set the Comple Local Name for the BLE device. This not only updates
     * the value of the Device Name Characteristic, it also updates the scan
     * response payload if the EddystoneService is currently in
     * EDDYSTONE_MODE_CONFIG.
     *
     * @param[in] deviceNameIn
     *              A pointer to a null terminated string containing the new
     *              device name.
     *
     * @return BLE_ERROR_NONE if the name was successfully set. Otherwise an
     *         appropriate error.
     *
     * @note EddystoneService does not make an internal copy of the string
     *       pointed to by @p deviceNameIn. Therefore, the user is responsible
     *       for ensuring that the string persists in memory as long as it is
     *       in use by the EddystoneService.
     *
     * @note The device name is not considered an Eddystone configuration
     *       parameter; therefore, it is not contained within the
     *       EddystoneParams_t structure and must be stored to persistent
     *       storage separately.
     */
    ble_error_t setCompleteDeviceName(const char *deviceNameIn);

    /**
     * Get the Eddystone Configuration parameters. This is particularly useful
     * for storing the configuration parameters in persistent storage.
     * It is not the responsibility of the Eddystone implementation to store
     * the configured parameters in persistent storage since this is
     * platform-specific.
     *
     * @param[out] params
     *              A reference to an EddystoneParams_t structure with the
     *              configured parameters of the EddystoneService.
     */
    void getEddystoneParams(EddystoneParams_t &params);

    /**
     * Start advertising packets indicating the Eddystone Configuration state
     * operationMode = EDDYSTONE_MODE_CONFIG
     */
    EddystoneService::EddystoneError_t startEddystoneConfigAdvertisements(void);

    /**
     * Free the resources acquired by a call to setupBeaconService() and
     * cancel all pending callbacks that operate the radio and frame queue.
     *
     * @note This call will not modify the current state of the BLE device.
     *       EddystoneService::stopBeaconService should only be called after
     *       a call to BLE::shutdown().
     */
    void stopEddystoneBeaconAdvertisements(void);

    /**
     * Initialize and start the BLE Eddystone Configuration Service
     * This will create the 12-characteristics of the service and make them
     * available when a client connects
     */
    void startEddystoneConfigService();

    /**
     * Stops the Eddystone Configuration Service and frees its resources
     * and cancels all pending callbacks that operate the radio and frame queue.
     *
     * @note This call will not modify the current state of the BLE device.
     *       EddystoneService::stopBeaconService should only be called after
     *       a call to BLE::shutdown().
     */
    void stopEddystoneConfigService();

    /**
     * Tests if the beacon is locked or not
     *
     * @return bool
     */
    bool isLocked();
    
    /**
     * Print an array as a set of hex values 
     *
     * @param[in] a
     *              The array to be printed.
     * 
     * @param[in] len
     *              The length of the array.
     *
     * @return void
     *
     */
    static void logPrintHex(uint8_t* a, int len);
    
    /**
     * Swaps the endianess of an array ptrIn[size] to ptrOut[size]
     *
     * @param[in] *ptrIn
     *              The input array
     * @param[in] *ptrOut
     *              The output array
     * @param[in] size
     *              The sizes of the arrays (num bytes to be reversed)
     */
    static void swapEndianArray(uint8_t *ptrIn, uint8_t *ptrOut, int size);
    
    /**
     * Generate a random array of bytes of length size
     *
     * @param[in] *ain
     *              The input/output array
     * @param[in] size
     *              The size of the array in bytes
     */
    static void generateRandom(uint8_t *ain, int size);
    
    /**
     * Timer that keeps track of the time since boot.
     */
    static Timer        timeSinceBootTimer;
    
private:

    static const uint8_t NO_EID_SLOT_SET = 0xff;
     
    static const uint8_t UNDEFINED_FRAME_FORMAT = 0xff;
     
    static const uint8_t REMAIN_CONNECTABLE_SET = 0x01;
          
    static const uint8_t REMAIN_CONNECTABLE_UNSET = 0x00;
    
    static const uint8_t CONFIG_FRAME_HDR_LEN = 4;
     
    /**
     * Helper funtion that will be registered as an initialization complete
     * callback when BLE::shutdown() is called. This is necessary when changing
     * Eddystone OperationModes. Once the BLE initialization is complete, this
     * callback will initialize all the necessary resource to operate
     * Eddystone service in the selected mode.
     *
     * @param[in] initContext
     *              The context provided by BLE API when initialization
     *              completes.
     */
    void bleInitComplete(BLE::InitializationCompleteCallbackContext* initContext);

    /**
     * When in EDDYSTONE_MODE_BEACON this function is called to update the
     * advertising payload to contain the information related to the specified
     * FrameType.
     *
     * @param[in] slot
     *              The slot to populate the advertising payload with.
     */
    void swapAdvertisedFrame(int slot);

    /**
     * Helper function that manages the BLE radio that is used to broadcast
     * advertising packets. To advertise frames at the configured intervals
     * the actual advertising interval of the BLE instance is set to the value
     * returned by Gap::getMaxAdvertisingInterval() from the BLE API. When a
     * frame needs to be advertised, the enqueueFrame() callbacks add the frame
     * type to the advFrameQueue and post a manageRadio() callback. When the
     * callback is executed, the frame is dequeued and advertised using the
     * radio (by updating the advertising payload). manageRadio() also posts a
     * callback to itself Gap::getMinNonConnectableAdvertisingInterval()
     * milliseconds later. In this callback, manageRadio() will advertise the
     * next frame in the queue, yet if there is none it calls
     * Gap::stopAdvertising() and does not post any further callbacks.
     */
    void manageRadio(void);

    /**
     * Regular callbacks posted at the rate of slotAdvPeriod[slot] milliseconds
     * enqueue frames to be advertised. If the
     * frame queue is currently empty, then this function directly calls
     * manageRadio() to broadcast the required FrameType.
     *
     * @param[in] frameType
     *              The FrameType to enqueue for broadcasting.
     */
    void enqueueFrame(int slot);

    /**
     * Helper function that updates the advertising payload when in
     * EDDYSTONE_MODE_BEACON to contain a new frame.
     *
     * @param[in] rawFrame
     *              The raw bytes of the frame to advertise.
     * @param[in] rawFrameLength
     *              The length in bytes of the array pointed to by @p rawFrame.
     */
    void updateAdvertisementPacket(const uint8_t* rawFrame, size_t rawFrameLength);

    /**
     * Helper function that updates the information in the Eddystone-TLM frames
     * Internally, this function executes the registered callbacks to update
     * beacon Battery Voltage and Temperature (if available). Furthermore, this
     * function updates the raw frame data. This operation must be done fairly
     * often because the Eddystone-TLM frame Time Since Boot must have a 0.1
     * seconds resolution according to the Eddystone specification.
     */
    void updateRawTLMFrame(uint8_t* frame);

    /**
     * Calculate the Frame pointer from the slot number
     */
    uint8_t* slotToFrame(int slot);

    /**
     * Free the characteric resources acquired by a call to
     * startEddystoneConfigService().
     */
    void freeConfigCharacteristics(void);

    /**
     * Helper function used to update the GATT database following any
     * change to the internal state of the service object.
     */
    void updateCharacteristicValues(void);

    /**
     * Helper function to setup the payload of scan response packets for
     * Eddystone-URL Configuration Service.
     */
    void setupEddystoneConfigScanResponse(void);

    /**
     * Callback registered to the BLE API to authorize write operations to the
     * Eddystone Configuration Service Lock characteristic.
     *
     * @param[in] authParams
     *              Write authentication information.
     */
    void writeLockAuthorizationCallback(GattWriteAuthCallbackParams *authParams);

    /**
     * Callback registered to the BLE API to authorize write operations to the
     * Eddystone Configuration Service Unlock characteristic.
     *
     * @param[in] authParams
     *              Write authentication information.
     */
    void writeUnlockAuthorizationCallback(GattWriteAuthCallbackParams *authParams);

    /**
     * Callback registered to the BLE API to authorize write operations to the
     * Eddystone Configuration Service advSlotData characteristic.
     *
     * @param[in] authParams
     *              Write authentication information.
     */
    void writeVarLengthDataAuthorizationCallback(GattWriteAuthCallbackParams *authParams);

    /**
     * Callback registered to the BLE API to authorize write operations to the
     * lockState characteristic which can be 1 byte or 17 bytes long.
     *
     * @param[in] authParams
     *              Write authentication information.
     */
    void writeLockStateAuthorizationCallback(GattWriteAuthCallbackParams *authParams);

    /**
     * Callback registered to the BLE API to authorize write operations to simple fixed length
     * value characteristic types.
     *
     * @param[in] authParams
     *              Write authentication information.
     */
    template <typename T>
    void writeBasicAuthorizationCallback(GattWriteAuthCallbackParams *authParams);

    /**
     * This callback is invoked when a GATT client attempts to write to the
     * Active Slot characteristic of the service.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    template <typename T>
    void writeActiveSlotAuthorizationCallback(GattWriteAuthCallbackParams *authParams);

    /**
     * READ AUTHORIZATIONS
     */

    /**
     * This callback is invoked when a GATT client attempts to read from a
     * basic characteristic of the Eddystone Configuration Service, which
     * is blocked if the beacon lock is set to LOCKED.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readBasicTestLockAuthorizationCallback(GattReadAuthCallbackParams *authParams);
    
    /**
     * This callback is invoked when a GATT client attempts to read from the
     * EidIdentityKey characteristic of the Eddystone Configuration Service,
     * which is blocked if the beacon lock is set to LOCKED, or the key has not
     * been set/initialized.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readEidIdentityAuthorizationCallback(GattReadAuthCallbackParams *authParams);
    
    /**
     * This callback is invoked when a GATT client attempts to read from the
     * PublicEcdhKey characteristic of the Eddystone Configuration Service,
     * which is blocked if the beacon lock is set to LOCKED, or the key has not
     * been set/initialized.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readPublicEcdhKeyAuthorizationCallback(GattReadAuthCallbackParams *authParams);
    

    /**
     * This callback is invoked when a GATT client attempts to read from the
     * Adv Slot Data characteristic of the Eddystone Configuration Service,
     * which isblocked if the beacon lock is set to LOCKED.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readDataAuthorizationCallback(GattReadAuthCallbackParams *authParams);
    
    /**
     * Checks if this is valid frame data (i.e. length > 0)
     *
     * @param[in] frame
     *              The frame being tested
     * @returns   frame is valid or not.
     */
    bool testValidFrame(uint8_t* frame);

    /**
     * This callback is invoked when a GATT client attempts to read the challenge
     * from the Unlock characteristic of the Eddystone Configuration Service,
     * which is blocked if the beacon lock is set to UNLOCKED.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readUnlockAuthorizationCallback(GattReadAuthCallbackParams *authParams);

    /**
     * This callback is invoked when a GATT client attempts to read from the
     * Radio Tx Power characteristic of the Eddystone Configuration Service,
     * which is blocked if the beacon lock is set to LOCKED.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readRadioTxPowerAuthorizationCallback(GattReadAuthCallbackParams *authParams);

    /**
     * This callback is invoked when a GATT client attempts to read from the
     * Radio Tx Power characteristic of the Eddystone Configuration Service,
     * which is blocked if the beacon lock is set to LOCKED.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readAdvTxPowerAuthorizationCallback(GattReadAuthCallbackParams *authParams);

    /**
     * This callback is invoked when a GATT client attempts to read from the
     * Adv Interval characteristic of the Eddystone Configuration Service,
     * which is blocked if the beacon lock is set to LOCKED.
     *
     * @param[in] authParams
     *              Information about the values that are being read.
     */
    void readAdvIntervalAuthorizationCallback(GattReadAuthCallbackParams *authParams);

    /**
     * Calculates the index in the radio power levels array which can be used
     * to index into the adv power levels array to find the calibrated adv power
     * used in the adv frame.
     */
    uint8_t radioTxPowerToIndex(int8_t txPower);

    /**
     * This callback is invoked when a GATT client attempts to modify any of the
     * characteristics of this service. Attempts to do so are also applied to
     * the internal state of this service object.
     *
     * @param[in] writeParams
     *              Information about the values that are being written.
     */
    void onDataWrittenCallback(const GattWriteCallbackParams *writeParams);

    /**
     * Sets the power for the frame in a particular slot using the
     * adv tx power parmeter
     *
     * @param[in] slot
     *              The the current slot number being considered
     * @param[in] advTxPower
     *              The adv power required in a frame
     */
    void setFrameTxPower(uint8_t slot, int8_t advTxPower);

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

    /**
     * AES128 ECB Deccrypts a 16-byte input array with a key, to an output array
     *
     * @param[in] *key
     *              The decryption key
     * @param[in] *input
     *              The input array
     * @param[in] *output
     *              The output array (containing the decrypted data)
     */
    void aes128Decrypt(uint8_t *key, uint8_t *input, uint8_t *output);



    /**
     * Swaps the endianess of a 16-bit unsigned int
     *
     * @param[in] arg
     *              The value with the byte order to be reversed
     *
     * @return The resulting 16-bit value with byte order reversed
     */
    uint16_t swapEndian(uint16_t arg);

    /**
     * Correct the advertising interval for non-connectable packets.
     *
     * @param[in] beaconPeriodIn
     *              The input interval in milliseconds.
     *
     * @return The corrected interval in milliseconds.
     *
     * @note For the acceptable range of advertising interval refer to the
     *       following functions in mbed BLE API:
     *       - Gap::getMinNonConnectableAdvertisingInterval()
     *       - Gap::getMaxAdvertisingInterval()
     */
    uint16_t correctAdvertisementPeriod(uint16_t beaconPeriodIn) const;
    
    /**
     * Swaps the endianess of a 16-bit unsigned int
     *
     * @param[in] arg
     *              The value with the byte order to be reversed
     *
     * @return The resulting 16-bit value with byte order reversed
     */
    void setRandomMacAddress(void);     
    
    /**
     * Finds the first EID slot set
     *
     * @return slot number (and if not, returns NO_EID_SLOT_SET = -1)
     */
    int getEidSlot(void);
    
    /**
     * Returns the current time in Secs (Prior Time + Time since boot)
     *
     * @return time
     */
    uint32_t getTimeSinceFirstBootSecs(void);
    

    /**
     * Returns the time since boot in Milliseconds
     *
     * @return time
     */
    static uint64_t getTimeSinceLastBootMs(void);
    
    /**
     * Saves only the Time Params in pStorage (a subset of all the Eddsytone Params)
     * This is more efficient than periodically saving all state (its just 8 bytes)
     */
    void nvmSaveTimeParams(void); 

    /**
     * BLE instance that EddystoneService will operate on.
     */
    BLE                                                             &ble;

    /**
     * The advertising interval for Eddystone-URL Config Service advertising
     * packets.
     */
    uint32_t                                                        advConfigInterval;
    /**
     * Current EddystoneServce operation mode.
     */
    uint8_t                                                         operationMode;
    
    /**
     * Parameter to consistently record the return code when generating Beacon Keys
     */
    int                                                             genBeaconKeyRC;
    
    /**
     * Keeps track of time in prior boots and current/last boot
     */
    TimeParams_t                                                    timeParams;

    /**
     * GATT Service Variables
     */

    /**
     * An array describing the capabilites of the beacon.
     */
    Capability_t                                                    capabilities;

    /**
     * The currenty defined active slot.
     */
    uint8_t                                                         activeSlot;

    /**
     * An array containing all the adv intervals for each slot index
     */
    SlotAdvIntervals_t                                              slotAdvIntervals;

    /**
     * The value of the Eddystone Configuration Service radioTX Power
     * characteristic.
     */
    SlotTxPowerLevels_t                                             slotRadioTxPowerLevels;

    /**
     * An array containing the supported radio tx power levels for this beacon
     */
    PowerLevels_t                                                   radioTxPowerLevels;

    /**
     * An array containing all possible values for advertised tx power in Eddystone
     * slots.
     */
    SlotTxPowerLevels_t                                             slotAdvTxPowerLevels;

    /**
     * An array containing the supported adv tx power levels for this beacon
     */
    PowerLevels_t                                                   advTxPowerLevels;

    /**
     * The value of the Eddystone Configuration Service Lock State
     * characteristic.
     */
    uint8_t                                                         lockState;


    /**
     * The value of the Eddystone Configuration Service Lock State
     * buffer
     */
    LockState_t                                                     lockStateBuf;

    /**
     * The value of the Eddystone Configuration Service unlock key
     */
    Lock_t                                                          unlockKey;

    /**
     * The value of the Eddystone Configuration Service unlock challenge
     */
    Lock_t                                                          challenge;

   /**
     * The value of the Eddystone Configuration Service unlock token. A write
     * to the unlock characteristic must contain this token to unlock the beacon
     */
    Lock_t                                                          unlockToken;


    /**
     * EID: An array holding the 256-bit private Ecdh Key (big endian)
     */
    PrivateEcdhKey_t                                                privateEcdhKey;

    /**
     * EID: An array holding the 256-bit public Ecdh Key (big endian)
     */
    PublicEcdhKey_t                                                 publicEcdhKey;
    
    /**
     * EID: An array holding the 256-bit public Ecdh Key (little endian)
     */
    PublicEcdhKey_t                                                 publicEcdhKeyLE;

    /**
     * EID: An array holding the slot rotation period exponents
     */
    SlotEidRotationPeriodExps_t                                     slotEidRotationPeriodExps;

    /**
     * EID: An array holding the slot Eid Identity Keys
     */
    SlotEidIdentityKeys_t                                           slotEidIdentityKeys;

    /**
     * EID: An array holding the slot Eid Public Ecdh Keys
     */
    //SlotEidPublicEcdhKeys_t                                         slotEidPublicEcdhKeys;

    /**
     * Instance of the UID frame.
     */
    UIDFrame                                                        uidFrame;

    /**
     * Instance of the URL frame.
     */
    URLFrame                                                        urlFrame;

    /**
     * Instance of the TLM frame.
     */
    TLMFrame                                                        tlmFrame;

    /**
     * Instance of the EID frame.
     */
    EIDFrame                                                        eidFrame;

    /**
     * The value of the Eddystone Configuration Service reset
     * characteristic.
     */
    uint8_t                                                         factoryReset;

    /**
     * The value of the Eddystone Configuration Service Remain Connectable
     * characteristic.
     */
    uint8_t                                                         remainConnectable;

    /**
     * CHARACTERISTIC STORAGE
     */

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Capabilities characteristic.
     */
    ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(Capability_t)>  *capabilitiesChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Active Slot characteristic.
     */
    ReadWriteGattCharacteristic<uint8_t>                            *activeSlotChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Adv Interval characteristic.
     */
    ReadWriteGattCharacteristic<uint16_t>                           *advIntervalChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Radio Tx Power characteristic.
     */
    ReadWriteGattCharacteristic<int8_t>                             *radioTxPowerChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Adv Tx Power characteristic.
     */
    ReadWriteGattCharacteristic<int8_t>                             *advTxPowerChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Lock State characteristic.
     */
    GattCharacteristic                                               *lockStateChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Unlock characteristic.
     */
    ReadWriteArrayGattCharacteristic<uint8_t, sizeof(Lock_t)>       *unlockChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone
     * Configuration Service Public ECDH Key characteristic.
     */
    GattCharacteristic                                              *publicEcdhKeyChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone-URL
     * Configuration Service EID Identity Key characteristic.
     */
    GattCharacteristic                                              *eidIdentityKeyChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone-URL
     * Configuration Service Adv Slot Data characteristic.
     */
    GattCharacteristic                                              *advSlotDataChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone-URL
     * Configuration Service Factory Reset characteristic.
     */
    WriteOnlyGattCharacteristic<uint8_t>                            *factoryResetChar;

    /**
     * Pointer to the BLE API characteristic encapsulation for the Eddystone-GATT
     * Configuration Service Remain Connectable characteristic.
     */
    ReadWriteGattCharacteristic<uint8_t>                            *remainConnectableChar;

    /**
     * END OF GATT CHARACTERISTICS
     */

    /**
     * EID: An array holding the slot next rotation times
     */
    SlotEidNextRotationTimes_t                                      slotEidNextRotationTimes;

    /**
     * EID: Storage for the current slot encrypted EID Identity Key
     */
    EidIdentityKey_t                                                encryptedEidIdentityKey;

    /*
     * Storage for all the slots / frames
     */
    SlotStorage_t                                                   slotStorage;

    /**
     * An array that defines the frame type of each slot using the slot number
     * as an index.
     */
    SlotFrameTypes_t                                                slotFrameTypes;

    /**
     * Circular buffer that represents of Eddystone frames to be advertised.
     */
    CircularBuffer<uint8_t, MAX_ADV_SLOTS>                          advFrameQueue;

    /**
     * The registered callback to update the Eddystone-TLM frame Battery
     * Voltage.
     */
    TlmUpdateCallback_t                                             tlmBatteryVoltageCallback;

    /**
     * The registered callback to update the Eddystone-TLM frame Beacon
     * Temperature.
     */
    TlmUpdateCallback_t                                             tlmBeaconTemperatureCallback;

    /**
     * Type for the array of callback handles for all the slot timers
     */
    typedef event_queue_t::event_handle_t SlotCallbackHandles_t[MAX_ADV_SLOTS];

    /**
     * An array of all the slot timer callbacks handles
     */
    SlotCallbackHandles_t                                           slotCallbackHandles;

    /**
     * Callback handle to keep track of manageRadio() callbacks.
     */
    event_queue_t::event_handle_t                                   radioManagerCallbackHandle;

    /**
     * GattCharacteristic table used to populate the BLE ATT table in the
     * GATT Server.
     */
    GattCharacteristic                                              *charTable[TOTAL_CHARACTERISTICS];

    /**
     * Pointer to the device name currently being used.
     */
    const char                                                      *deviceName;

    /**
     * Defines an array of string constants (a container) used to initialise any URL slots
     */
    static const char* const slotDefaultUrls[];

    /**
     * Defines an array of UIDs to initialize UID slots
     */
    static const uint8_t slotDefaultUids[MAX_ADV_SLOTS][16];

    /**
     * Defines an array of EID (Identity keys) to initialize EID slots
     */
    static const uint8_t slotDefaultEidIdentityKeys[MAX_ADV_SLOTS][16];

    /**
     * Defines default EID payload before being updated with the first EID rotation value
     */
    static const uint8_t nullEid[8];

    /**
     * Reference to the event queue used to post tasks
     */
    event_queue_t&                  eventQueue;
    
    /**
     * Next EID slot frame that will be transmitted
     */
    uint8_t                         nextEidSlot;                     
};

#endif  /* __EDDYSTONESERVICE_H__ */
