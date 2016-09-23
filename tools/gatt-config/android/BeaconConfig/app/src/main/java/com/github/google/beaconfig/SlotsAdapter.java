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

package com.github.google.beaconfig;

import android.os.Bundle;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;

import java.util.ArrayList;
import java.util.List;

import layout.BeaconTabFragment;
import layout.SlotFragment;

/**
 * The SlotsAdapter handles transforming slot data to corresponding fragments which will then be
 * displayed as tabs in the configuration page of BeaconConfig.
 */
public class SlotsAdapter extends FragmentStatePagerAdapter {
    private List<BeaconTabFragment> slotFragments;

    public SlotsAdapter(FragmentManager fm) {
        super(fm);
        slotFragments = new ArrayList<>();
    }

    public void createNewFragment(final Bundle bundle) {
        SlotFragment newSlotFragment = SlotFragment.newInstance(bundle);
        slotFragments.add(newSlotFragment);
    }

    public void addFragment(BeaconTabFragment fragment) {
        slotFragments.add(fragment);
    }

    @Override
    public BeaconTabFragment getItem(int position) {
        return slotFragments.get(position);
    }

    @Override
    public int getCount() {
        return slotFragments.size();
    }

}