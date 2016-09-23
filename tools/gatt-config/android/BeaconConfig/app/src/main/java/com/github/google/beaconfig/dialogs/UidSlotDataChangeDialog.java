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
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.TextView;

import com.github.google.beaconfig.R;

/**
 * Dialog which pops up when the user wants to change either the namespace or the instance of the
 * Uid slot. It verifies the input and doesn't allow invalid information to be sent to the beacon.
 */
public class UidSlotDataChangeDialog {
    public static void show(String oldNamespace, String oldInstance,
                            final Context ctx, final UidChangeListener uidChangeListener) {
        final AlertDialog editUidDialog = new AlertDialog.Builder(ctx).show();
        editUidDialog.setContentView(R.layout.dialog_change_uid);
        editUidDialog.setCanceledOnTouchOutside(false);

        // This is needed because there are some flags being set which prevent the keyboard from
        // popping up when an EditText is clicked
        editUidDialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

        final EditText editNamespaceView
                = (EditText) editUidDialog.findViewById(R.id.edit_namespace);
        final TextView namespaceTracker
                = (TextView) editUidDialog.findViewById(R.id.namespace_tracker);
        editNamespaceView.setText(oldNamespace);
        editNamespaceView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                namespaceTracker.setText("(" + editNamespaceView.getText().length() + "/20)");
            }
        });

        final EditText editInstanceView
                = (EditText) editUidDialog.findViewById(R.id.edit_instance);
        editInstanceView.setText(oldInstance);
        final TextView instanceTracker
                = (TextView) editUidDialog.findViewById(R.id.instance_tracker);

        editInstanceView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                instanceTracker.setText("(" + editInstanceView.getText().length() + "/12)");
            }
        });

        editUidDialog.findViewById(R.id.confirm_button).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        final String newNamespace = editNamespaceView.getText().toString();
                        final String newInstance = editInstanceView.getText().toString();

                        if (newNamespace.length() < 20) {
                            editNamespaceView.setError("Must be exactly 20 hex characters");
                        }

                        if (newInstance.length() < 12) {
                            editInstanceView.setError("Must be exactly 12 hex characters");
                        }

                        if (newNamespace.length() == 20 && newInstance.length() == 12) {
                            uidChangeListener.setNewUid(newNamespace, newInstance);
                            editUidDialog.dismiss();
                        }
                    }
                });

        editUidDialog.findViewById(R.id.cancel_change_uid).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        editUidDialog.dismiss();
                    }
                });
    }

    /**
     * Listener interface to be called when password has been types in.
     */
    public interface UidChangeListener {
        void setNewUid(String namespace, String instance);
    }
}
