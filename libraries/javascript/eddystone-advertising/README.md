[![Build Status](https://img.shields.io/travis/google/eddystone.svg?style=flat-square)](https://travis-ci.org/google/eddystone)

# Eddystone Advertising Library

The Eddystone Advertising Library abstracts low level concepts of the Eddystone
protocol and wraps existing Advertising APIs so that developers don't have to
worry about these low level details. The library exposes simple functions that
developers can use to advertise a Valid Eddystone packet from their device.

**NOTE** Currently only ChromeOS is supported.  
**NOTE** Currently only Eddystone-URL is supported.

## Usage
The Eddystone Advertising Library creates `window.eddystone` of type
[`Eddystone`](#eddystone)

To advertise a url:
**Example**
```js
let registered_adv;
eddystone.registerAdvertisement({
  type: 'url',
  url: 'https://example.com',
  advertisedTxPower: -20
}).then(advertisement => {
  registered_adv = advertisement;
  console.log('Advertising: ' + advertisement.url)
}).catch(error => console.log(error.message));
```

To stop advertising:
**Example**
```js
registered_adv.unregisterAdvertisement().then(() => {
  console.log('Advertisement unregistered successfully.');
}).catch(error => console.log(error.message));
```
Or if you have multiple advertisements:
```js
eddystone.advertisements.forEach(advertisement => {
  advertisement.unregisterAdvertisement()
    .then(() => console.log('Unregistered successfully'))
    .catch(error => console.log('Couldn\'t unregister the advertisement: ' + error.message));
});
```
## API
## Modules
<dl>
<dt><a href="#module_eddystone-advertisement">eddystone-advertisement</a></dt>
<dd></dd>
<dt><a href="#module_eddystone-advertising">eddystone-advertising</a></dt>
<dd></dd>
<dt><a href="#module_eddystone-chrome-os">eddystone-chrome-os</a></dt>
<dd></dd>
<dt><a href="#module_eddystone-url">eddystone-url</a></dt>
<dd></dd>
<dt><a href="#module_platform">platform</a></dt>
<dd></dd>
</dl>
## Members
<dl>
<dt><a href="#eddystone">eddystone</a></dt>
<dd><p>The global eddystone instance.</p>
</dd>
</dl>
<a name="module_eddystone-advertisement"></a>
## eddystone-advertisement
**Example**  
```js
const eddystoneAd = require('eddystone-advertisement')
```

* [eddystone-advertisement](#module_eddystone-advertisement)
  * [.EddystoneAdvertisement](#module_eddystone-advertisement.EddystoneAdvertisement)
    * [new EddystoneAdvertisement(id, options, platform)](#new_module_eddystone-advertisement.EddystoneAdvertisement_new)
    * [.unregisterAdvertisement()](#module_eddystone-advertisement.EddystoneAdvertisement+unregisterAdvertisement) ⇒ <code>Promise.&lt;void&gt;</code>
  * [.EddystoneFrameType](#module_eddystone-advertisement.EddystoneFrameType) : <code>enum</code>

<a name="module_eddystone-advertisement.EddystoneAdvertisement"></a>
### eddystoneAd.EddystoneAdvertisement
Represents the Advertisement being broadcasted.

**Kind**: static class of <code>[eddystone-advertisement](#module_eddystone-advertisement)</code>  

* [.EddystoneAdvertisement](#module_eddystone-advertisement.EddystoneAdvertisement)
  * [new EddystoneAdvertisement(id, options, platform)](#new_module_eddystone-advertisement.EddystoneAdvertisement_new)
  * [.unregisterAdvertisement()](#module_eddystone-advertisement.EddystoneAdvertisement+unregisterAdvertisement) ⇒ <code>Promise.&lt;void&gt;</code>

<a name="new_module_eddystone-advertisement.EddystoneAdvertisement_new"></a>
#### new EddystoneAdvertisement(id, options, platform)
**Throws**:

- <code>TypeError</code> If no platform was passed.
- <code>Error</code> If type is an unsupported Frame Type.


| Param | Type | Description |
| --- | --- | --- |
| id | <code>number</code> | Unique between browser restarts meaning the id will        no longer be valid upon browser restart. |
| options | <code>EddystoneAdvertisementOptions</code> | The options used when        creating the advertisement. |
| platform | <code>Object</code> | The underlying platform; used to unregister the        advertisement. |

<a name="module_eddystone-advertisement.EddystoneAdvertisement+unregisterAdvertisement"></a>
#### eddystoneAdvertisement.unregisterAdvertisement() ⇒ <code>Promise.&lt;void&gt;</code>
Unregisters the current advertisement.

**Kind**: instance method of <code>[EddystoneAdvertisement](#module_eddystone-advertisement.EddystoneAdvertisement)</code>  
**Fulfill**: <code>void</code> - If the advertisement was unregistered successfully.  
**Reject**: <code>Error</code> - If the advertisement failed to be registered. If
       the promise rejects the advertisment may still be broadcasting. The only
       way to recover may be to reboot your machine.  
<a name="module_eddystone-advertisement.EddystoneFrameType"></a>
### eddystoneAd.EddystoneFrameType : <code>enum</code>
Possible Eddystone frame types.

**Kind**: static constant of <code>[eddystone-advertisement](#module_eddystone-advertisement)</code>  
**Read only**: true  
**See**

- [Protocol Specification](https://github.com/google/eddystone/blob/master/protocol-specification.md)
- [Eddystone-URL](https://github.com/google/eddystone/tree/master/eddystone-url)
- [Eddystone-UID](https://github.com/google/eddystone/tree/master/eddystone-uid)
- [Eddystone-TLM](https://github.com/google/eddystone/tree/master/eddystone-tlm)

**Properties**

| Name | Type | Default |
| --- | --- | --- |
| URL | <code>string</code> | <code>&quot;url&quot;</code> | 
| UID | <code>string</code> | <code>&quot;uid&quot;</code> | 
| TLM | <code>string</code> | <code>&quot;tlm&quot;</code> | 

<a name="module_eddystone-advertising"></a>
## eddystone-advertising

* [eddystone-advertising](#module_eddystone-advertising)
  * [Eddystone](#exp_module_eddystone-advertising--Eddystone) ⏏
    * _instance_
      * [.registerAdvertisement()](#module_eddystone-advertising--Eddystone+registerAdvertisement) ⇒ <code>Promise.&lt;EddystoneAdvertisement&gt;</code>
    * _inner_
      * [~EddystoneAdvertisementOptions](#module_eddystone-advertising--Eddystone..EddystoneAdvertisementOptions) : <code>Object</code>

<a name="exp_module_eddystone-advertising--Eddystone"></a>
### Eddystone ⏏
Exposes platform independent functions to register/unregister Eddystone
     Advertisements.

**Kind**: Exported class  
<a name="module_eddystone-advertising--Eddystone+registerAdvertisement"></a>
#### eddystone.registerAdvertisement() ⇒ <code>Promise.&lt;EddystoneAdvertisement&gt;</code>
Function to register an Eddystone BLE advertisement.

**Kind**: instance method of <code>[Eddystone](#exp_module_eddystone-advertising--Eddystone)</code>  
**Params**: <code>EddystoneAdvertisementOptions</code> options The characteristics
       of the advertised Eddystone.  
**Fulfill**: <code>EddystoneAdvertisement</code> - If the advertisement was registered
       successfully.  
**Reject**: <code>Error</code> - If the advertisement failed to be registered.  
<a name="module_eddystone-advertising--Eddystone..EddystoneAdvertisementOptions"></a>
#### Eddystone~EddystoneAdvertisementOptions : <code>Object</code>
Object that contains the characteristics of the package to advertise.

**Kind**: inner typedef of <code>[Eddystone](#exp_module_eddystone-advertising--Eddystone)</code>  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| type | <code>EddystoneFrameType</code> | Type of Eddystone. For now only `'url'` is      supported. |
| url | <code>string</code> &#124; <code>undefined</code> | The URL to advertise |
| advertisedTxPower | <code>number</code> &#124; <code>undefined</code> | The Tx Power to advertise |

<a name="module_eddystone-chrome-os"></a>
## eddystone-chrome-os

* [eddystone-chrome-os](#module_eddystone-chrome-os)
  * [EddystoneChromeOS](#exp_module_eddystone-chrome-os--EddystoneChromeOS) ⏏
    * [.registerAdvertisement(options)](#module_eddystone-chrome-os--EddystoneChromeOS.registerAdvertisement) ⇒ <code>Promise.&lt;EddystoneAdvertisement&gt;</code>
    * [.unregisterAdvertisement(advertisement)](#module_eddystone-chrome-os--EddystoneChromeOS.unregisterAdvertisement) ⇒ <code>Promise.&lt;void&gt;</code>
    * [._constructAdvertisement()](#module_eddystone-chrome-os--EddystoneChromeOS._constructAdvertisement) ⇒ <code>ChromeOSAdvertisement</code>

<a name="exp_module_eddystone-chrome-os--EddystoneChromeOS"></a>
### EddystoneChromeOS ⏏
This class wraps the underlying ChromeOS BLE Advertising API.

**Kind**: Exported class  
**Todo**

- [ ] Add link to API.

<a name="module_eddystone-chrome-os--EddystoneChromeOS.registerAdvertisement"></a>
#### EddystoneChromeOS.registerAdvertisement(options) ⇒ <code>Promise.&lt;EddystoneAdvertisement&gt;</code>
Function that registers an Eddystone BLE advertisement.

**Kind**: static method of <code>[EddystoneChromeOS](#exp_module_eddystone-chrome-os--EddystoneChromeOS)</code>  
**Fulfill**: <code>EddystoneAdvertisement</code> - If the advertisement was registered
       successfully.  
**Reject**: <code>Error</code> - If the advertisement failed to be regsitered.  

| Param | Type | Description |
| --- | --- | --- |
| options | <code>EddystoneAdvertisementOptions</code> | The characteristics of the        advertisement. |

<a name="module_eddystone-chrome-os--EddystoneChromeOS.unregisterAdvertisement"></a>
#### EddystoneChromeOS.unregisterAdvertisement(advertisement) ⇒ <code>Promise.&lt;void&gt;</code>
Function to unregister an advertisement.

**Kind**: static method of <code>[EddystoneChromeOS](#exp_module_eddystone-chrome-os--EddystoneChromeOS)</code>  
**Fulfill**: <code>void</code> - If the advertisment was unregistered successfully.  
**Reject**: <code>Error</code> - If the advertisment failed to be unregistered.  

| Param | Type | Description |
| --- | --- | --- |
| advertisement | <code>EddystoneAdvertisement</code> | The advertisement to        unregister. |

<a name="module_eddystone-chrome-os--EddystoneChromeOS._constructAdvertisement"></a>
#### EddystoneChromeOS._constructAdvertisement() ⇒ <code>ChromeOSAdvertisement</code>
Construct the ChromeOS specific advertisement to register.

**Kind**: static method of <code>[EddystoneChromeOS](#exp_module_eddystone-chrome-os--EddystoneChromeOS)</code>  
**Returns**: <code>ChromeOSAdvertisement</code> - advertisement  
**Throws**:

- <code>Error</code> If the frame type is not supported
- <code>Error</code> If the Tx Power value is not in the allowed range. See:
       https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level
- <code>Error</code> If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see:
       https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
- <code>Error</code> If the URL contains an invalid character. For a list of
       invalid characters see the Note in:
       https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding

**Params**: <code>EddystoneAdvertisementOptions</code> options The characteristics of the
       advertisement.  
<a name="module_eddystone-url"></a>
## eddystone-url

* [eddystone-url](#module_eddystone-url)
  * [EddystoneURL](#exp_module_eddystone-url--EddystoneURL) ⏏
    * [.constructServiceData(url, advertisedTxPower)](#module_eddystone-url--EddystoneURL.constructServiceData) ⇒ <code>Array.&lt;number&gt;</code>
    * [.encodeURL(url)](#module_eddystone-url--EddystoneURL.encodeURL) ⇒ <code>Array.&lt;number&gt;</code>

<a name="exp_module_eddystone-url--EddystoneURL"></a>
### EddystoneURL ⏏
This class provides helper functions that relate to Eddystone-URL.

**Kind**: Exported class  
**See**: [Eddystone-URL](https://github.com/google/eddystone/tree/master/eddystone-url)  
<a name="module_eddystone-url--EddystoneURL.constructServiceData"></a>
#### EddystoneURL.constructServiceData(url, advertisedTxPower) ⇒ <code>Array.&lt;number&gt;</code>
Constructs a valid Eddystone-URL service data from a URL and a Tx Power
       value.

**Kind**: static method of <code>[EddystoneURL](#exp_module_eddystone-url--EddystoneURL)</code>  
**Returns**: <code>Array.&lt;number&gt;</code> - The service data.  
**See**: [URL Frame Specification](https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification)  

| Param | Type | Description |
| --- | --- | --- |
| url | <code>string</code> | The URL to use in the service data. |
| advertisedTxPower | <code>number</code> | The Tx Power to use in the service data. |

<a name="module_eddystone-url--EddystoneURL.encodeURL"></a>
#### EddystoneURL.encodeURL(url) ⇒ <code>Array.&lt;number&gt;</code>
Encodes the given string using the encoding defined in the Eddystone-URL
       Spec.

**Kind**: static method of <code>[EddystoneURL](#exp_module_eddystone-url--EddystoneURL)</code>  
**Returns**: <code>Array.&lt;number&gt;</code> - The encoded url.  
**Throws**:

- <code>Error</code> If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see:
       https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
- <code>Error</code> If the URL contains an invalid character. For a list of
       invalid characters see the Note in:
       https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding

**See**: [Eddystone-URL](https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url)  

| Param | Type | Description |
| --- | --- | --- |
| url | <code>string</code> | The url to encode. |

<a name="module_platform"></a>
## platform
<a name="exp_module_platform--platform"></a>
### platform() ⇒ <code>Object</code> ⏏
Detects what API is available in the platform.

**Kind**: Exported function  
**Returns**: <code>Object</code> - An object that wraps the underlying BLE
     Advertising API  
**Throws**:

- <code>Error</code> If the platform is unsupported

<a name="eddystone"></a>
## eddystone
The global eddystone instance.

**Kind**: global variable  
