(() => {
  'use strict';

  /**
   * @module eddystone-advertising
   * @typicalname advertising
   */

  let platform = require('./platform.js');
  let EddystoneFrameType = require('./eddystone-advertisement.js').EddystoneFrameType;

  /**
     Object that contains the characteristics of the package to advertise.
     @typedef {Object} EddystoneAdvertisementOptions
     @property {EddystoneFrameType} type Type of Eddystone. For now only `'url'` is
     supported.
     @property {string|undefined} url The URL to advertise
     @property {number|undefined} advertisedTxPower The Tx Power to advertise
   */

  /**
     Exposes platform independent functions to register/unregister Eddystone
     Advertisements.
     @alias module:eddystone-advertising
   */
  class Eddystone {
    constructor() {
      this._platform = platform();
      /**
         Contains all previously registered advertisements.

         ***Note:** In a Chrome App, if the event page gets killed users
         won't be able to unregister the advertisement.
         @type {EddystoneAdvertisement[]}
       */
      this.advertisements = [];
    }
    /**
       Function to register an Eddystone BLE advertisement.
       @params {EddystoneAdvertisementOptions} options The characteristics
       of the advertised Eddystone.
       @fulfill {EddystoneAdvertisement} - If the advertisement was registered
       successfully.
       @reject {Error} - If the advertisement failed to be registered.
       @returns {Promise.<EddystoneAdvertisement>}
     */
    registerAdvertisement(options) {
      let self = this;
      return new Promise((resolve, reject) => {
        if (!self._platform) {
          reject(new Error('Platform not supported.'));
          return;
        }
        Eddystone._checkAdvertisementOptions(options);
        return self._platform.registerAdvertisement(options).then(advertisement => {
          self.advertisements.push(advertisement);
          resolve(advertisement);
        }, reject);
      });
    }

    /**
       Checks that all the required properties are present for the specific
       Eddystone Frame Type
       @private
       @params {EddystoneAdvertisementOptions} options
       @throws {Error} If a required property is missing
     */
    static _checkAdvertisementOptions(options) {
      if (!('type' in options)) {
        throw new TypeError('Required member type is undefined.');
      }
      if (options.type === EddystoneFrameType.URL) {
        Eddystone._checkURLOptions(options);
      } else if (options.type === EddystoneFrameType.UID) {
        Eddystone._checkUIDOptions(options);
      } else {
        throw new TypeError('Unsupported Frame Type: ' + options.type);
      }
    }

    /**
       Checks that all required properties are present for a
       Eddystone-URL advertisement
       @private
       @params {EddystoneAdvertisementOptions} options
       @throws {Error} If a required property is missing
     */
    static _checkURLOptions(options) {
      if (!('url' in options)) {
        throw new TypeError('Required member url is undefined.');
      }
      if (!('advertisedTxPower' in options)) {
        throw new TypeError('Required member advertisedTxPower is undefined.');
      }
    }
    static _checkUIDOptions(options) {
      if (!('advertisedTxPower' in options)) {
        throw new TypeError('Required member advertisedTxPower is undefined.');
      }
      if (!('namespace' in options)) {
        throw new TypeError('Required member namespace is undefined.');
      }
      if (!('instance' in options)) {
        throw new TypeError('Required member instance is undefined.');
      }
    }
  }

  module.exports = Eddystone;
})();
