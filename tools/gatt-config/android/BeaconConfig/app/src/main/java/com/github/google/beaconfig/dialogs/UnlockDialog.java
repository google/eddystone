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
import android.app.Dialog;
import android.content.DialogInterface;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.TextView;

import com.github.google.beaconfig.R;
import com.github.google.beaconfig.utils.Utils;

/**
 *  This Dialog pops up when a password is required by the user to unlock the beacon manually.
 */
public class UnlockDialog {
    public static void show(final Activity ctx, final UnlockListener unlockListener) {
        ctx.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final AlertDialog unlockDialog = new AlertDialog.Builder(ctx).show();
                unlockDialog.setContentView(R.layout.dialog_unlock);
                unlockDialog.setCanceledOnTouchOutside(false);

                // This is needed because there are some flags being set which prevent the keyboard
                // from popping up when an EditText is clicked
                unlockDialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

                final EditText unlockCodeView
                        = (EditText) unlockDialog.findViewById(R.id.enter_lock_code);
                final TextView newPasswordLengthTracker
                        = (TextView) unlockDialog.findViewById(R.id.lock_code_tracker);

                unlockCodeView.addTextChangedListener(new TextWatcher() {
                    @Override
                    public void beforeTextChanged(CharSequence charSequence,
                                                  int i, int i1, int i2) {

                    }

                    @Override
                    public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

                    }

                    @Override
                    public void afterTextChanged(Editable editable) {
                        newPasswordLengthTracker.setText("("
                                + unlockCodeView.getText().length() + "/32)");
                    }
                });

                unlockCodeView.setOnEditorActionListener(new EditText.OnEditorActionListener() {
                    @Override
                    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                        if (actionId == EditorInfo.IME_ACTION_DONE) {
                            if (unlockCodeView.getText().toString().length() < 32) {
                                unlockCodeView.setError("Lock code must be 32 hex characters.");
                                return true;
                            } else {
                                return false;
                            }
                        }
                        return false;
                    }
                });

                unlockCodeView.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View view, boolean hasFocus) {
                        if (!hasFocus) {
                            String input = unlockCodeView.getText().toString();
                            if (input.length() < 32 && input.length() > 0) {
                                unlockCodeView.setError("Lock code too short!");
                            }
                        }
                    }
                });

                unlockDialog.findViewById(R.id.confirm_button).setOnClickListener(
                        new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                final String newPassword = unlockCodeView.getText().toString();

                                if (newPassword.length() < 32) {
                                    unlockCodeView.setError(
                                            "Please enter a 32 character lock code");
                                    return;
                                }
                                unlockListener.unlock(Utils.toByteArray
                                        (unlockCodeView.getText().toString()));
                                unlockDialog.dismiss();
                            }
                        });

                unlockDialog.findViewById(R.id.exit).setOnClickListener(
                        new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                unlockListener.unlockingDismissed();
                                unlockDialog.dismiss();
                            }
                        });

                unlockDialog.setOnKeyListener(new Dialog.OnKeyListener() {

                    @Override
                    public boolean onKey(DialogInterface arg0, int keyCode,
                                         KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            unlockListener.unlockingDismissed();
                            unlockDialog.dismiss();
                        }
                        return true;
                    }
                });
            }
        });
    }

    /**
     * Listener interface to be called when password has been types in.
     */
    public interface UnlockListener {
        void unlockingDismissed();
        void unlock(byte[] unlockCode);
    }
}