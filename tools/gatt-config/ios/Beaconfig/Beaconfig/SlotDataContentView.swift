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
import Foundation

let kLeadingConstraint: CGFloat = 8
let kSlotDataMarginConstraint: CGFloat = 8
let kLabelWidth: CGFloat = 250
let kLabelHeight: CGFloat = 20
let kPickerViewHeight: CGFloat = 100
let kTextViewHeight: CGFloat = 25
let kTextFieldHeight: CGFloat = 30
let kSliderHeight: CGFloat = 25
let kLightGrayColor = UIColor(red:0.96, green:0.96, blue:0.96, alpha:1.0)
let kGreenColor = UIColor(hue: kNavBarTintColourHue,
                          saturation: kNavBarTintSaturation,
                          brightness: kNavBarTintBrightness,
                          alpha: kNavBarTintAlpha)
let kHolderViewBorderWidth: CGFloat = 0.3
let kURLTextViewWidth: CGFloat = 55
let kUIDNamespaceTextViewWidth: CGFloat = 125
let kUIDInstanceTextViewWidth: CGFloat = 62
let kUIDInstanceLength = 12
let kUIDNamespaceLength = 20
let kPassCodeLength = 32

class SlotDataContentView:  UIViewController, UIPickerViewDelegate,
                            UIPickerViewDataSource, UITextFieldDelegate,  GIDSignInUIDelegate {
  var didChangeSlotData = false
  var frameTypePickerView: UIPickerView = UIPickerView()
  let slotContentScrollView = UIScrollView()
  let slotContentView = UIView()
  var txPowerSlider: UISlider?
  var txPowerTextView: UITextView?
  var advIntervalSlider: UISlider?
  var advIntervalTextView: UITextView?
  var slotDataURL: UITextField?
  var namespaceTextView: UITextField?
  var instanceTextView: UITextField?
  var slotData: Dictionary <String, NSData> = [:]
  var broadcastCapabilities: NSDictionary = [:]
  var currentSlotUpdateData: Dictionary <String, NSData> = [:]
  var lockCodeBottomConstraints: NSLayoutConstraint?
  var lockButtonBottomConstraints: NSLayoutConstraint?
  var changeLockCodeButton: UIButton?
  var factoryResetButton: UIButton?
  var changeLockCodeView: UIView?
  var factoryResetView: UIView?
  var globalHolder: UIView?
  var oldLockCode: UITextField?
  var newLockCode: UITextField?
  var changeLockCodeCallback: ((oldCode: String, newCode: String) -> Void)?
  var factoryResetCallback: (() -> Void)?
  var remainConnectableCallback: ((on: Bool) -> Void)?
  var remainConnectableSwitch: UISwitch?
  var instanceText: UITextView!
  var namespaceText: UITextView!
  var oldCodeCharactersCounter: UITextView!
  var newCodeCharactersCounter: UITextView!
  var frameTypeChange: FrameTypeChange = .NotSet
  var signInHolder: UIView!
  var selectHolder: UIView!
  var signInWithGoogleCallback: ((viewToDisable: UIView, selectViewHolder: UIView) -> Void)?
  var showAlert: ((title: String, description: String, buttonText: String) -> Void)?
  let frameTypes: NSMutableArray = [BeaconInfo.EddystoneFrameType.UIDFrameType.description,
                                    BeaconInfo.EddystoneFrameType.URLFrameType.description,
                                    BeaconInfo.EddystoneFrameType.TelemetryFrameType.description,
                                    BeaconInfo.EddystoneFrameType.EIDFrameType.description,
                                    BeaconInfo.EddystoneFrameType.NotSetFrameType.description]

  enum FrameTypeChange: NSNumber {
    case UID = 0
    case URL = 1
    case TLM = 2
    case EID = 3
    case NotSet = 4
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
    if textField == instanceTextView {
      if newLength <= kUIDInstanceLength {
        hasRightLength = true
      }
    } else if textField == namespaceTextView {
      if newLength <= kUIDNamespaceLength {
        hasRightLength = true
      }
    } else if textField == oldLockCode || textField == newLockCode {
      if newLength <= kPassCodeLength {
        hasRightLength = true
      }
    }
    return StringUtils.inHexadecimalString(text) && hasRightLength
  }

  func textFieldDidChange(textField : UITextField){
    if let length = textField.text?.characters.count {
      if textField == instanceTextView {
        instanceText.text = "Instance (\(length)/\(kUIDInstanceLength)):"
      } else if textField == namespaceTextView {
        namespaceText.text = "Namespace (\(length)/\(kUIDNamespaceLength)):"
      } else if textField == oldLockCode {
        oldCodeCharactersCounter.text = "\(length)/\(kPassCodeLength)"
      } else if textField == newLockCode {
        newCodeCharactersCounter.text = "\(length)/\(kPassCodeLength)"
      }
    }
  }

  func textFieldShouldReturn(textField: UITextField) -> Bool {
    textField.resignFirstResponder()
    return false
  }

  func setUICallbacks(changeLockCodeCallback: (oldCode: String, newCode: String) -> Void,
                      factoryResetCallback: () -> Void,
                      remainConnectableCallback: (on: Bool) -> Void,
                      showAlert: (title: String, description: String, buttonText: String) -> Void,
                      signIn: (viewToDisable: UIView, selectViewHolder: UIView) -> Void) {
    self.changeLockCodeCallback = changeLockCodeCallback
    self.factoryResetCallback = factoryResetCallback
    self.remainConnectableCallback = remainConnectableCallback
    self.showAlert = showAlert
    self.signInWithGoogleCallback = signIn
  }


  override func viewDidLoad() {
    super.viewDidLoad()
    frameTypePickerView.dataSource = self
    frameTypePickerView.delegate = self
  }

  func numberOfComponentsInPickerView(pickerView: UIPickerView) -> Int {
    return 1
  }

  func pickerView(pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
    return frameTypes.count
  }

  func pickerView(pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
    for subview in slotContentView.subviews {
      subview.removeFromSuperview()
    }
    switch row {
    case FrameTypeChange.UID.rawValue:
      frameTypeChange = .UID;
      setAsUIDTypeContentView(slotData,
                              capabilities: broadcastCapabilities)
    case FrameTypeChange.URL.rawValue:
      frameTypeChange = .URL;
      setAsURLTypeContentView(slotData,
                              capabilities: broadcastCapabilities)
    case FrameTypeChange.TLM.rawValue:
      frameTypeChange = .TLM;
      setAsTLMTypeContentView(slotData,
                              capabilities: broadcastCapabilities)
    // Not sure what to do with the EID frame for now, so setting
    // the EID would mean setting no frame
    case FrameTypeChange.EID.rawValue:
      frameTypeChange = .EID;
      setAsEIDTypeContentView(slotData, capabilities: broadcastCapabilities)
    case FrameTypeChange.NotSet.rawValue:
      frameTypeChange = .NotSet;
      setAsNoFrameTypeContentView(broadcastCapabilities)
    default:
      break
    }
  }

  func pickerView(pickerView: UIPickerView,
                  viewForRow row: Int,
                             forComponent component: Int,
                                          reusingView view: UIView?) -> UIView {
    let pickerLabel = UILabel()
    pickerLabel.textColor = UIColor.blackColor()
    pickerLabel.text = frameTypes[row] as? String
    pickerLabel.font = UIFont(name: "Arial-BoldMT", size: 15)
    pickerLabel.textAlignment = NSTextAlignment.Center
    return pickerLabel
  }

  func createContentView() {
    slotContentScrollView.frame = view.bounds
    slotContentView.backgroundColor = kLightGrayColor
    slotContentView.frame.origin = CGPointMake(0,0)
    /// TODO: set slotContentView height to fit the content inside it
    slotContentView.frame = CGRectMake(0, 0, self.view.frame.width, self.view.frame.height + 100)
    slotContentScrollView.addSubview(slotContentView)
    slotContentScrollView.contentSize = slotContentView.bounds.size
  }

  /// Creates a label with a specific text and sets its constraints
  func createLabelWithText(text: String, view: UIView) -> UILabel {
    let textLabel = UILabel()
    textLabel.text = text
    textLabel.textColor = kGreenColor
    textLabel.font = UIFont.systemFontOfSize(19, weight: UIFontWeightSemibold)
    view.addSubview(textLabel)
    CustomViews.setConstraints(textLabel,
                               holderView: view,
                               topView: view,
                               leftView: nil,
                               rightView: nil,
                               height: kLabelHeight,
                               width: kLabelWidth,
                               pinTopAttribute: .Top,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)

    return textLabel
  }

  func createControlButton(text: String,
                           holderView: UIView,
                           topElement: UIView,
                           setBottomConstraints: Bool,
                           rightView: UIView?) -> UIButton {
    let button = UIButton()
    holderView.addSubview(button)
    CustomViews.setConstraints(button,
                               holderView: holderView,
                               topView: topElement,
                               leftView: nil,
                               rightView: rightView,
                               height: 20,
                               width: 80,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: setBottomConstraints,
                               marginConstraint: kSlotDataMarginConstraint)

    button.setTitle(text, forState: UIControlState.Normal)
    button.setTitleColor(kGreenColor, forState: UIControlState.Normal)
    button.titleLabel!.font = UIFont.systemFontOfSize(14, weight: UIFontWeightSemibold)
    button.titleLabel?.textAlignment = .Left
    button.titleLabel!.font = UIFont(name: "Arial-BoldMT", size: 14)
    button.titleLabel?.textAlignment = .Right
    return button
  }

  func createButtonWithText(text: String,
                            holderView: UIView,
                            topElement: UIView,
                            setBottomConstraints: Bool) -> UIButton {
    let button = UIButton()
    holderView.addSubview(button)
    CustomViews.setConstraints(button,
                               holderView: holderView,
                               topView: topElement,
                               leftView: nil,
                               rightView: nil,
                               height: 25,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: setBottomConstraints,
                               marginConstraint: kSlotDataMarginConstraint)

    button.setTitle(text, forState: UIControlState.Normal)
    button.setTitleColor(UIColor.darkGrayColor(), forState: UIControlState.Normal)
    button.titleLabel!.font = UIFont.systemFontOfSize(15, weight: UIFontWeightRegular)
    button.contentHorizontalAlignment = UIControlContentHorizontalAlignment.Left
    button.titleLabel?.font = UIFont(name: "Arial", size: kTextFontSize)
    button.titleEdgeInsets = UIEdgeInsets(top: 0, left: 5, bottom: -5, right: 0)
    return button
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
    return textField
  }

  func createChangeLockCodeView() {
    changeLockCodeView = UIView()
    changeLockCodeView?.backgroundColor = kLightGrayColor
    globalHolder?.addSubview(changeLockCodeView!)
    CustomViews.setConstraints(changeLockCodeView!,
                               holderView: globalHolder!,
                               topView: changeLockCodeButton!,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
  }

  func createFactoryResetView() {
    factoryResetView = UIView()
    factoryResetView?.backgroundColor = kLightGrayColor
    globalHolder?.addSubview(factoryResetView!)
    CustomViews.setConstraints(factoryResetView!,
                               holderView: globalHolder!,
                               topView: factoryResetButton!,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
  }

  func showChangeLockCodeView() {
    oldCodeCharactersCounter = createTextView("0/\(kPassCodeLength)",
                                              holderView: changeLockCodeView!,
                                              topView: changeLockCodeView!,
                                              leftView: nil,
                                              rightView: nil,
                                              width: kFrameTitleWidth,
                                              pinTopAttribute: .Top,
                                              setBottomConstraint: false)
    oldCodeCharactersCounter.backgroundColor = kLightGrayColor

    oldLockCode = createTextField("Insert old lock code",
                                  holderView: changeLockCodeView!,
                                  topView: changeLockCodeView!,
                                  leftView: oldCodeCharactersCounter,
                                  topAttribute: .Top)
    oldLockCode?.delegate = self
    oldLockCode?.addTarget(self,
                           action: #selector(textFieldDidChange),
                           forControlEvents: .EditingChanged)
    newCodeCharactersCounter = createTextView("0/\(kPassCodeLength)",
                                              holderView: changeLockCodeView!,
                                              topView: oldCodeCharactersCounter!,
                                              leftView: nil,
                                              rightView: nil,
                                              width: kFrameTitleWidth,
                                              pinTopAttribute: .Bottom,
                                              setBottomConstraint: false)
    newLockCode = createTextField("Insert new lock code",
                                  holderView: changeLockCodeView!,
                                  topView: oldLockCode!,
                                  leftView: newCodeCharactersCounter,
                                  topAttribute: .Bottom)
    newLockCode?.delegate = self
    newLockCode?.addTarget(self,
                           action: #selector(textFieldDidChange),
                           forControlEvents: .EditingChanged)
    newCodeCharactersCounter.backgroundColor = kLightGrayColor
    let saveButton = createControlButton("SAVE",
                                         holderView: changeLockCodeView!,
                                         topElement: newLockCode!,
                                         setBottomConstraints: true,
                                         rightView: changeLockCodeView)
    let cancelButton = createControlButton("CANCEL",
                                           holderView: changeLockCodeView!,
                                           topElement: newLockCode!,
                                           setBottomConstraints: true,
                                           rightView: saveButton)
    cancelButton.addTarget(self,
                           action: #selector(cancelButtonPressed),
                           forControlEvents: UIControlEvents.TouchUpInside)
    saveButton.addTarget(self,
                         action: #selector(saveLockCodeButtonPressed),
                         forControlEvents: UIControlEvents.TouchUpInside)
  }

  func showFactoryResetView() {
    let warningTextView = createTextView("Are you sure you want to factory reset all the data?",
                                         holderView: factoryResetView!,
                                         topView: factoryResetView!,
                                         leftView: nil,
                                         rightView: nil,
                                         width: nil,
                                         pinTopAttribute: .Top,
                                         setBottomConstraint: false)
    warningTextView.backgroundColor = kLightGrayColor

    let resetButton = createControlButton("RESET",
                                          holderView: factoryResetView!,
                                          topElement: warningTextView,
                                          setBottomConstraints: true,
                                          rightView: factoryResetView)

    let cancelButton = createControlButton("CANCEL",
                                           holderView: factoryResetView!,
                                           topElement: warningTextView,
                                           setBottomConstraints: true,
                                           rightView: resetButton)
    cancelButton.addTarget(self,
                           action: #selector(cancelFactoryResetButtonPressed),
                           forControlEvents: UIControlEvents.TouchUpInside)
    resetButton.addTarget(self,
                          action: #selector(performFactoryReset),
                          forControlEvents: UIControlEvents.TouchUpInside)

  }

  func saveLockCodeButtonPressed() {
    if let
      callback = changeLockCodeCallback,
      oldCode = oldLockCode?.text,
      newCode = newLockCode?.text {
      if oldCode.characters.count != kPassCodeLength {
        if let alert = showAlert {
          alert(title: "Old Passkey",
                description: "The old passkey does not have the right number of characters.",
                buttonText: "Dismiss")
        }
        return
      }
      if newCode.characters.count != kPassCodeLength {
        if let alert = showAlert {
          alert(title: "New Passkey",
                description: "The new passkey does not have the right number of characters.",
                buttonText: "Dismiss")
        }
        return
      }
      callback(oldCode: oldCode, newCode: newCode)
    }
  }

  func lockCodeChanged() {
    for subview in changeLockCodeView!.subviews {
      subview.removeFromSuperview()
    }
    let textView = createTextView("Lock code has been successfully changed!",
                                  holderView: changeLockCodeView!,
                                  topView: changeLockCodeView!,
                                  leftView: nil,
                                  rightView: nil,
                                  width: nil,
                                  pinTopAttribute: .Top,
                                  setBottomConstraint: true)
    textView.backgroundColor = kLightGrayColor
  }

  func performFactoryReset() {
    if let callback = factoryResetCallback {
      callback()
    }
  }

  func cancelFactoryResetButtonPressed() {
    for subview in factoryResetView!.subviews {
      subview.removeFromSuperview()
    }
  }

  func cancelButtonPressed() {
    for subview in changeLockCodeView!.subviews {
      subview.removeFromSuperview()
    }
  }

  func changeLockCodeButtonPressed() {
    if changeLockCodeView!.subviews.count == 0 {
      showChangeLockCodeView()
    } else if changeLockCodeView!.subviews.count == 1 {
      for subview in changeLockCodeView!.subviews {
        subview.removeFromSuperview()
      }
    } else {
      cancelButtonPressed()
    }
  }

  func factoryResetButtonPressed() {
    if factoryResetView?.subviews.count == 0 {
      showFactoryResetView()
    } else {
      cancelFactoryResetButtonPressed()
    }
  }

  func createTextView(text: String,
                      holderView: UIView,
                      topView: UIView,
                      leftView: UIView?,
                      rightView: UIView?,
                      width: CGFloat?,
                      pinTopAttribute: NSLayoutAttribute,
                      setBottomConstraint: Bool) -> UITextView {
    let textView = UITextView()
    holderView.addSubview(textView)
    textView.textAlignment = NSTextAlignment.Left
    textView.text = text
    textView.editable = false
    textView.dataDetectorTypes = UIDataDetectorTypes.All
    textView.textContainer.maximumNumberOfLines = 3
    textView.scrollEnabled = false
    CustomViews.setConstraints(textView,
                               holderView: holderView,
                               topView: topView,
                               leftView: leftView,
                               rightView: rightView,
                               height: nil,
                               width: width,
                               pinTopAttribute: pinTopAttribute,
                               setBottomConstraints: setBottomConstraint,
                               marginConstraint: kSlotDataMarginConstraint)
    textView.textColor = UIColor.darkGrayColor()

    return textView
  }

  func createHolder(topElement: UIView,
                    setBottomConstraints: Bool,
                    pinTopAttribute: NSLayoutAttribute) -> UIView {
    let holder: UIView = UIView()
    slotContentView.addSubview(holder)
    CustomViews.setConstraints(holder,
                               holderView: slotContentView,
                               topView: topElement,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: pinTopAttribute,
                               setBottomConstraints: setBottomConstraints,
                               marginConstraint: kSlotDataMarginConstraint)
    holder.backgroundColor = UIColor.whiteColor()
    CustomViews.addShadow(holder)
    return holder
  }

  func createURLTextView(slotData: Dictionary <String, NSData>, topElement: UIView) -> UIView {
    let URLHolder = createHolder(topElement, setBottomConstraints: false, pinTopAttribute: .Bottom)
    let broadcastedURLLabel = createLabelWithText("Broadcasted URL", view: URLHolder)
    var text: String = ""
    let container = UIView()
    URLHolder.addSubview(container)
    CustomViews.setConstraints(container,
                               holderView: URLHolder,
                               topView: broadcastedURLLabel,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)
    container.backgroundColor = kLightGrayColor

    if let
      urlData = slotData[slotDataURLKey],
      url = String(data: urlData, encoding: NSUTF8StringEncoding) {
      text = "\(url)"
    } else {
      text = "URL not set"
    }
    let URLTextView = createTextView("URL: ",
                                     holderView: container,
                                     topView: container,
                                     leftView: nil,
                                     rightView: nil,
                                     width: kURLTextViewWidth,
                                     pinTopAttribute: .Top,
                                     setBottomConstraint: true)
    URLTextView.backgroundColor = kLightGrayColor
    slotDataURL = createTextField(text,
                                  holderView: container,
                                  topView: container,
                                  leftView: URLTextView,
                                  topAttribute: .Top)

    return URLHolder
  }

  func createSlider(value: Int,
                    holderView: UIView,
                    topView: UIView,
                    minVal: Float,
                    maxVal: Float) -> UISlider {
    let slider = UISlider()
    slider.minimumValue = minVal
    slider.maximumValue = maxVal
    slider.setValue(Float(value), animated: false)
    slider.continuous = true
    holderView.addSubview(slider)
    CustomViews.setConstraints(slider,
                               holderView: holderView,
                               topView: topView,
                               leftView: nil,
                               rightView: nil,
                               height: kSliderHeight,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)

    return slider
  }

  func createFrameTypePicker(frameType: Int, topView: UIView, setPageBottomConstraints: Bool)
    -> UIView {
      let frameTypePickerHolder = createHolder(topView,
                                               setBottomConstraints: false,
                                               pinTopAttribute: .Top)
      let frameTypeLabel = createLabelWithText("Frame Type", view: frameTypePickerHolder)
      frameTypePickerHolder.addSubview(frameTypePickerView)
      CustomViews.setConstraints(frameTypePickerView,
                                 holderView: frameTypePickerHolder,
                                 topView: frameTypeLabel,
                                 leftView: nil,
                                 rightView: nil,
                                 height: kPickerViewHeight,
                                 width: nil,
                                 pinTopAttribute: .Bottom,
                                 setBottomConstraints: true,
                                 marginConstraint: kSlotDataMarginConstraint)
      frameTypePickerView.selectRow(frameType, inComponent: 0, animated: false)

      return frameTypePickerHolder
  }


  func createTxPowerHolder(slotData: Dictionary <String, NSData>,
                           topElement: UIView,
                           topAttribute: NSLayoutAttribute,
                           setBottomConstraints: Bool) -> UIView {
    let txPowerHolder = createHolder(topElement,
                                     setBottomConstraints: setBottomConstraints,
                                     pinTopAttribute: topAttribute)
    let txPowerLabel = createLabelWithText("TX Power", view: txPowerHolder)
    var txPower: Int8 = 0
    var txPowerText: String!
    if let txPowerData = slotData[slotDataTxPowerKey] {
      txPowerData.getBytes(&txPower, length: sizeof(Int8))
      txPowerText = "\(txPower)"
    } else {
      txPowerText = "Unknown"
    }
    txPowerTextView = createTextView("Radio TX Power: \(txPowerText) dBm",
                                     holderView: txPowerHolder,
                                     topView: txPowerLabel,
                                     leftView: nil,
                                     rightView: nil,
                                     width: nil,
                                     pinTopAttribute: .Bottom,
                                     setBottomConstraint: false)
    txPowerSlider = createSlider(Int(txPower),
                                 holderView: txPowerHolder,
                                 topView: txPowerTextView!,
                                 minVal: -40,
                                 maxVal: 4)
    txPowerSlider?.addTarget(self,
                             action: #selector(SlotDataContentView.txPowerChanged),
                             forControlEvents: UIControlEvents.AllEvents)

    return txPowerHolder
  }

  func txPowerChanged() {
    let stepSize = 4
    let value = Int(txPowerSlider!.value)
    txPowerSlider!.value = Float(value - value % stepSize)
    txPowerTextView!.text = "Radio TX Power: \(Int(txPowerSlider!.value)) dBm"
  }

  func createAdvIntervalHolder(slotData: Dictionary <String, NSData>,
                               topElement: UIView,
                               pinTop: NSLayoutAttribute,
                               setBottomConstraints: Bool) -> UIView {
    let advIntervalHolder = createHolder(topElement,
                                         setBottomConstraints: setBottomConstraints,
                                         pinTopAttribute: pinTop)
    let advIntervalLabel = createLabelWithText("Advertising Interval", view: advIntervalHolder)
    var advInterval: UInt16 = 0
    var advIntervalText: String!
    if let advIntervalData = slotData[slotDataAdvIntervalKey] {
      advIntervalData.getBytes(&advInterval, length: sizeof(UInt16))
      advIntervalText = "\(advInterval)"
    } else {
      advIntervalText = "Unknown"
    }
    advIntervalTextView = createTextView("Advertising Interval: \(advIntervalText)",
                                         holderView: advIntervalHolder,
                                         topView: advIntervalLabel,
                                         leftView: nil,
                                         rightView: nil,
                                         width: nil,
                                         pinTopAttribute: .Bottom,
                                         setBottomConstraint: false)
    advIntervalSlider = createSlider(Int(advInterval),
                                     holderView: advIntervalHolder,
                                     topView: advIntervalTextView!,
                                     minVal: 0,
                                     maxVal: 1000)

    advIntervalSlider?.addTarget(self,
                                 action: #selector(SlotDataContentView.advIntervalChanged),
                                 forControlEvents: UIControlEvents.AllEvents)

    return advIntervalHolder
  }

  func advIntervalChanged() {
    let stepSize = 50
    let value = Int(advIntervalSlider!.value)
    advIntervalSlider!.value = Float(value - value % stepSize)
    advIntervalTextView!.text = "Advertising Interval: \(Int(advIntervalSlider!.value))"
  }

  func byteToString(byte: UInt8) -> String {
    var value: String = String(byte, radix: 16)
    if value.characters.count == 1 {
      value += "0"
    }
    return value
  }

  func createUIDTextView(slotData: Dictionary <String, NSData>, topElement: UIView) -> UIView {
    let UIDHolder = createHolder(topElement, setBottomConstraints: false, pinTopAttribute: .Bottom)
    let UIDLabel = createLabelWithText("Beacon ID", view: UIDHolder)
    var strNamespace: String = ""
    var strInstance: String = ""
    if let value = slotData[slotDataUIDKey] {
      /// The UID has 10 bytes for Namespace and 6 bytes for Instance
      var namespace: [UInt8] = [UInt8](count: 10, repeatedValue: 0)
      var instance: [UInt8] = [UInt8](count: 6, repeatedValue: 0)
      value.getBytes(&namespace, length: 10 * sizeof(UInt8))
      value.getBytes(&instance, range: NSMakeRange(10, 6 * sizeof(UInt8)))
      for byte in namespace {
        strNamespace += byteToString(byte)
      }

      for byte in instance {
        strInstance += byteToString(byte)
      }
    } else {
      strNamespace = "nil"
      strInstance = "nil"
    }

    let container = UIView()
    UIDHolder.addSubview(container)
    container.backgroundColor = kLightGrayColor
    CustomViews.setConstraints(container,
                               holderView: UIDHolder,
                               topView: UIDLabel,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)

    namespaceText = createTextView("Namespace:",
                                   holderView: container,
                                   topView: container,
                                   leftView: nil,
                                   rightView: nil,
                                   width: kUIDNamespaceTextViewWidth,
                                   pinTopAttribute: .Top,
                                   setBottomConstraint: false)

    let namespaceDescription = "\(strNamespace)"
    let instanceDescription = "\(strInstance)"
    namespaceTextView = createTextField(namespaceDescription,
                                        holderView: container,
                                        topView: container,
                                        leftView: namespaceText,
                                        topAttribute: .Top)
    instanceText = createTextView("Instance:",
                                  holderView: container,
                                  topView: namespaceText,
                                  leftView: nil,
                                  rightView: nil,
                                  width: kUIDNamespaceTextViewWidth,
                                  pinTopAttribute: .Bottom,
                                  setBottomConstraint: true)
    instanceTextView = createTextField(instanceDescription,
                                       holderView: container,
                                       topView: namespaceTextView!,
                                       leftView: instanceText,
                                       topAttribute: .Bottom)
    instanceText.dataDetectorTypes = .None
    namespaceText.dataDetectorTypes = .None
    namespaceText.backgroundColor = kLightGrayColor
    instanceText.backgroundColor = kLightGrayColor
    namespaceTextView!.delegate = self
    instanceTextView!.delegate = self
    if strNamespace != "nil" {
      namespaceTextView?.text = strNamespace
    }
    if strInstance != "nil" {
      instanceTextView?.text = strInstance
    }
    namespaceTextView?.addTarget(self,
                                 action: #selector(textFieldDidChange),
                                 forControlEvents: .EditingChanged)
    instanceTextView?.addTarget(self,
                                action: #selector(textFieldDidChange),
                                forControlEvents: .EditingChanged)

    return UIDHolder
  }

  func createTLMTextView(slotData: Dictionary <String, NSData>, topElement: UIView) -> UIView {
    let TLMHolder = createHolder(topElement, setBottomConstraints: false, pinTopAttribute: .Bottom)
    let TLMLabel = createLabelWithText("Telemetry", view: TLMHolder)
    let container = UIView()
    TLMHolder.addSubview(container)
    CustomViews.setConstraints(container,
                               holderView: TLMHolder,
                               topView: TLMLabel,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)
    if let
      value = slotData[slotDataTLMKey],
      (battery, temperature, PDU, time) = BeaconInfo.parseTLMFromFrame(value) {
      let (days, hours, minutes) = BeaconInfo.convertDeciseconds(time)
      let batteryDescription = "Battery voltage:"
      let batteryVoltageTextView = createTextView(batteryDescription,
                                                  holderView: container,
                                                  topView: TLMLabel,
                                                  leftView: nil,
                                                  rightView: nil,
                                                  width: kFrameDescriptionWidth,
                                                  pinTopAttribute: .Bottom,
                                                  setBottomConstraint: false)
      createTextView("\(battery) mV/bit",
                     holderView: container,
                     topView: TLMLabel,
                     leftView: nil,
                     rightView: container,
                     width: nil,
                     pinTopAttribute: .Bottom,
                     setBottomConstraint: false)
      let line1 = createLine(container, topView: batteryVoltageTextView)
      let temperatureDescription = "Beacon temperature:"
      let temperatureTextView = createTextView(temperatureDescription,
                                               holderView: container,
                                               topView: line1,
                                               leftView: nil,
                                               rightView: nil,
                                               width: kFrameDescriptionWidth,
                                               pinTopAttribute: .Bottom,
                                               setBottomConstraint: false)
      createTextView("\(temperature)°C",
                     holderView: container,
                     topView: line1,
                     leftView: nil,
                     rightView: container,
                     width: nil,
                     pinTopAttribute: .Bottom,
                     setBottomConstraint: false)
      let line2 = createLine(container, topView: temperatureTextView)
      let advPDUDescription = "Advertising PDU count:"
      let advPDUTextView = createTextView(advPDUDescription,
                                          holderView: container,
                                          topView: line2,
                                          leftView: nil,
                                          rightView: nil,
                                          width: kFrameDescriptionWidth,
                                          pinTopAttribute: .Bottom,
                                          setBottomConstraint: false)
      createTextView("\(PDU)",
                     holderView: container,
                     topView: line2,
                     leftView: nil,
                     rightView: container,
                     width: nil,
                     pinTopAttribute: .Bottom,
                     setBottomConstraint: false)
      let line3 = createLine(container, topView: advPDUTextView)
      let timeSinceRebootDescription = "Time since power-on or reboot:"
      createTextView(timeSinceRebootDescription,
                     holderView: container,
                     topView: line3,
                     leftView: nil,
                     rightView: nil,
                     width: nil,
                     pinTopAttribute: .Bottom,
                     setBottomConstraint: false)

      let timeView = createTextView("\(days) days\n\(hours) hours\n\(minutes) mins",
                                    holderView: container,
                                    topView: line3,
                                    leftView: nil,
                                    rightView: container,
                                    width: nil,
                                    pinTopAttribute: .Bottom,
                                    setBottomConstraint: true)
      timeView.textAlignment = .Right
    } else {
      createTextView("No telemetry data",
                     holderView: TLMHolder,
                     topView: TLMLabel,
                     leftView: nil,
                     rightView: nil,
                     width: nil,
                     pinTopAttribute: .Bottom,
                     setBottomConstraint: true)
    }

    return TLMHolder
  }

  func createLine(holderView: UIView, topView: UIView) -> UIView {
    let lineView = UIView()
    holderView.addSubview(lineView)
    CustomViews.setConstraints(lineView,
                               holderView: holderView,
                               topView: topView,
                               leftView: nil,
                               rightView: nil,
                               height: 1.0,
                               width: nil,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: false,
                               marginConstraint: kSlotDataMarginConstraint)
    lineView.backgroundColor = UIColor.lightGrayColor()
    return lineView
  }

  func createGlobalView() -> UIView {
    globalHolder = createHolder(slotContentView,
                                setBottomConstraints: false,
                                pinTopAttribute: .Top)
    let globalLabel = createLabelWithText("Global Settings", view: globalHolder!)
    changeLockCodeButton = createButtonWithText("Change lock code",
                                                holderView: globalHolder!,
                                                topElement: globalLabel,
                                                setBottomConstraints: false)
    changeLockCodeButton!.addTarget(self,
                                    action: #selector(changeLockCodeButtonPressed),
                                    forControlEvents: UIControlEvents.TouchUpInside)
    createChangeLockCodeView()
    let lineView = createLine(globalHolder!, topView: changeLockCodeView!)
    factoryResetButton = createButtonWithText("Factory reset",
                                              holderView: globalHolder!,
                                              topElement: lineView,
                                              setBottomConstraints: false)
    factoryResetButton!.addTarget(self,
                                  action: #selector(factoryResetButtonPressed),
                                  forControlEvents: UIControlEvents.TouchUpInside)
    createFactoryResetView()

    let secondLineView = createLine(globalHolder!, topView: factoryResetView!)
    createButtonWithText("Remain Connectable",
                         holderView: globalHolder!,
                         topElement: secondLineView,
                         setBottomConstraints: false)
    remainConnectableSwitch = UISwitch()
    remainConnectableSwitch!.onTintColor = kGreenColor
    remainConnectableSwitch!.addTarget(self,
                                       action: #selector(switchValueChanged),
                                       forControlEvents: UIControlEvents.ValueChanged)
    globalHolder?.addSubview(remainConnectableSwitch!)
    CustomViews.setConstraints(remainConnectableSwitch!,
                               holderView: globalHolder!,
                               topView: secondLineView,
                               leftView: nil,
                               rightView: globalHolder,
                               height: nil,
                               width: kFrameTitleWidth,
                               pinTopAttribute: .Bottom,
                               setBottomConstraints: true,
                               marginConstraint: kSlotDataMarginConstraint)

    return globalHolder!
  }

  func displaySignInWarningView(topView: UIView) {
    signInHolder = createHolder(topView, setBottomConstraints: false, pinTopAttribute: .Bottom)
    let signInLabel = createLabelWithText("Sign In", view: signInHolder)
    let signInWarningTextView = createTextView("In order to configure the EID, you must sign in " +
                                               "with Google. Do you want to do that now?",
                                               holderView: signInHolder,
                                               topView: signInLabel,
                                               leftView: nil,
                                               rightView: nil,
                                               width: nil,
                                               pinTopAttribute: .Bottom,
                                               setBottomConstraint: false)
    signInWarningTextView.backgroundColor = kLightGrayColor
    let signInButton = createControlButton("SIGN IN",
                                          holderView: signInHolder,
                                          topElement: signInWarningTextView,
                                          setBottomConstraints: true,
                                          rightView: signInHolder)

    let cancelButton = createControlButton("CANCEL",
                                           holderView: signInHolder,
                                           topElement: signInWarningTextView,
                                           setBottomConstraints: true,
                                           rightView: signInButton)
    cancelButton.addTarget(self,
                           action: #selector(cancelSignInButtonPressed),
                           forControlEvents: UIControlEvents.TouchUpInside)
    signInButton.addTarget(self,
                          action: #selector(signInButtonPressed),
                          forControlEvents: UIControlEvents.TouchUpInside)

  }

  func displayProjectSelectionWarningView(topView: UIView) {
    selectHolder = createHolder(topView, setBottomConstraints: false, pinTopAttribute: .Bottom)
    let signInLabel = createLabelWithText("Select Project", view: selectHolder)
    let selectProjectTextView = createTextView("In order to configure the EID, you must select a " +
                                               "Google project associated with the user you are " +
                                               "currently signed in with. " +
                                               "Do you want to do that now?",
                                               holderView: selectHolder,
                                               topView: signInLabel,
                                               leftView: nil,
                                               rightView: nil,
                                               width: nil,
                                               pinTopAttribute: .Bottom,
                                               setBottomConstraint: false)
    selectProjectTextView.backgroundColor = kLightGrayColor
    let selectButton = createControlButton("SELECT",
                                           holderView: selectHolder,
                                           topElement: selectProjectTextView,
                                           setBottomConstraints: true,
                                           rightView: selectHolder)

    let cancelButton = createControlButton("CANCEL",
                                           holderView: selectHolder,
                                           topElement: selectProjectTextView,
                                           setBottomConstraints: true,
                                           rightView: selectButton)
    cancelButton.addTarget(self,
                           action: #selector(cancelSignInButtonPressed),
                           forControlEvents: UIControlEvents.TouchUpInside)
    selectButton.addTarget(self,
                           action: #selector(signInButtonPressed),
                           forControlEvents: UIControlEvents.TouchUpInside)
  }

  func signInButtonPressed() {
    signInWithGoogleCallback!(viewToDisable: signInHolder, selectViewHolder: selectHolder)
  }

  func cancelSignInButtonPressed() {
    frameTypePickerView.selectRow(Int(FrameTypeChange.NotSet.rawValue),
                                  inComponent: 0,
                                  animated: true)
    signInHolder.removeFromSuperview()
    selectHolder.removeFromSuperview()
  }

  func switchValueChanged() {
    if let callback = remainConnectableCallback {
      callback(on: remainConnectableSwitch!.on)
    }
  }


  func setAsURLTypeContentView(slotData: Dictionary <String, NSData>,
                               capabilities: NSDictionary) {
    frameTypeChange = .URL
    self.slotData = slotData
    self.broadcastCapabilities = capabilities
    createContentView()
    slotContentScrollView.addSubview(slotContentView)
    let pickerView = createFrameTypePicker(frameTypes
      .indexOfObject(BeaconInfo.EddystoneFrameType.URLFrameType.description),
                                           topView: slotContentView,
                                           setPageBottomConstraints: false)
    let textView = createURLTextView(slotData, topElement: pickerView)
    var txPowerView: UIView?
    if let
      perSlotTxPowerSupported = capabilities[perSlotTxPowerSupportedKey] as? Bool,
      perSlotAdvIntervSupported = capabilities[perSlotAdvIntervalsSupportedKey] as? Bool {
      if perSlotTxPowerSupported && !perSlotAdvIntervSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: textView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
      } else if perSlotTxPowerSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: textView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
        createAdvIntervalHolder(slotData,
                                topElement: txPowerView!,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      } else if perSlotAdvIntervSupported {
        createAdvIntervalHolder(slotData,
                                topElement: textView,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      }
    }
  }

  func setAsUIDTypeContentView(slotData: Dictionary <String, NSData>,
                               capabilities: NSDictionary) {
    frameTypeChange = .UID
    self.slotData = slotData
    self.broadcastCapabilities = capabilities
    createContentView()

    slotContentScrollView.addSubview(slotContentView)
    let pickerView = createFrameTypePicker(frameTypes
      .indexOfObject(BeaconInfo.EddystoneFrameType.UIDFrameType.description),
                                           topView: slotContentView,
                                           setPageBottomConstraints: false)
    let textView = createUIDTextView(slotData, topElement: pickerView)
    var txPowerView: UIView?
    if let
      perSlotTxPowerSupported = capabilities[perSlotTxPowerSupportedKey] as? Bool,
      perSlotAdvIntervSupported = capabilities[perSlotAdvIntervalsSupportedKey] as? Bool {
      if perSlotTxPowerSupported && !perSlotAdvIntervSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: textView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
      } else if perSlotTxPowerSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: textView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
        createAdvIntervalHolder(slotData,
                                topElement: txPowerView!,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      } else if perSlotAdvIntervSupported {
        createAdvIntervalHolder(slotData,
                                topElement: textView,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      }
    }
  }

  func setAsTLMTypeContentView(slotData: Dictionary <String, NSData>,
                               capabilities: NSDictionary) {
    frameTypeChange = .TLM
    self.slotData = slotData
    self.broadcastCapabilities = capabilities

    createContentView()
    slotContentScrollView.addSubview(slotContentView)
    let pickerView = createFrameTypePicker(frameTypes
      .indexOfObject(BeaconInfo.EddystoneFrameType.TelemetryFrameType.description),
                                           topView: slotContentView,
                                           setPageBottomConstraints: false)
    let textView = createTLMTextView(slotData, topElement: pickerView)
    var txPowerView: UIView?
    if let
      perSlotTxPowerSupported = capabilities[perSlotTxPowerSupportedKey] as? Bool,
      perSlotAdvIntervSupported = capabilities[perSlotAdvIntervalsSupportedKey] as? Bool {
      if perSlotTxPowerSupported && !perSlotAdvIntervSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: textView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
      } else if perSlotTxPowerSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: textView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
        createAdvIntervalHolder(slotData,
                                topElement: txPowerView!,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      } else if perSlotAdvIntervSupported {
        createAdvIntervalHolder(slotData,
                                topElement: textView,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      }
    }
  }

  func setAsEIDTypeContentView(slotData: Dictionary <String, NSData>,
                               capabilities: NSDictionary) {
    frameTypeChange = .EID
    self.slotData = slotData
    self.broadcastCapabilities = capabilities
    createContentView()
    slotContentScrollView.addSubview(slotContentView)

    let pickerView = createFrameTypePicker(frameTypes
      .indexOfObject(BeaconInfo.EddystoneFrameType.EIDFrameType.description),
                          topView: slotContentView,
                          setPageBottomConstraints: false)
    displayProjectSelectionWarningView(pickerView)
    selectHolder.hidden = true
    if GIDSignIn.sharedInstance().currentUser == nil {
      displaySignInWarningView(pickerView)
    } else if EIDConfiguration.projectID == nil {
      selectHolder.hidden = false
    }
  }

  func setAsNoFrameTypeContentView(capabilities: NSDictionary) {
    self.broadcastCapabilities = capabilities
    frameTypeChange = .NotSet
    createContentView()
    slotContentScrollView.addSubview(slotContentView)
    createFrameTypePicker(frameTypes
      .indexOfObject(BeaconInfo.EddystoneFrameType.NotSetFrameType.description),
                          topView: slotContentView,
                          setPageBottomConstraints: false)
  }

  func setAsGlobalContentView(broadcastCapabilities: NSDictionary,
                              slotData: Dictionary <String, NSData> ) {
    var txPowerView: UIView?
    createContentView()
    self.slotData = slotData
    self.broadcastCapabilities = broadcastCapabilities
    slotContentScrollView.addSubview(slotContentView)
    let globalView = createGlobalView()
    slotContentView.addSubview(globalView)
    if let
      perSlotTxPowerSupported = broadcastCapabilities[perSlotTxPowerSupportedKey] as? Bool,
      perSlotAdvIntervSupported = broadcastCapabilities[perSlotAdvIntervalsSupportedKey] as? Bool {
      if !perSlotTxPowerSupported && perSlotAdvIntervSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: globalView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
      } else if !perSlotTxPowerSupported {
        txPowerView = createTxPowerHolder(slotData,
                                          topElement: globalView,
                                          topAttribute: .Bottom,
                                          setBottomConstraints: false)
        createAdvIntervalHolder(slotData,
                                topElement: txPowerView!,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      } else if !perSlotAdvIntervSupported {
        createAdvIntervalHolder(slotData,
                                topElement: globalView,
                                pinTop: .Bottom,
                                setBottomConstraints: false)
      }
    }
  }

  func parseURLTextToFrame(URLText: String) -> NSData {
    var URL: String? = URLText
    var byte: UInt8 = 0
    var URLBytes: [UInt8] = []
    var URLData: NSData
    URLBytes.append(BeaconInfo.EddystoneURLFrameTypeID)
    while URL?.characters.count != 0 {
      (byte, URL) = BeaconInfo.byteFromEncodedString(URL!)
      URLBytes.append(byte)
    }
    URLData = NSData(bytes: URLBytes, length: URLBytes.count * sizeof(UInt8))
    return URLData
  }

  func parseDataToUIDFrame(namespace: String, instance: String) -> NSData {
    var UIDData: NSData = NSData()
    var UIDBytes: [UInt8] = []
    UIDBytes.append(BeaconInfo.EddystoneUIDFrameTypeID)
    let namespaceBytes = StringUtils.transformStringToByteArray(namespace)
    let instanceBytes = StringUtils.transformStringToByteArray(instance)

    UIDBytes.appendContentsOf(namespaceBytes)
    UIDBytes.appendContentsOf(instanceBytes)

    UIDData = NSData(bytes: UIDBytes, length: UIDBytes.count * sizeof(UInt8))
    return UIDData
  }


  func getUpdateData() -> Dictionary <String, NSData>? {
    currentSlotUpdateData.removeAll()
    if let currentTxPowerSlider = txPowerSlider {
      var txPower: Int8 = Int8(currentTxPowerSlider.value)
      let value = NSData(bytes: &txPower, length: sizeof(Int8))
      if value != slotData[slotDataTxPowerKey] {
        currentSlotUpdateData[slotDataTxPowerKey] = value
        didChangeSlotData = true
      }
    }

    if let currentAdvIntervalSlider = advIntervalSlider {
      var advInterval: UInt16 = UInt16(currentAdvIntervalSlider.value)
      let value = NSData(bytes: &advInterval, length: sizeof(UInt16))
      if value != slotData[slotDataAdvIntervalKey] {
        currentSlotUpdateData[slotDataAdvIntervalKey] = value
        didChangeSlotData = true
      }
    }

    switch frameTypeChange {
    case .URL:
      if var slotDataURLtext = slotDataURL?.text {
        if slotDataURLtext.characters.count == 0 {
          slotDataURLtext = (slotDataURL?.placeholder)!
        }
        currentSlotUpdateData[slotDataURLKey] = parseURLTextToFrame(slotDataURLtext)
      }
    case .UID:
      if var
        instanceData = instanceTextView?.text,
        namespaceData = namespaceTextView?.text {
        if namespaceData.characters.count == 0 {
          namespaceData = (namespaceTextView?.placeholder)!
        } else if namespaceData.characters.count != kUIDNamespaceLength {
          if let alert = showAlert {
            alert(title: "Namespace",
                  description: "Namespace too short. It must have exactly 20 characters.",
                  buttonText: "OK")
          }
          return nil
        }
        if instanceData.characters.count == 0 {
          instanceData = (instanceTextView?.placeholder)!
        } else if instanceData.characters.count != kUIDInstanceLength {
          if let alert = showAlert {
            alert(title: "Instance",
                  description: "Instance too short. It must have exactly 12 characters.",
                  buttonText: "OK")
          }
          return nil
        }
        if namespaceData != "nil" && instanceData != "nil" {
          currentSlotUpdateData[slotDataUIDKey] = parseDataToUIDFrame(namespaceData,
                                                                      instance: instanceData)
        }
      }
    case .TLM:
      currentSlotUpdateData[slotDataTLMKey] = NSData(bytes: [BeaconInfo.EddystoneTLMFrameTypeID],
                                                     length: 1)
    case .NotSet:
      currentSlotUpdateData[slotDataNoFrameKey] = NSData(bytes: [], length: 0)
    case .EID:
      if GIDSignIn.sharedInstance().currentUser != nil {
        if EIDConfiguration.projectID != nil {
          currentSlotUpdateData[slotDataEIDKey] =
              NSData(bytes: [BeaconInfo.EddystoneEIDFrameTypeID], length: 1)
        } else {
          if let alert = showAlert {
            alert(title: "Project",
                  description: "You are unable to configure the beacon to broadcast EID " +
                               "without having a Google Project selected.",
                  buttonText: "OK")
          }
          return nil
        }
      } else {
        if let alert = showAlert {
          alert(title: "Sign In",
                description: "You are unable to configure the beacon to broadcast EID " +
                             "without signing in with a Google account.",
                buttonText: "OK")
        }
        return nil
      }
    }
    return currentSlotUpdateData
  }
}
