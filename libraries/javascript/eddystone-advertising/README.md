# Eddystone Advertising Library

The Eddystone Advertising Library abstracts low level concepts of the Eddystone
protocol and wraps existing Advertising APIs so that developers don't have to
worry about these low level details. The library exposes simple functions that
developers can use to advertise a Valid Eddystone packet from their device.

** NOTE ** Currently only ChromeOS is supported.
** NOTE ** Currently only Eddystone-URL is supported.

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
## Classes
<dl>
<dt><a href="#EddystoneAdvertisement">EddystoneAdvertisement</a></dt>
<dd><p>Represents the Advertisement being broadcasted.</p>
</dd>
<dt><a href="#Eddystone">Eddystone</a></dt>
<dd><p>Exposes platform independent functions to register/unregister Eddystone
     Advertisements.</p>
</dd>
<dt><a href="#EddystoneURL">EddystoneURL</a></dt>
<dd><p>This class provides helper functions that relate to Eddystone-URL.</p>
</dd>
</dl>
## Constants
<dl>
<dt><a href="#EddystoneFrameType">EddystoneFrameType</a> : <code>enum</code></dt>
<dd><p>Possible Eddystone frame types.</p>
</dd>
</dl>
## Typedefs
<dl>
<dt><a href="#EddystoneAdvertisementOptions">EddystoneAdvertisementOptions</a> : <code>Object</code></dt>
<dd><p>Object that contains the characteristics of the package to advertise.</p>
</dd>
</dl>
<a name="EddystoneAdvertisement"></a>
## EddystoneAdvertisement
Represents the Advertisement being broadcasted.

**Kind**: global class  

* [EddystoneAdvertisement](#EddystoneAdvertisement)
  * [new EddystoneAdvertisement(id, options, platform)](#new_EddystoneAdvertisement_new)
  * [.id](#EddystoneAdvertisement+id) : <code>number</code>
  * [.type](#EddystoneAdvertisement+type) : <code>string</code>
  * [.url](#EddystoneAdvertisement+url) : <code>string</code> &#124; <code>undefined</code>
  * [.advertisedTxPower](#EddystoneAdvertisement+advertisedTxPower) : <code>number</code> &#124; <code>undefined</code>
  * [.unregisterAdvertisement()](#EddystoneAdvertisement+unregisterAdvertisement) ⇒ <code>Promise.&lt;void&gt;</code>

<a name="new_EddystoneAdvertisement_new"></a>
### new EddystoneAdvertisement(id, options, platform)

| Param | Type | Description |
| --- | --- | --- |
| id | <code>number</code> | Unique between browser restarts meaning the id will        no longer be valid upon browser restart. |
| options | <code>[EddystoneAdvertisementOptions](#EddystoneAdvertisementOptions)</code> | The options used when        creating the advertisement. |
| platform | <code>Object</code> | The underlying platform; used to unregister the        advertisement. |

<a name="EddystoneAdvertisement+id"></a>
### eddystoneAdvertisement.id : <code>number</code>
The ID of this advertisment.

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+type"></a>
### eddystoneAdvertisement.type : <code>string</code>
The Eddystone Type

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+url"></a>
### eddystoneAdvertisement.url : <code>string</code> &#124; <code>undefined</code>
URL being advertised.
         Only present if `type === 'url'`.

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+advertisedTxPower"></a>
### eddystoneAdvertisement.advertisedTxPower : <code>number</code> &#124; <code>undefined</code>
Tx Power included in
         the advertisement. Only present if `type === 'url'`.

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+unregisterAdvertisement"></a>
### eddystoneAdvertisement.unregisterAdvertisement() ⇒ <code>Promise.&lt;void&gt;</code>
Unregisters the current advertisement.

**Kind**: instance method of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
**Fulfill**: <code>void</code> - If the advertisement was unregistered successfully.  
**Reject**: <code>Error</code> - If the advertisement failed to be registered. If
       the promise rejects the advertisment may still be broadcasting. The only
       way to recover may be to reboot your machine.  
<a name="Eddystone"></a>
## Eddystone
Exposes platform independent functions to register/unregister Eddystone
     Advertisements.

**Kind**: global class  

* [Eddystone](#Eddystone)
  * [.advertisements](#Eddystone+advertisements) : <code>[Array.&lt;EddystoneAdvertisement&gt;](#EddystoneAdvertisement)</code>
  * [.registerAdvertisement()](#Eddystone+registerAdvertisement) ⇒ <code>[Promise.&lt;EddystoneAdvertisement&gt;](#EddystoneAdvertisement)</code>

<a name="Eddystone+advertisements"></a>
### eddystone.advertisements : <code>[Array.&lt;EddystoneAdvertisement&gt;](#EddystoneAdvertisement)</code>
Contains
         all previously registered advertisements.<br>
**Note:** In a Chrome App, if the event page gets killed users won't
         be able to unregister the advertisement.

**Kind**: instance property of <code>[Eddystone](#Eddystone)</code>  
<a name="Eddystone+registerAdvertisement"></a>
### eddystone.registerAdvertisement() ⇒ <code>[Promise.&lt;EddystoneAdvertisement&gt;](#EddystoneAdvertisement)</code>
Function to register an Eddystone BLE advertisement.

**Kind**: instance method of <code>[Eddystone](#Eddystone)</code>  
**Params**: <code>[EddystoneAdvertisementOptions](#EddystoneAdvertisementOptions)</code> options The characteristics
       of the advertised Eddystone.  
**Fulfill**: <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code> - If the advertisement was registered
       successfully.  
**Reject**: <code>Error</code> - If the advertisement failed to be registered.  
<a name="EddystoneURL"></a>
## EddystoneURL
This class provides helper functions that relate to Eddystone-URL.

**Kind**: global class  
**See**: [Eddystone-URL](https://github.com/google/eddystone/tree/master/eddystone-url)  

* [EddystoneURL](#EddystoneURL)
  * [.constructServiceData(url, advertisedTxPower)](#EddystoneURL.constructServiceData) ⇒ <code>Array.&lt;number&gt;</code>
  * [.encodeURL(url)](#EddystoneURL.encodeURL) ⇒ <code>Array.&lt;number&gt;</code>

<a name="EddystoneURL.constructServiceData"></a>
### EddystoneURL.constructServiceData(url, advertisedTxPower) ⇒ <code>Array.&lt;number&gt;</code>
Constructs a valid Eddystone-URL service data from a URL and a Tx Power
       value.

**Kind**: static method of <code>[EddystoneURL](#EddystoneURL)</code>  
**Returns**: <code>Array.&lt;number&gt;</code> - The service data.  
**Throws**:

- <code>Error</code> If the Tx Power value is not in the allowed range. See
       [Tx Power Level](https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level).
- <code>Error</code> If the URL Scheme prefix is unsupported. For a list of
       supported Scheme prefixes see
       [URL Scheme Prefix](https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix)
- <code>Error</code> If the URL contains an invalid character. For a list of
       invalid characters see the Note in
       [HTTP URL Encoding](https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding)

**See**: [URL Frame Specification](https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification)  

| Param | Type | Description |
| --- | --- | --- |
| url | <code>string</code> | The URL to use in the service data. |
| advertisedTxPower | <code>number</code> | The Tx Power to use in the service data. |

<a name="EddystoneURL.encodeURL"></a>
### EddystoneURL.encodeURL(url) ⇒ <code>Array.&lt;number&gt;</code>
Encodes the given string using the encoding defined in the Eddystone-URL
       Spec.

**Kind**: static method of <code>[EddystoneURL](#EddystoneURL)</code>  
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

<a name="EddystoneFrameType"></a>
## EddystoneFrameType : <code>enum</code>
Possible Eddystone frame types.

**Kind**: global constant  
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

<a name="EddystoneAdvertisementOptions"></a>
## EddystoneAdvertisementOptions : <code>Object</code>
Object that contains the characteristics of the package to advertise.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| type | <code>[EddystoneFrameType](#EddystoneFrameType)</code> | Type of Eddystone. For now only `'url'` is      supported. |
| url | <code>string</code> &#124; <code>undefined</code> | The URL to advertise |
| advertisedTxPower | <code>number</code> &#124; <code>undefined</code> | The Tx Power to advertise |

