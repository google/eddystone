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

import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.github.google.beaconfig.dialogs.ConfirmationDialog;
import com.github.google.beaconfig.dialogs.SaveConfigurationDialog;
import com.github.google.beaconfig.utils.SlotDataManager;

/**
 * Adapter used for the RecyclerView which presents the saved configurations that the user has saved
 * from all beacons. It is shows inside ManageConfigurationsActivity. Each list entry presents one
 * saved configuration.
 */
public class ManageConfigurationsAdapter
        extends RecyclerView.Adapter<ManageConfigurationsAdapter.ViewHolder> {

    private static final String TAG = ManageConfigurationsAdapter.class.getSimpleName();
    private final ManageConfigurationsActivity activity;
    private final SavedConfigurationsManager configurationsManager;

    public ManageConfigurationsAdapter(ManageConfigurationsActivity activity) {
        this.activity = activity;
        configurationsManager = new SavedConfigurationsManager(activity);
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.fragment_manage_configurations_item, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, final int position) {
        final BeaconConfiguration configuration
                = configurationsManager.getConfigurationAtPosition(position);
        holder.configNameView.setText(configuration.getName());

        holder.deleteView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ConfirmationDialog.confirm("Delete configuration", "Are you sure you want to "
                        + "delete configuration \"" + configuration.getName() + "\"?", "yes", "no",
                        activity, new ConfirmationDialog.ConfirmationListener() {
                    @Override
                    public void confirm() {
                        deleteConfiguration(configuration.getName());
                    }

                    @Override
                    public void cancel() {

                    }
                });
            }
        });

        // on click the card inflates to show detailed information about this configuration
        holder.cardView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                holder.clickCard();
                holder.contentView.removeAllViews();

                LayoutInflater inflater = activity.getLayoutInflater();
                for (int i = 0; i < configuration.getNumberOfConfiguredSlots(); i++) {
                    Log.d(TAG, "Printing slot " + i);
                    View child = inflater.inflate(R.layout.saved_slot_configuration, null);
                    holder.contentView.addView(child);

                    ((TextView) child.findViewById(R.id.header)).setText("Slot " + i + ": ");
                    setUpSlotDataConfigurationViews(child, configuration.getSlotDataForSlot(i));
                    ((TextView) child.findViewById(R.id.radio_tx_power))
                            .setText(Integer.toString(configuration.getRadioTxPowerForSlot(i)));
                    ((TextView) child.findViewById(R.id.adv_tx_power))
                            .setText(Integer.toString(configuration.getAdvTxPowerForSlot(i)));
                    ((TextView) child.findViewById(R.id.adv_interval))
                            .setText(Integer.toString(configuration.getAdvIntervalForSlot(i)));
                }
            }
        });

        //long click opens a window to change the name of this configuration
        holder.cardView.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View view) {
                SaveConfigurationDialog.show(activity,
                        new SaveConfigurationDialog.SaveConfigListener() {
                    @Override
                    public void configNameChosen(String configName) {
                        deleteConfiguration(configuration.getName());
                        configuration.setName(configName);
                        new SavedConfigurationsManager(activity)
                                .saveNewConfiguration(configuration);
                        notifyDataSetChanged();
                    }
                });
                return false;
            }
        });
    }

    private void setUpSlotDataConfigurationViews(View view, byte[] slotData) {
        TextView header = (TextView) view.findViewById(R.id.header);
        switch (slotData[0]) {
            case Constants.UID_FRAME_TYPE:
                header.setText(header.getText() + Constants.UID);
                ((TextView) view.findViewById(R.id.namespace))
                        .setText(SlotDataManager.getNamespaceFromWriteByteArray(slotData));
                ((TextView) view.findViewById(R.id.instance))
                        .setText(SlotDataManager.getInstanceFromWriteByteArray(slotData));
                view.findViewById(R.id.uid_config).setVisibility(View.VISIBLE);
                break;
            case Constants.URL_FRAME_TYPE:
                header.setText(header.getText() + Constants.URL);
                ((TextView) view.findViewById(R.id.url))
                        .setText(SlotDataManager.getUrlFromWriteByteArray(slotData));
                view.findViewById(R.id.url_config).setVisibility(View.VISIBLE);
                break;
            case Constants.TLM_FRAME_TYPE:
                header.setText(header.getText() + Constants.TLM);
                break;
        }
    }

    @Override
    public int getItemCount() {
        return configurationsManager.getNumberOfConfigurations();
    }

    private void deleteConfiguration(String name) {
        configurationsManager.deleteConfigurationWithName(name);
        notifyDataSetChanged();
    }

    /**
     * Holder for a single configuration inside the list of saved configurations. It shows the
     * name of the configuration and on click, shows the configuration in detail:
     * - number of configured slots
     * - frame that each slot is configured to
     * - slot data
     * - radio tx power
     * - advertised tx power
     * advertised interval
     *
     * There is also an option to delete this list entry by clicking on the deleteView.
     */
    public class ViewHolder extends RecyclerView.ViewHolder {
        public CardView cardView;
        public final TextView configNameView;
        public final ImageView deleteView;
        public final LinearLayout contentView;

        public ViewHolder(final View itemView) {
            super(itemView);
            configNameView = (TextView) itemView.findViewById(R.id.config_name);
            deleteView = (ImageView) itemView.findViewById(R.id.config_delete);
            contentView = (LinearLayout) itemView.findViewById(R.id.config_content);

            cardView = (CardView) itemView.findViewById(R.id.configuration_item_card);
            cardView.setUseCompatPadding(true);

            final ImageView expandArrow
                    = (ImageView) itemView.findViewById(R.id.expand_configuration);
            expandArrow.setOnClickListener(new View.OnClickListener() {
                private boolean expanded = false;
                @Override
                public void onClick(View view) {
                    if (expanded) {
                        itemView.findViewById(R.id.configuration_view).setVisibility(View.GONE);
                        expandArrow.setImageResource(R.drawable.expand_arrow);
                        expanded = false;
                    } else {
                        itemView.findViewById(R.id.configuration_view).setVisibility(View.VISIBLE);
                        expandArrow.setImageResource(R.drawable.close_arrow);
                        expanded = true;
                    }

                }
            });
        }

        public void clickCard() {
            cardView.findViewById(R.id.expand_configuration).performClick();
        }

    }
}
