// jshint node: true
// We need this so chai expect's don't throw an error.
// jshint expr: true

// Ignore vars in require statements
// jshint ignore:start
var chai = require('chai');
var expect = chai.expect;
// jshint ignore:end

describe('Eddystone', () => {
  describe('global', () => {
    it('eddystone should exist in window', () => {
      expect(window).to.have.property('eddystone');
    });
    it('Eddystone class shouldn\'t be global', () => {
      expect(typeof Eddystone).to.equal('undefined');
    });
  });
});
