(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
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

},{}],2:[function(require,module,exports){
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

},{"./eddystone-advertisement.js":1,"./platform.js":7}],3:[function(require,module,exports){
(() => {
  'use strict';

  /**
   * @module eddystone-chrome-os
   * @typicalname chromeOS
   */

  let EddystoneURL = require('./eddystone-url.js');
  let EddystoneUID = require('./eddystone-uid.js');
  let EddystoneAdvertisement = require('./eddystone-advertisement.js').EddystoneAdvertisement;
  const EddystoneFrameType = require('./eddystone-advertisement.js').EddystoneFrameType;
  const EDDYSTONE_UUID = require('./eddystone-advertisement.js').EDDYSTONE_UUID;
  /**
     This class wraps the underlying ChromeOS BLE Advertising API.
     @todo Add link to API.
     @alias module:eddystone-chrome-os
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
      let data;
      if (options.type === EddystoneFrameType.URL) {
        data = EddystoneURL.constructServiceData(options.url, options.advertisedTxPower);
      } else if (options.type === EddystoneFrameType.UID) {
        data = EddystoneUID.constructServiceData(
          options.advertisedTxPower, options.namespace, options.instance);
      } else {
        throw new Error('Unsupported Frame Type: ' + options.type);
      }
      return {
        type: 'broadcast',
        serviceUuids: [EDDYSTONE_UUID],
        serviceData: [{
          uuid: EDDYSTONE_UUID,
          data: data
        }]
      };
    }
  }
  module.exports = EddystoneChromeOS;
})();

},{"./eddystone-advertisement.js":1,"./eddystone-uid.js":4,"./eddystone-url.js":5}],4:[function(require,module,exports){
(() => {
  'use strict';

  /**
   * @module eddystone-uid
   * @typicalname uid
   */

  const EDDYSTONE_UUID = require('./eddystone-advertisement.js').EDDYSTONE_UUID;

  /**
     Eddystone-UID Frame type.
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-uid#frame-specification|Eddystone-UID}
     @private
     @constant {number}
     @default
   */
  const EDDYSTONE_UID_FRAME_TYPE = 0x00;

  const NAMESPACE_LENGTH = 10;
  const INSTANCE_LENGTH = 6;

  const HEX_REGEX = /[0-9a-f]/i;

  /**
     This class provides helper functions that relate to Eddystone-UID.
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-uid|Eddystone-UID}
     @alias module:eddystone-uid
   */
  class EddystoneUID {
    /**
       Constructs a valid Eddystone-UID service data from a Tx Power value, namespace
       and instance.
       @see {@link https://github.com/google/eddystone/tree/master/eddystone-uid#frame-specification|UID Frame Specification}
       @param {number} advertisedTxPower The Tx Power included in the service data.
       @param {number[]|string} namespace The namespace to advertise.
       @param {number[]|string} instance The instance to advertise.
       @returns {number[]} The service data.
     */
    static constructServiceData(advertisedTxPower, namespace, instance) {
      // Check that it's a valid Tx Power.
      if (advertisedTxPower < -100 || advertisedTxPower > 20) {
        throw new Error('Invalid Tx Power value: ' + advertisedTxPower + '.');
      }
      let base_frame = [EDDYSTONE_UID_FRAME_TYPE, advertisedTxPower];
      Array.prototype.push.apply(base_frame, EddystoneUID._getNamespaceByteArray(namespace));
      Array.prototype.push.apply(base_frame, EddystoneUID._getInstanceByteArray(instance));
      return base_frame;
    }

    /**
       Validates the give array of bytes or converts the hex string into an array of bytes.
       @param {number[]|string} value The value to encode.
       @throws {TypeError} If |value| is not an array or a string.
       @throws {Error} If |value| contains out-of-range numbers or characters.
       @returns {number[]} Array of bytes.
    */
    static getByteArray(value, expected_length) {
      if (typeof value === 'string') {
        // A hex string is twice as long as the byte array it represents.
        let str_expected_length = expected_length * 2;
        return EddystoneUID._encodeString(value, str_expected_length);
      }
      if (Array.isArray(value)) {
        return EddystoneUID._validateByteArray(value, expected_length);
      }
      throw new TypeError('Only string or array are supported');
    }

    static getHexString(bytes) {
      let hex_string = '';
      for (let i = 0; i < bytes.length; i++) {
        if (bytes[i] > 0xFF) {
          throw new Error('Invalid value \'' + bytes[i] + '\' at index ' + i + '.');
        }
        hex_string = hex_string.concat((bytes[i] >>> 4).toString(16));
        hex_string = hex_string.concat((bytes[i] & 0xF).toString(16));
      }
      return hex_string;
    }

    static _getNamespaceByteArray(namespace) {
      return EddystoneUID.getByteArray(namespace, NAMESPACE_LENGTH);
    }

    static _getInstanceByteArray(instance) {
      return EddystoneUID.getByteArray(instance, INSTANCE_LENGTH);
    }

    static _encodeString(str, expected_length) {
      if (expected_length % 2 !== 0) {
        throw new Error('expected_length should be an even number.');
      }
      if (str.length !== expected_length) {
        throw new Error('Expected length to be: ' + expected_length + '. ' +
                        'But was: ' + str.length  + '. Remember a hex string is twice ' +
                        'as long as the number of bytes desired.');
      }
      let bytes = [];
      for (let i = 0; i < str.length; i += 2) {
        if (!HEX_REGEX.test(str[i])) {
          throw new Error('Invalid character \'' + str[i] + '\' at index ' + i);
        }
        if (!HEX_REGEX.test(str[i + 1])) {
          throw new Error('Invalid character \'' + str[i + 1] + '\' at index ' + (i + 1));
        }
        bytes.push(parseInt(str.substr(i, 2), 16));
      }
      return bytes;
    }

    static _validateByteArray(arr, expected_length) {
      if (arr.length !== expected_length) {
        throw new Error('Expected length to be: ' + expected_length + '. ' +
                        'But was: ' + arr.length + '.');
      }
      for (let i = 0; i < arr.length; i++) {
        if (typeof arr[i] !== 'number') {
          throw new Error('Unexpected value \'' + arr[i] + '\' at index ' + i + '.');
        }
        if (!(arr[i] >= 0x00 && arr[i] <= 0xFF)) {
          throw new Error('Unexpected value \'' + arr[i] + '\' at index ' + i + '.');
        }
      }
      return arr;
    }
  }
  module.exports = EddystoneUID;
})();

},{"./eddystone-advertisement.js":1}],5:[function(require,module,exports){
(function (global){
(() => {
  'use strict';

  /**
   * @module eddystone-url
   * @typicalname url
   */

  const EDDYSTONE_UUID = require('./eddystone-advertisement.js').EDDYSTONE_UUID;

  // If we are in a browser TextEncoder should be available already.
  if (typeof global.TextEncoder === 'undefined') {
    global.TextEncoder = require('text-encoding').TextEncoder;
  }

  /**
     Eddystone-URL Frame type.
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification|Eddystone-URL}
     @private
     @constant {number}
     @default
   */
  const EDDYSTONE_URL_FRAME_TYPE = 0x10;

  /**
     These schemes are used to encode/decode a URL Scheme prefix.
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix|URL Scheme Prefix}
     @private
     @constant {string[]}
     @default
   */
  const URL_SCHEMES = [
    'http://www.',
    'https://www.',
    'http://',
    'https://'
  ];

  /**
     These codes are used to encode/decode a URL.
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding|HTTP URL Encoding}
     @private
     @constant {string[]}
     @default
   */
  const URL_CODES = [
    '.com/',
    '.org/',
    '.edu/',
    '.net/',
    '.info/',
    '.biz/',
    '.gov/',
    '.com',
    '.org',
    '.edu',
    '.net',
    '.info',
    '.biz',
    '.gov'
  ];

  // The Eddystone-URL spec says to use 'us-ascii' encoding but not all
  // browsers support it, so we use 'utf-8' which is a super set of
  // 'us-ascii' and wildly supported. We later check the encoded characters
  // are in the range allowed by the Eddystone-URL Spec.
  // https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
  let encoder = new TextEncoder('utf-8');

  /**
     This class provides helper functions that relate to Eddystone-URL.
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-url|Eddystone-URL}
     @alias module:eddystone-url
   */
  class EddystoneURL {
    /**
       Constructs a valid Eddystone-URL service data from a URL and a Tx Power
       value.
       @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification|URL Frame Specification}
       @param {string} url The URL to use in the service data.
       @param {number} advertisedTxPower The Tx Power to use in the service data.
       @returns {number[]} The service data.
     */
    static constructServiceData(url, advertisedTxPower) {
      // Check that it's a valid Tx Power
      if (advertisedTxPower < -100 || advertisedTxPower > 20) {
        throw new Error('Invalid Tx Power value: ' + advertisedTxPower);
      }
      let base_frame = [EDDYSTONE_URL_FRAME_TYPE, advertisedTxPower];
      Array.prototype.push.apply(base_frame, EddystoneURL.encodeURL(url));
      return base_frame;
    }

    /**
       Encodes the given string using the encoding defined in the Eddystone-URL
       Spec.
       @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url|Eddystone-URL}
       @param {string} url The url to encode.
       @returns {number[]} The encoded url.
       @throws {Error} If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see:
       https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
       @throws {Error} If the URL contains an invalid character. For a list of
       invalid characters see the Note in:
       https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
     */
    static encodeURL(url) {
      let encoded_url = [];

      let scheme = EddystoneURL._encodeURLScheme(url);
      encoded_url.push(scheme.encoded);

      let position = scheme.length;

      while (position < url.length) {
        let encoded_fragment = EddystoneURL._encodeURLFragment(url,
                                                               position);
        encoded_url.push(encoded_fragment.encoded);
        position += encoded_fragment.realLength;
      }

      if (encoded_url.length > 18) {
        throw new Error('Encoded URL shouldn\'t be longer than 18 bytes. Please use a URL shortener.');
      }

      return encoded_url;
    }

    /**
       Encodes the URL scheme of an URL.
       @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix|URL Scheme Prefix}
       @private
       @param {string} url
       @returns {Object} {encoded: number, length: number}. `encoded`: value after encoding, `length`: length before encoding.
       @throws {Error} If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see:
     */
    static _encodeURLScheme(url) {
      for (let i = 0 ; i < URL_SCHEMES.length; i++) {
        if (url.startsWith(URL_SCHEMES[i])) {
          return {
            encoded: i,
            length: URL_SCHEMES[i].length
          };
        }
      }
      throw new Error('URL Scheme not supported.');
    }

    /**
       This functions finds the next substring that can be encoded from the
       give starting_position.
       @private
       @param {string} url The unencoded url.
       @param {number} starting_position The position to begin looking for a substring to encode.
       @return {Object} {encoded: number, realLength: number}. `encoded`: value after encoding, `realLength`: length before encoding.
       @throws {Error} If the URL contains a invalid character. For a list of
       invalid characters see the Note in {@link https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding| HTTP URL Encoding}
     */
    static _encodeURLFragment(url, starting_position) {
      for (let i = 0; i < URL_CODES.length; i++) {
        // FIXME: Use a faster encoding algorithm e.g. a trie.
        if (url.startsWith(URL_CODES[i], starting_position)) {
          return {
            encoded: i,
            realLength: URL_CODES[i].length
          };
        }
      }
      let encoded_character = encoder.encode(url[starting_position])[0];
      // Per spec we check it's an allowed character:
      // https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
      if (encoded_character <= 0x20 || encoded_character >= 0x7f) {
        throw new Error(
          'Only graphic printable characters of the US-ASCII coded character set are supported.');
      }
      return {
        encoded: encoder.encode(url[starting_position])[0],
        realLength: 1 // Since are just returning a letter
      };
    }
  }
  module.exports = EddystoneURL;
})();

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{"./eddystone-advertisement.js":1,"text-encoding":undefined}],6:[function(require,module,exports){
(function (global){
(() => {
  'use strict';

  // Only require top level module. Browserify will walk
  // the dependency graph and load all needed modules.
  let Eddystone = require('./eddystone-advertising.js');

  /**
   * The global eddystone instance.
   *
   * @global
   * @type module:eddystone-advertising
   */
  global.eddystone = new Eddystone();
})();

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{"./eddystone-advertising.js":2}],7:[function(require,module,exports){
(() => {
  'use strict';

  /**
   * @module platform
   */

  let EddystoneChromeOS = require('./eddystone-chrome-os.js');
  /**
     Detects what API is available in the platform.
     @returns {Object} An object that wraps the underlying BLE
     Advertising API
     @throws {Error} If the platform is unsupported
     @alias module:platform
   */
  function platform() {
    if (typeof chrome !== 'undefined') {
      return EddystoneChromeOS;
    } else if (typeof _eddystone_test !== 'undefined') {
      return {};
    } else {
      throw new Error('Unsupported platform.');
    }
  }

  module.exports = platform;
})();

},{"./eddystone-chrome-os.js":3}]},{},[6]);
