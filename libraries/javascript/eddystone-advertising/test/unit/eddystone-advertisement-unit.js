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

var EddystoneAdvertisement = require('../../lib/eddystone-advertisement.js').EddystoneAdvertisement;
// jshint ignore:end

describe('EddystoneAdvertisement', () => {
  'use strict';

  // Tell platform.js we are running tests.
  before(() => global._eddystone_test = true);
  // Remove global.
  after(() => global._eddystone_test = undefined);

  describe('constructor()', () => {
    it('Check assignment', () => {
      let id = 100;
      let type = 'url';
      let url = 'http://www.example.com';
      let tx_power = -10;
      let adv = new EddystoneAdvertisement(
        id, {
          type: type,
          url: url,
          txPower: tx_power
        },
        {});
      expect(adv.id).to.eql(id);
      expect(adv.type).to.eql(type);
      expect(adv.url).to.eql(url);
      expect(adv.txPower).to.eql(tx_power);
      expect(adv._platform).to.eql({});
    });
  });

  describe('unregisterAdvertisement()', () => {
    it('Unregistering fails', () => {
      let advertisement = new EddystoneAdvertisement(100 /* id */, {
        type: 'url',
        url: 'https://www.example.com/',
        txPower: -10
      }, {});
      let error = new Error('Failed');
      advertisement._platform.unregisterAdvertisement = sinon.stub().returns(Promise.reject(error));
      return expect(advertisement.unregisterAdvertisement()).to.be.rejectedWith(error);
    });

    it('Unregistering succeeds', () => {
      let advertisement = new EddystoneAdvertisement(100 /* id */, {
        type: 'url',
        url: 'https://www.example.com/',
        txPower: -10
      }, {});
      advertisement._platform.unregisterAdvertisement = sinon.stub().returns(
        Promise.resolve());
      return expect(advertisement.unregisterAdvertisement()).to.be.fulfilled;
    });
  });
});
