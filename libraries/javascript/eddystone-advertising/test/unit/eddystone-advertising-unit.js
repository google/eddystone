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
        expect(() => Eddystone._checkAdvertisementOptions({type: 'tlm'}))
                              .to.throw(TypeError, /Frame Type/);
      });

      it('No type', () => {
        expect(() => Eddystone._checkAdvertisementOptions({})).to.throw(TypeError, /type/);
      });
    });

    describe('url', () => {
      [{
        name: 'No  type, no url, no advertisedTxPower',
        options: {},
        errorRegex: /type/
      }, {
        name: 'Yes type, no url, no advertisedTxPower',
        options: {type: 'url'},
        errorRegex: /url|advertisedTxPower/
      }, {
        name: 'No type, no url, yes advertisedTxPower',
        options: {advertisedTxPower: 0},
        errorRegex: /type/
      }, {
        name: 'Yes type, no url, yes advertisedTxPower',
        options: {type: 'url', advertisedTxPower: 0},
        errorRegex: /url/
      }, {
        name: 'No type, yes url, no advertisedTxPower',
        options: {url: ''},
        errorRegex: /type/
      }, {
        name: 'Yes type, yes url, no advertisedTxPower',
        options: {type: 'url', url: ''},
        errorRegex: /advertisedTxPower/
      }, {
        name: 'No type, yes url, yes advertisedTxPower',
        options: {url: '', advertisedTxPower: 0},
        errorRegex: /type/
      }].forEach(test => {
        it(test.name, () => {
          expect(() => Eddystone._checkAdvertisementOptions(test.options))
            .to.throw(TypeError, test.errorRegex);
        });
      });

      it('Yes type, yes url, yes advertisedTxPower', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'url',
          url: '',
          advertisedTxPower: 0
        })).to.not.throw();
      });
    });
    describe('uid', () => {
      let test_namespace = '00000000000000000000';
      let test_instance = '000000000000';
      [{
        name: 'No type, no advertisedTxPower, no namespace, no instance',
        options: {},
        errorRegex: /type|advertisedTxPower|namespace|instance/
      }, {
        name: 'No type, no advertisedTxPower, no namespace, yes instance',
        options: {instance: test_instance},
        errorRegex: /type|advertisedTxPower|namespace/
      }, {
        name: 'No type, no advertisedTxPower, yes namespace, no instance',
        options: {namespace: test_namespace},
        errorRegex: /type|advertisedTxPower|instance/
      }, {
        name: 'No type, no advertisedTxPower, yes namespace, yes instance',
        options: {namespace: test_namespace, instance: test_instance},
        errorRegex: /type|advertisedTxPower/
      }, {
        name: 'No type, yes advertisedTxPower, no namespace, no instance',
        options: {advertisedTxPower: -10},
        errorRegex: /type|namespace|instance/
      }, {
        name: 'No type, yes advertisedTxPower, no namespace, yes instance',
        options: {advertisedTxPower: -10, instance: test_instance},
        errorRegex: /type|namespace/
      }, {
        name: 'No type, yes advertisedTxPower, yes namespace, no instance',
        options: {advertisedTxPower: -10, namespace: test_namespace},
        errorRegex: /type|instance/
      }, {
        name: 'No type, yes advertisedTxPower, yes namespace, yes instance',
        options: {advertisedTxPower: -10, namespace: test_namespace,
                  instance: test_instance},
        errorRegex: /type/
      }, {
        name: 'Yes type, no advertisedTxPower, no namespace, no instance',
        options: {type: 'uid'},
        errorRegex: /advertisedTxPower|namespace|instance/
      }, {
        name: 'Yes type, no advertisedTxPower, no namespace, yes instance',
        options: {type: 'uid', instance: test_instance},
        errorRegex: /advertisedTxPower|namespace/
      }, {
        name: 'Yes type, no advertisedTxPower, yes namespace, no instance',
        options: {type: 'uid',namespace: test_namespace},
        errorRegex: /advertisedTxPower|instance/
      }, {
        name: 'Yes type, no advertisedTxPower, yes namespace, yes instance',
        options: {type: 'uid', namespace: test_namespace,
                  instance: test_instance},
        errorRegex: /advertisedTxPower/
      }, {
        name: 'Yes type, yes advertisedTxPower, no namespace, no instance',
        options: {type: 'uid', advertisedTxPower: -10},
        errorRegex: /namespace|instance/
      }, {
        name: 'Yes type, yes advertisedTxPower, no namespace, yes instance',
        options: {type: 'uid', advertisedTxPower: -10,
                  instance: test_instance},
        errorRegex: /namespace/
      }, {
        name: 'Yes type, yes advertisedTxPower, yes namespace, no instance',
        options: {type: 'uid', advertisedTxPower: -10,
                  namespace: test_namespace},
        errorRegex: /instance/
      }].forEach(test => {
        it(test.name, () => {
          expect(() => Eddystone._checkAdvertisementOptions(test.options))
            .to.throw(TypeError, test.errorRegex);
        });
      });
      it('Yes type, yes advertisedTxPower, yes namespace, yes instance', () => {
        expect(() => Eddystone._checkAdvertisementOptions({
          type: 'uid', advertisedTxPower: -10,
          namespace: test_namespace, instance: test_instance
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
