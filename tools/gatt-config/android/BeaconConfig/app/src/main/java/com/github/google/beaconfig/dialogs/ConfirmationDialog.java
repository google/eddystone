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
import android.content.DialogInterface;

/**
 * This dialog is displayed whenever there is a risky request from the user. It asks for
 * confirmation before executing the request.
 */
public class ConfirmationDialog {
    public static void confirm(final String title, final String message, final String confirmText,
                               final String cancelText, final Activity ctx,
                               final ConfirmationListener listener) {
        ctx.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final AlertDialog confirmDialog = new AlertDialog.Builder(ctx)
                    .setTitle(title)
                    .setMessage(message)
                    .setPositiveButton(confirmText, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            listener.confirm();
                            dialog.dismiss();
                        }
                    })
                    .setNegativeButton(cancelText, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            listener.cancel();
                            dialog.dismiss();
                        }
                    })
                    .show();

                confirmDialog.setCanceledOnTouchOutside(false);

            }
        });
    }

    /**
     * Used for communication between the activity calling ConfirmationDialog
     * and ConfirmationDialog.
     */
    public interface ConfirmationListener {
        void confirm();
        void cancel();
    }
}