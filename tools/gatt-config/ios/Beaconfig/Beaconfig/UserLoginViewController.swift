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

///
/// Manages the view where the user signs in with Google and chooses a project.
///
class UserLoginViewController: UIViewController, GIDSignInUIDelegate,
                               UIPickerViewDelegate, UIPickerViewDataSource {
  @IBOutlet weak var userInfoView: UIView!
  @IBOutlet weak var signInButton: GIDSignInButton!
  @IBOutlet weak var signOutButton: UIButton!
  @IBOutlet weak var statusText: UILabel!
  @IBOutlet weak var profileImage: UIImageView!
  @IBOutlet weak var emailTextView: UILabel!
  @IBOutlet weak var projectIDPickerView: UIPickerView!
  @IBOutlet weak var projectSelectionDescriptionView: UILabel!
  var viewToDisable: UIView?
  var selectHolder: UIView?
  var projectsArray: NSMutableArray = ["Not Selected"]
  var userInfo: UserInformation!
  var additionalScopes = ["https://www.googleapis.com/auth/userlocation.beacon.registry",
                          "https://www.googleapis.com/auth/cloud-platform"]

  override func viewDidLoad() {
    super.viewDidLoad()
    GIDSignIn.sharedInstance().uiDelegate = self
    projectIDPickerView.delegate = self
    projectIDPickerView.dataSource = self
    projectIDPickerView.showsSelectionIndicator = true
    projectSelectionDescriptionView.text = "Configuration of Eddystone EID beacons requires you\n" +
                                           "to select a Google Cloud project."
    CustomViews.addShadow(userInfoView)
    projectSelectionDescriptionView.backgroundColor = kLightGrayColor
    CustomViews.addShadow(projectSelectionDescriptionView)
    ///
    /// We need to manually add scopes in order to do the beacon registrations and to use the
    /// clout platform. We only do this if the user is not signed in.
    ///
    if !userInfo.userCurrentlySignedIn {
      for scope in additionalScopes {
        GIDSignIn.sharedInstance().scopes.append(scope)
      }
    }
    reloadDataInUIElements()
    NSNotificationCenter.defaultCenter()
      .addObserver(self,
                   selector: #selector(UserLoginViewController.receiveToggleAuthUINotification(_:)),
                   name: "ToggleAuthUINotification",
                   object: nil)

    signOutButton.setTitle("Sign Out", forState: UIControlState.Normal)
    signOutButton.setTitleColor(kGreenColor, forState: UIControlState.Normal)
    signOutButton.addTarget(self,
                            action: #selector(UserLoginViewController.didTapSignOut),
                            forControlEvents: .TouchUpInside)

  }

  func prepareForUse(userInfo: UserInformation) {
    self.userInfo = userInfo
  }
  override func didReceiveMemoryWarning() {
    super.didReceiveMemoryWarning()
  }

  func numberOfComponentsInPickerView(pickerView: UIPickerView) -> Int {
    return 1
  }

  func pickerView(pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
    return projectsArray.count
  }

  ///
  /// When the user selects a project, immediately save it to be used in EID configuration
  /// and remove all the notifications that tell the user to select a project.
  ///
  func pickerView(pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
    EIDConfiguration.projectID = (userInfo.projectsArray.objectAtIndex(row) as! String)
    userInfo.selectedProjectIndex = row
    if let selectView = self.selectHolder {
      selectView.removeFromSuperview()
    }
  }

  func pickerView(pickerView: UIPickerView,
                  viewForRow row: Int,
                             forComponent component: Int,
                                          reusingView view: UIView?) -> UIView {
    let pickerLabel = UILabel()
    pickerLabel.textColor = UIColor.blackColor()
    pickerLabel.text = projectsArray[row] as? String
    pickerLabel.font = UIFont(name: "Arial-BoldMT", size: 15)
    pickerLabel.textAlignment = NSTextAlignment.Center
    return pickerLabel
  }

  func signIn(signIn: GIDSignIn!,
              presentViewController viewController: UIViewController!) {
    print("sign in button pressed")
    self.presentViewController(viewController, animated: true, completion: nil)
  }

  /// Dismiss the "Sign in with Google" view.
  func signIn(signIn: GIDSignIn!,
              dismissViewController viewController: UIViewController!) {
    self.dismissViewControllerAnimated(true, completion: nil)
  }

  func didTapSignOut() {
    GIDSignIn.sharedInstance().signOut()
    NSNotificationCenter.defaultCenter().postNotificationName(
      "ToggleAuthUINotification", object: nil, userInfo: nil)
  }


  override func preferredStatusBarStyle() -> UIStatusBarStyle {
    return UIStatusBarStyle.LightContent
  }

  deinit {
    NSNotificationCenter.defaultCenter().removeObserver(self,
                                                        name: "ToggleAuthUINotification",
                                                        object: nil)
  }

  func reloadDataInUIElements() {
    profileImage.frame.size = CGSizeMake(100, 100)
    profileImage.image = userInfo.userImage
    profileImage.layer.cornerRadius = profileImage.frame.width/2
    profileImage.clipsToBounds = true

    if userInfo.userCurrentlySignedIn {
      statusText.text = userInfo.userName!
      emailTextView.hidden = false
      emailTextView.text = userInfo.userEmail!
      signInButton.hidden = true
      signOutButton.hidden = false
      projectIDPickerView.hidden = false
      projectSelectionDescriptionView.hidden = false
      projectsArray = userInfo.projectsArray
      projectIDPickerView.reloadAllComponents()
      if let row = userInfo.selectedProjectIndex {
        projectIDPickerView.selectRow(row, inComponent: 0, animated: true)
      }
    } else {
      statusText.text = userInfo.statusInfo
      emailTextView.hidden = true
      signInButton.hidden = false
      signOutButton.hidden = true
      projectsArray.removeAllObjects()
      projectIDPickerView.reloadAllComponents()
      projectIDPickerView.hidden = true
      projectSelectionDescriptionView.hidden = true
      EIDConfiguration.projectID = nil
    }
  }

  @objc func receiveToggleAuthUINotification(notification: NSNotification) {
    if (notification.name == "ToggleAuthUINotification") {
      if let data = notification.userInfo {
        userInfo.userName = data["statusText"] as? String
        let imageData = NSData(contentsOfURL: data["imageURL"] as! NSURL)
        userInfo.userImage = UIImage(data: imageData!)!
        userInfo.userEmail = data["email"] as? String
        userInfo.userCurrentlySignedIn = true

        /// When the user signs in, we need to get a list of the projects using the Cloud platform.
        userInfo.getProjectList() {state in
          dispatch_async(dispatch_get_main_queue()) {
            if state == GettingProjectsListState.GotUserProjects {
              self.reloadDataInUIElements()
              if let view = self.viewToDisable {
                view.removeFromSuperview()
              }
              if let selectView = self.selectHolder {
                if EIDConfiguration.projectID != nil {
                  selectView.removeFromSuperview()
                } else {
                  selectView.hidden = false
                }
              }
            } else if state == GettingProjectsListState.UnableToGetProjects {
              self.didTapSignOut()
              self.showAlert("Projects List",
                              description: "Unable to get projects list.",
                              buttonText: "Dismiss")
            } else if state == GettingProjectsListState.UserHasNoProjects {
              self.didTapSignOut()
              self.showAlert("Projects List",
                              description: "The account you selected has no projects." +
                                           "You will not be able to register an EID Beacon." +
                                           "Create a Google project or change the account.",
                              buttonText: "Dismiss")
            }
          }
        }
      } else {
        userInfo.clearData()
        reloadDataInUIElements()
      }
    }
  }

  func showAlert(title: String, description: String, buttonText:String) {
    let alertController = UIAlertController(title: title,
                                            message: description,
                                            preferredStyle: UIAlertControllerStyle.Alert)
    alertController.addAction(UIAlertAction(title: buttonText,
      style: UIAlertActionStyle.Default,
      handler: nil))

    self.presentViewController(alertController, animated: true, completion: nil)
  }
}
