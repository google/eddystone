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

import java.util.ArrayList;
import java.util.List;

/**Keeps one configuration of a beacon. This includes all the slots specific
 * information, tx power, adv tx power and advertised interval. This class only holds
 * slot information about UID, URL and TLM frames. EID frames will not be saved.
 */
public class BeaconConfiguration {
    private String configurationName;
    private List<Slot> slots;

    public BeaconConfiguration(String configurationName) {
        this.configurationName = configurationName;
        slots = new ArrayList<>();
    }

    /**
     * Saved this information about a beacon's slot. It does not save EID configured slots.
     *
     * @param slotData slotData of this slot of the configuration
     * @param txPower radio tx power of this slot of the configuration
     * @param advTxPower advertised tx power of this slot of the configuration
     * @param advInterval advertised interval of this slot of the configuration
     */
    public void addSlot(byte[] slotData, int txPower, int advTxPower, int advInterval) {
        if (slotData[0] == Constants.EID_FRAME_TYPE) {
            return;
        }
        slots.add(new Slot(slotData, txPower, advTxPower, advInterval));
    }

    public String getName() {
        return configurationName;
    }

    public int getNumberOfConfiguredSlots() {
        return slots.size();
    }

    public byte[] getSlotDataForSlot(int slotNumber) {
        return slots.get(slotNumber).slotData;
    }

    public int getRadioTxPowerForSlot(int slotNumber) {
        return slots.get(slotNumber).txPower;
    }

    public int getAdvTxPowerForSlot(int slotNumber) {
        return slots.get(slotNumber).advTxPower;
    }

    public int getAdvIntervalForSlot(int slotNumber) {
        return slots.get(slotNumber).advInterval;
    }

    public void setName(String newConfigurationName) {
        this.configurationName = newConfigurationName;
    }

    private class Slot {
        byte[] slotData;
        int txPower;
        int advTxPower;
        int advInterval;

        Slot(byte[] slotData, int txPower, int advTxPower, int advInterval) {
            this.slotData = slotData;
            this.txPower = txPower;
            this.advTxPower = advTxPower;
            this.advInterval = advInterval;
        }
    }

}
