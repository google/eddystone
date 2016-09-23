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

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;

import com.github.google.beaconfig.Constants;
import com.github.google.beaconfig.R;
import com.github.google.beaconfig.dialogs.ChangePasswordDialog;
import com.github.google.beaconfig.dialogs.ConfirmationDialog;

/**
 * First fragment in the tab layout in the Beacon Activity. It shows global information about the
 * beacon and provides a way to set global configurations. If per slot radio tx power is not
 * supported, it will show here. Same with advertising interval. Also factory reset, change of lock
 * code and remain connectable configuration is set from here.
 */
public class GlobalFragment extends BeaconTabFragment {
    private String beaconAddress;
    private String beaconName;

    private boolean isRemainConnectableSupported;

    private boolean remainConnectableChanged;
    private boolean remainConnectable;

    public static GlobalFragment newInstance(Bundle bundle) {
        GlobalFragment globalFragment = new GlobalFragment();
        globalFragment.setArguments(bundle);
        globalFragment.slotNumber = -1;
        return globalFragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_frame_slot,
                (LinearLayout) container.findViewById(R.id.global_content), false);

        setUpFragment(v);
        return v;
    }

    /**
     * Updates slot information by pulling values from the bundle. These values can be:
     *  - beacon address                type: String       key: BEACON_ADDRESS
     *  - beacon name                   type: String       key: BEACON_NAME
     *  - remainConnectable             type: Byte         key: REMAIN_CONNECTABLE
     *  - broadcast capabilities        type: byte[]       key: BROADCAST_CAPABILITIES
     *  - radio tx power                type: String       key: TX_POWER
     *  - adv tx power                  type: String       key: ADV_POWER
     *  - adv interval                  type: String       key: ADV_INTERVAL
     *
     * @param bundle bundle containing all the information to be updated
     */
    public void updateInformation(final Bundle bundle) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                GlobalFragment.super.updateInformation(bundle);

                String beaconAddress = bundle.getString(Constants.BEACON_ADDRESS);
                if (beaconAddress != null) {
                    GlobalFragment.this.beaconAddress = beaconAddress;
                }

                String beaconName = bundle.getString(Constants.BEACON_NAME);
                if (beaconName != null) {
                    GlobalFragment.this.beaconName = beaconName;
                }

                Byte remainConnectable = bundle.getByte(Constants.REMAIN_CONNECTABLE);
                if (remainConnectable != null) {
                    GlobalFragment.this.isRemainConnectableSupported = (remainConnectable != 0);
                }
            }
        });
    }

    @Override
    protected void setUpFragment(final View v) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                GlobalFragment.super.setUpFragment(v);
                setUpGlobalInformation(v);
                setupRemainConnectable(v);
                setUpLockCodeChange(v);
                setUpFactoryReset(v);
            }
        });
    }

    /**
     * Changing the lock code opens a dialog which asks to give new lock code and repeat it.
     *  When the user is done, the new lock code is written to the beacon.
     *
     * @param v container for the change lock code component. This is usually the main container in
     *          the tab. It has to be added to the activity before calling this method.
     */
    private void setUpLockCodeChange(View v) {
        v.findViewById(R.id.lock_code_slot).setOnClickListener(new View.OnClickListener() {
               @Override
               public void onClick(View view) {
                   ChangePasswordDialog.show(getActivity(),
                           new ChangePasswordDialog.PasswordChangeListener() {
                       @Override
                       public void passwordChangeRequest(String newPassword) {
                           configurationListener.lockCodeChanged(newPassword);
                       }
                   });
               }
            });
    }

    /**
     * This sets all the TextViews in the UI to show the information read from the beacon.
     *
     * @param v container for the global information component. This is usually the main container
     *          in the tab. It has to be added to the activity before calling this method.
     */
    private void setUpGlobalInformation(View v) {
        v.findViewById(R.id.global_content).setVisibility(View.VISIBLE);
        ((TextView) v.findViewById(R.id.beacon_address)).setText(beaconAddress);
        ((TextView) v.findViewById(R.id.beacon_name)).setText(beaconName);
        ((TextView) v.findViewById(R.id.max_eid_slots))
                .setText(Integer.toString(capabilities.getMaxSupportedEidSlots()));
        ((TextView) v.findViewById(R.id.max_total_slots))
                .setText(Integer.toString(capabilities.getMaxSupportedTotalSlots()));
    }

    /**
     * This asks for confirmation before factory resetting the beacon.
     *
     * @param v container for the factory reset component. This is usually the main container in
     *          the tab. It has to be added to the activity before calling this method.
     */
    private void setUpFactoryReset(View v) {
        v.findViewById(R.id.factory_reset).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ConfirmationDialog.confirm("Factory Reset", "Are you sure you want to factory "
                        + "reset this beacon? \n \n The beacon will be reset to its initial state"
                        + " set by its manufacturer. \n \n Its unlock code will NOT be changed.",
                        "RESET", "CANCEL", getActivity(),
                        new ConfirmationDialog.ConfirmationListener() {
                            @Override
                            public void confirm() {
                                configurationListener.factoryResetCalled();
                            }

                            @Override
                            public void cancel() {

                            }
                        });
            }
        });
    }

    /**
     * Remain connectable is controlled by a radio group of 2 buttons. Initially these buttons are
     * hidden and on click the buttons show and remain connectable will be configured.
     *
     * @param v container for the remain connectable component. This is usually the main container
     *          in the tab. It has to be added to the activity before calling this method.
     */
    private void setupRemainConnectable(final View v) {
        ViewGroup connectableSlot = (ViewGroup) v.findViewById(R.id.remain_connectable_slot);
        if (isRemainConnectableSupported) {
            connectableSlot.setVisibility(View.VISIBLE);

            // Set expand and hide of radio group buttons on click of remain connectable field
            connectableSlot.setOnClickListener(new View.OnClickListener() {
                boolean expanded = false;
                @Override
                public void onClick(View view) {
                    if (expanded) {
                        v.findViewById(R.id.set_remain_connectable).setVisibility(View.GONE);
                        ((ImageView) v.findViewById(R.id.remain_connectable_icon))
                                .setImageResource(R.drawable.expand_arrow);
                        expanded = false;
                    } else {
                        v.findViewById(R.id.set_remain_connectable).setVisibility(View.VISIBLE);
                        ((ImageView) v.findViewById(R.id.remain_connectable_icon))
                                .setImageResource(R.drawable.close_arrow);
                        expanded = true;
                    }
                }
            });

            // Set first radio button to configure turning ON of remain connectable
            final RadioButton onButton = (RadioButton) v.findViewById(R.id.remain_connectable_on);
            onButton.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    remainConnectableChanged = true;
                    RadioButton offBtn = (RadioButton) v.findViewById(R.id.remain_connectable_off);
                    if (b) {
                        offBtn.setChecked(false);
                        onButton.setChecked(true);
                        remainConnectable = true;
                    }
                }
            });

            // Set first radio button to configure turning OFF of remain connectable
            final RadioButton offButton = (RadioButton) v.findViewById(R.id.remain_connectable_on);
            offButton.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    remainConnectableChanged = true;
                    RadioButton onBtn = (RadioButton) v.findViewById(R.id.remain_connectable_off);
                    if (b) {
                        onBtn.setChecked(false);
                        offButton.setChecked(true);
                        remainConnectable = false;
                    }
                }
            });
        } else {
            connectableSlot.setVisibility(View.GONE);
        }
    }

    @Override
    public void saveChanges() {
        super.saveChanges();

        if (remainConnectableChanged) {
            configurationListener.remainConnectableChanged(remainConnectable);
            remainConnectableChanged = false;
        }
    }

    @Override
    public boolean changesPending() {
        return super.changesPending() || remainConnectableChanged;
    }

}
