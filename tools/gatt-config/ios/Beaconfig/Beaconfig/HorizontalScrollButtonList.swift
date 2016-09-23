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

let globalButtonIndex = 0

import UIKit

///
/// Displays a custom made horizontal scroll button list, which appears on top of the view.
/// There can be only one selected button at a time and the class deselects the previously
/// selected button. The first button to be pressed is the Global one, since the Global view is
/// the default view to be displayed.
///
class HorizontalScrollButtonList: UIView {

  /// Keeps track of the previous slelected button in order to be able to deselect it
  var previousButtonPressed: UIButton?

  /// Displays a line under the selected button
  var previousUnderlineView: UIView?
  var linesArray: [UIView] = []
  var buttonsArray: [UIButton] = []

  ///
  /// Callback to the main class in order for it to handle displaying a different view
  /// for each one of the buttons
  ///
  var buttonPressedCallback: ((buttonNumber: Int) -> Void)?

  required init?(coder aDecoder: NSCoder) {
    super.init(coder: aDecoder)
  }

  init(buttonSize:CGSize, buttonNames: NSMutableArray, callback: (buttonNumber: Int) -> Void) {
    buttonPressedCallback = callback
    super.init(frame: CGRectMake(0, 0, 0, 0))
    let buttonCount = buttonNames.count

    self.frame.origin = CGPointMake(0,0)
    self.frame.size.width = buttonSize.width * CGFloat(buttonCount)
    self.frame.size.height = buttonSize.height
    self.backgroundColor = UIColor.whiteColor()

    configureLine(buttonSize.width * CGFloat(buttonCount))

    var buttonPosition = CGPointMake(0, 0)
    let padding: CGFloat = 1
    let buttonIncrement = buttonSize.width + padding

    for i in 0..<buttonCount {
      var button: UIButton!
      if let title = buttonNames[i] as? String {
        button = configureButtonDesign(buttonSize, buttonPosition: buttonPosition, title: title)
      }

      let underlineView = configureButtonUnderlineView(buttonSize, buttonPosition: buttonPosition)
      /// Show the Global button pressed, as the Global view is the first one to be displayed
      if i == globalButtonIndex {
        button.selected = true
        underlineView.hidden = false
        previousButtonPressed = button
        previousUnderlineView = underlineView
      }
      buttonPosition.x = buttonPosition.x + buttonIncrement
      let separatorPadding: CGFloat = 20
      if i != buttonCount - 1 {
        configureButtonSeparatorView(buttonSize.height - separatorPadding,
                                     position: CGPointMake(buttonPosition.x - padding,
                                      buttonPosition.y + separatorPadding / 2))
      }
      self.addSubview(underlineView)
    }
  }

  func configureButtonSeparatorView(height: CGFloat, position: CGPoint) {
    let rightView = UIView()
    rightView.backgroundColor = UIColor.lightGrayColor()
    rightView.frame.size = CGSize(width: 1, height: height)
    rightView.frame.origin = position
    self.addSubview(rightView)
  }

  func configureButtonDesign(buttonSize: CGSize,
                             buttonPosition: CGPoint,
                             title: String) -> UIButton {
    let button = UIButton(type: .Custom)
    button.frame.size = buttonSize
    button.frame.origin = buttonPosition
    button.setTitle(title, forState: UIControlState.Normal)
    button.setTitleColor(UIColor.darkGrayColor(), forState: UIControlState.Normal)
    button.setTitleColor(UIColor.grayColor(), forState: UIControlState.Selected)
    button.titleLabel?.font = UIFont(name: "Arial-BoldMT", size: kTextFontSize)
    button.addTarget(self,
                     action: #selector(HorizontalScrollButtonList.slotButtonPressed(_:)),
                     forControlEvents: .TouchUpInside)
    button.backgroundColor = UIColor.whiteColor()

    buttonsArray.append(button)
    self.addSubview(button)

    return button
  }

  func configureButtonUnderlineView(buttonSize: CGSize, buttonPosition: CGPoint) -> UIView {
    let view = UIView()
    let lineHeight: CGFloat = 3
    view.frame.size = CGSizeMake(buttonSize.width, lineHeight)
    view.frame.origin = CGPointMake(buttonPosition.x, buttonPosition.y + buttonSize.height)
    view.backgroundColor = kGreenColor
    view.hidden = true
    linesArray.append(view)

    return view
  }

  func configureLine(width: CGFloat) -> UIView {
    let line = UIView()
    line.backgroundColor = UIColor.whiteColor()
    line.frame.size.height = 2
    line.frame.size.width = width
    line.frame.origin = CGPointMake(0, self.frame.height)
    CustomViews.addShadow(line)
    self.addSubview(line)

    return line
  }

  ///
  /// Handle the swipe motions between the views, changing the selected button with
  /// the one that coresponds to the view the user is currently seeing.
  ///
  func swipeLeft() {
    if previousButtonPressed != buttonsArray[buttonsArray.count - 1] {
      for i in 0 ..< buttonsArray.count {
        if previousButtonPressed == buttonsArray[i] {
          slotButtonPressed(buttonsArray[i + 1])
          break
        }
      }
    }
  }

  func swipeRight() {
    if previousButtonPressed != buttonsArray[globalButtonIndex] {
      for i in 0 ..< buttonsArray.count {
        if previousButtonPressed == buttonsArray[i] {
          slotButtonPressed(buttonsArray[i - 1])
          break
        }
      }
    }
  }

  func slotButtonPressed(sender: UIButton) {
    if previousButtonPressed != nil {
      previousButtonPressed!.selected = false
      previousUnderlineView!.hidden = true
    }
    sender.selected = true
    if let
      character = sender.titleLabel?.text?.characters.last,
      buttonNumber = Int(String(character)),
      callback = buttonPressedCallback {
      linesArray[buttonNumber].hidden = false
      previousUnderlineView = linesArray[buttonNumber]
      callback(buttonNumber: buttonNumber)
    } else if let callback = buttonPressedCallback {
      linesArray[globalButtonIndex].hidden = false
      previousUnderlineView = linesArray[globalButtonIndex]
      callback(buttonNumber: globalButtonIndex)
    }
    previousButtonPressed = sender
  }
}
