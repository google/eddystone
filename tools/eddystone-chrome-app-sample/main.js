// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
(() => {
  'use strict';

  const FRAME_TYPE = ['url', 'uid'];
  let current_advertisement;

  // General
  let frame_type_dropdown = document.querySelector('#frame-type');
  // Eddystone URL
  let url_prefix_dropdown = document.querySelector('#url-prefix-dropdown');
  let url_prefix = document.querySelector('#url-prefix');
  let url_field = document.querySelector('#url');
  let adv_tx_power_url = document.querySelector('#adv-tx-power-url');
  // Eddystone UID
  let namespace_field = document.querySelector('#namespace');
  let instance_field = document.querySelector('#instance');
  let adv_tx_power_uid = document.querySelector('#adv-tx-power-uid');

  let update_button = document.querySelector('#update-button');
  let stop_button = document.querySelector('#stop-button');

  let result_toast = document.querySelector('#result-toast');
  let close_toast_button = document.querySelector('#close-toast');

  frame_type_dropdown.addEventListener('iron-select', event => {
    switch (getFrameType()) {
      case 'url':
        hideUIDFields();
        showURLFields();
        break;
      case 'uid':
        hideURLFields();
        showUIDFields();
    }
  });

  update_button.addEventListener('tap', () => {
    switch (getFrameType()) {
      case 'url':
        updateAdvertisement({
          type: getFrameType(),
          advertisedTxPower: getURLTxPower(),
          url: getURL()
        }).then(() => {
          showResult('Advertising. URL: ' + getURL() + ' ' +
                'Advertised Tx Power :' + getURLTxPower());
        }).catch(e => {
          showResult('Not advertising: ' + e.message);
        });
        break;
      case 'uid':
        updateAdvertisement({
          type: getFrameType(),
          advertisedTxPower: getUIDTxPower(),
          namespace: namespace_field.value,
          instance: instance_field.value
        }).then(() => {
          showResult('Advertising. Namespace: ' + namespace_field.value + ' ' +
                'Instance: ' + instance_field.value + ' ' +
                'Advertised Tx Power :' + getURLTxPower());
        }).catch(e => {
          showResult('Not advertising: ' + e.message);
        });
        break;
    }
  });
  stop_button.addEventListener('tap', () => {
    if (current_advertisement) {
      current_advertisement.unregisterAdvertisement()
        .then(() => {
          current_advertisement = undefined;
          result_toast.duration = 3;
          showResult('Not advertising.');
        });
    }
  });
  close_toast_button.addEventListener('tap', () => {
    result_toast.toggle();
  });

  function getFrameType() {
    if (frame_type_dropdown.selected === 0) {
      return FRAME_TYPE[0]; // URL
    } else if (frame_type_dropdown.selected === 1) {
      return FRAME_TYPE[1]; // UID
    } else {
      throw Error('Invalid Frame Type');
    }
  }
  function getURLTxPower() {
    return parseInt(adv_tx_power_url.value, 10);
  }
  function getUIDTxPower() {
    return parseInt(adv_tx_power_uid.value, 10);
  }
  function getURL() {
    let url = '';
    if (url_prefix.selected === 0 || url_prefix.selected === '0') {
      url += 'https://';
    } else if (url_prefix.selected === 1 || url_prefix.selected === '1') {
      url += 'http://';
    } else {
      throw Error('Unsupported prefix');
    }
    url += url_field.value;
    return url;
  }

  function showURLFields() {
    adv_tx_power_url.hidden = false;
    url_prefix_dropdown.hidden = false;
    url_field.hidden = false;
    update_button.hidden = false;
    stop_button.hidden = false;
  }
  function showUIDFields() {
    adv_tx_power_uid.hidden = false;
    namespace_field.hidden = false;
    instance_field.hidden = false;
    update_button.hidden = false;
    stop_button.hidden = false;
  }
  function hideURLFields() {
    adv_tx_power_url.hidden = true;
    url_prefix_dropdown.hidden = true;
    url_field.hidden = true;
  }
  function hideUIDFields() {
    adv_tx_power_uid.hidden = true;
    namespace_field.hidden = true;
    instance_field.hidden = true;
  }

  function updateAdvertisement(options) {
    let promise = Promise.resolve();
    if (current_advertisement) {
      promise = promise
        .then(() => current_advertisement.unregisterAdvertisement())
        .then(() => current_advertisement = undefined);
    }
    return promise
      .then(() => eddystone.registerAdvertisement(options))
      .then(adv => current_advertisement = adv);
  }
  function showResult(message) {
    result_toast.duration = 0;
    result_toast.text = message;
    result_toast.open();
  }
})();
