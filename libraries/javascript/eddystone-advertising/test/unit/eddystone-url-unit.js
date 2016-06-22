// jshint node: true
// We need this so chai `expect` statements don't throw an error.
// jshint expr: true
'use strict';

import chai, {expect} from 'chai';
import {TextEncoder} from 'text-encoding';
import EddystoneURL from '../../lib/eddystone-url.js';

describe('Eddystone-URL', () => {

  describe('encodeURLScheme', () => {
    it('http://www.', () => {
      expect(typeof EddystoneURL._encodeURLScheme).to.eql('function');
      expect(EddystoneURL._encodeURLScheme('http://www.example.com')).to.eql({
        length: 11,
        encoded: 0});
    });

    it('https://www.', () => {
      expect(EddystoneURL._encodeURLScheme('https://www.example.com')).to.eql({
        length: 12,
        encoded: 1});
    });

    it('http://', () => {
      expect(EddystoneURL._encodeURLScheme('http://example.com')).to.eql({
        length: 7,
        encoded: 2});
    });

    it('https://', () => {
      expect(EddystoneURL._encodeURLScheme('https://example.com')).to.eql({
        length: 8,
        encoded: 3});
    });

    it('Empty URL', () => {
      expect(() => EddystoneURL._encodeURLScheme('')).to.throw(Error);
    });

    it('http:/', () => {
      expect(() => EddystoneURL._encodeURLScheme('http:/')).to.throw(Error);
    });
  });
  describe('encodeURLFragment()', () => {
    it('URL code at the beginning', () => {
      expect(EddystoneURL._encodeURLFragment('.org/', 0)).to.eql({
        encoded: 0x1,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('.edu/', 0)).to.eql({
        encoded: 0x2,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('.net/', 0)).to.eql({
        encoded: 0x3,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('.info/', 0)).to.eql({
        encoded: 0x4,
        realLength: 6
      });
      expect(EddystoneURL._encodeURLFragment('.biz/', 0)).to.eql({
        encoded: 0x5,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('.gov/', 0)).to.eql({
        encoded: 0x6,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('.com', 0)).to.eql({
        encoded: 0x7,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('.org', 0)).to.eql({
        encoded: 0x8,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('.edu', 0)).to.eql({
        encoded: 0x9,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('.net', 0)).to.eql({
        encoded: 0xa,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('.info', 0)).to.eql({
        encoded: 0xb,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('.biz', 0)).to.eql({
        encoded: 0xc,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('.gov', 0)).to.eql({
        encoded: 0xd,
        realLength: 4
      });
    });
    it('URL code at the end', () => {
      // Since 7 is the argument passed to _encodeURLFragment the function
      // starts looking for a URL Code at that position and therefore finds one.
      expect(EddystoneURL._encodeURLFragment('example.com/', 7)).to.eql({
        encoded: 0x0,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.org/', 7)).to.eql({
        encoded: 0x1,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.edu/', 7)).to.eql({
        encoded: 0x2,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.net/', 7)).to.eql({
        encoded: 0x3,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.info/', 7)).to.eql({
        encoded: 0x4,
        realLength: 6
      });
      expect(EddystoneURL._encodeURLFragment('example.biz/', 7)).to.eql({
        encoded: 0x5,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.gov/', 7)).to.eql({
        encoded: 0x6,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.com', 7)).to.eql({
        encoded: 0x7,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.org', 7)).to.eql({
        encoded: 0x8,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.edu', 7)).to.eql({
        encoded: 0x9,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.net', 7)).to.eql({
        encoded: 0xa,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.info', 7)).to.eql({
        encoded: 0xb,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.biz', 7)).to.eql({
        encoded: 0xc,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.gov', 7)).to.eql({
        encoded: 0xd,
        realLength: 4
      });
    });

    it('URL code in the middle', () => {
      expect(EddystoneURL._encodeURLFragment('example.com/?q=10', 7)).to.eql({
        encoded: 0x0,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.org/?q=10', 7)).to.eql({
        encoded: 0x1,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.edu/?q=10', 7)).to.eql({
        encoded: 0x2,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.net/?q=10', 7)).to.eql({
        encoded: 0x3,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.info/?q=10', 7)).to.eql({
        encoded: 0x4,
        realLength: 6
      });
      expect(EddystoneURL._encodeURLFragment('example.biz/?q=10', 7)).to.eql({
        encoded: 0x5,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.gov/?q=10', 7)).to.eql({
        encoded: 0x6,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.com?q=10', 7)).to.eql({
        encoded: 0x7,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.org?q=10', 7)).to.eql({
        encoded: 0x8,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.edu?q=10', 7)).to.eql({
        encoded: 0x9,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.net?q=10', 7)).to.eql({
        encoded: 0xa,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.info?q=10', 7)).to.eql({
        encoded: 0xb,
        realLength: 5
      });
      expect(EddystoneURL._encodeURLFragment('example.biz?q=10', 7)).to.eql({
        encoded: 0xc,
        realLength: 4
      });
      expect(EddystoneURL._encodeURLFragment('example.gov?q=10', 7)).to.eql({
        encoded: 0xd,
        realLength: 4
      });
    });

    it('No url code', () => {
      expect(EddystoneURL._encodeURLFragment('example.com', 0)).to.eql({
        encoded: 101, // ASCII 'e'
        realLength: 1
      });
    });

    it('Invalid characters', () => {
      expect(() => EddystoneURL._encodeURLFragment(String.fromCharCode(0x0), 0)).to.throw(Error);
      expect(() => EddystoneURL._encodeURLFragment(String.fromCharCode(0x20), 0)).to.throw(Error);
      expect(() => EddystoneURL._encodeURLFragment(String.fromCharCode(0x7f), 0)).to.throw(Error);

      expect(() => EddystoneURL._encodeURLFragment(String.fromCharCode(0x21), 0)).to.not.throw();
      expect(() => EddystoneURL._encodeURLFragment(String.fromCharCode(0x7e), 0)).to.not.throw();
    });
  });

  describe('encodeURL()', () => {
    it('https://www.example.com', () => {
      expect(EddystoneURL.encodeURL('https://www.example.com')).to.eql([
        1, // https://www
        101, 120, 97, 109, 112, 108, 101, // example
        7 // .com
      ]);
    });

    it('https://inbox.google.com/u/0/?pli=1', () => {
      let url = 'https://inbox.google.com/u/0/?pli=1';
      expect(() => EddystoneURL.encodeURL(url)).to.throw(/longer/);
    });
  });

  describe('constructServiceData()', () => {
    it('Invalid Tx Power', () => {
      let url = 'https://www.example.com';
      expect(() => EddystoneURL.constructServiceData(url, -101)).to.throw(Error);
      expect(() => EddystoneURL.constructServiceData(url, -100)).to.not.throw();
      expect(() => EddystoneURL.constructServiceData(url, 20)).to.not.throw();
      expect(() => EddystoneURL.constructServiceData(url, 21)).to.throw(Error);
    });

    it('Valid URL and valid Tx Power', () => {
      expect(EddystoneURL.constructServiceData(
        'https://www.example.com', 10)).to.eql([
          0x10, // URL Frame Type
          10, // Tx Power
          1, // https://www
          101, 120, 97, 109, 112, 108, 101, // example
          7 // .com
        ]);
    });
  });
});
