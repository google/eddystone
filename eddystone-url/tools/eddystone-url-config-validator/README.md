# UriBeacon Validation App #

## Overview ##
The purpose of the Eddystone-URL Validator app is to allow developers to test
their implementations of the Eddystone-URL [Configuration Service](../../docs/config-service-spec.md).
The app consists of two sets of tests: Core Eddystone-URL Tests and Spec Eddystone-URL Tests.
NOTE: The validator is designed to work on Android Lollipop (5.1) or later, and will not run on earlier
Android releases.

### Core Eddystone-URL Tests ###
The Core Eddystone-URL Tests are design to make sure the app works seamlessly with the Physical Web App 
and the Eddystone-URL configuration app. The following table shows the expected results for each of the tests. 

| Test Name  | Value Written | Expected value read | Expected Return Code |
| ------------- | :-------------: | :----------: | -------------------- |
| [Connect to UriBeacon](../../docs/config-service-spec.md)  | N/A  | N/A | 0 |
| [Lock Beacon](../../docs/config-service-spec.md#32-lock)** | [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] | N/A | 0 |
| [Unlock Beacon](../../docs/config-service-spec.md#33-unlock)** | [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] | N/A | 0 |
| Locking...** | [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] | N/A | 0 |
| [Try Unlock with Wrong Key](../../docs/config-service-spec.md#33-unlock)** | [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1] | N/A | 8 |
| Unlocking...** | [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] | N/A | 0 |
| [Read Lock State](../../docs/config-service-spec.md#31-lock-state) | N/A | [0] | 0 |
| [Write Reset](../../docs/config-service-spec.md#39-reset)  | [1]  | N/A | 0 |
| [Write and Read Data 1](../../docs/config-service-spec.md#34-uri-data)| [2] |  [2] | 0 |
| [Write and Read Data 2](../../docs/config-service-spec.md#34-uri-data)| [1] |  [1] | 0 |
| [Write and Read Flags 1](../../docs/config-service-spec.md#35-flags)| [2] |  [2] | 0 |
| [Write and Read Flags 2](../../docs/config-service-spec.md#35-flags)| [1] |  [1] | 0 |
| [Write and Read Tx Power Levels 1](../../docs/config-service-spec.md#36-advertised-tx-power-levels)| [1, 1, 1, 1] | [1, 1, 1, 1]  | 0 |
| [Write and Read Tx Power Levels 2](../../docs/config-service-spec.md#36-advertised-tx-power-levels)| [0, 0, 0, 0] | [0, 0, 0, 0]  | 0 |
| [Write and Read Tx Power Mode 1](../../docs/config-service-spec.md#37-tx-power-mode)| [2] | [2] | 0 |
| [Write and Read Tx Power Mode 2](../../docs/config-service-spec.md#37-tx-power-mode)| [1] | [1] | 0 |
| [Write and Read Period 1](../../docs/config-service-spec.md#38-beacon-period)| [233, 3] |  [233, 3] | 0 |
| [Write and Read Period 2](../../docs/config-service-spec.md#38-beacon-period)| [231, 3] |  [231, 3] | 0 |
| [Disable Beacon using period = 0](../../docs/config-service-spec.md#38-beacon-period) | [0] | N/A | 0|
| [Floor Period](../../docs/config-service-spec.md#38-beacon-period) | [1] | value != [1] and value != [0] | 0 |
| Enabling Beacon Again | [231, 3] | [231, 3] | 0 |
| Disconnecting | N/A | N/A | 0 |
| [Has Valid Advertisement Packet](../../README.md) | N/A | Looking for any valid Advertisement Packet | N/A |
| Flags Written are Broadcasted | N/A | [1] | N/A |
| [Tx Power Written is Broadcasted](../../README.md#tx-power-level) | N/A | [0] | N/A |
| [Uri Written is Broadcasted](../../README.md#url-scheme-prefix) | N/A | [1] | N/A |

** Only if Lock/Unlock is implemented
