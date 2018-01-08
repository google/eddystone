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

let kNavBarTintColourHue: CGFloat = 0.2111
let kNavBarTintSaturation: CGFloat = 1
let kNavBarTintBrightness: CGFloat = 0.71
let kNavBarTintAlpha: CGFloat = 1.0
let kTableViewEstimatedRowHeight: CGFloat = 140
let kSettingsFontSize: CGFloat = 22
let kSettingsButtonSize: CGFloat = 30

///
/// Displays a table view with the results of the scanning.
///
class ScannedBeaconsTableViewController: UITableViewController {
  var timer: NSTimer?
  var beaconItems: [BeaconItem] = []
  var beaconScanner: BeaconScanner?
  var scanButton: UIButton!
  var refreshController = UIRefreshControl()
  let settingsViewController = UserLoginViewController()
  var userInfo = UserInformation()

  override func viewDidLoad() {
    super.viewDidLoad()
    navigationController?.interactivePopGestureRecognizer?.enabled = false
    self.refreshController.addTarget(self,
                                     action: #selector(startScanning),
                                     forControlEvents: UIControlEvents.ValueChanged)
    tableView.addSubview(refreshController)
    self.view.backgroundColor = kLightGrayColor
    self.tableView.separatorStyle = UITableViewCellSeparatorStyle.None
    self.tableView.rowHeight = UITableViewAutomaticDimension
    self.tableView.estimatedRowHeight = kTableViewEstimatedRowHeight
    navigationController!.navigationBar.barTintColor = UIColor(hue: kNavBarTintColourHue,
                                                               saturation: kNavBarTintSaturation,
                                                               brightness: kNavBarTintBrightness,
                                                               alpha: kNavBarTintAlpha)
    navigationController!.navigationBar.tintColor = UIColor.whiteColor()
    UIApplication.sharedApplication().statusBarStyle = .LightContent
    /// The left bar button opens the Login view.
    let settingsButton = UIButton()
    settingsButton.setImage(UIImage(named: "settings"), forState: .Normal)
    settingsButton.frame = CGRectMake(0, 0, kSettingsButtonSize, kSettingsButtonSize)
    settingsButton.addTarget(self,
                             action:
                              #selector(ScannedBeaconsTableViewController.settingsButtonPressed),
                             forControlEvents: .TouchUpInside)
    let settingsBarButton = UIBarButtonItem(customView: settingsButton)
    self.navigationItem.leftBarButtonItem = settingsBarButton
    refreshController.beginRefreshing()
    startScanning(nil)
  }

  func settingsButtonPressed() {
    performSegueWithIdentifier("userInformationSegue", sender: self)
  }

  func setupScan() {
    self.beaconScanner = BeaconScanner()
    self.tableView.reloadData()
  }

  /// Displays throbber to block the UI while scanning.
  func displayThrobber() {
    let alert = UIAlertController(title: nil, message: "Scanning...", preferredStyle: .Alert)

    alert.view.tintColor = UIColor.blackColor()
    /// Need to give UIActivityIndicator nonzero values at init.
    let loadingIndicator: UIActivityIndicatorView =
      UIActivityIndicatorView(frame: CGRectMake(10, 5, 50, 50)) as UIActivityIndicatorView
    loadingIndicator.hidesWhenStopped = true
    loadingIndicator.activityIndicatorViewStyle = UIActivityIndicatorViewStyle.Gray
    loadingIndicator.startAnimating()
    alert.view.addSubview(loadingIndicator)
    self.presentViewController(alert, animated: true, completion: nil)
  }


  func stopScanning() {
    NSLog("Stop scanning")
    dispatch_async(dispatch_get_main_queue()) {
      self.refreshController.endRefreshing()
      self.dismissViewControllerAnimated(false, completion: nil)
      /// We finished scanning, now we can put together the results and display them!
      if let scanner = self.beaconScanner {
        self.beaconItems = scanner.populateBeaconItems()
        self.tableView.reloadData()
      }
    }
  }

  func startScanning(sender: UIButton?) {
    NSLog("Start scanning")
    self.setupScan()
    /// We're starting the scan and we send the function to be called when the scanning is over.
    if let scanner = beaconScanner {
      scanner.startScanning(stopScanning)
    }
  }

  override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
    return beaconItems.count
  }

  override func tableView(tableView: UITableView,
                          cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
    let identifier = "BeaconTableViewCell"
    let createdCell = self.tableView.dequeueReusableCellWithIdentifier(identifier,
                                                                       forIndexPath: indexPath)
      as? BeaconTableViewCell

    if let cell = createdCell {
      for subview in cell.framesView.subviews {
        subview.removeFromSuperview()
      }
      cell.beacon = beaconItems[indexPath.row].peripheral
      cell.nameLabel.text = cell.beacon.name ?? "Unnamed Beacon"
      cell.framesView.addSubview(beaconItems[indexPath.row].framesView)
      CustomViews.setConstraints(beaconItems[indexPath.row].framesView,
                                 holderView: cell.framesView,
                                 topView: cell.framesView,
                                 leftView: nil,
                                 rightView: nil,
                                 height: nil,
                                 width: nil,
                                 pinTopAttribute: .Top,
                                 setBottomConstraints: true,
                                 marginConstraint: kTableViewCellMargin)
      cell.operations = beaconItems[indexPath.row].operations
      cell.connectableDot.layer.cornerRadius = cell.connectableDot.frame.width / 2.0
      cell.connectableDot.backgroundColor = UIColor.redColor()
      if beaconItems[indexPath.row].frames.EddystoneGATTServiceFrameSupported {
        cell.connectableDot.backgroundColor = UIColor.greenColor()
      } else if let
        EID = beaconItems[indexPath.row].frames.EIDData,
        scanner = beaconScanner {
        if let myBeacon: Bool = scanner.myEIDBeacons[EID] as? Bool  {
          if myBeacon {
            cell.connectableDot.backgroundColor = UIColor.greenColor()
          } else {
            cell.connectableDot.backgroundColor = UIColor.redColor()
          }
        }
      } else {
        cell.connectableDot.backgroundColor = UIColor.redColor()
      }
      return cell
    } else {
      return BeaconTableViewCell(style:UITableViewCellStyle.Subtitle, reuseIdentifier:identifier)
    }
  }

  @IBAction func scanButton(sender: UIButton) {
    displayThrobber()
    self.startScanning(sender)
  }

  override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
    if segue.identifier == "connectToBeacon" {
      if let
        cell = sender as? BeaconTableViewCell {
        if let beaconDetailsViewController = segue.destinationViewController as?
          BeaconDetailsViewController,
          scanner = beaconScanner {
          beaconDetailsViewController.prepareForUseWithBeacon(scanner,
                                                              beacon: cell.beacon,
                                                              operations: cell.operations!,
                                                              beaconName: cell.nameLabel.text,
                                                              userInfo: userInfo)
        }
      }
    } else if segue.identifier == "userInformationSegue" {
      if let userInfoViewController = segue.destinationViewController as? UserLoginViewController {
        userInfoViewController.prepareForUse(userInfo)
      }
    }
  }
}
