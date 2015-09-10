(() => {
  'use strict';

  // Only require top level module. Browserify will walk
  // the dependency graph and load all needed modules.
  let Eddystone = require('./eddystone-advertising.js');

  // browserify will replace global with window.
  global.eddystone = new Eddystone();
})();
