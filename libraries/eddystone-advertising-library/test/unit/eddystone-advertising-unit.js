// jshint node: true
// We need this so chai `expect` statements don't throw an error.
// jshint expr: true

// Ignore vars in require statements
// jshint ignore:start
var chai = require('chai');
var sinon = require('sinon');
var expect = chai.expect;
var chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);

var Eddystone = require('../../lib/eddystone-advertising.js');
// jshint ignore:end

describe('Eddystone', () => {
  'use strict';

  // Tell platform.js we are running tests.
  before(() => global._eddystone_test = true);
  // Remove global.
  after(() => global._eddystone_test = undefined);

  describe('checkURLOptions', () => {
    it('No url, no txPower', () => {
      expect(() => Eddystone._checkURLOptions({})).to.throw(TypeError, /url/);
    });

    it('No url, yes txPower', () => {
      expect(() => Eddystone._checkURLOptions({txPower: 0})).to.throw(TypeError, /url|txPower/);
    });

    it('Yes url, no txPower', () => {
      expect(() => Eddystone._checkURLOptions({url: ''})).to.throw(TypeError, /txPower/);
    });

    it('Yes url, yes txPower', () => {
      expect(() => Eddystone._checkURLOptions({url: '', txPower: 0})).to.not.throw();
    });
  });

  describe('checkAdvertisementOptions()', () => {
    describe('general', () => {
      it('Unsupported type, no url, no txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({type: 'uid'}))
                              .to.throw(TypeError, /Frame Type/);
      });

      it('No type', () => {
        expect(() => Eddystone._checkAdvertisementOptions({})).to.throw(TypeError, /type/);
      });
    });

    describe('url', () => {
      it('No type, no url, no txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({}))
                              .to.throw(TypeError, /type/);
      });

      it('Yes type, no url, no txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({type: 'url'}))
                              .to.throw(TypeError, /url|txPower/);
      });

      it('No type, no url, yes txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({txPower: 0}))
                              .to.throw(TypeError, /type/);
      });

      it('Yes type, no url, yes txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          txPower: 0
        })).to.throw(TypeError, /url/);
      });

      it('No type, yes url, no txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({url: ''}))
                              .to.throw(TypeError, /type/);
      });

      it('Yes type, yes url, no txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          url: ''
        })).to.throw(TypeError, /txPower/);
      });

      it('No type, yes url, yes txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          url: '',
          txPower: 0
        })).to.throw(TypeError, /type/);
      });

      it('Yes type, yes url, yes txPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          url: '',
          txPower: 0
        })).to.not.throw();
      });
    });
  });

  describe('registerAdvertisement()', () => {
    let options = {
      type: 'url',
      url: 'http://example.com',
      txPower: -20
    };

    it('Invalid options', () => {
      let eddystone = new Eddystone();
      return expect(eddystone.registerAdvertisement({}))
                             .to.be.rejected;
    });

    it('Successfully register advertisement', () => {
      let mockAdvertisement = {};
      let eddystone = new Eddystone();
      eddystone._platform.registerAdvertisement = sinon.stub().returns(
        Promise.resolve(mockAdvertisement));

      return expect(eddystone
        .registerAdvertisement(options)).to.eventually.eql(mockAdvertisement)
        .then(() => {
          expect(eddystone.advertisements).to.include(mockAdvertisement);
        });
    });

    it('Fail to register advertisement', () => {
      let error = new Error('Failed');
      let eddystone = new Eddystone();
      eddystone._platform.registerAdvertisement = sinon.stub().returns(
        Promise.reject(error));
      // Use sinon.stub(object, 'method') so that we can use
      // object.method.restore() after the test finishes.

      return expect(eddystone
        .registerAdvertisement(options)).to.be.rejectedWith(error);
    });
  });
});
