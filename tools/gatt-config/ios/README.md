## BeaConfig App (iOS)

The BeaConfig application enables unconstrained configuration of BLE beacons that support the Eddystone configuration GATT service, as well as EID registration.

### Building The App
In order to set up the application, you have to set up various Google APIs, their authentication keys, and OAuth stuff. The actual app is CocoaPod and is easy to get running.

1. Make sure your computer has [CocoaPods](http://cocoapods.org) installed, as well as Xcode 7.
2. In order to get all the component pieces:
```
open the terminal
go to the folder that has “Podfile” in it
pod install
open Beaconfig.xcworkspace/
```
The code will not compile just yet. We need to add a couple of _.plist_ files. Also, so not open the _.xcodeproj/_. From now you should only use the **.xcworkspace/** one!

3. Go to [Google API Developer Console](https://console.developers.google.com/). You now have to create a new project (Project -> Create Project). Give that project a name, save and then wait a couple of seconds for the project to be created.

4. Now we’re gonna enable some APIs. In the left menu, go to Library and then search and enable the following APIs:
- Google Beacon Proximity API
- Google Cloud Resource Manager API

5. Using the menu on the left, go to _Credentials_. Click on the _Create credentials_ drop down menu and then choose _API key_. Once you get it, you’re going to want to restrict it. Click _Restrict_ and choose _iOS apps_. When required a bundle identifier, you can either add _com.google.ios.beaconfig_ or just leave it blank.

6. Go to [Start Integrating Google Sign-In](https://developers.google.com/identity/sign-in/ios/start-integrating) from a browser logged in to the account that created the API project you just created.

7. Go to the button _”Get a configuration file”_ and click that.

8. For _”App Name”_, Enter / select the project name you just created in API console.

9. For __iOS BundleID__, enter `com.google.ios.beaconfig`.

10. “Continue to Choose and Configure Services”.

11.  Enable __Google Sign-In__ (that’s all).

12. “Continue to Generate configuration files”.

13. Download the configuration file, `GoogleService-Info.plist`. Put it in the same folder as your Info.plist. Make sure any other _GoogleService-Info.plist_ files are deleted.

14. You now have to set your project’s reversed client ID in Xcode:
* Click on the blue project “_Beaconfig_” at the top of the Project Navigator.
* Click on “_Beaconfig_” under “_Targets_” in the main part of Xcode.
* Click on “_Info_” across the top.
* Look for the scheme under “_URL Types_”
* Replace that scheme with the value of the REVERSED_CLIENT_ID from the *GoogleService-Info.plist* file (i.e. `com.googleusercontent.apps.2384234-35892859asdfashdoaiusdasf`)
