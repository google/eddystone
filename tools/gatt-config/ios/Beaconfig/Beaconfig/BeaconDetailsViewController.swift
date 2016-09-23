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
import CoreBluetooth

enum ChangeLockCodeStatus {
  case Success
  case IncorrectOldPasskey
  case Fail
}

let kButtonWidth: CGFloat = 110

class BeaconDetailsViewController:  UIViewController,
                                    UIGestureRecognizerDelegate {
  var beacon: CBPeripheral?
  var beaconScanner: BeaconScanner?
  var beaconGATTOperations: GATTOperations?
  var slotDataContentViews: [SlotDataContentView] = []
  @IBOutlet weak var slotTabsScrollView: UIScrollView!
  @IBOutlet weak var beaconNameLabel: UILabel!
  @IBOutlet weak var tabsContentView: UIView!
  @IBOutlet weak var beaconConnectionView: UIView!
  @IBOutlet weak var saveButton: UIBarButtonItem!
  var unlockView: UIView?
  var globalView: SlotDataContentView!
  var previousButton: UIButton?
  var beaconName: String?
  var slotViews: [UIView] = []
  var beaconPasskey: String?
  var beaconInvestigation: BeaconInvestigation?
  var beaconConfiguration: ConfigureBeaconSlotData?
  var slotData: Dictionary <NSNumber, Dictionary <String, NSData>> = [:]
  var slotUpdateData: Dictionary <NSNumber, Dictionary <String, NSData>> = [:]
  var beaconCapabilities: NSDictionary = [:]
  var connectionViewObject: ConnectionView!
  var scrollingView: HorizontalScrollButtonList?
  var connectionTimer: NSTimer?
  ///
  /// the passkey must have 16 bytes in hex format, which makes it
  /// 32 characters when inserted as String
  ///
  let passkeyRequiredLength = 32
  var scanningTime = 0
  var timer: NSTimer?
  var viewToDisable: UIView?
  var selectHolder: UIView?
  var userInfo: UserInformation!
  var throbberAlert: UIAlertController!

  override func viewDidLoad() {
    super.viewDidLoad()
    if let name = self.beaconName {
      beaconNameLabel.text = name
    }
    tabsContentView.backgroundColor = kLightGrayColor
    slotTabsScrollView.backgroundColor = kLightGrayColor
    beaconConnectionView.backgroundColor = kLightGrayColor
    /// Dismiss keyboard when user touches outside it.
    let tap: UITapGestureRecognizer =
      UITapGestureRecognizer(target: self,
                             action: #selector(UIInputViewController.dismissKeyboard))
    tap.delegate = self
    view.addGestureRecognizer(tap)
    /// Swipe the tab views left and right.
    let swipeRight = UISwipeGestureRecognizer(target: self,
                         action: #selector(BeaconDetailsViewController.respondToSwipeGesture(_:)))
    let swipeLeft = UISwipeGestureRecognizer(target: self,
                        action: #selector(BeaconDetailsViewController.respondToSwipeGesture(_:)))
    swipeRight.direction = UISwipeGestureRecognizerDirection.Right
    swipeLeft.direction = UISwipeGestureRecognizerDirection.Left
    self.view.addGestureRecognizer(swipeRight)
    self.view.addGestureRecognizer(swipeLeft)
    connectToBeacon()
  }

  func dismissKeyboard() {
    view.endEditing(true)
  }

  func slotButtonPressed(buttonNumber: Int) {
    for view in slotViews {
      if !view.hidden {
        view.hidden = true
      }
    }
    slotViews[buttonNumber].hidden = false
  }

  func signInWithGoogle(viewToDisable: UIView, selectHolder: UIView) {
    self.viewToDisable = viewToDisable
    self.selectHolder = selectHolder
    self.performSegueWithIdentifier("signInSegue", sender: self)
  }

  ///
  /// When the user decides to save the changes for the beacon configuration, we want to
  /// collect all the information and to create a dictionary with it. We must also display
  /// errors in case if we know that the desired configuration is incorrect.
  ///
  @IBAction func saveBeaconData(sender: AnyObject) {
    view.endEditing(true)
    slotUpdateData.removeAll()
    var EIDConfiguration = false
    var URLConflict = false
    var i = 0
    for slotView in slotDataContentViews {
      if slotUpdateData[i] == nil {
        slotUpdateData[i] = [:]
      }
      if let data = slotView.getUpdateData() {
        if data[slotDataEIDKey] != nil {
          EIDConfiguration = true
        }
        slotUpdateData[i] = data
      } else {
        return
      }
      i += 1
    }

    if EIDConfiguration {
      i = 0
      for _ in slotDataContentViews {
        if slotUpdateData[i]?[slotDataURLKey] != nil {
          URLConflict = true
        }
        i += 1
      }
    }

    ///
    /// Broadcasting an URL frame while also broadcasting EID is a huge security problem
    /// and it should not be allowed, because it makes the beacon easily recognisable.
    ///
    if URLConflict {
      showAlert("URL Conflict",
                description: "Broadcasting URL while also broadcasting EID would " +
                             "make your beacon easy to be recognised. Please " +
                             "consider removing the URL frame.",
                buttonText: "Dismiss")
    } else {
      for slotView in slotDataContentViews {
        slotView.slotContentView.removeFromSuperview()
      }
      displayThrobber("Saving...")
      if let currentBeacon = beacon {
        /// We have all the data we need in order to update the beacon's configuration.
        beaconConfiguration = ConfigureBeaconSlotData(peripheral: currentBeacon)
        beaconConfiguration!.beginBeaconConfiguration(beaconCapabilities,
                                                      statusInfoAlert: throbberAlert!,
                                                      slotUpdateData: slotUpdateData) {
                                                        dispatch_async(dispatch_get_main_queue()) {
                                                          ///
                                                          /// If everything in the update went well,
                                                          /// we want to investigate the beacon
                                                          /// again, because some values, such as
                                                          /// advertising interval, can be changed
                                                          /// by the beacon with the closest
                                                          /// available value.
                                                          ///
                                                          self.investigateBeacon()
                                                        }
        }
      }
    }
  }

  /// Creates the Global settings tab view for specific beacon configurations.
  func createGlobalView() -> UIView {
    globalView = SlotDataContentView()
    slotDataContentViews.append(globalView)
    if let data = slotData[0] {
      globalView.setAsGlobalContentView(beaconCapabilities, slotData: data)
    }
    slotDataContentViews.last!.setUICallbacks(changeLockCode,
                                              factoryResetCallback: startFactoryReset,
                                              remainConnectableCallback: changeRemainConnectable,
                                              showAlert: showAlert,
                                              signIn: signInWithGoogle)
    return slotDataContentViews.last!.slotContentView
  }

  func changeRemainConnectable(on: Bool) {
    displayThrobber("Changing state...")
    if let operations = beaconGATTOperations {
      operations.changeRemainConnectableState(on) {
        dispatch_async(dispatch_get_main_queue()) {
          self.dismissViewControllerAnimated(false, completion: nil)
        }
      }
    }
  }

  func startFactoryReset() {
    displayThrobber("Factory resetting...")
    if let operations = beaconGATTOperations {
      operations.factoryReset() {
        dispatch_async(dispatch_get_main_queue()) {
          for view in self.slotDataContentViews {
            view.slotContentView.removeFromSuperview()
          }
          self.slotDataContentViews.removeAll()
          self.investigateBeacon()
        }
      }
    }
  }

  func changeLockCode(oldCode: String, newCode: String) {
    if oldCode == beaconPasskey {
      displayThrobber("Changing lock code...")
      if let operations = beaconGATTOperations {
        operations.changeLockCode(oldCode, newCode: newCode) { lockState in
          dispatch_async(dispatch_get_main_queue()) {
            self.dismissViewControllerAnimated(false, completion: nil)
            if lockState == LockState.Unlocked {
              self.globalView.lockCodeChanged()
            } else {
              ///
              /// The lock code is changed and it is not the one that the user wanted.
              /// This IS a disaster.
              ///
              assert(false)
            }
          }
        }
      }
    } else {
      self.showAlert("Passkey",
                     description: "The old passkey is incorrect.",
                     buttonText: "Dismiss")
    }
  }

  func respondToSwipeGesture(gesture: UIGestureRecognizer) {

    if let swipeGesture = gesture as? UISwipeGestureRecognizer {

      switch swipeGesture.direction {

      case UISwipeGestureRecognizerDirection.Right:
        scrollingView?.swipeRight()
      case UISwipeGestureRecognizerDirection.Left:
        scrollingView?.swipeLeft()
      default:
        break
      }
    }
  }

  func slotDataContentView(slotNumber: Int, slotData: Dictionary <String, NSData>) -> UIView? {
    /// Creates the content views for each slot of the beacon
    let object = SlotDataContentView()
    slotDataContentViews.append(object)
    if let frameType = slotData[slotDataFrameTypeKey] {
      let frameTypeName = NSString(data:frameType, encoding:NSUTF8StringEncoding) as String!
      switch frameTypeName {
      case BeaconInfo.EddystoneFrameType.URLFrameType.description:
        slotDataContentViews.last!.setAsURLTypeContentView(slotData,
                                                           capabilities: beaconCapabilities)
      case BeaconInfo.EddystoneFrameType.UIDFrameType.description:
        slotDataContentViews.last!.setAsUIDTypeContentView(slotData,
                                                           capabilities: beaconCapabilities)
      case BeaconInfo.EddystoneFrameType.TelemetryFrameType.description:
        slotDataContentViews.last!.setAsTLMTypeContentView(slotData,
                                                           capabilities: beaconCapabilities)
      case BeaconInfo.EddystoneFrameType.EIDFrameType.description:
        slotDataContentViews.last!.setAsEIDTypeContentView(slotData,
                                                           capabilities: beaconCapabilities)
      default:
        slotDataContentViews.last!.setAsNoFrameTypeContentView(beaconCapabilities)
      }
    }
    slotDataContentViews.last!.setUICallbacks(changeLockCode,
                                              factoryResetCallback: startFactoryReset,
                                              remainConnectableCallback: changeRemainConnectable,
                                              showAlert: showAlert,
                                              signIn: signInWithGoogle)
    return slotDataContentViews.last!.slotContentScrollView
  }

  func showAlert(title: String, description: String, buttonText:String) {
    let alertController = UIAlertController(title: title,
                                            message: description,
                                            preferredStyle: UIAlertControllerStyle.Alert)
    alertController.addAction(UIAlertAction(title: buttonText,
                                            style: UIAlertActionStyle.Default,
                                            handler: nil))
    alertController.view.tintColor = kGreenColor

    self.presentViewController(alertController, animated: true, completion: nil)
  }

  func displayThrobber(message: String) {
    throbberAlert = UIAlertController(title: nil, message: message, preferredStyle: .Alert)

    throbberAlert.view.tintColor = UIColor.blackColor()
    /// Need to give UIActivityIndicator nonzero values at init.
    let loadingIndicator: UIActivityIndicatorView =
      UIActivityIndicatorView(frame: CGRectMake(10, 5, 50, 50)) as UIActivityIndicatorView
    loadingIndicator.hidesWhenStopped = true
    loadingIndicator.activityIndicatorViewStyle = UIActivityIndicatorViewStyle.Gray
    loadingIndicator.startAnimating()
    throbberAlert.view.addSubview(loadingIndicator)
    self.presentViewController(throbberAlert, animated: true, completion: nil)
  }

  func unlockBeacon(passkey: String) {
    dismissKeyboard()
    if let
      beaconOperations = beaconGATTOperations {
      beaconOperations.beginUnlockingBeacon(passkey) { lockState in
        dispatch_async(dispatch_get_main_queue()) {
          if lockState == LockState.Locked {
            /// User inserted a wrong password.
            self.showAlert("Password",
                           description: "The password is incorrect.",
                           buttonText: "Dismiss")
          } else if lockState == LockState.Unlocked {
            /// The beacon is now unlocked!
            self.beaconPasskey = passkey
            self.displayThrobber("Reading slot data...")
            self.investigateBeacon()
          }
        }
      }
    }
  }

  func displayScrollBar(beaconSlotDataCount: Int) {
    // +1 for 'Global' settings button / tab
    let buttonsToDisplay = beaconSlotDataCount + 1
    let buttonNames: NSMutableArray = []
    for i in 0...buttonsToDisplay - 1 {
        if i == 0 {
          buttonNames.addObject("GLOBAL")
        } else {
          buttonNames.addObject("SLOT \(i)")
        }
    }
    if scrollingView == nil {
      scrollingView = HorizontalScrollButtonList(buttonSize:
                                                    CGSizeMake(kButtonWidth,
                                                               slotTabsScrollView.frame.height - 5),
                                                 buttonNames: buttonNames,
                                                 callback: slotButtonPressed(_:))
      self.slotTabsScrollView.addSubview(scrollingView!)
      self.slotTabsScrollView.showsHorizontalScrollIndicator = true
      self.slotTabsScrollView.indicatorStyle = .Default
      self.slotTabsScrollView.contentSize = CGSize(width: scrollingView!.frame.size.width,
                                                   height: 1.0)
      self.slotTabsScrollView.backgroundColor = UIColor.whiteColor()
    } else {
      scrollingView!.slotButtonPressed(scrollingView!.previousButtonPressed!)
    }
  }

  func createContentViews(beaconDataSlotsCount: Int) {
    slotViews.removeAll()
    slotDataContentViews.removeAll()
    let globalView = createGlobalView()
    slotViews.append(globalView)
    tabsContentView.addSubview(globalView)
    if slotDataContentViews.count == 0 {
      globalView.hidden = false
    }
    for i in 0...beaconDataSlotsCount - 1 {
      if let data = slotData[i as NSNumber] {
        if let slotView = slotDataContentView(i, slotData: data) {
          slotView.hidden = true
          slotViews.append(slotView)
          self.tabsContentView.addSubview(slotView)
        }
      }
    }
  }

  func investigateBeacon() {
    if let connectedBeacon = self.beacon {
      self.beaconInvestigation = BeaconInvestigation(peripheral: connectedBeacon)
      if let investigation = self.beaconInvestigation {
        investigation.finishedUnlockingBeacon() { beaconCapabilities, slotData in
          /// Creates the buttons and the pages for them and populate them with the information.
          dispatch_async(dispatch_get_main_queue()) {
            self.beaconConnectionView.hidden = true
            self.dismissViewControllerAnimated(false, completion: nil)
            self.slotData = slotData
            self.beaconCapabilities = beaconCapabilities
            if let beaconDataSlotsCount = beaconCapabilities[maxSupportedSlotsKey] as? NSNumber {
              self.createContentViews(beaconDataSlotsCount as Int)
              self.displayScrollBar(beaconDataSlotsCount as Int)
              self.saveButton.enabled = true
            }
          }
        }
      }
    }
  }

  func prepareForUseWithBeacon(beaconScanner: BeaconScanner,
                               beacon: CBPeripheral,
                               operations: GATTOperations,
                               beaconName: String?,
                               userInfo: UserInformation) {
    self.beaconScanner = beaconScanner
    self.beacon = beacon
    self.beaconGATTOperations = operations
    self.beaconName = beaconName
    self.userInfo = userInfo
  }

  func activityIndicatorChanger() {
    self.connectionViewObject.stateTextView!.text.appendContentsOf(".")
  }

  func connectToBeacon() {
    connectionViewObject = ConnectionView()
    connectionTimer = NSTimer.scheduledTimerWithTimeInterval(
      1.0,
      target: self,
      selector: #selector(BeaconDetailsViewController.activityIndicatorChanger),
      userInfo: nil,
      repeats: true)

    let connectionView = connectionViewObject.configureView(connectToBeacon,
                                                            unlockBeaconCallback: unlockBeacon)
    beaconConnectionView.addSubview(connectionView)
    if let
      scanner = beaconScanner,
      currentBeacon = beacon {
      scanner.connectToBeacon(currentBeacon) { operationState in
        dispatch_async(dispatch_get_main_queue()) {
          ///
          /// There are several reasons why a connection to the beacon cannot be established,
          /// some of them being that the beacon does not implement the GATT Configuration
          /// Service, that the beacon is too far or that it is not in a connectable state.
          ///
          if let
            _ = operationState.name,
            errorDescription = operationState.description {
            self.connectionTimer?.invalidate()
            self.connectionViewObject.stateTextView!.text = errorDescription
            self.connectionViewObject.addRetryButton()
          } else {
            ///
            /// If we managed to connect to the beacon, we want to check its lock state and
            /// as the user to unlock it if that's necessary.
            ///
            self.connectionTimer?.invalidate()
            self.connectionViewObject.stateTextView!.text = "Connection successful!"
            self.checkLockState()
          }
        }
      }
    }
  }

  func checkLockState() {
    saveButton.enabled = false
    if let
      beaconOperations = beaconGATTOperations {
      beaconOperations.checkLockState(nil) { lockState in
        dispatch_async(dispatch_get_main_queue()) {
          if lockState == LockState.Locked {
            /// Display the view that asks the user to insert passkey for unlocking.
            self.connectionViewObject.createUnlockingView()
          } else if lockState == LockState.Unlocked
            || lockState == LockState.UnlockedPreventAutolock {
            /// We have an unlocked beacon. We can now start reading all its slot data.
            self.displayThrobber("Reading slot data...")
            self.investigateBeacon()
          }
        }
      }
    }
  }

  override func viewWillDisappear(animated : Bool) {
    super.viewWillDisappear(animated)

    ///
    /// The user pressed the "Back" key. There's no reason for us to mentain the connection
    /// to the beacon and we're disconnecting.
    ///
    if (self.isMovingFromParentViewController()){
      if let peripheral = beacon, scanner = beaconScanner {
        scanner.disconnectFromBeacon(peripheral)
      }
    }
  }

  override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
    if segue.identifier == "signInSegue" {
      if let
        signInController = segue.destinationViewController as? UserLoginViewController,
        view = viewToDisable,
        selectView = selectHolder {
        signInController.userInfo = userInfo
        signInController.viewToDisable = view
        signInController.selectHolder = selectView
      }
    }
  }
}
