/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.sample.eddystoneurlconfigvalidator;

import android.app.AlertDialog;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;

public class MultipleBeaconsAdapter extends BaseAdapter {
  private LayoutInflater mInflater;
  private ArrayList<ScanResult> mScanResults;
  private TestRunner mTestRunner;
  private AlertDialog mDialog;


  public MultipleBeaconsAdapter(Context context, ArrayList<ScanResult> scanResults,
      TestRunner testRunner, AlertDialog dialog) {
    mInflater = LayoutInflater.from(context);
    mScanResults = scanResults;
    mTestRunner = testRunner;
    mDialog = dialog;
  }

  @Override
  public int getCount() {
    return mScanResults.size();
  }

  @Override
  public Object getItem(int position) {
    return mScanResults.get(position);
  }

  @Override
  public long getItemId(int position) {
    return position;
  }

  @Override
  public View getView(final int position, View convertView, ViewGroup parent) {
    View view;
    ViewHolder holder;
    if (convertView == null) {
      view = mInflater.inflate(R.layout.beacon_info_row, parent, false);
      holder = new ViewHolder();
      holder.primary_text = (TextView) view.findViewById(R.id.primary_text);
      holder.secondary_text = (TextView) view.findViewById(R.id.secondary_text);
      view.setTag(holder);
    } else {
      view = convertView;
      holder = (ViewHolder) view.getTag();
    }

    ScanResult result = mScanResults.get(position);
    if (result.getDevice().getName() != null && !result.getDevice().getName().isEmpty()) {
      holder.primary_text.setText(result.getDevice().getName());
      holder.secondary_text.setText(result.getDevice().getAddress()
          + " (" + result.getRssi() + "dBm)");
    } else {
      holder.primary_text.setText(result.getDevice().getAddress());
      holder.secondary_text.setText(result.getRssi() + "dBm");
    }
    view.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View v) {
        mTestRunner.continueTest(position);
        mDialog.dismiss();
      }
    });
    return view;
  }

  private class ViewHolder {
    public TextView primary_text, secondary_text;
  }
}
