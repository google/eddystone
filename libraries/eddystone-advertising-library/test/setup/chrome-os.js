// jshint ignore:start
// TextEncoder is a Web API so we use the text-encoding polyfill.
var TextEncoder = require('text-encoding').TextEncoder;
var sinon = require('sinon');
// Since we are simulating a browser:
var window = global;
// jshint ignore:end

// Helper functions to mock the bluetoothLowEnergy API.
(() => {
  'use strict';
  // Keep track of whether chrome is clean or not.
  let _clean = false;

  // This method cleans the `chrome` mock object so that
  // a test can clean up after modifying it.
  global.cleanChromeMock = function() {
  // `chrome` is the object to expose APIs to Chrome Apps.
    global.chrome = {
      bluetoothLowEnergy: {},
      runtime: {}
    };
    _clean = true;
  };

  // This method sets up `chrome` mock so registering a BLE advertisment fails.
  global.mockRegisteringFailsWithMessage = function() {
    _checkCleanChromeMock();
    _clean = false;

    let fail_to_register = sinon.stub();
    fail_to_register.onFirstCall().callsArgAsync(1);
    fail_to_register.throws(new Error('This stub should only be used once.'));
    chrome.bluetoothLowEnergy.registerAdvertisement = fail_to_register;

    chrome.runtime.lastError = {message: 'Failed to register advertisement.'};
  };

  // This method sets up `chrome` mock so that registering a BLE advertisement
  // succeeds.
  global.mockRegisteringSucceeds = function() {
    _checkCleanChromeMock();
    _clean = false;
    let register = sinon.stub();
    register.onFirstCall().callsArgWithAsync(1);
    chrome.bluetoothLowEnergy.registerAdvertisement = register;
  };

  // This method sets up `chrome` mock so that unregistering a BLE advertisement
  // fails.
  global.mockUnregisteringFails = function() {
    _checkCleanChromeMock();
    _clean = false;
    let unregister = sinon.stub();
    unregister.onFirstCall().callsArgAsync(1);
    chrome.bluetoothLowEnergy.unregisterAdvertisement = unregister;

    chrome.runtime.lastError = {message: 'Failed to unregister.'};
  };

  // This method sets up `chrome` mock so that unregistering a BLE advertisement
  // succeeds.
  global.mockUnregisteringSucceeds = function() {
    _checkCleanChromeMock();
    _clean = false;
    let unregister = sinon.stub();
    unregister.onFirstCall().callsArgAsync(1);
    chrome.bluetoothLowEnergy.unregisterAdvertisement = unregister;
  };

  function _checkCleanChromeMock() {
    if (!_clean) {
      throw new Error('Need to clean chrome before starting another test.');
    }
  }
})();
cleanChromeMock();
