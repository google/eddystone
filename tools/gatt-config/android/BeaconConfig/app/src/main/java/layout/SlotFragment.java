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
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.github.google.beaconfig.Constants;
import com.github.google.beaconfig.R;
import com.github.google.beaconfig.dialogs.UidSlotDataChangeDialog;
import com.github.google.beaconfig.dialogs.UrlChangeDialog;
import com.github.google.beaconfig.utils.SlotDataManager;
import com.github.google.beaconfig.utils.Utils;

/**
 * Fragment for displaying and configuring information of a frame slot in the beacon. This is a
 * unifying fragment which can be used for a UID, URL, TLM or EID frame.
 */
public class SlotFragment extends BeaconTabFragment implements AdapterView.OnItemSelectedListener {
    public static final String TAG =  SlotFragment.class.getSimpleName();
    private boolean slotDataChanged;

    private byte currFrameType;
    private byte[] slotData;

    /* UID fields */
    private String namespace;
    private String instance;

    /* URL field */
    private String url;

    /* TLM fields */
    private String voltage;
    private String temperature;
    private String advCnt;
    private String secCnt;

    /* EID field */
    private String ephemeralId;

    public static SlotFragment newInstance(Bundle bundle) {
        SlotFragment newSlotFragment = new SlotFragment();
        newSlotFragment.slotNumber = bundle.getInt(Constants.SLOT_NUMBER);
        newSlotFragment.setArguments(bundle);
        newSlotFragment.name
                = Utils.getFrameNameFromSlotData(bundle.getByteArray(Constants.SLOT_DATA));
        return newSlotFragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d(TAG, "SlotFragment.onCreateView");
        View v = inflater.inflate(R.layout.fragment_frame_slot,
                    (LinearLayout) container.findViewById(R.id.global_content), false);
        v.findViewById(R.id.slot_information).setVisibility(View.VISIBLE);

        setUpFrameTypeSpinner(v);
        setUpFragment(v);
        return v;
    }

    /**
     * Setting up the spinner which configures the frame type of a beacon slot.
     *
     * @param v container for the spinner. This is usually the main container in
     *          the tab. It has to be added to the activity before calling this method.
     */
    private void setUpFrameTypeSpinner(View v) {
        final Spinner frameTypeSpinner =  (Spinner) v.findViewById(R.id.frame_type_spinner);
        ArrayAdapter<CharSequence> spinnerAdapter = ArrayAdapter.createFromResource(getContext(),
                R.array.frame_types, android.R.layout.simple_spinner_item);
        spinnerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        frameTypeSpinner.setAdapter(spinnerAdapter);
        frameTypeSpinner.setOnItemSelectedListener(this);
        v.findViewById(R.id.frame_type_slot).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                frameTypeSpinner.performClick();
            }
        });
    }

    /**
     * Reads information from the bundle according to which frame type this fragment is set to.
     * The values which can be read from the bundle are:
     *  - slot data                     type: byte[]       key: SLOT_DATA
     *  - broadcast capabilities        type: byte[]       key: BROADCAST_CAPABILITIES
     *  - radio tx power                type: String       key: TX_POWER
     *  - adv tx power                  type: String       key: ADV_POWER
     *  - adv interval                  type: String       key: ADV_INTERVAL
     *
     * @param dataBundle bundle containing all the information to be updated
     */
    public void updateInformation(final Bundle dataBundle) {
        if (dataBundle == null) {
            Log.d(TAG, "Updating information for slot " + slotNumber
                    + " failed due to empty bundle");
            return;
        }
        executor.execute(new Runnable() {
            @Override
            public void run() {
                SlotFragment.super.updateInformation(dataBundle);
                byte[] slotData = dataBundle.getByteArray(Constants.SLOT_DATA);
                if (slotData != null) {
                    SlotFragment.this.slotData = slotData;
                    if (Utils.slotIsEmpty(slotData)) {
                        currFrameType = Constants.EMPTY_FRAME_TYPE;
                    } else {
                        currFrameType = slotData[0];
                        switch (currFrameType) {
                            case Constants.UID_FRAME_TYPE:
                                name = Constants.UID;
                                namespace = SlotDataManager.getNamespaceFromSlotData(slotData);
                                instance = SlotDataManager.getInstanceFromSlotData(slotData);
                                break;
                            case Constants.URL_FRAME_TYPE:
                                name = Constants.URL;

                                url = SlotDataManager.getUrlFromSlotData(slotData);
                                break;
                            case Constants.TLM_FRAME_TYPE:
                                name = Constants.TLM;
                                voltage = Short.toString(SlotDataManager
                                        .getVoltageFromSlotData(slotData));

                                temperature = Float.toString(SlotDataManager
                                        .getTemperatureFromSlotData(slotData));

                                advCnt = Integer.toString(SlotDataManager
                                        .getAdvertisingPDUCountFromSlotData(slotData));

                                secCnt = Utils.getTimeString(SlotDataManager
                                        .getTimeSinceOnFromSlotData(slotData));
                                break;
                            case Constants.EID_FRAME_TYPE:
                                name = Constants.EID;

                                ephemeralId = SlotDataManager.getEphemeralIdFromSlotData(slotData);
                                break;
                            default:
                                name = "--";
                        }
                    }
                }
            }
        });
    }

    /**
     * Sets up the fragment according to the frame type this slot is configured to broadcast
     *
     * @param v container for the tab. It has to be added to the activity before calling this method
     */
    protected void setUpFragment(final View v) {
        if (getActivity() != null) {
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    hideAllViews(v);

                    switch (currFrameType) {
                        case Constants.UID_FRAME_TYPE:
                            setUpFragmentAsUidSlot(v);
                            break;
                        case Constants.URL_FRAME_TYPE:
                            setUpFragmentAsUrlSlot(v);
                            break;
                        case Constants.TLM_FRAME_TYPE:
                            setUpFragmentAsTlmSlot(v);
                            break;
                        case Constants.EID_FRAME_TYPE:
                            setUpFragmentAsEidSlot(v);
                            break;
                        default:
                            ((Spinner) v.findViewById(R.id.frame_type_spinner)).setSelection(4);
                            name = "--";
                            return;
                    }

                    SlotFragment.super.setUpFragment(v);
                }
            });
        }
    }

    @Override
    public boolean changesPending() {
        return  super.changesPending() || slotDataChanged;
    }

    /**
     * @return true whenever the frame this slot is configured to is empty
     */
    public boolean isEmpty() {
        return currFrameType == Constants.EMPTY_FRAME_TYPE;
    }

    private void hideAllViews(final View v) {
        v.findViewById(R.id.uid_frame_slot).setVisibility(View.GONE);
        v.findViewById(R.id.url_frame_slot).setVisibility(View.GONE);
        v.findViewById(R.id.tlm_frame_slot).setVisibility(View.GONE);
        v.findViewById(R.id.eid_frame_slot).setVisibility(View.GONE);
        v.findViewById(R.id.tx_power_info).setVisibility(View.GONE);
        v.findViewById(R.id.adv_int_info).setVisibility(View.GONE);
    }

    /**
     * Called when a new entry in the frame type spinner is selected. This will trigger reloading
     * of the fragment as new fields will need to be shown.
     */
    private boolean firstTime = true;
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
        String newFrameSelected = (String) parent.getItemAtPosition(pos);
        currFrameType = Utils.getFrameTypeFromString(newFrameSelected);
        if (firstTime) {
            firstTime = false;
        } else {
            slotDataChanged = true;
        }
        setUpFragment(getView());
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {
    }

    private void setUpFragmentAsEidSlot(View v) {
        Log.d(TAG, "Setting tab " + slotNumber + " as EID slot");
        ((Spinner) v.findViewById(R.id.frame_type_spinner)).setSelection(3);
        v.findViewById(R.id.eid_frame_slot).setVisibility(View.VISIBLE);

        ((TextView) v.findViewById(R.id.ephemeral_id)).setText(ephemeralId);
    }

    private void setUpFragmentAsTlmSlot(View v) {
        Log.d(TAG, "Setting tab " + slotNumber + " as TLM slot");
        ((Spinner) v.findViewById(R.id.frame_type_spinner)).setSelection(2);
        v.findViewById(R.id.tlm_frame_slot).setVisibility(View.VISIBLE);

        ((TextView) v.findViewById(R.id.voltage)).setText(voltage);
        ((TextView) v.findViewById(R.id.temperature)).setText(temperature);
        ((TextView) v.findViewById(R.id.adv_pdu_count)).setText(advCnt);
        ((TextView) v.findViewById(R.id.time_since_alive)).setText(secCnt);
    }

    private void setUpFragmentAsUrlSlot(View v) {
        Log.d(TAG, "Setting tab " + slotNumber + " as URL slot");
        ((Spinner) v.findViewById(R.id.frame_type_spinner)).setSelection(1);
        final ViewGroup urlSlot = (ViewGroup) v.findViewById(R.id.url_frame_slot);
        urlSlot.setVisibility(View.VISIBLE);

        final TextView urlView = (TextView) urlSlot.findViewById(R.id.url);
        urlView.setText(url);


        urlSlot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                UrlChangeDialog.show(getContext(), new UrlChangeDialog.UrlChangeListener() {
                    @Override
                    public void setNewUrl(String newUrl) {
                        urlView.setText(newUrl);
                        url = newUrl;
                        slotDataChanged = true;
                    }
                });
            }
        });
    }

    private void setUpFragmentAsUidSlot(View v) {
        Log.d(TAG, "Setting tab " + slotNumber + " as UID slot");
        ((Spinner) v.findViewById(R.id.frame_type_spinner)).setSelection(0);
        ViewGroup uidFrameSlot = (ViewGroup) v.findViewById(R.id.uid_frame_slot);
        uidFrameSlot.setVisibility(View.VISIBLE);

        final TextView namespaceView = (TextView) v.findViewById(R.id.namespace);
        namespaceView.setText(namespace);

        final TextView instanceView = (TextView) v.findViewById(R.id.instance);
        instanceView.setText(instance);

        uidFrameSlot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                slotDataChanged = true;
                UidSlotDataChangeDialog.show(namespace, instance, getContext(),
                        new UidSlotDataChangeDialog.UidChangeListener() {
                    @Override
                    public void setNewUid(String namespace, String instance) {
                        SlotFragment.this.namespace = namespace;
                        SlotFragment.this.instance = instance;
                        namespaceView.setText(namespace);
                        instanceView.setText(instance);
                    }
                });
            }
        });


    }

    @Override
    public void saveChanges() {
        super.saveChanges();

        if (slotDataChanged) {
            slotData = buildNewSlotDataInfo();
            configurationListener.slotDataChanged(slotNumber, slotData);
            slotDataChanged = false;
        }

        setUpFragment(getView());
    }

    /**
     * Whenever the slot data has been changed and the user wishes to save this change, a new
     * byte[] will have to be written to the beacon. This method uses the SlotDataManager to
     * build the byte[] according to the current frame the spinner is set to.
     *
     * @return byte[] to be written to the beacon to configure new slot data information
     */
    public byte[] buildNewSlotDataInfo() {
        byte[] newSlotData;
        switch(currFrameType) {
            case Constants.UID_FRAME_TYPE:
                String namespace
                        = ((TextView) getView().findViewById(R.id.namespace)).getText().toString();
                String instance
                        = ((TextView) getView().findViewById(R.id.instance)).getText().toString();
                newSlotData = SlotDataManager.buildNewUidSlotData(namespace, instance);
                break;
            case Constants.URL_FRAME_TYPE:
                String newUrl = ((TextView) getView().findViewById(R.id.url)).getText().toString();
                newSlotData = SlotDataManager.buildNewUrlSlotData(newUrl);
                break;
            case Constants.TLM_FRAME_TYPE:
                newSlotData = SlotDataManager.buildNewTlmSlotData();
                break;
            case Constants.EID_FRAME_TYPE:
                newSlotData = SlotDataManager.buildNewEidSlotData();
                break;
            default:
                //empty slot
                newSlotData = new byte[0];
        }
        return newSlotData;
    }
}
