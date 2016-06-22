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
