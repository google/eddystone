(() => {
  'use strict';
  let EddystoneURL = require('./eddystone-url.js');
  let EddystoneAdvertisement = require('./eddystone-advertisement.js').EddystoneAdvertisement;
  const EddystoneFrameType = require('./eddystone-advertisement.js').EddystoneFrameType;
  const EDDYSTONE_UUID = require('./eddystone-advertisement.js').EDDYSTONE_UUID;
  /**
     This class wraps the underlying ChromeOS BLE Advertising API.
     TODO: Add link to API.
     @private
     @class EddystoneChromeOS
   */
  class EddystoneChromeOS {
    /**
       ChromeOS Specific Advertisement
       @typedef {Object} ChromeOSAdvertisement
       @private
       @property {string} type
       @property {Array} serviceUuids
       @property {Object} serviceData {uuid: {string}, data: {number[]}}
     */

    /**
       Function that registers an Eddystone BLE advertisement.
       @param {EddystoneAdvertisementOptions} options The characteristics of the
       advertisement.
       @fulfill {EddystoneAdvertisement} - If the advertisement was registered
       successfully.
       @reject {Error} - If the advertisement failed to be regsitered.
       @returns {Promise.<EddystoneAdvertisement>}
     */
    static registerAdvertisement(options) {
      return new Promise((resolve, reject) => {
        let chrome_adv = EddystoneChromeOS._constructAdvertisement(options);

        chrome.bluetoothLowEnergy.registerAdvertisement(chrome_adv, (advertisement_id) => {
          if (chrome.runtime.lastError) {
            reject(new Error(chrome.runtime.lastError.message));
            return;
          }
          resolve(new EddystoneAdvertisement(advertisement_id, options, EddystoneChromeOS));
        });
      });
    }

    /**
       Function to unregister an advertisement.
       @param {EddystoneAdvertisement} advertisement The advertisement to
       unregister.
       @fulfill {void} - If the advertisment was unregistered successfully.
       @reject {Error} - If the advertisment failed to be unregistered.
       @returns {Promise.<void>}
     */
    static unregisterAdvertisement(advertisement) {
      return new Promise((resolve, reject) => {
        chrome.bluetoothLowEnergy.unregisterAdvertisement(advertisement.id, () => {
          if (chrome.runtime.lastError) {
            reject(new Error(chrome.runtime.lastError.message));
            return;
          }
          resolve();
        });
      });
    }

    /**
       Construct the ChromeOS specific advertisement to register.
       @params {EddystoneAdvertisementOptions} options The characteristics of the
       advertisement.
       @returns {ChromeOSAdvertisement} advertisement
       @throws {Error} If the frame type is not supported
       @throws {Error} If the Tx Power value is not in the allowed range. See:
       https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level
       @throws {Error} If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see:
       https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
       @throws {Error} If the URL contains an invalid character. For a list of
       invalid characters see the Note in:
       https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
     */
    static _constructAdvertisement(options) {
      if (options.type === EddystoneFrameType.URL) {
        return {
          type: 'broadcast',
          serviceUuids: [EDDYSTONE_UUID],
          serviceData: [{
            uuid: EDDYSTONE_UUID,
            data: EddystoneURL.constructServiceData(options.url, options.txPower)
          }]
        };
      } else {
        throw new Error('Unsupported Frame Type: ' + options.type);
      }
    }
  }
  module.exports = EddystoneChromeOS;
})();
