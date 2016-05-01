// jshint node: true
// We need this so chai `expect` statements don't throw an error.
// jshint expr: true
'use strict';

import chai, {expect} from 'chai';
import EddystoneUID from '../../lib/eddystone-uid.js';

describe('Eddystone-UID', () => {
  describe('getByteArray()', () => {
    it('Invalid type', () => {
      expect(() => EddystoneUID.getByteArray({}, 5)).to.throw(TypeError);
    });
    it('Valid String', () => {
      expect(EddystoneUID.getByteArray('FF00', 2)).to.eql([0xFF, 0]);
    });
    it('Invalid String', () => {
      expect(() => EddystoneUID.getByteArray('GGGG', 2)).to.throw(Error);
    });
    it('Valid byte array', () => {
      expect(EddystoneUID.getByteArray([1, 2, 3, 4], 4)).to.eql([1, 2, 3, 4]);
    });
    it('Invalid byte array', () => {
      expect(() => EddystoneUID.getByteArray([0xF00], 1)).to.throw(Error);
    });
  });
  describe('getHexString()', () => {
    it('Valid String', () => {
      expect(EddystoneUID.getHexString([0xFF])).to.eql('ff');
    });
    it('Invalid String', () => {
      expect(() => EddystoneUID.getHexString([0xF00])).to.throw(Error);
    });
  });
  describe('_getNamespaceByteArray()', () => {
    it('Valid String Namespace', () => {
      expect(EddystoneUID._getNamespaceByteArray('0102030405060708090a'))
        .to.eql([1,2,3,4,5,6,7,8,9,10]);
    });
    it('Invalid Length String Namespace', () => {
      expect(() => EddystoneUID._getNamespaceByteArray('0102030405'))
        .to.throw(Error);
    });
    it('Valid Byte Namespace', () => {
      expect(EddystoneUID._getNamespaceByteArray([1,2,3,4,5,6,7,8,9,0]))
        .to.eql([1,2,3,4,5,6,7,8,9,0]);
    });
    it('Invalid Length Byte Namespace', () => {
      expect(() => EddystoneUID._getNamespaceByteArray([1,2,3,4]))
        .to.throw(Error);
    });
  });
  describe('_getInstanceByteArray()', () => {
    it('Valid String Instance', () => {
      expect(EddystoneUID._getInstanceByteArray('010203040506')).to.eql([1,2,3,4,5,6]);
    });
    it('Invalid Length String Instance', () => {
      expect(() => EddystoneUID._getInstanceByteArray('010203')).to.throw(Error);
    });
    it('Valid Byte Array Instance', () => {
      expect(EddystoneUID._getInstanceByteArray([1,2,3,4,5,6])).to.eql([1,2,3,4,5,6]);
    });
    it('Invalid Length Byte Array Instance', () => {
      expect(() => EddystoneUID._getInstanceByteArray([1,2,3])).to.throw(Error);
    });
  });
  describe('_encodeString()', () => {
    it('Odd expected length', () => {
      expect(() => EddystoneUID._encodeString('012', 3)).to.throw(/length/);
    });
    it('Too Long String', () => {
      expect(() => EddystoneUID._encodeString('010203', 4)).to.throw(/length/);
    });
    it('Too Short String', () => {
      expect(() => EddystoneUID._encodeString('01', 4)).to.throw(/length/);
    });
    it('Correct Length String', () => {
      expect(EddystoneUID._encodeString('0102', 4)).to.eql([1,2]);
    });
    it('Invalid characters String', () => {
      expect(() => EddystoneUID._encodeString('010G', 4)).to.throw(/character/);
    });
    it('Valid String', () => {
      expect(EddystoneUID._encodeString('abcdef', 6)).eql([0xab, 0xcd, 0xef]);
    });
  });
  describe('_validateByteArray()', () => {
    it('Too long array', () => {
      expect(() => EddystoneUID._validateByteArray([1,2,3], 2)).to.throw(/length/);
    });
    it('Too short array', () => {
      expect(() => EddystoneUID._validateByteArray([1], 2)).to.throw(/length/);
    });
    it('Wrong type in array', () => {
      expect(() => EddystoneUID._validateByteArray([1, {}], 2)).to.throw(/value/);
    });
    it('Wrong value in array', () => {
      expect(() => EddystoneUID._validateByteArray([1, 0xF00], 2)).to.throw(/value/);
    });
  });
});
