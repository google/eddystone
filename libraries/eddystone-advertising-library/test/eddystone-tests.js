// jshint node: true
// We need this so chai expect's don't throw an error.
// jshint expr: true

// Ignore vars in require statements
// jshint ignore:start
var chai = require('chai');
var sinon = require('sinon');
var expect = chai.expect;
// jshint ignore:end

describe('Eddystone', () => {
  describe('global', () => {
    it('eddystone should exist in window', () => {
      expect(window).to.have.property('eddystone');
    });
  });
  describe('getPlatform()', () => {
    it('Make sure Eddystone recognizes the platform', () => {
      expect(() => Eddystone._getPlatform()).not.to.throw();
      // TODO: Mock each platform. Since we only support ChromeOS
      // we can expect the test to return the EddystoneChromeOS class.
      expect(Eddystone._getPlatform()).to.eql(EddystoneChromeOS);
    });
  });
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
    'use strict';
    let url = 'http://example.com';
    let tx_power = -20;
    let options = {
      type: 'url',
      url: url,
      txPower: tx_power
    };
    // Clean up after adding advertisements.
    afterEach(() => {
      if (eddystone._platform.registerAdvertisement.restore) {
        eddystone._platform.registerAdvertisement.restore();
      }
      eddystone.advertisements = [];
    });
    it('Invalid options', () => {
      return expect(eddystone.registerAdvertisement({}))
                             .to.be.rejected;
    });
    it('Successfully register advertisement', () => {
      let mockAdvertisement = {
        id: 100,
        type: 'url',
        url: url,
        txPower: tx_power
      };
      // Use sinon.stub(object, 'method') so that we can use
      // object.method.restore() after the test finishes.
      sinon.stub(eddystone._platform, 'registerAdvertisement').returns(Promise.resolve(mockAdvertisement));

      return expect(eddystone.registerAdvertisement(options))
                             .to.eventually.eql(mockAdvertisement)
                             .then(() => expect(eddystone.advertisements)
                                                         .to.include(mockAdvertisement));
    });
    it('Fail to register advertisement', () => {
      let error = new Error('Failed');
      // Use sinon.stub(object, 'method') so that we can use
      // object.method.restore() after the test finishes.
      sinon.stub(eddystone._platform, 'registerAdvertisement')
           .returns(Promise.reject(error));
      return expect(eddystone.registerAdvertisement(options))
                             .to.be.rejectedWith(error);
    });
  });
  describe('unregister()', () => {
    'use strict';
    // Hooks
    afterEach(() => {
      if (eddystone._platform.unregisterAdvertisement.restore) {
        eddystone._platform.unregisterAdvertisement.restore();
      }
      eddystone.advertisements = [];
    });

    it('Fail to unregister advertisement', () => {
      let advertisement = new EddystoneAdvertisement(100 /* id */, {
        type: 'url',
        url: 'https://www.example.com/',
        txPower: -10
      });
      let error = new Error('Failed');
      sinon.stub(eddystone._platform, 'unregisterAdvertisement').returns(Promise.reject(error));
      return expect(advertisement.unregisterAdvertisement()).to.be.rejectedWith(error);
    });
  });
});
