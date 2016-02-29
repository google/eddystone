// jshint node: true
// We need this so chai `expect` statements don't throw an error.
// jshint expr: true
'use strict';

import chai, {expect} from 'chai';
import sinon from 'sinon';
import chaiAsPromised from 'chai-as-promised';
chai.use(chaiAsPromised);

import Eddystone from '../../lib/eddystone-advertising.js';

describe('Eddystone', () => {
  // Tell platform.js we are running tests.
  before(() => global._eddystone_test = true);
  // Remove global.
  after(() => global._eddystone_test = undefined);

  describe('checkURLOptions', () => {
    it('No url, no advertisedTxPower', () => {
      expect(() => Eddystone._checkURLOptions({})).to.throw(TypeError, /url/);
    });

    it('No url, yes advertisedTxPower', () => {
      expect(() => Eddystone._checkURLOptions({advertisedTxPower: 0})).to.throw(TypeError, /url|advertisedTxPower/);
    });

    it('Yes url, no advertisedTxPower', () => {
      expect(() => Eddystone._checkURLOptions({url: ''})).to.throw(TypeError, /advertisedTxPower/);
    });

    it('Yes url, yes advertisedTxPower', () => {
      expect(() => Eddystone._checkURLOptions({url: '', advertisedTxPower: 0})).to.not.throw();
    });
  });

  describe('checkAdvertisementOptions()', () => {
    describe('general', () => {
      it('Unsupported type, no url, no advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({type: 'uid'}))
                              .to.throw(TypeError, /Frame Type/);
      });

      it('No type', () => {
        expect(() => Eddystone._checkAdvertisementOptions({})).to.throw(TypeError, /type/);
      });
    });

    describe('url', () => {
      it('No type, no url, no advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({}))
                              .to.throw(TypeError, /type/);
      });

      it('Yes type, no url, no advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({type: 'url'}))
                              .to.throw(TypeError, /url|advertisedTxPower/);
      });

      it('No type, no url, yes advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({advertisedTxPower: 0}))
                              .to.throw(TypeError, /type/);
      });

      it('Yes type, no url, yes advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          advertisedTxPower: 0
        })).to.throw(TypeError, /url/);
      });

      it('No type, yes url, no advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({url: ''}))
                              .to.throw(TypeError, /type/);
      });

      it('Yes type, yes url, no advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          url: ''
        })).to.throw(TypeError, /advertisedTxPower/);
      });

      it('No type, yes url, yes advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          url: '',
          advertisedTxPower: 0
        })).to.throw(TypeError, /type/);
      });

      it('Yes type, yes url, yes advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          url: '',
          advertisedTxPower: 0
        })).to.not.throw();
      });
    });
  });

  describe('registerAdvertisement()', () => {
    let options = {
      type: 'url',
      url: 'http://example.com',
      advertisedTxPower: -20
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
