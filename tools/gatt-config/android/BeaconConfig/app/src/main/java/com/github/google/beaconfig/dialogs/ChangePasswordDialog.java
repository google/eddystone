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
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.TextView;

import com.github.google.beaconfig.R;

/**
 * Dialog for when a lock code change is requested
 */
public class ChangePasswordDialog {
    public static void show(final Activity activity, final PasswordChangeListener l) {
        final AlertDialog passwordChangeDialog = new AlertDialog.Builder(activity).show();

        // This is needed because there are some flags being set which prevent the keyboard from
        // popping up when an EditText is clicked
        passwordChangeDialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

        passwordChangeDialog.setContentView(R.layout.dialog_change_lock_code);
        passwordChangeDialog.setCanceledOnTouchOutside(false);

        final EditText newPasswordView
                = (EditText) passwordChangeDialog.findViewById(R.id.enter_new_lock_code);
        final TextView newPasswordLengthTracker
                = (TextView) passwordChangeDialog.findViewById(R.id.new_code_tracker);

        newPasswordView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                newPasswordLengthTracker.setText("(" + newPasswordView.getText().length() + "/32)");
            }
        });

        newPasswordView.setOnEditorActionListener(new EditText.OnEditorActionListener() {
                    @Override
                    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                        if (actionId == EditorInfo.IME_ACTION_DONE) {
                            if (newPasswordView.getText().toString().length() < 32) {
                                newPasswordView.setError
                                        ("Lock code too short!");
                                return true;
                            } else {
                                return false;
                            }
                        }
                        return false;
                    }
                });

        newPasswordView.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean hasFocus) {
                if (!hasFocus) {
                    String input = newPasswordView.getText().toString();
                    if (input.length() < 32) {
                        newPasswordView.setError("Lock code too short!");
                    }
                }
            }
        });


        final EditText repeatPasswordView
                = (EditText) passwordChangeDialog.findViewById(R.id.repeat_new_lock_code);
        final TextView repeatLengthTracker
                = (TextView) passwordChangeDialog.findViewById(R.id.repeat_code_tracker);

        repeatPasswordView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                repeatLengthTracker.setText("(" + repeatPasswordView.getText().length() + "/32)");
            }
        });

        repeatPasswordView.setOnEditorActionListener(new EditText.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    final String input = repeatPasswordView.getText().toString();

                    if (newPasswordView.getText().toString().length() < 32) {
                        newPasswordView.setError("Lock code too short!");
                    }

                    if (input.length() < 32) {
                        repeatPasswordView.setError("Lock code too short!");
                    } else if (!input.equals(newPasswordView.getText().toString())) {
                        repeatPasswordView.setError("Lock codes do not match.");
                    } else if (input.equals(newPasswordView.getText().toString())) {
                        ConfirmationDialog.confirm("Change Lock Code", "Are you sure you want "
                                + "to change the lock code of this beacon? \n \n You will not be "
                                + "able to restore the previous lock code.", "YES", "NO", activity,
                                new ConfirmationDialog.ConfirmationListener() {
                                    @Override
                                    public void confirm() {
                                        l.passwordChangeRequest(input);
                                        passwordChangeDialog.dismiss();
                                    }

                                    @Override
                                    public void cancel() {
                                    }
                                });
                        return false;
                    }
                }
                return false;
            }
        });

        repeatPasswordView.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean hasFocus) {
                if (!hasFocus) {
                    String input = repeatPasswordView.getText().toString();
                    if (input.length() < 32) {
                        repeatPasswordView.setError("Lock code too short!");
                    } else if (!input.equals(newPasswordView.getText().toString())) {
                        repeatPasswordView.setError("Lock codes do not match.");
                    }
                }
            }
        });



//         Override the button so we can prevent closing the dialog if the input is bogus.
        passwordChangeDialog.findViewById(R.id.confirm_button).setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    final String newPassword = newPasswordView.getText().toString();
                    String repeatNewPassword = repeatPasswordView.getText().toString();

                    if (newPassword.isEmpty()) {
                        newPasswordView.setError("Lock code too short!");
                    }

                    if (repeatNewPassword.isEmpty()) {
                        repeatPasswordView.setError("Lock code too short!");
                    } else if (newPassword.equals(repeatNewPassword)) {
                        ConfirmationDialog.confirm("Change Lock Code", "Are you sure you want "
                                + "to change the lock code of this beacon? \n \n You will not be "
                                + "able to restore the previous lock code.", "OK", "CANCEL",
                                activity, new ConfirmationDialog.ConfirmationListener() {
                                    @Override
                                    public void confirm() {
                                        l.passwordChangeRequest(newPassword);
                                        passwordChangeDialog.getWindow().setSoftInputMode(
                                            WindowManager.LayoutParams.
                                                    SOFT_INPUT_STATE_ALWAYS_HIDDEN);
                                        passwordChangeDialog.dismiss();
                                    }

                                    @Override
                                    public void cancel() {
                                    }
                                });
                    } else if (!newPassword.equals(repeatNewPassword)) {
                        repeatPasswordView.setError("Lock codes do not match.");
                    }
                }
            });

        passwordChangeDialog.findViewById(R.id.cancel_change_password).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        passwordChangeDialog.dismiss();
                    }
                });
    }

    /**
     * Used for communication between the activity calling ChangePasswordListener
     * and ChangePasswordListener.
     */
    public interface PasswordChangeListener {
        void passwordChangeRequest(String newPassword);
    }
}
