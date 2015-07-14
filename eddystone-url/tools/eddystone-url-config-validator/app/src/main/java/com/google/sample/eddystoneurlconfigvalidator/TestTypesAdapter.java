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

import com.google.sample.eddystoneurlconfigvalidator.MainActivity.TestInfo;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.TextView;

public class TestTypesAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
  private static final int TYPE_SUBHEADER = 0;
  private static final int TYPE_TEST = 1;
  private final TestInfo[] mDataset;
  private final StartTestType mCallback;
  private String mSubheader;

  /**
   * Adapter to show the available test sets to run
   * @param tests Available tests
   * @param callback Callback to notify which test to run
   * @param subheader List subheader
   */
  public TestTypesAdapter(TestInfo[] tests, StartTestType callback, String subheader) {
    mDataset = tests;
    mCallback = callback;
    mSubheader = subheader;
  }

  // Create new views
  @Override
  public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
    if (viewType == TYPE_TEST) {
      View v = LayoutInflater.from(parent.getContext())
          .inflate(R.layout.type_view, parent, false);
      return new TestViewHolder(v);
    } else if (viewType == TYPE_SUBHEADER) {
      View v = LayoutInflater.from(parent.getContext())
          .inflate(R.layout.list_subheader, parent, false);
      return new SubheaderViewHolder(v);
    }
    throw new RuntimeException("there is no type that matches the type " + viewType + " + make sure your using types correctly");
  }

  // Replace the contents of a view
  @Override
  public void onBindViewHolder(final RecyclerView.ViewHolder holder, final int position) {
    if (holder instanceof TestViewHolder) {
      TestViewHolder testHolder = (TestViewHolder) holder;
      testHolder.mTextView.setText(getItem(position).testName);
      testHolder.itemView.setOnClickListener(new OnClickListener() {
        @Override
        public void onClick(View v) {
          mCallback.startTestType(getItem(position).className);
        }
      });
    } else if (holder instanceof SubheaderViewHolder) {
      SubheaderViewHolder subheaderHolder = (SubheaderViewHolder) holder;
      subheaderHolder.mSubheader.setText(mSubheader);
    }
  }
  private TestInfo getItem(int position) {
    return mDataset[position - 1];
  }
  @Override
  public int getItemCount() {
    return mDataset.length + 1;
  }


  @Override
  public int getItemViewType(int position) {
    if (position == 0) {
      return TYPE_SUBHEADER;
    } else {
      return TYPE_TEST;
    }
  }

  /**
   * View holder for the subheader of the list
   */
  public static class SubheaderViewHolder extends RecyclerView.ViewHolder {

    public final TextView mSubheader;

    public SubheaderViewHolder(View v) {
      super(v);
      mSubheader = (TextView) v.findViewById(R.id.subheader_textView);
    }
  }

  /**
   * View Holder for test types
   */
  public static class TestViewHolder extends RecyclerView.ViewHolder {

    public final TextView mTextView;

    public TestViewHolder(View v) {
      super(v);
      mTextView = (TextView) v.findViewById(R.id.test_type);
    }
  }

  // Interface to notify which test should be started
  public interface StartTestType {

    public void startTestType(String type);
  }
}
