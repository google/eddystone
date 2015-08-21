(() => {
  'use strict';

  /**
      Assigned UUID for Eddystone Beacons
      @private
      @constant {string}
      @default
   */
  const EDDYSTONE_UUID = 'FEAA';

  /**
      Eddystone-URL Frame type.
      See: https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification
      @private
      @constant {number}
      @default
   */
  const EDDYSTONE_URL_FRAME_TYPE = 0x10;

  /**
      These schemes are used to encode/decode a URL Scheme prefix.
      See: https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
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
      See: https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
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
  // are in the range allowed by the Eddystone-URL Spec:
  // https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
  let encoder = new TextEncoder('utf-8');

  /**
      This class provides helper functions that relate to Eddystone-URL.
      https://github.com/google/eddystone/tree/master/eddystone-url
      @class
   */
  class EddystoneURL {
    /**
        Constructs a valid Eddystone-URL service data from a URL and a Tx Power
        value. See:
        https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification
        @param {string} url The URL to use in the service data.
        @param {number} txPower The Tx Power to use in the service data.
        @returns {number[]} The service data.
        @throws {Error} If the Tx Power value is not in the allowed range. See:
        https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level
        @throws {Error} If the URL Scheme prefix is unsupported. For a list of
        supported Scheme prefixes see:
        https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
        @throws {Error} If the URL contains an invalid character. For a list of
        invalid characters see the Note in:
        https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
     */
    static constructServiceData(url, txPower) {
      // Check that it's a valid Tx Power
      if (txPower < -100 || txPower > 20) {
        throw new Error('Invalid Tx Power value: ' + txPower);
      }
      let base_frame = [EDDYSTONE_URL_FRAME_TYPE, txPower];
      Array.prototype.push.apply(base_frame, EddystoneURL.encodeURL(url));
      return base_frame;
    }

    /**
        Encodes the given string using the encoding defined in the Eddystone-URL
        Spec:
        https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url
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
        Encodes the URL scheme of an URL. See:
        https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
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
        invalid characters see the Note in:
        https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
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

  class Eddystone {
  }

  window.eddystone = new Eddystone();
  window.EddystoneURL = EddystoneURL;
})();
