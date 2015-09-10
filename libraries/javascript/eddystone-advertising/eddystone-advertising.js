(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
(() => {
  'use strict';

  /**
     Possible Eddystone frame types.
     @see {@link https://github.com/google/eddystone/blob/master/protocol-specification.md|Protocol Specification}
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-url|Eddystone-URL}
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-uid|Eddystone-UID}
     @see {@link https://github.com/google/eddystone/tree/master/eddystone-tlm|Eddystone-TLM}
     @readonly
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
   */
  const EDDYSTONE_UUID = 'FEAA';

  /**
     Represents the Advertisement being broadcasted.
     @class
   */
  class EddystoneAdvertisement {
    /**
       @constructs EddystoneAdvertisement
       @param {number} id Unique between browser restarts meaning the id will
       no longer be valid upon browser restart.
       @param {EddystoneAdvertisementOptions} options The options used when
       creating the advertisement.
       @param {Object} platform The underlying platform; used to unregister the
       advertisement.
       @throws {TypeError} If no platform was passed.
       @throws {Error} If type is an unsupported Frame Type.
     */
    constructor(id, options, platform) {
      if (typeof platform === 'undefined') {
        throw new TypeError('Required member platform is undefined');
      }
      this._platform = platform;
      /**
         @member EddystoneAdvertisement#id {number} The ID of this advertisment.
       */
      this.id = undefined;
      /**
         @member EddystoneAdvertisement#type {string} The Eddystone Type
       */
      this.type = undefined;
      /**
         @member EddystoneAdvertisement#url {string|undefined} URL being advertised.
         Only present if `type === 'url'`.
       */
      this.url = undefined;
      /**
         @member EddystoneAdvertisement#advertisedTxPower {number|undefined} Tx Power included in
         the advertisement. Only present if `type === 'url'`.
       */
      this.advertisedTxPower = undefined;
      if (options.type == EddystoneFrameType.URL) {
        this.id = id;
        this.type = options.type;
        this.url = options.url;
        this.advertisedTxPower = options.advertisedTxPower;
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
  module.exports.EddystoneAdvertisement = EddystoneAdvertisement;
  module.exports.EddystoneFrameType = EddystoneFrameType;
  module.exports.EDDYSTONE_UUID = EDDYSTONE_UUID;
})();

},{}],2:[function(require,module,exports){
(() => {
  'use strict';
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
     @class
   */
  class Eddystone {
    /**
       @constructs Eddystone
     */
    constructor() {
      this._platform = platform();
      /**
         @member Eddystone#advertisements {EddystoneAdvertisement[]} Contains
         all previously registered advertisements.<br>
         ***Note:** In a Chrome App, if the event page gets killed users won't
         be able to unregister the advertisement.
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
  }
  module.exports = Eddystone;
})();

},{"./eddystone-advertisement.js":1,"./platform.js":6}],3:[function(require,module,exports){
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
            data: EddystoneURL.constructServiceData(options.url, options.advertisedTxPower)
          }]
        };
      } else {
        throw new Error('Unsupported Frame Type: ' + options.type);
      }
    }
  }
  module.exports = EddystoneChromeOS;
})();

},{"./eddystone-advertisement.js":1,"./eddystone-url.js":4}],4:[function(require,module,exports){
(function (global){
(() => {
  'use strict';

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
     @class
   */
  class EddystoneURL {
    /**
       Constructs a valid Eddystone-URL service data from a URL and a Tx Power
       value.
       @see {@link https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification|URL Frame Specification}
       @param {string} url The URL to use in the service data.
       @param {number} advertisedTxPower The Tx Power to use in the service data.
       @returns {number[]} The service data.
       @throws {Error} If the Tx Power value is not in the allowed range. See
       {@link https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level|Tx Power Level}.
       @throws {Error} If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see
       {@link https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix|URL Scheme Prefix}
       @throws {Error} If the URL contains an invalid character. For a list of
       invalid characters see the Note in
       {@link https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding|HTTP URL Encoding}
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
},{"./eddystone-advertisement.js":1,"text-encoding":undefined}],5:[function(require,module,exports){
(function (global){
(() => {
  'use strict';

  // Only require top level module. Browserify will walk
  // the dependency graph and load all needed modules.
  let Eddystone = require('./eddystone-advertising.js');

  // browserify will replace global with window.
  global.eddystone = new Eddystone();
})();

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{"./eddystone-advertising.js":2}],6:[function(require,module,exports){
(() => {
  'use strict';
  let EddystoneChromeOS = require('./eddystone-chrome-os.js');
  /**
     Detects what API is available in the platform.
     @private
     @returns {Object} An object that wraps the underlying BLE
     Advertising API
     @throws {Error} If the platform is unsupported
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

},{"./eddystone-chrome-os.js":3}]},{},[5]);
