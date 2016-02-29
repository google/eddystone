// jshint node: true
// We need this so chai `expect` statements don't throw an error.
// jshint expr: true
'use strict';

import chai, {expect} from 'chai';
import sinon from 'sinon';
import chaiAsPromised from 'chai-as-promised';
chai.use(chaiAsPromised);

import {EddystoneAdvertisement} from '../../lib/eddystone-advertisement.js';
// jshint ignore:end

describe('EddystoneAdvertisement', () => {
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
          advertisedTxPower: tx_power
        },
        {});
      expect(adv.id).to.eql(id);
      expect(adv.type).to.eql(type);
      expect(adv.url).to.eql(url);
      expect(adv.advertisedTxPower).to.eql(tx_power);
      expect(adv._platform).to.eql({});
    });
  });

  describe('unregisterAdvertisement()', () => {
    it('Unregistering fails', () => {
      let advertisement = new EddystoneAdvertisement(100 /* id */, {
        type: 'url',
        url: 'https://www.example.com/',
        advertisedTxPower: -10
      }, {});
      let error = new Error('Failed');
      advertisement._platform.unregisterAdvertisement = sinon.stub().returns(Promise.reject(error));
      return expect(advertisement.unregisterAdvertisement()).to.be.rejectedWith(error);
    });

    it('Unregistering succeeds', () => {
      let advertisement = new EddystoneAdvertisement(100 /* id */, {
        type: 'url',
        url: 'https://www.example.com/',
        advertisedTxPower: -10
      }, {});
      advertisement._platform.unregisterAdvertisement = sinon.stub().returns(
        Promise.resolve());
      return expect(advertisement.unregisterAdvertisement()).to.be.fulfilled;
    });
  });
});
