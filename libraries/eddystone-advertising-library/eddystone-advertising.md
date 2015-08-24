## Classes
<dl>
<dt><a href="#EddystoneURL">EddystoneURL</a></dt>
<dd><p>This class provides helper functions that relate to Eddystone-URL.
      <a href="https://github.com/google/eddystone/tree/master/eddystone-url">https://github.com/google/eddystone/tree/master/eddystone-url</a></p>
</dd>
<dt><a href="#EddystoneAdvertisement">EddystoneAdvertisement</a></dt>
<dd><p>This is the object that holds the information about the registered BLE
     Advertisement.</p>
</dd>
</dl>
## Typedefs
<dl>
<dt><a href="#EddystoneAdvertisementOptions">EddystoneAdvertisementOptions</a> : <code>Object</code></dt>
<dd><p>Object that contains the characteristics of the package to adverstise.</p>
</dd>
</dl>
<a name="EddystoneURL"></a>
## EddystoneURL
This class provides helper functions that relate to Eddystone-URL.
      https://github.com/google/eddystone/tree/master/eddystone-url

**Kind**: global class  

* [EddystoneURL](#EddystoneURL)
  * [.constructServiceData(url, txPower)](#EddystoneURL.constructServiceData) ⇒ <code>Array.&lt;number&gt;</code>
  * [.encodeURL(url)](#EddystoneURL.encodeURL) ⇒ <code>Array.&lt;number&gt;</code>

<a name="EddystoneURL.constructServiceData"></a>
### EddystoneURL.constructServiceData(url, txPower) ⇒ <code>Array.&lt;number&gt;</code>
Constructs a valid Eddystone-URL service data from a URL and a Tx Power
        value. See:
        https://github.com/google/eddystone/tree/master/eddystone-url#frame-specification

**Kind**: static method of <code>[EddystoneURL](#EddystoneURL)</code>  
**Returns**: <code>Array.&lt;number&gt;</code> - The service data.  
**Throws**:

- <code>Error</code> If the Tx Power value is not in the allowed range. See:
        https://github.com/google/eddystone/tree/master/eddystone-url#tx-power-level
- <code>Error</code> If the URL Scheme prefix is unsupported. For a list of
        supported Scheme prefixes see:
        https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
- <code>Error</code> If the URL contains an invalid character. For a list of
        invalid characters see the Note in:
        https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding


| Param | Type | Description |
| --- | --- | --- |
| url | <code>string</code> | The URL to use in the service data. |
| txPower | <code>number</code> | The Tx Power to use in the service data. |

<a name="EddystoneURL.encodeURL"></a>
### EddystoneURL.encodeURL(url) ⇒ <code>Array.&lt;number&gt;</code>
Encodes the given string using the encoding defined in the Eddystone-URL
        Spec:
        https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url

**Kind**: static method of <code>[EddystoneURL](#EddystoneURL)</code>  
**Returns**: <code>Array.&lt;number&gt;</code> - The encoded url.  
**Throws**:

- <code>Error</code> If the URL Scheme prefix is unsupported. For a list of
        supported Scheme prefixes see:
        https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
- <code>Error</code> If the URL contains an invalid character. For a list of
        invalid characters see the Note in:
        https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding


| Param | Type | Description |
| --- | --- | --- |
| url | <code>string</code> | The url to encode. |

<a name="EddystoneAdvertisement"></a>
## EddystoneAdvertisement
This is the object that holds the information about the registered BLE
     Advertisement.

**Kind**: global class  

* [EddystoneAdvertisement](#EddystoneAdvertisement)
  * [new EddystoneAdvertisement(id, options)](#new_EddystoneAdvertisement_new)
  * [.id](#EddystoneAdvertisement+id) : <code>number</code>
  * [.type](#EddystoneAdvertisement+type) : <code>string</code>
  * [.url?](#EddystoneAdvertisement+url?) : <code>string</code>
  * [.txPower?](#EddystoneAdvertisement+txPower?) : <code>number</code>

<a name="new_EddystoneAdvertisement_new"></a>
### new EddystoneAdvertisement(id, options)

| Param | Type | Description |
| --- | --- | --- |
| id | <code>number</code> | Unique between browser restarts |
| options | <code>[EddystoneAdvertisementOptions](#EddystoneAdvertisementOptions)</code> | The options used when        creating the advertisement. |

<a name="EddystoneAdvertisement+id"></a>
### eddystoneAdvertisement.id : <code>number</code>
The ID of this advertisment.

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+type"></a>
### eddystoneAdvertisement.type : <code>string</code>
The Eddystone Type

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+url?"></a>
### eddystoneAdvertisement.url? : <code>string</code>
URL being advertised.
         Only present if `type === 'url'`.

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisement+txPower?"></a>
### eddystoneAdvertisement.txPower? : <code>number</code>
Tx Power included in
         the advertisement. Only present if `type === 'url'`.

**Kind**: instance property of <code>[EddystoneAdvertisement](#EddystoneAdvertisement)</code>  
<a name="EddystoneAdvertisementOptions"></a>
## EddystoneAdvertisementOptions : <code>Object</code>
Object that contains the characteristics of the package to adverstise.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| type | <code>EddystoneType</code> | Type of Eddystone. |
| url? | <code>string</code> | The URL to advertise |
| txPower? | <code>number</code> | The Tx Power to advertise |

