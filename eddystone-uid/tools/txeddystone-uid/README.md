# TxEddystone-UID

This is a simple Android app that allows your phone to broadcast Eddystone-UID
BLE packets. The namespace and instance parts of the beacon ID are separately
configurable.

## Requirements

The app was developed on the Android Lollipop 5.0 (API 21) platform with
[Android Studio](http://developer.android.com/sdk/).

Only some more recent devices are capable of using the [BluetoothLeAdvertiser](https://developer.android.com/reference/android/bluetooth/le/BluetoothLeAdvertiser.html)
classes. The app will check if your device is compatible and show an error
dialog if not. The Nexus 6 phone and Nexus 9 tablet are known to work well.
We also have reports that the Android One device is capable.

Note that the advertised device address will be a private resolvable addresses
and is randomly generated. It rotates every time you start advertising, and
will automatically rotate every few minutes while the advertiser is running.

## Dependencies

None.

## Building and Running

Import the `TxEddystone-UID` project in this directory into Android Studio and
select `Run > Run app`.

A pre-built APK is available in the [project's releases](https://github.com/google/eddystone/releases).
