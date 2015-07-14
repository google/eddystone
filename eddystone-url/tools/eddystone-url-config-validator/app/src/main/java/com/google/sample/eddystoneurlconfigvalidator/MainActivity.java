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

import com.google.sample.eddystoneurlconfigvalidator.TestTypesAdapter.StartTestType;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Switch;

public class MainActivity extends Activity {

  public static final String TEST_TYPE = "MainActivity.TestType";
  public static final String LOCK_IMPLEMENTED = "MainActivity.LockImplemented";
  private boolean lockImplemented = false;


  private final StartTestType mStartTestType = new StartTestType() {
    @Override
    public void startTestType(String testType) {
      Intent intent = new Intent(MainActivity.this, TestActivity.class);
      intent.putExtra(MainActivity.TEST_TYPE, testType);
      intent.putExtra(MainActivity.LOCK_IMPLEMENTED, lockImplemented);
      startActivity(intent);
    }
  };

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    RecyclerView recyclerView = (RecyclerView) findViewById(R.id.recyclerView_types);

    recyclerView.setHasFixedSize(true);

    RecyclerView.LayoutManager layoutManager = new LinearLayoutManager(this);
    recyclerView.setLayoutManager(layoutManager);

    RecyclerView.Adapter mAdapter = new TestTypesAdapter(getTestsInfo(), mStartTestType, getString(R.string.test_type_header));
    recyclerView.setAdapter(mAdapter);

  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    // Inflate the menu items for use in the action bar
    MenuInflater inflater = getMenuInflater();
    inflater.inflate(R.menu.main_activity_actions, menu);
    // Set listener for switch
    MenuItem toggleLockImplemented = menu.findItem(R.id.action_lock);
    View switchLockImplemented = toggleLockImplemented.getActionView();
    Switch s = (Switch) switchLockImplemented.findViewById(R.id.switch_lock_implemented);
    s.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View v) {
        lockImplemented = ((Switch) v).isChecked();
      }
    });

    return super.onCreateOptionsMenu(menu);
  }

  private TestInfo[] getTestsInfo() {
    return new TestInfo[]{
        new TestInfo(CoreEddystoneURLTests.TEST_NAME, CoreEddystoneURLTests.class.getName()),
        new TestInfo(SpecEddystoneURLTests.TEST_NAME, SpecEddystoneURLTests.class.getName())
    };
  }

  // Object to hold the test name and the test type.
  // The test type is then passed to the test activity
  public class TestInfo {

    public final String testName;
    public final String className;

    public TestInfo(String testName, String className) {
      this.testName = testName;
      this.className = className;
    }
  }

}
