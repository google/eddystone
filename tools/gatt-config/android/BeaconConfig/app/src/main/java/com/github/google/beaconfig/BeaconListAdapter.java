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

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.github.google.beaconfig.utils.SlotDataManager;
import com.github.google.beaconfig.utils.Utils;

import java.util.List;

/**
 * Adapter for the recyclerView displaying all the results from scanning for beacons nearby in
 * ScanningActivity. Each list entry is a CardView whose structure changes depending on the
 * information available from the scan result. It is inflated for every frame which the beacon
 * is broadcasting with 1 row which displays this information.
 */

public class BeaconListAdapter extends RecyclerView.Adapter<BeaconListAdapter.ViewHolder> {
    private List<BeaconScanData> scanDataList;
    private Context context;

    public BeaconListAdapter(List<BeaconScanData> scanDataList, Context context) {
        this.scanDataList = scanDataList;
        this.context = context;
    }

    @Override
    public BeaconListAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.scan_result_layout, parent, false);
        ViewHolder vh = new ViewHolder(v);
        return vh;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        BeaconScanData b = scanDataList.get(position);
        holder.name.setText(b.name);
        holder.address.setText(b.deviceAddress);
        holder.distance.setText(Integer.toString(b.rssi));

        holder.bottom.removeAllViews();
        boolean uidPresent = false;
        for (byte[] uidFrameType : b.uidFrameTypes) {
             uidPresent = setupUidFrameView(uidFrameType, holder);
        }
        boolean urlPresent = false;
        for (byte[] urlFrameType : b.urlFrameTypes) {
             urlPresent = setupUrlFrameView(urlFrameType, holder);
        }
        boolean tlmPresent = setupTlmFrameView(b.tlmFrameType, holder);
        boolean eidPresent = setupEidFrameView(b.eidFrameType, holder);

        //This part is making sure that "Device is configurable" is displayed only when there is
        // no frame data available to display. The card looks very empty is there wasn't a
        // message like that.
        if (b.connectable) {
            ((GradientDrawable) holder.connectable.getBackground()).setColor(
                    context.getResources().getColor(R.color.colorPrimary));
            if (!(uidPresent || urlPresent || tlmPresent || eidPresent)) {
                holder.bottom.addView(holder.connectableRow);
            }
        } else {
            ((GradientDrawable) holder.connectable.getBackground()).setColor(
                    context.getResources().getColor(R.color.red));
        }
    }

    private boolean setupUidFrameView(byte[] data, ViewHolder holder) {
        if (data != null) {
            LayoutInflater inflater = holder.inflater;
            LinearLayout row = (LinearLayout) inflater.inflate(R.layout.frame_row_uid, null);
            TextView namespaceView = (TextView) row.findViewById(R.id.namespace);
            namespaceView.setText(SlotDataManager.getNamespaceFromSlotData(data));
            TextView instanceView = (TextView) row.findViewById(R.id.instance);
            instanceView.setText(SlotDataManager.getInstanceFromSlotData(data));
            holder.bottom.addView(row);
            return true;
        }
        return false;
    }

    private boolean setupUrlFrameView(byte[] data, ViewHolder holder) {
        if (data != null) {
            LayoutInflater inflater = holder.inflater;
            LinearLayout row = (LinearLayout) inflater.inflate(R.layout.frame_row_url, null);
            TextView serviceDataView = (TextView) row.findViewById(R.id.url);
            serviceDataView.setText(SlotDataManager.getUrlFromSlotData(data));
            holder.bottom.addView(row);
            return true;
        }
        return false;
    }

    private boolean setupTlmFrameView(byte[] data, ViewHolder holder) {
        if (data != null) {
            String voltage = Short.toString(SlotDataManager.getVersionFromSlotData(data));

            String temperature = Float.toString(SlotDataManager.getTemperatureFromSlotData(data));

            String advCnt
                    = Integer.toString(SlotDataManager.getAdvertisingPDUCountFromSlotData(data));

            String timeOn = Utils.getTimeString(SlotDataManager.getTimeSinceOnFromSlotData(data));

            LayoutInflater inflater = holder.inflater;
            LinearLayout row = (LinearLayout) inflater.inflate(R.layout.frame_row_tlm, null);
            TextView voltageView = (TextView) row.findViewById(R.id.voltage);
            voltageView.setText(voltage);

            TextView temperatureView = (TextView) row.findViewById(R.id.temperature);
            temperatureView.setText(temperature);
            TextView pduView = (TextView) row.findViewById(R.id.pdu_cnt);
            pduView.setText(advCnt);

            TextView timeOnView = (TextView) row.findViewById(R.id.time_on);
            timeOnView.setText(timeOn);
            holder.bottom.addView(row);
            return true;
        }
        return false;
    }

    private boolean setupEidFrameView(byte[] data, ViewHolder holder) {
        if (data != null) {
            LayoutInflater inflater = holder.inflater;
            LinearLayout row = (LinearLayout) inflater.inflate(R.layout.frame_type_row, null);
            ((TextView) row.findViewById(R.id.frame_type)).setText(Constants.EID);
            TextView serviceDataView = (TextView) row.findViewById(R.id.service_data);
            serviceDataView.setText(SlotDataManager.getEphemeralIdFromSlotData(data));
            holder.bottom.addView(row);
            return true;
        }
        return false;
    }

    @Override
    public int getItemCount() {
        return scanDataList.size();
    }

    public void setData(List<BeaconScanData> scanData) {
        this.scanDataList = scanData;
        notifyDataSetChanged();
    }

    /**
     *  View holder for each displaying card in the recycler view.
     */
    public static class ViewHolder extends RecyclerView.ViewHolder {
        public static final String BEACON_ADDRESS = "com.bluetooth.beaconconfig.BEACON_ADDRESS";
        public static final String BEACON_NAME = "com.bluetooth.beaconconfig.BEACON_NAME";

        private CardView cardView;
        LinearLayout bottom;

        private TextView name;
        private TextView address;
        private TextView distance;

        LayoutInflater inflater;

        View connectable;

        private LinearLayout connectableRow;

        public ViewHolder(View itemView) {
            super(itemView);
            this.cardView = (CardView) itemView.findViewById(R.id.card_view);
            cardView.setUseCompatPadding(true);

            cardView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent configBeacon =
                            new Intent(cardView.getContext(), BeaconConfigActivity.class);
                    configBeacon.putExtra(BEACON_ADDRESS, address.getText());
                    configBeacon.putExtra(BEACON_NAME, name.getText());
                    cardView.getContext().startActivity(configBeacon);
                }
            });

            name = (TextView) cardView.findViewById(R.id.name);
            address = (TextView) cardView.findViewById(R.id.address);
            distance = (TextView) cardView.findViewById(R.id.rssi);

            bottom = (LinearLayout) itemView.findViewById(R.id.bottom);
            inflater = (LayoutInflater) cardView.getContext()
                                                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);

            connectable = cardView.findViewById(R.id.connectable);
            connectableRow = (LinearLayout) inflater.inflate(R.layout.frame_type_row, null);
            connectableRow.removeAllViews();
            TextView textView = new TextView(connectableRow.getContext());
            textView.setText("Device is configurable.");
            textView.setTextColor(Color.GRAY);
            connectableRow.addView(textView);
        }

    }
}
