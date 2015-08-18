# Eddystone Advertising Library

The Eddystone Advertising Library abstracts low level concepts of the Eddystone
protocol and wraps existing Advertising APIs so that developers don't have to
worry about these low level details. The library exposes simple functions that
developers can use to advertise a Valid Eddystone packet from their device.

** NOTE ** Currently only ChromeOS is supported.

## Eddystone
The Eddystone Advertising Library creates `window.eddystone` with the following
properties and methods:

### Properties
```javascript
Eddystone.advertisements
```
An array containing all the previously registered [`EddystoneAdvertisement`](#eddystoneadvertisement)s.

### Methods
```javascript
Eddystone.registerAdvertisement(advertisement_options)
```

The `registerAdvertisement` function returns a promise. If the advertisment was
registered succesfully the promise will resolve with an
`EddystoneAdvertisement`. Otherwise the promise will reject with `error`.

#### Parameters
* `advertisement_options.type`: One of
  [`'url'`](https://github.com/google/eddystone/tree/master/eddystone-url),
  [`'uid'`](https://github.com/google/eddystone/tree/master/eddystone-uid), or
  [`'tlm'`](https://github.com/google/eddystone/tree/master/eddystone-tlm).
  For now, only `'url'` is supported.

If `advertisement_options.type === 'url'`:
* `advertisement_options.url`: the URL to broadcast.
* `advertisement_options.txPower`: the txPower value to broadcast.
   Determine this by measuring the actual output of your beacon from 1 meter
   away and then adding 41dBm to that. 41dBm is the signal loss that occurs over
   1 meter. See [Tx Power Level](https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level)

** NOTE ** For now the only supported Frame Type is the
[URL Frame Type](https://github.com/google/eddystone/tree/master/eddystone-url).
Support for the other frame types will be added in the future.

#### Example
```javascript
 eddystone.registerAdvertisement({
    type: 'url',
    url: 'https://example.com',
    txPower: -20
  }).then(advertisement => console.log("Advertising: " + advertisement.url))
    .catch(error => console.log(error.message));
```

## EddystoneAdvertisement
This is the object that holds the information about the registered BLE
Advertisement.

### Properties
* `EddystoneAdvertisement.id`: ID of the EddystoneAdvertisement, unique between
browser restarts.
* `EddystoneAdvertisement.type`: the EddystoneAdvertisement frame type:
  [`'url'`](https://github.com/google/eddystone/tree/master/eddystone-url),
  [`'uid'`](https://github.com/google/eddystone/tree/master/eddystone-uid), or
  [`'tlm'`](https://github.com/google/eddystone/tree/master/eddystone-tlm).

If `EddystoneAdvertisment.type === 'url'`
* `EddystoneAdvertisement.url`: the URL being advertised.
* `EddystoneAdvertisement.txPower`: the [Tx Power]
  (https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level)
  included in the Advertisement.

### Methods
```javascript
EddystoneAdvertisement.unregisterAdvertisement()
```

This function returns a promise. If the advertisement is unregistered
successfully the promise will resolve, otherwise the promise will reject. If the
promise rejects, the advertisement may still be broadcasting. The only way to
recover may be to reboot your machine.

### Example
```javascript
myAdvertisement.unregisterAdvertisement()
  .then(() => console.log('Unregistered Successfully'))
  .catch(error => console.log(error.message));
```
Or if you have multiple advertisements:
```javascript
eddystone.advertisements.forEach(advertisement => {
  advertisement.unregisterAdvertisement()
    .then(() => console.log('Unregistered successfully'))
    .catch(error => console.log('Couldn\'t unregister the advertisement'));
});