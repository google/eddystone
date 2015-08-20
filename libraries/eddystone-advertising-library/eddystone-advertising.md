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

