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

let kTableViewCellMargin: CGFloat = 4
let kFrameDescriptionWidth: CGFloat = 250
let kFrameTitleWidth: CGFloat = 50
let kUIDRequiredLength = 32
let kShadowOpacity: Float = 0.5
let kShadowOffsetWidth: CGFloat = 0.3
let kShadowOffsetHeight: CGFloat = 0.7
let kTitleFontSize: CGFloat = 15
let kTextFontSize: CGFloat = 14
let kTextFieldFontSize: CGFloat = 12.5
let kLabelFontSize: CGFloat = 11

///
/// Class that enables setting constraints to views and placing them in
/// certain places of the screen, in relation to other views; this is useful
/// because it makes layout easier - all views are designed using this class
/// which saves from a lot of repetitive code.
///
class CustomViews {
  ///
  /// Places the current view inside the holder view, surrounded by the top, left and right views,
  /// with a certain margin constraint. In order to establish the size of the holderView,
  /// if the current view is the last one, then bottom constraints must be set.
  ///
  class func setConstraints(view: UIView,
                            holderView: UIView,
                            topView: UIView,
                            leftView: UIView?,
                            rightView: UIView?,
                            height: CGFloat?,
                            width: CGFloat?,
                            pinTopAttribute: NSLayoutAttribute,
                            setBottomConstraints: Bool,
                            marginConstraint: CGFloat) {
    if rightView == nil {
      if leftView == nil {
        let leadingConstraint = NSLayoutConstraint(item: view,
                                                   attribute:.LeadingMargin,
                                                   relatedBy: .Equal,
                                                   toItem: holderView,
                                                   attribute: .LeadingMargin,
                                                   multiplier: 1.0,
                                                   constant: marginConstraint)
        NSLayoutConstraint.activateConstraints([leadingConstraint])
      } else {
        let leadingConstraint = NSLayoutConstraint(item: view,
                                                   attribute:.LeadingMargin,
                                                   relatedBy: .Equal,
                                                   toItem: leftView,
                                                   attribute: .TrailingMargin,
                                                   multiplier: 1.0,
                                                   constant: marginConstraint * 2)
        NSLayoutConstraint.activateConstraints([leadingConstraint])
      }
    }

    if width != nil {
      let widthConstraint = NSLayoutConstraint(item: view,
                                               attribute:.Width,
                                               relatedBy: .Equal,
                                               toItem: nil,
                                               attribute: .NotAnAttribute,
                                               multiplier: 1.0,
                                               constant: width!)
      NSLayoutConstraint.activateConstraints([widthConstraint])
    } else if rightView == nil {
      let trailingConstraints = NSLayoutConstraint(item: view,
                                                   attribute:.TrailingMargin,
                                                   relatedBy: .Equal,
                                                   toItem: holderView,
                                                   attribute: .TrailingMargin,
                                                   multiplier: 1.0,
                                                   constant: -marginConstraint)
      NSLayoutConstraint.activateConstraints([trailingConstraints])
    }
    if rightView != nil {
      let marginAttribute: NSLayoutAttribute!
      if rightView == holderView {
        marginAttribute = .TrailingMargin
      } else {
        marginAttribute = .LeadingMargin
      }
      let trailingConstraints = NSLayoutConstraint(item: view,
                                                   attribute:.TrailingMargin,
                                                   relatedBy: .Equal,
                                                   toItem: rightView,
                                                   attribute: marginAttribute,
                                                   multiplier: 1.0,
                                                   constant: -marginConstraint)
      NSLayoutConstraint.activateConstraints([trailingConstraints])
    }
    if height != nil {
      let heightConstraint = NSLayoutConstraint(item: view,
                                                attribute: .Height,
                                                relatedBy: .Equal,
                                                toItem: nil,
                                                attribute: .NotAnAttribute,
                                                multiplier: 1.0,
                                                constant: height!)
      NSLayoutConstraint.activateConstraints([heightConstraint])
    }

    let pinTopConstraints = NSLayoutConstraint(item: view,
                                               attribute: .Top,
                                               relatedBy: .Equal,
                                               toItem: topView,
                                               attribute: pinTopAttribute,
                                               multiplier: 1.0,
                                               constant: marginConstraint)
    if setBottomConstraints {
      let bottomConstraints = NSLayoutConstraint(item: view,
                                                 attribute: .Bottom,
                                                 relatedBy: .Equal,
                                                 toItem: holderView,
                                                 attribute: .Bottom,
                                                 multiplier: 1.0,
                                                 constant: -marginConstraint)
      NSLayoutConstraint.activateConstraints([bottomConstraints])
    }

    view.translatesAutoresizingMaskIntoConstraints = false
    NSLayoutConstraint.activateConstraints([pinTopConstraints])
  }

  class func createFrameDisplayView(type: BeaconInfo.EddystoneFrameType,
                                    data: String,
                                    holder: UIView,
                                    topView: UIView,
                                    pinTop: NSLayoutAttribute,
                                    setBottomConstraints: Bool) -> UIView {
    var title: String
    var description: String
    var dataDescription: String
    switch type {
    case .UIDFrameType:
      title = "UID"
      description = "Namespace: \nInstance:"
      dataDescription = getUIDDescription(data)
    case .URLFrameType:
      title = "URL"
      description = "Link:"
      dataDescription = data
    case .TelemetryFrameType:
      title = "TLM"
      description = "Battery:\nTemperature:\nTime working:\nPDU Count:"
      dataDescription = data
    case .EIDFrameType:
      title = "EID"
      description = "Ephemeral ID:"
      dataDescription = data
    default:
      return UIView()
    }

    let titleLabel = UILabel()
    holder.addSubview(titleLabel)
    titleLabel.font = UIFont(name: "Arial-BoldMT", size: kTitleFontSize)
    CustomViews.setConstraints(titleLabel,
                               holderView: holder,
                               topView: topView,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: kFrameTitleWidth,
                               pinTopAttribute: pinTop,
                               setBottomConstraints: setBottomConstraints,
                               marginConstraint: kTableViewCellMargin)
    titleLabel.text = title
    titleLabel.textColor = UIColor.darkGrayColor()

    let infoLabel = UILabel()
    holder.addSubview(infoLabel)
    CustomViews.setConstraints(infoLabel,
                               holderView: holder,
                               topView: topView,
                               leftView: titleLabel,
                               rightView: nil,
                               height: nil,
                               width: kFrameDescriptionWidth,
                               pinTopAttribute: pinTop,
                               setBottomConstraints: setBottomConstraints,
                               marginConstraint: kTableViewCellMargin)
    infoLabel.text = description
    infoLabel.numberOfLines = 4
    infoLabel.font = UIFont(name: "Arial", size: kLabelFontSize)
    infoLabel.textColor = UIColor.darkGrayColor()

    let dataLabel = UILabel()
    holder.addSubview(dataLabel)
    CustomViews.setConstraints(dataLabel,
                               holderView: holder,
                               topView: topView,
                               leftView: infoLabel,
                               rightView: holder,
                               height: nil,
                               width: nil,
                               pinTopAttribute: pinTop,
                               setBottomConstraints: setBottomConstraints,
                               marginConstraint: kTableViewCellMargin)

    dataLabel.text = dataDescription
    dataLabel.textAlignment = .Right
    dataLabel.numberOfLines = 4
    dataLabel.font = UIFont(name: "Arial", size: kLabelFontSize)
    dataLabel.textColor = UIColor.darkGrayColor()
    return dataLabel
  }

  class func getUIDDescription(data: String) -> String {
    var description = ""
    if data.characters.count == kUIDRequiredLength {
      ///
      /// The first 20 characters represent the 10 bytes of namespace,
      /// the following 12 characters are the 6 bytes of instance.
      ///
      let namespace = (data as NSString).substringWithRange(NSRange(location: 0, length: 20))
      let instance = (data as NSString).substringWithRange(NSRange(location: 20, length: 12))
      description = "\(namespace)\n\(instance)"
    }
    return description
  }

  class func displayConfigurableBeacon(holder: UIView, topView: UIView, pinTop: NSLayoutAttribute) {
    let textLabel = UILabel()
    holder.addSubview(textLabel)
    textLabel.font = UIFont(name: "Arial", size: kTextFieldFontSize)
    CustomViews.setConstraints(textLabel,
                               holderView: holder,
                               topView: topView,
                               leftView: nil,
                               rightView: nil,
                               height: nil,
                               width: kFrameDescriptionWidth,
                               pinTopAttribute: pinTop,
                               setBottomConstraints: true,
                               marginConstraint: kTableViewCellMargin)
    textLabel.text = "Configurable beacon"
    textLabel.textColor = UIColor.darkGrayColor()
  }

  class func addShadow(view: UIView) {
    view.layer.shadowColor = UIColor.darkGrayColor().CGColor
    view.layer.shadowOpacity = kShadowOpacity
    view.layer.shadowOffset = CGSize(width: kShadowOffsetWidth, height: kShadowOffsetHeight)
    view.layer.shadowRadius = 1
  }
}
