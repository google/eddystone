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

var EddystoneAdvertisement = require('../../lib/eddystone-advertisement.js').EddystoneAdvertisement;;
var EddystoneURL = require('../../lib/eddystone-url.js');
var EddystoneChromeOS = require('../../lib/eddystone-chrome-os.js');
// jshint ignore:end

describe('EddystoneChromeOS', () => {
  'use strict';
  after(() => global.chrome = undefined);

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
                                       'id', 'url', 'txPower', 'type',
                                       '_platform');
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
  describe('unregisterAdvertisement()', () => {
    // Hooks
    afterEach(() => cleanChromeMock());

    it('Unregistering fails', () => {
      mockUnregisteringFails();
      return expect(EddystoneChromeOS.unregisterAdvertisement({id: 1}))
                                     .to.be.rejected;
    });

    it('Unregistering succeeds', () => {
      mockUnregisteringSucceeds();
      return expect(EddystoneChromeOS.unregisterAdvertisement({id: 1}))
                                     .to.be.fulfilled;
    });
  });
});

function cleanChromeMock() {
  global.chrome = undefined;
}

// This method sets up `chrome` mock so registering a BLE advertisment fails.
function mockRegisteringFailsWithMessage() {
  'use strict';
  _checkAndSetChromeMock();

  let fail_to_register = sinon.stub();
  fail_to_register.onFirstCall().callsArgAsync(1);
  fail_to_register.throws(new Error('This stub should only be used once.'));
  chrome.bluetoothLowEnergy.registerAdvertisement = fail_to_register;

  chrome.runtime.lastError = {message: 'Failed to register advertisement.'};
}

// This method sets up `chrome` mock so that registering a BLE advertisement
// succeeds.
function mockRegisteringSucceeds() {
  'use strict';
  _checkAndSetChromeMock();

  let register = sinon.stub();
  register.onFirstCall().callsArgWithAsync(1);
  chrome.bluetoothLowEnergy.registerAdvertisement = register;
}
  // This method sets up `chrome` mock so that unregistering a BLE advertisement
  // fails.
function mockUnregisteringFails() {
  'use strict';
  _checkAndSetChromeMock();

  let unregister = sinon.stub();
  unregister.onFirstCall().callsArgAsync(1);
  chrome.bluetoothLowEnergy.unregisterAdvertisement = unregister;

  chrome.runtime.lastError = {message: 'Failed to unregister.'};
}

// This method sets up `chrome` mock so that unregistering a BLE advertisement
// succeeds.
function mockUnregisteringSucceeds() {
  'use strict';
  _checkAndSetChromeMock();

  let unregister = sinon.stub();
  unregister.onFirstCall().callsArgAsync(1);
  chrome.bluetoothLowEnergy.unregisterAdvertisement = unregister;
}

function _checkAndSetChromeMock() {
  'use strict' ;
  if (typeof global.chrome !== 'undefined') {
    throw new Error('Need to clean chrome before starting another test.');
  }
  // `chrome` is the object to expose APIs to Chrome Apps.
  global.chrome = {
    bluetoothLowEnergy: {},
    runtime: {},
    // Keep track of whether chrome is clean or not.
  };
}
