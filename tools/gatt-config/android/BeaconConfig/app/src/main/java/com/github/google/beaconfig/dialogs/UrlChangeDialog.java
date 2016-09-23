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

import android.app.AlertDialog;
import android.content.Context;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;

import com.github.google.beaconfig.R;

/**
 * Dialog used to change url in a url frame. It detects when the input is wrongly formulated and
 * does not allow wrong urls to be sent to the beacon.
 */
public class UrlChangeDialog {
    public static void show(final Context ctx, final UrlChangeListener urlChangeListener) {
        final AlertDialog editUrlDialog = new AlertDialog.Builder(ctx).show();
        editUrlDialog.setContentView(R.layout.dialog_change_url);
        editUrlDialog.setCanceledOnTouchOutside(false);

        // This is needed because there are some flags being set which prevent the keyboard from
        // popping up when an EditText is clicked
        editUrlDialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

        final EditText editUrlView
                = (EditText) editUrlDialog.findViewById(R.id.edit_url);

        ArrayAdapter<CharSequence> spinnerAdapter = ArrayAdapter.createFromResource(ctx,
                R.array.url_schemes, R.layout.spinner_item);
        spinnerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        final Spinner urlSpinner = (Spinner) editUrlDialog.findViewById(R.id.url_spinner);
        urlSpinner.setAdapter(spinnerAdapter);

        editUrlDialog.findViewById(R.id.confirm_button).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        String newUrl = urlSpinner.getSelectedItem()
                                + editUrlView.getText().toString();
                        urlChangeListener.setNewUrl(newUrl);
                        editUrlDialog.dismiss();
                    }
                });

        editUrlDialog.findViewById(R.id.cancel_change_url).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        editUrlDialog.dismiss();
                    }
                });
    }

    /**
     * Listener interface to be called when password has been types in.
     */
    public interface UrlChangeListener {
        void setNewUrl(String newUrl);
    }
}
