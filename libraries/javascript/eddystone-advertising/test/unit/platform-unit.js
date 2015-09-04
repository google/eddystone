// jshint node: true
// We need this so chai `expect` statements don't throw an error.
// jshint expr: true

// Ignore vars in require statements
// jshint ignore:start
var chai = require('chai');
var expect = chai.expect;

var platform = require('../../lib/platform.js');
var EddystoneChromeOS = require('../../lib/eddystone-chrome-os.js');
// jshint ignore:end

describe('Platform', () => {
  'use strict';

  describe('platform()', () => {
    // Hooks
    afterEach(() => {
      global._eddystone_test = undefined;
      global.chrome = undefined;
    });

    it('unsupported', () => {
      expect(() => platform()).to.throw(/Unsupported platform./);
    });

    it('chrome', () => {
      global.chrome = {};
      expect(platform()).to.eql(EddystoneChromeOS);
    });

    it('test', () => {
      global._eddystone_test = {};
      expect(platform()).to.eql({});
    });
  });
});
