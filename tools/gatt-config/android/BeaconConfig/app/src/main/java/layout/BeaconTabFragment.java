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

package layout;

import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.app.AlertDialog;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.github.google.beaconfig.Constants;
import com.github.google.beaconfig.R;
import com.github.google.beaconfig.dialogs.ConfirmationDialog;
import com.github.google.beaconfig.utils.BroadcastCapabilities;
import com.github.google.beaconfig.utils.UiUtils;
import com.github.google.beaconfig.utils.Utils;

/**
 * Abstract class to unite common behaviour between global fragment (displaying features of the
 * beacon) and slot fragments (displaying features of each particular slot of the beacon).
 */
public abstract class BeaconTabFragment extends Fragment {
    public String name;
    protected ConfigurationListener configurationListener;
    protected ExecutorService executor;

    private String txPower;
    private String advTxPower;
    private String advInterval;
    protected int slotNumber;
    protected BroadcastCapabilities capabilities;

    private boolean txPowerChanged;
    private boolean advTxPowerChanged;
    private boolean advIntervalChanged;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        executor = Executors.newSingleThreadExecutor();
        configurationListener = (ConfigurationListener) getActivity();

        updateInformation(getArguments());
        setHasOptionsMenu(true);
    }

    /**
     * Updates slot information by pulling values from the bundle. These values can be:
     *  - broadcast capabilities        type: byte[]       key: BROADCAST_CAPABILITIES
     *  - radio tx power                type: String       key: TX_POWER
     *  - adv tx power                  type: String       key: ADV_POWER
     *  - adv interval                  type: String       key: ADV_INTERVAL
     *
     * @param bundle bundle containing all the information to be updated
     */
    public void updateInformation(Bundle bundle) {
        byte[] data = bundle.getByteArray(Constants.BROADCAST_CAPABILITIES);
        if (data != null) {
            capabilities = new BroadcastCapabilities(data);
        }

        String txPower = bundle.getString(Constants.TX_POWER);
        if (txPower != null) {
            this.txPower = txPower;
        }

        String advTxPower = bundle.getString(Constants.ADV_POWER);
        if (advTxPower != null) {
            this.advTxPower = advTxPower;
        }

        String advInterval = bundle.getString(Constants.ADV_INTERVAL);
        if (advInterval != null) {
            this.advInterval = advInterval;
        }
    }

    protected void setUpFragment(View v) {
        setUpRadioTxPower(v);
        setUpAdvertisedTxPower(v);
        setUpAdvertisingInterval(v);
    }

    /**
     * Sets up the advertising tx power view. It is initially disabled. It is enabled by clicking
     * on the radio button in the UI. This opens a dialog to confirm that the user really wants to
     * change the advertising radio tx power. After this the advertising interval can be configured.
     * A list pops up where the user can choose which value they want to be broadcast as the
     * advertising tx power.
     */
    private void setUpAdvertisedTxPower(final View v) {
        if (advTxPower != null) {
            LinearLayout advTxPowerContainer
                    = (LinearLayout) v.findViewById(R.id.change_adv_tx_slot);

            final TextView advTxPowerView = (TextView) v.findViewById(R.id.adv_tx_power);
            advTxPowerView.setText(advTxPower);
            advTxPowerView.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                @Override
                public void onFocusChange(View view, boolean hasFocus) {
                    if (!hasFocus) {
                        String newAdvTxPower = advTxPowerView.getText().toString();
                        if (newAdvTxPower.length() == 0) {
                            advTxPowerView.setText(advTxPower);
                        }

                    } else {
                        advTxPowerChanged = true;
                    }
                }
            });

            advTxPowerContainer.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    advTxPowerChanged = true;
                    final String[] possibleValues = Utils.generateArrayWithStringValuesInRange(
                            Utils.findMinValue(capabilities.getSupportedTxPowers()) - 5,
                            Utils.findMaxValue(capabilities.getSupportedTxPowers()) + 5);
                    new AlertDialog.Builder(getActivity())
                            .setTitle("Choose advertising tx power:")
                            .setItems(possibleValues, new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialogInterface, int i) {
                                    advTxPower = possibleValues[i];
                                    advTxPowerView.setText(possibleValues[i]);
                                }
                            })
                            .show();
                }
            });

            UiUtils.disableAllChildren(advTxPowerContainer);

            final RadioButton enableChangeAdvTxPowerButton
                    = (RadioButton) v.findViewById(R.id.change_slot_adv_tx_power);
            enableChangeAdvTxPowerButton.setChecked(false);
            enableChangeAdvTxPowerButton.setOnClickListener(new View.OnClickListener() {
                private boolean enabled = false;
                @Override
                public void onClick(View view) {
                    if (enabled) {
                        UiUtils.disableAllChildren(v.findViewById(R.id.change_adv_tx_slot));
                        enableChangeAdvTxPowerButton.setChecked(false);
                        enabled = false;
                    } else {
                        ConfirmationDialog.confirm("Change Advertising Interval", "Are you sure "
                                + "you want to change the advertising interval of this beacon?",
                                "YES", "NO", getActivity(),
                                new ConfirmationDialog.ConfirmationListener() {
                                    @Override
                                    public void confirm() {
                                        UiUtils.enableAllChildren(
                                                v.findViewById(R.id.change_adv_tx_slot));
                                        enableChangeAdvTxPowerButton.setChecked(true);
                                        enabled = true;
                                    }

                                    @Override
                                    public void cancel() {
                                        enableChangeAdvTxPowerButton.setChecked(false);
                                        enabled = false;
                                    }
                                });
                    }
                }
            });
        }
    }

    /**
     * Setting up the radio tx power component with a seek bar which reads the supported tx powers
     * and only allows the user to set these. There is also a tracking TextView which shows the
     * numeric representation of the tx power.
     *
     * @param v container for the radio tx power component. This is usually the main container in
     *          the tab. It has to be added to the activity before calling this method.
     */
    private void setUpRadioTxPower(View v) {
        if (txPower != null) {
            v.findViewById(R.id.tx_power_info).setVisibility(View.VISIBLE);

            final TextView txPowerView = (TextView) v.findViewById(R.id.radio_tx_power);
            txPowerView.setText(txPower);
            SeekBar seekBar = (SeekBar) v.findViewById(R.id.tx_power_seek_bar);

            final byte[] allowedValues = capabilities.getSupportedTxPowers();
            final int maxValue = Utils.findMaxValue(allowedValues);
            final int minValue = Utils.findMinValue(allowedValues);

            seekBar.setMax(maxValue - minValue);
            seekBar.setProgress(Integer.parseInt(txPower) - minValue);

            seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    int newValue;
                    if (allowedValues != null) {
                        newValue = Utils.findValueClosestTo(progress + minValue, allowedValues);
                    } else {
                        newValue = progress + minValue;
                    }
                    seekBar.setProgress(newValue - minValue);
                    txPowerView.setText(Integer.toString(newValue));
                    BeaconTabFragment.this.txPower = Integer.toString(newValue);
                }
            });
        }
    }

    /**
     * Setting up the advertising interval component with a seek bar which can be set to values from
     * 100 to 1024. There is also a tracking TextView which shows the numeric representation of the
     * advertising interval.
     *
     * @param v container for the advertising interval component. This is usually the main container
     *          in the tab. It has to be added to the activity before calling this method.
     */
    private void setUpAdvertisingInterval(View v) {
        if (advInterval != null) {

            v.findViewById(R.id.adv_int_info).setVisibility(View.VISIBLE);
            final TextView advIntervalView = (TextView) v.findViewById(R.id.adv_interval);
            advIntervalView.setText(advInterval);
            SeekBar advInterSeekBar = (SeekBar) v.findViewById(R.id.adv_interval_seek_bar);
            advInterSeekBar.setMax(1024 - 100);
            int progress = Integer.parseInt(advInterval) - 100;
            advInterSeekBar.setProgress(progress);
            advInterSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                    int progress = seekBar.getProgress() + 100;
                    advIntervalView.setText(Integer.toString(progress));
                    advIntervalChanged = true;
                    advInterval = advIntervalView.getText().toString();
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });
        }
    }

    /**
     * Reads the state variables txPowerChanged, advTxPowerChanged and advIntervalChanged and
     *  uses the ConfigurationListener to send signals to the activity to write data to the beacon.
     */
    public void saveChanges() {
        if (txPowerChanged) {
            int newRadioTxPower = Integer.parseInt(txPower);
            configurationListener.txPowerChanged(slotNumber, newRadioTxPower);
            txPowerChanged = false;
        }

        if (advTxPowerChanged) {
            int newAdvTxPower = Integer.parseInt(advTxPower);
            configurationListener.advTxPowerChanged(slotNumber, newAdvTxPower);
            advTxPowerChanged = false;
        }

        if (advIntervalChanged) {
            int newAdvInterval = Integer.parseInt(advInterval);
            configurationListener.advIntervalChanged(slotNumber, newAdvInterval);
            advIntervalChanged = false;
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        configurationListener = null;
    }

    public boolean changesPending() {
        return advIntervalChanged || advTxPowerChanged || txPowerChanged;
    }

    public String getTxPower() {
        return txPower;
    }

    public String getAdvTxPower() {
        return advTxPower;
    }

    public String getAdvInterval() {
        return advInterval;
    }

    public void setTxPower(int txPower) {
        this.txPower = Integer.toString(txPower);
        txPowerChanged = true;
    }

    public void setAdvTxPower(int advTxPower) {
        this.advTxPower = Integer.toString(advTxPower);
        advTxPowerChanged = true;
    }

    public void setAdvInterval(int advInterval) {
        this.advInterval = Integer.toString(advInterval);
        advIntervalChanged = true;
    }

    /**
     * Used for communication between the configuration activity
     * and all the fragments in it.
     */
    public interface ConfigurationListener {
        void txPowerChanged(int slot, int txPower);
        void advTxPowerChanged(int slot, int advTxPower);
        void advIntervalChanged(int slot, int advInterval);
        void slotDataChanged(int slot, byte[] slotData);
        void lockCodeChanged(String newLockCode);
        void factoryResetCalled();
        void remainConnectableChanged(boolean remainConnectable);
    }
}
