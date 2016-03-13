(() => {
  'use strict';

  /**
   * @module eddystone-advertisement
   * @typicalname advertisement
   * @example
   * const advertisement = require('eddystone-advertisement')
   */

  /**
     Possible Eddystone frame types.
     @see {@link https://github.com/google/eddystone/blob/master/protocol-specification.md|Protocol Specification}
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-url|Eddystone-URL}
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-uid|Eddystone-UID}
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-tlm|Eddystone-TLM}
     @readonly
     @alias module:eddystone-advertisement.EddystoneFrameType
     @enum {string}
   */
  const EddystoneFrameType = {
    URL: 'url',
    UID: 'uid',
    TLM: 'tlm'
  };

  /**
     Assigned UUID for Eddystone Beacons
     @private
     @constant {string}
     @default
     @alias module:eddystone-advertisement.EDDYSTONE_UUID
   */
  const EDDYSTONE_UUID = 'FEAA';

  /**
     Represents the Advertisement being broadcasted.
     @throws {TypeError} If no platform was passed.
     @throws {Error} If type is an unsupported Frame Type.
     @alias module:eddystone-advertisement.EddystoneAdvertisement
   */
  class EddystoneAdvertisement {
    /**
       @param {number} id Unique between browser restarts meaning the id will
       no longer be valid upon browser restart.
       @param {EddystoneAdvertisementOptions} options The options used when
       creating the advertisement.
       @param {Object} platform The underlying platform; used to unregister the
       advertisement.
     */
    constructor(id, options, platform) {
      if (typeof platform === 'undefined') {
        throw new TypeError('Required member platform is undefined');
      }
      this._platform = platform;
      /**
         The ID of this advertisment.
         @type {number}
       */
      this.id = undefined;
      /**
         The Eddystone Type
         @type {string}
       */
      this.type = undefined;
      /**
         Tx Power included in the advertisement. Only present if `type === 'url'`
         or `type === 'uid'`.
         @type {number|undefined}
       */
      this.advertisedTxPower = undefined;
      /**
         URL being advertised. Only present if `type === 'url'`.
         @type {string|undefined}
       */
      this.url = undefined;
      /**
         Hex string of the namespace being advertised. Only present if `type === 'uid'`.
         @type {string|undefined}
       */
      this.namespace = undefined;
      /**
         Hex string of the instance being advertised. Only present if `type === 'uid'`.
         @type {string|undefined}
      */
      this.instance = undefined;

      if (options.type == EddystoneFrameType.URL) {
        this.id = id;
        this.type = options.type;
        this.url = options.url;
        this.advertisedTxPower = options.advertisedTxPower;
      } else if (options.type == EddystoneFrameType.UID) {
        this.id = id;
        this.type = options.type;
        this.advertisedTxPower = options.advertisedTxPower;
        this.namespace = options.namespace;
        this.instance = options.instance;
      } else {
        throw new Error('Unsupported Frame Type');
      }
    }

    /**
       Unregisters the current advertisement.
       @fulfill {void} - If the advertisement was unregistered successfully.
       @reject {Error} - If the advertisement failed to be registered. If
       the promise rejects the advertisment may still be broadcasting. The only
       way to recover may be to reboot your machine.
       @returns {Promise.<void>}
     */
    unregisterAdvertisement() {
      return this._platform.unregisterAdvertisement(this);
    }
  }

  exports.EddystoneAdvertisement = EddystoneAdvertisement;
  exports.EddystoneFrameType = EddystoneFrameType;
  exports.EDDYSTONE_UUID = EDDYSTONE_UUID;
})();
