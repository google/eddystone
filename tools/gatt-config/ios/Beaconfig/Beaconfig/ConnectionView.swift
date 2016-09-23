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

import Foundation
import UIKit

///
/// Displays the state of the attempt to connect to the beacon and unlocking beacon options.
/// The class was created in order to provide details about connection problems and
/// to allow the user to retry to connect to the beacon.
///
class ConnectionView: UIViewController, UITextFieldDelegate, UIGestureRecognizerDelegate {
  let connectionView = UIView()
  var stateTextView: UITextView?
  var retryButtonView: UIView?
  var holderView: UIView!
  var unlockHolderView: UIView!
  var textField: UITextField!
  var connectionHolderView: UIView!
  var retryConnectionCallback: (() -> Void)?
  var unlockBeaconCallback: ((passkey: String) -> Void)?
  var unlockButton: UIButton!

  override func viewDidLoad() {
    super.viewDidLoad()
  }

  func textField(textField: UITextField,
                 shouldChangeCharactersInRange range: NSRange,
                                               replacementString string: String) -> Bool {
    if string.characters.count == 0 {
      return true
    }
    let currentText = textField.text ?? ""
    let text = (currentText as NSString).stringByReplacingCharactersInRange(range,
                                                                            withString: string)

    let currentCharacterCount = textField.text?.characters.count ?? 0
    if (range.length + range.location > currentCharacterCount){
      return false
    }
    let newLength = currentCharacterCount + string.characters.count - range.length
    var hasRightLength: Bool = false
    if textField == self.textField {
      if newLength <= kPassCodeLength {
        hasRightLength = true
      }
      if newLength == kPassCodeLength {
        unlockButton.enabled = true
      }
    }
    return StringUtils.inHexadecimalString(text) && hasRightLength
  }

  func textFieldShouldReturn(textField: UITextField) -> Bool {
    textField.resignFirstResponder()
    return false
  }

  func createHolderView(topView: UIView, pinTop: NSLayoutAttribute) -> UIView {
    let holder: UIView = UIView()
    connectionView.addSubview(holder)
    CustomViews.setConstraints(holder,
                               holderView: connectionView,
                               topView: topView,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: pinTop,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
    holder.backgroundColor = UIColor.whiteColor()
    CustomViews.addShadow(holder)
    return holder
  }


  func createLabelWithText(text: String, holder: UIView, image: UIImageView) -> UILabel {
    let textLabel = UILabel()
    textLabel.text = text
    textLabel.textColor = kGreenColor
    textLabel.font = UIFont.systemFontOfSize(19, weight: UIFontWeightSemibold)
    holder.addSubview(textLabel)

    holder.addSubview(image)
    CustomViews.setConstraints(image,
                               holderView: holder,
                               topView: holder,
                               leftView: nil,
                               rightView: nil,
                               height: 20,
                               width: 20,
                               pinTopAttribute: .Top,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
    CustomViews.setConstraints(textLabel,
                               holderView: holder,
                               topView: holder,
                               leftView: image,
                               rightView: nil,
                               height: kLabelHeight,
                               width: kLabelWidth,
                               pinTopAttribute: .Top,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)

    return textLabel
  }

  func createTextView(text: String, holder: UIView, topView: UIView) -> UITextView {
    let textView = UITextView()
    holder.addSubview(textView)
    textView.textAlignment = NSTextAlignment.Left
    textView.text = text
    textView.editable = false
    textView.dataDetectorTypes = UIDataDetectorTypes.All
    textView.textContainer.maximumNumberOfLines = 3
    textView.scrollEnabled = false
    CustomViews.setConstraints(textView,
                               holderView: holder,
                               topView: topView,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
    textView.textColor = UIColor.darkGrayColor()

    return textView
  }

  func addRetryButton() {
    let retryButton = UIButton()
    retryButtonView?.addSubview(retryButton)
    CustomViews.setConstraints(retryButton,
                               holderView: retryButtonView!,
                               topView: retryButtonView!,
                               leftView: nil,
                               rightView: retryButtonView,
                               height: 20,
                               width: 100,
                               pinTopAttribute: .Top,
                               setBottomConstraints: true,
                               marginConstraint: kTableViewCellMargin)
    retryButton.setTitle("RETRY", forState: UIControlState.Normal)
    retryButton.setTitleColor(kGreenColor, forState: UIControlState.Normal)
    retryButton.titleLabel!.font = UIFont.systemFontOfSize(14, weight: UIFontWeightSemibold)
    retryButton.titleLabel!.font = UIFont(name: "Arial-BoldMT", size: kTextFontSize)
    retryButton.titleLabel?.textAlignment = .Right
    retryButton.addTarget(self,
                          action: #selector(ConnectionView.retryButtonPressed),
                          forControlEvents: .TouchUpInside)
  }

  func retryButtonPressed() {
    if let callback = retryConnectionCallback {
      callback()
    }
  }

  func createConnectionView() {
    connectionHolderView = createHolderView(connectionView, pinTop: .Top)
    let connectionImage = UIImage(named: "bluetoothConnection")
    let connectionImageView = UIImageView(image: connectionImage!)
    connectionImageView.alpha = 0.5
    let label = createLabelWithText("Connection",
                                    holder: connectionHolderView,
                                    image: connectionImageView)
    stateTextView = createTextView("Attempting to connect to the beacon...",
                                   holder: connectionHolderView,
                                   topView: label)
    retryButtonView = UIView()
    connectionHolderView.addSubview(retryButtonView!)
    CustomViews.setConstraints(retryButtonView!,
                               holderView: connectionHolderView,
                               topView: stateTextView!,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
    let progressView = UIProgressView()
    connectionHolderView.addSubview(progressView)
    CustomViews.setConstraints(progressView,
                               holderView: connectionHolderView,
                               topView: retryButtonView!,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)
  }

  func createUnlockingView() -> UIView {
    unlockHolderView = createHolderView(connectionHolderView, pinTop: .Bottom)
    let unlockingImage = UIImage(named: "unlock")
    let unlockingImageView = UIImageView(image: unlockingImage)
    unlockingImageView.alpha = 0.5
    let label = createLabelWithText("Unlock Beacon",
                                    holder: unlockHolderView,
                                    image: unlockingImageView)
    stateTextView = createTextView("Enter the key to unlock the beacon.\n" +
                                   "The key should be a 32 hex characters string.",
                                   holder: unlockHolderView,
                                   topView: label)
    textField = createTextField("Insert unlock code",
                                holderView: unlockHolderView,
                                topView: stateTextView!,
                                leftView: nil,
                                topAttribute: .Bottom)
    textField.delegate = self
    createUnlockButton(unlockHolderView, topView: textField)
    return unlockHolderView
  }

  func configureView(retryConnectionCallback: () -> Void,
                     unlockBeaconCallback: (passkey: String) -> Void) -> UIView {
    self.retryConnectionCallback = retryConnectionCallback
    self.unlockBeaconCallback = unlockBeaconCallback
    connectionView.frame = view.bounds
    connectionView.backgroundColor = kLightGrayColor
    createConnectionView()
    connectionView.addSubview(connectionHolderView)
    CustomViews.setConstraints(connectionHolderView,
                               holderView: connectionView,
                               topView: connectionView,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Top,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)

    return connectionView
  }
  func dismissKeyboard() {
    view.endEditing(true)
  }

  func createTextField(placeholder: String,
                       holderView: UIView,
                       topView: UIView,
                       leftView: UIView?,
                       topAttribute: NSLayoutAttribute) -> UITextField {
    let textField = UITextField()
    holderView.addSubview(textField)
    textField.attributedPlaceholder = NSAttributedString(string:placeholder,
                                                         attributes:[NSForegroundColorAttributeName:
                                                          UIColor.grayColor()])
    CustomViews.setConstraints(textField,
                               holderView: holderView,
                               topView: topView,
                               leftView: leftView,
                               rightView: nil,
                               height: kTextFieldHeight,
                               width: nil,
                               pinTopAttribute: topAttribute,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)

    textField.layer.backgroundColor = UIColor.whiteColor().CGColor
    textField.layer.borderColor = UIColor.grayColor().CGColor
    textField.layer.borderWidth = 0.0
    textField.layer.masksToBounds = false
    CustomViews.addShadow(textField)
    textField.font = UIFont(name: "Menlo-Regular", size: kTextFieldFontSize)
    textField.autocapitalizationType = .None
    textField.layer.sublayerTransform = CATransform3DMakeTranslation(3, 0, 0)
    textField.userInteractionEnabled = true
    textField.enabled = true
    return textField
  }

  func createUnlockButton(holder: UIView, topView: UIView) -> UIView {
    unlockButton = UIButton()
    holder.addSubview(unlockButton)
    CustomViews.setConstraints(unlockButton,
                               holderView: holder,
                               topView: topView,
                               leftView: nil,
                               rightView: holder,
                               height: kButtonWidth/5,
                               width: kButtonWidth,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)
    unlockButton.setTitle("UNLOCK", forState: UIControlState.Normal)
    unlockButton.setTitleColor(kGreenColor, forState: UIControlState.Normal)
    unlockButton.setTitleColor(UIColor.darkGrayColor(), forState: UIControlState.Disabled)
    unlockButton.titleLabel!.font = UIFont(name: "Arial-BoldMT", size: kTextFontSize)
    unlockButton.titleLabel?.textAlignment = .Right
    unlockButton.addTarget(self,
                           action: #selector(ConnectionView.unlockButtonPressed),
                           forControlEvents: .TouchUpInside)
    unlockButton.enabled = false
    return unlockButton
  }

  func unlockButtonPressed() {
    if let callback = unlockBeaconCallback {
      callback(passkey: textField.text!)
    }
  }
}
