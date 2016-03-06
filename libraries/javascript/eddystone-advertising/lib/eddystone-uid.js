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
