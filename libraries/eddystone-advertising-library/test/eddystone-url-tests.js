// jshint node: true
// We need this so chai expect's don't throw an error.
// jshint expr: true

// jshint ignore:start
var chai = require('chai');
var expect = chai.expect;
// jshint ignore:end

describe('Eddystone-URL', () => {
  describe('global', () => {
    it('EddystoneURL should be in window', () => {
      expect(window).to.have.property('EddystoneURL').that.is.a('function');
    });
  });
  describe('encodeURLScheme', () => {
    it('http://www.', () => {
      expect(window.EddystoneURL._encodeURLScheme('http://www.example.com')).to.eql({
        length: 11,
        encoded: 0});
    });
    it('https://www.', () => {
      expect(window.EddystoneURL._encodeURLScheme('https://www.example.com')).to.eql({
        length: 12,
        encoded: 1});
    });
    it('http://', () => {
      expect(window.EddystoneURL._encodeURLScheme('http://example.com')).to.eql({
        length: 7,
        encoded: 2});
    });
    it('https://', () => {
      expect(window.EddystoneURL._encodeURLScheme('https://example.com')).to.eql({
        length: 8,
        encoded: 3});
    });
    it('Empty URL', () => {
      expect(() => window.EddystoneURL._encodeURLScheme('')).to.throw(Error);
    });
    it('http:/', () => {
      expect(() => window.EddystoneURL._encodeURLScheme('http:/')).to.throw(Error);
    });
  });
  describe('encodeURLFragment()', () => {
    it('URL code at the beginning', () => {
      expect(window.EddystoneURL._encodeURLFragment('.org/', 0)).to.eql({
        encoded: 0x1,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('.edu/', 0)).to.eql({
        encoded: 0x2,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('.net/', 0)).to.eql({
        encoded: 0x3,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('.info/', 0)).to.eql({
        encoded: 0x4,
        realLength: 6
      });
      expect(window.EddystoneURL._encodeURLFragment('.biz/', 0)).to.eql({
        encoded: 0x5,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('.gov/', 0)).to.eql({
        encoded: 0x6,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('.com', 0)).to.eql({
        encoded: 0x7,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('.org', 0)).to.eql({
        encoded: 0x8,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('.edu', 0)).to.eql({
        encoded: 0x9,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('.net', 0)).to.eql({
        encoded: 0xa,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('.info', 0)).to.eql({
        encoded: 0xb,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('.biz', 0)).to.eql({
        encoded: 0xc,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('.gov', 0)).to.eql({
        encoded: 0xd,
        realLength: 4
      });
    });
    it('URL code at the end', () => {
      // Since 7 is the argument passed to _encodeURLFragment the function
      // starts looking for a URL Code at that position and therefore finds one.
      expect(window.EddystoneURL._encodeURLFragment('example.com/', 7)).to.eql({
        encoded: 0x0,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.org/', 7)).to.eql({
        encoded: 0x1,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.edu/', 7)).to.eql({
        encoded: 0x2,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.net/', 7)).to.eql({
        encoded: 0x3,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.info/', 7)).to.eql({
        encoded: 0x4,
        realLength: 6
      });
      expect(window.EddystoneURL._encodeURLFragment('example.biz/', 7)).to.eql({
        encoded: 0x5,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.gov/', 7)).to.eql({
        encoded: 0x6,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.com', 7)).to.eql({
        encoded: 0x7,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.org', 7)).to.eql({
        encoded: 0x8,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.edu', 7)).to.eql({
        encoded: 0x9,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.net', 7)).to.eql({
        encoded: 0xa,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.info', 7)).to.eql({
        encoded: 0xb,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.biz', 7)).to.eql({
        encoded: 0xc,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.gov', 7)).to.eql({
        encoded: 0xd,
        realLength: 4
      });
    });
    it('URL code in the middle', () => {
      expect(window.EddystoneURL._encodeURLFragment('example.com/?q=10', 7)).to.eql({
        encoded: 0x0,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.org/?q=10', 7)).to.eql({
        encoded: 0x1,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.edu/?q=10', 7)).to.eql({
        encoded: 0x2,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.net/?q=10', 7)).to.eql({
        encoded: 0x3,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.info/?q=10', 7)).to.eql({
        encoded: 0x4,
        realLength: 6
      });
      expect(window.EddystoneURL._encodeURLFragment('example.biz/?q=10', 7)).to.eql({
        encoded: 0x5,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.gov/?q=10', 7)).to.eql({
        encoded: 0x6,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.com?q=10', 7)).to.eql({
        encoded: 0x7,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.org?q=10', 7)).to.eql({
        encoded: 0x8,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.edu?q=10', 7)).to.eql({
        encoded: 0x9,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.net?q=10', 7)).to.eql({
        encoded: 0xa,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.info?q=10', 7)).to.eql({
        encoded: 0xb,
        realLength: 5
      });
      expect(window.EddystoneURL._encodeURLFragment('example.biz?q=10', 7)).to.eql({
        encoded: 0xc,
        realLength: 4
      });
      expect(window.EddystoneURL._encodeURLFragment('example.gov?q=10', 7)).to.eql({
        encoded: 0xd,
        realLength: 4
      });
    });
    it('No url code', () => {
      expect(window.EddystoneURL._encodeURLFragment('example.com', 0)).to.eql({
        encoded: 101, // ASCII 'e'
        realLength: 1
      });
    });
    it('Invalid characters', () => {
      expect(() => window.EddystoneURL._encodeURLFragment(String.fromCharCode(0x0), 0)).to.throw(Error);
      expect(() => window.EddystoneURL._encodeURLFragment(String.fromCharCode(0x20), 0)).to.throw(Error);
      expect(() => window.EddystoneURL._encodeURLFragment(String.fromCharCode(0x7f), 0)).to.throw(Error);

      expect(() => window.EddystoneURL._encodeURLFragment(String.fromCharCode(0x21), 0)).to.not.throw();
      expect(() => window.EddystoneURL._encodeURLFragment(String.fromCharCode(0x7e), 0)).to.not.throw();
    });
  });


  describe('encodeURL()', () => {
    it('https://www.example.com', () => {
      expect(window.EddystoneURL.encodeURL('https://www.example.com')).to.eql([
        1, // https://www
        101, 120, 97, 109, 112, 108, 101, // example
        7 // .com
      ]);
    });
    it('https://inbox.google.com/u/0/?pli=1', () => {
      expect(window.EddystoneURL.encodeURL('https://inbox.google.com/u/0/?pli=1')).to.eql([
        3, // https://
        105, 110, 98, 111, 120, 46, 103, 111, 111, 103, 108, 101,  // inbox.google
        0, // .com/
        117, 47, 48, 47, 63, 112, 108, 105, 61, 49 // u/0/?pli=1
      ]);
    });
  });
  describe('constructServiceData()', () => {
    it('Invalid Tx Power', () => {
      'use strict';
      let url = 'https://www.example.com';
      expect(() => window.EddystoneURL.constructServiceData(url, -101)).to.throw(Error);
      expect(() => window.EddystoneURL.constructServiceData(url, -100)).to.not.throw();
      expect(() => window.EddystoneURL.constructServiceData(url, 20)).to.not.throw();
      expect(() => window.EddystoneURL.constructServiceData(url, 21)).to.throw(Error);
    });
    it('Valid URL and valid Tx Power', () => {
      'use strict';
      expect(window.EddystoneURL.constructServiceData(
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
