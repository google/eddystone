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

package com.github.google.beaconfig.dialogs;

import android.app.Activity;
import android.app.AlertDialog;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

import com.github.google.beaconfig.R;
import com.github.google.beaconfig.SavedConfigurationsManager;

/**
 * This dialog pops up when the user wants to save the current configuration of the beacon. It asks
 * the user for a name which will uniquely identify this configuration. It does not allow the user
 * to choose a name which laready exists.
 */
public class SaveConfigurationDialog {

    public static void show(final Activity ctx, final SaveConfigListener saveConfigListener) {
        final AlertDialog saveConfigDialog = new AlertDialog.Builder(ctx).show();
        saveConfigDialog.setContentView(R.layout.dialog_save_config);
        saveConfigDialog.setCanceledOnTouchOutside(false);

        // This is needed because there are some flags being set which prevent the keyboard from
        // popping up when an EditText is clicked
        saveConfigDialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

        final EditText configNameView
                = (EditText) saveConfigDialog.findViewById(R.id.configuration_name);

        saveConfigDialog.findViewById(R.id.confirm_button).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        String configName = configNameView.getText().toString();
                        if (configName == null || configName.isEmpty()) {
                            configNameView.setError("Please enter a name");
                        } else if (new SavedConfigurationsManager(ctx)
                                .getConfigurationNamesList().contains(configName)) {
                            configNameView.setError("This configuration is already in use");
                        } else {
                            saveConfigListener.configNameChosen(configName);
                            saveConfigDialog.dismiss();
                        }
                    }
                });

        saveConfigDialog.findViewById(R.id.cancel_save_config).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        saveConfigDialog.dismiss();
                    }
                });
    }

    /**
     * Listener interface to be called when a name has been chosen for this configuration
     */
    public interface SaveConfigListener {
        void configNameChosen(String configName);
    }
}
