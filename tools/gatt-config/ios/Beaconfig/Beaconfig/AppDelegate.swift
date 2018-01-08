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

import UIKit

let kImageDimension: UInt = 180

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate, GIDSignInDelegate {

  var window: UIWindow?

  func application(application: UIApplication,
                   didFinishLaunchingWithOptions launchOptions: [NSObject: AnyObject]?) -> Bool {
    var configureError: NSError?
    GGLContext.sharedInstance().configureWithError(&configureError)
    assert(configureError == nil, "Error configuring Google services: \(configureError)")

    GIDSignIn.sharedInstance().delegate = self
    return true
  }

  func application(application: UIApplication,
                   openURL url: NSURL,
                           options: [String: AnyObject]) -> Bool {
    return GIDSignIn.sharedInstance().handleURL(url,
        sourceApplication: options[UIApplicationOpenURLOptionsSourceApplicationKey] as? String,
        annotation: options[UIApplicationOpenURLOptionsAnnotationKey])
  }

  func signIn(signIn: GIDSignIn!, didSignInForUser user: GIDGoogleUser!,
              withError error: NSError!) {
    if (error == nil) {
      let fullName = user.profile.name
      var picture: NSURL?
      if user.profile.hasImage {
        picture = user.profile.imageURLWithDimension(kImageDimension)
      }
      let email = user.profile.email
      NSNotificationCenter.defaultCenter().postNotificationName(
        "ToggleAuthUINotification",
        object: nil,
        userInfo: ["statusText": "\(fullName)",
                    "email": email,
                    "imageURL": picture!])
    } else {
      print("\(error.localizedDescription)")
      NSNotificationCenter.defaultCenter().postNotificationName(
        "ToggleAuthUINotification", object: nil, userInfo: nil)
    }
  }
}

