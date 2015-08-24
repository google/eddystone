// jshint node: true
// We need this so chai expect's don't throw an error.
// jshint expr: true

// Ignore vars in require statements
// jshint ignore:start
var chai = require('chai');
var chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);
var expect = chai.expect;
// jshint ignore:end

describe('EddystoneChromeOS', () => {
  describe('global', () => {
    it('EddystoneChromeOS should be global', () => {
      expect(typeof EddystoneChromeOS).to.not.equal('undefined');
    });
  });
  describe('constructAdvertisement()', () => {
    it('Unsupported frame type', () => {
        expect(() => EddystoneChromeOS._constructAdvertisement({type: 'url'}))
                                      .not.to.throw(/Unsupported Frame Type/);
        expect(() => EddystoneChromeOS._constructAdvertisement({type: 'uid'}))
                                      .to.throw(Error, /Unsupported Frame Type/);
        expect(() => EddystoneChromeOS._constructAdvertisement({type: 'tlm'}))
                                      .to.throw(Error, /Unsupported Frame Type/);
        expect(() => EddystoneChromeOS._constructAdvertisement({}))
                                      .to.throw(Error, /Unsupported Frame Type/);
    });
    describe('Eddystone-URL', () => {
      'use strict';
      let url = 'https://www.example.com';
      let tx_power = -10;
      it('Valid URL and Tx Power', () => {
        expect(EddystoneChromeOS._constructAdvertisement({
          type: 'url',
          url: url,
          txPower: tx_power
        })).to.eql({
          type: 'broadcast',
          serviceUuids: ['FEAA'],
          serviceData: [{
            uuid: 'FEAA',
            data: EddystoneURL.constructServiceData(url, tx_power)
          }]
        });
      });
      it('Valid URL, valid Tx Power', () => {
        expect(() => EddystoneChromeOS._constructAdvertisement({
          type: 'url',
          url: url,
          txPower: -101
        })).to.throw(Error, /Invalid Tx Power value/);
      });
      it('Invalid URL, invalid Tx Power', () => {
        expect(() => EddystoneChromeOS._constructAdvertisement({
          type: 'url',
          url: 'INVALID',
          txPower: tx_power
        })).to.throw(Error, /Scheme/);
        expect(() => EddystoneChromeOS._constructAdvertisement({
          type: 'url',
          url: 'http://' + String.fromCharCode(0x0),
          txPower: tx_power
        })).to.throw(Error, /character/);
      });
      it('Invalid URL, invalid Tx Power', () => {
        expect(() => EddystoneChromeOS._constructAdvertisement({
          type: 'url',
          url: 'INVALID',
          txPower: -101
        })).to.throw(Error);
        expect(() => EddystoneChromeOS._constructAdvertisement({
          type: 'url',
          url: 'http://' + String.fromCharCode(0x0),
          txPower: -101
        })).to.throw(Error);
      });
    });
  });
  describe('registerAdvertisement()', () => {
    'use strict';
    let valid_options = {
      type: 'url',
      url: 'https://www.example.com',
      txPower: -10
    };

    // Hooks
    afterEach(() => cleanChromeMock());

    it('Registering fails', () => {
      mockRegisteringFailsWithMessage();
      return expect(EddystoneChromeOS.registerAdvertisement(valid_options))
                                     .to.be.rejected;
    });
    it('Registering succeeds. Huzzah!', () => {
      mockRegisteringSucceeds();
      return expect(EddystoneChromeOS.registerAdvertisement(valid_options))
                                     .to.eventually.have.all.keys(
                                       'id', 'url', 'txPower', 'type');
    });
    describe('Unsupported frame types', () => {
      it('undefined', () => {
        return expect(EddystoneChromeOS.registerAdvertisement({}))
                                       .to.be.rejectedWith(Error);
      });
      it('uid', () => {
        return expect(EddystoneChromeOS.registerAdvertisement({type: 'uid'}))
                                       .to.be.rejectedWith(Error);
      });
      it('tlm', () => {
        return expect(EddystoneChromeOS.registerAdvertisement({type: 'tlm'}))
                                       .to.be.rejectedWith(Error);
      });
    });
  });
});
