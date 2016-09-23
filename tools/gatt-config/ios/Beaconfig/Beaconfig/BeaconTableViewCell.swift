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

///
/// Customizes each cell from the scanned beacons table view in order to display all the information
/// that can be found without connecting to the beacon.
///
class BeaconTableViewCell: UITableViewCell {

  @IBOutlet weak var nameLabel: UILabel!
  @IBOutlet weak var framesLabel: UITextView!
  @IBOutlet weak var cellBackgroundView: UIView!
  @IBOutlet weak var connectableDot: UIView!
  @IBOutlet weak var framesView: UIView!
  /// Holds a reference to the beacon in order to be able to connect to it.
  var beacon: CBPeripheral!
  var operations: GATTOperations?

  override func awakeFromNib() {
    super.awakeFromNib()
    CustomViews.addShadow(cellBackgroundView)
  }

  override func setSelected(selected: Bool, animated: Bool) {
    super.setSelected(selected, animated: animated)
  }
}
