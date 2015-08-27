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
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/**
 * Simple ArrayAdapter to manage the UI for displaying validation results.
 */
public class BeaconArrayAdapter extends ArrayAdapter<Beacon> implements Filterable {

  private static final int DARK_GREEN = Color.argb(255, 0, 150, 0);
  private static final int DARK_RED = Color.argb(255, 150, 0, 0);

  private List<Beacon> allBeacons;
  private List<Beacon> filteredBeacons;

  public BeaconArrayAdapter(Context context, int resource, List<Beacon> allBeacons) {
    super(context, resource, allBeacons);
    this.allBeacons = allBeacons;
    this.filteredBeacons = allBeacons;
  }

  @Override
  public int getCount() {
    return filteredBeacons.size();
  }

  @Override
  public Beacon getItem(int position) {
    return filteredBeacons.get(position);
  }

  private double distanceFromRssi(int rssi, int txPower0m) {
    int pathLoss = txPower0m - rssi;
    return Math.pow(10, (pathLoss - 41) / 20.0);
  }

  @Override
  public View getView(int position, View convertView, ViewGroup parent) {
    if (convertView == null) {
      convertView = LayoutInflater.from(getContext())
          .inflate(R.layout.beacon_list_item, parent, false);
    }

    // Note: this is a listView and the convertView object here is likely to be
    // a recycled view of some other row that isn't in view. You need to set every
    // field regardless of emptiness to avoid displaying erroneous data.

    final Beacon beacon = getItem(position);

    TextView deviceAddress = (TextView) convertView.findViewById(R.id.deviceAddress);
    deviceAddress.setText(beacon.deviceAddress);

    TextView rssi = (TextView) convertView.findViewById(R.id.rssi);
    rssi.setText(String.valueOf(beacon.rssi));

    TextView distance = (TextView) convertView.findViewById(R.id.distance);
    if (beacon.hasUidFrame) {
      distance.setText(
          String.format("%.2f m", distanceFromRssi(beacon.rssi, beacon.uidStatus.txPower)));
    } else {
      distance.setText("unknown");
    }

    TextView uidLabel = (TextView) convertView.findViewById(R.id.uidLabel);
    TextView uidNamespace = (TextView) convertView.findViewById(R.id.uidNamespace);
    TextView uidInstance = (TextView) convertView.findViewById(R.id.uidInstance);
    TextView uidTxPower = (TextView) convertView.findViewById(R.id.uidTxPower);
    View uidErrorGroup = convertView.findViewById(R.id.uidErrorGroup);

    View uidGroup = convertView.findViewById(R.id.uidGroup);
    if (!beacon.hasUidFrame) {
      grey(uidLabel);
      uidGroup.setVisibility(View.GONE);
    } else {
      if (beacon.uidStatus.getErrors().isEmpty()) {
        green(uidLabel);
        uidErrorGroup.setVisibility(View.GONE);
      } else {
        red(uidLabel);
        uidErrorGroup.setVisibility(View.VISIBLE);
        ((TextView) convertView.findViewById(R.id.uidErrors)).setText(beacon.uidStatus.getErrors());
      }
      uidNamespace.setText(beacon.uidStatus.uidValue.substring(0, 20));
      uidInstance.setText(beacon.uidStatus.uidValue.substring(20, 32));
      uidTxPower.setText(String.valueOf(beacon.uidStatus.txPower));
      uidGroup.setVisibility(View.VISIBLE);
    }

    TextView tlmLabel = (TextView) convertView.findViewById(R.id.tlmLabel);
    TextView tlmVersion = (TextView) convertView.findViewById(R.id.tlmVersion);
    TextView tlmVoltage = (TextView) convertView.findViewById(R.id.tlmVoltage);
    TextView tlmTemp = (TextView) convertView.findViewById(R.id.tlmTemp);
    TextView tlmAdvCnt = (TextView) convertView.findViewById(R.id.tlmAdvCount);
    TextView tlmSecCnt = (TextView) convertView.findViewById(R.id.tlmSecCnt);
    View tlmErrorGroup = convertView.findViewById(R.id.tlmErrorGroup);

    View tlmGroup = convertView.findViewById(R.id.tlmGroup);
    if (!beacon.hasTlmFrame) {
      grey(tlmLabel);
      tlmGroup.setVisibility(View.GONE);
    } else {
      if (beacon.tlmStatus.toString().isEmpty()) {
        green(tlmLabel);
        tlmErrorGroup.setVisibility(View.GONE);
      } else {
        red(tlmLabel);
        tlmErrorGroup.setVisibility(View.VISIBLE);
        ((TextView) convertView.findViewById(R.id.tlmErrors)).setText(beacon.tlmStatus.getErrors());

      }
      tlmVersion.setText(beacon.tlmStatus.version);
      tlmVoltage.setText(beacon.tlmStatus.voltage);
      tlmTemp.setText(beacon.tlmStatus.temp);
      tlmAdvCnt.setText(beacon.tlmStatus.advCnt);
      tlmSecCnt.setText(beacon.tlmStatus.secCnt);
      tlmGroup.setVisibility(View.VISIBLE);
    }

    TextView urlLabel = (TextView) convertView.findViewById(R.id.urlLabel);
    TextView urlStatus = (TextView) convertView.findViewById(R.id.urlStatus);
    if (!beacon.hasUrlFrame) {
      grey(urlLabel);
      urlStatus.setText("");
    } else {
      if (beacon.urlStatus.getErrors().isEmpty()) {
        green(urlLabel);
      } else {
        red(urlLabel);
      }
      urlStatus.setText(beacon.urlStatus.toString());
    }

    LinearLayout frameStatusGroup = (LinearLayout) convertView.findViewById(R.id.frameStatusGroup);
    if (!beacon.frameStatus.getErrors().isEmpty()) {
      TextView frameStatus = (TextView) convertView.findViewById(R.id.frameStatus);
      frameStatus.setText(beacon.frameStatus.toString());
      frameStatusGroup.setVisibility(View.VISIBLE);
    } else {
      frameStatusGroup.setVisibility(View.GONE);
    }

    return convertView;
  }

  @Override
  public Filter getFilter() {
    return new Filter() {
      @Override
      protected FilterResults performFiltering(CharSequence constraint) {
        FilterResults results = new FilterResults();
        List<Beacon> filteredBeacons;
        if (constraint != null && constraint.length() != 0) {
          filteredBeacons = new ArrayList<>();
          for (Beacon beacon : allBeacons) {
            if (beacon.contains(constraint.toString())) {
              filteredBeacons.add(beacon);
            }
          }
        } else {
          filteredBeacons = allBeacons;
        }
        results.count = filteredBeacons.size();
        results.values = filteredBeacons;
        return results;
      }

      @Override
      protected void publishResults(CharSequence constraint, FilterResults results) {
        filteredBeacons = (List<Beacon>) results.values;
        if (results.count == 0) {
          notifyDataSetInvalidated();
        } else {
          notifyDataSetChanged();
        }
      }
    };
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
