// Copyright 2015 Google Inc. All rights reserved.
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

package com.google.sample.eddystonevalidator;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.List;

/**
 * Simple ArrayAdapter to manage the UI for displaying validation results.
 */
public class BeaconArrayAdapter extends ArrayAdapter<Beacon> {

  private static final int DARK_GREEN = Color.argb(255, 0, 150, 0);
  private static final int DARK_RED = Color.argb(255, 150, 0, 0);

  public BeaconArrayAdapter(Context context, int resource, List<Beacon> objects) {
    super(context, resource, objects);
  }

  @Override
  public View getView(int position, View convertView, ViewGroup parent) {
    if (convertView == null) {
      convertView = LayoutInflater.from(getContext())
        .inflate(R.layout.beacon_list_item, parent, false);
    }

    final Beacon beacon = getItem(position);

    TextView deviceAddress = (TextView)convertView.findViewById(R.id.deviceAddress);
    deviceAddress.setText(beacon.deviceAddress);

    TextView uidLabel = (TextView)convertView.findViewById(R.id.uidLabel);
    TextView uidStatus = (TextView)convertView.findViewById(R.id.uidStatus);
    if (!beacon.hasUidFrame) {
      grey(uidLabel);
      uidStatus.setText("no data");
    } else {
      if (beacon.uidStatus.getErrors().isEmpty()) {
        green(uidLabel);
      } else {
        red(uidLabel);
      }
      uidStatus.setText(beacon.uidStatus.toString());
    }

    TextView tlmLabel = (TextView)convertView.findViewById(R.id.tlmLabel);
    TextView tlmStatus = (TextView)convertView.findViewById(R.id.tlmStatus);
    if (!beacon.hasTlmFrame) {
      grey(tlmLabel);
      tlmStatus.setText("not detected");
    } else {
      if (beacon.tlmStatus.toString().isEmpty()) {
        green(tlmLabel);
        tlmStatus.setText("OK");
      } else {
        red(tlmLabel);
        tlmStatus.setText(beacon.tlmStatus.toString());
      }
    }

    TextView urlLabel = (TextView)convertView.findViewById(R.id.urlLabel);
    TextView urlStatus = (TextView)convertView.findViewById(R.id.urlStatus);
    if (!beacon.hasUrlFrame) {
      grey(urlLabel);
      urlStatus.setText("not detected");
    } else {
      if (beacon.urlStatus.getErrors().isEmpty()) {
        green(urlLabel);
      } else {
        red(urlLabel);
      }
      urlStatus.setText(beacon.urlStatus.toString());
    }

    LinearLayout frameStatusGroup = (LinearLayout)convertView.findViewById(R.id.frameStatusGroup);
    if (!beacon.frameStatus.getErrors().isEmpty()) {
      TextView frameStatus = (TextView)convertView.findViewById(R.id.frameStatus);
      frameStatus.setText(beacon.frameStatus.toString());
      frameStatusGroup.setVisibility(View.VISIBLE);
    } else {
      frameStatusGroup.setVisibility(View.GONE);
    }

    return convertView;
  }

  private void green(TextView v) {
    v.setTextColor(DARK_GREEN);
  }

  private void red(TextView v) {
    v.setTextColor(DARK_RED);
  }

  private void grey(TextView v) {
    v.setTextColor(Color.GRAY);
  }
}
