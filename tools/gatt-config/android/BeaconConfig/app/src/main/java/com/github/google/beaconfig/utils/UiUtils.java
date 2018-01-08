package com.github.google.beaconfig.utils;

import android.app.Activity;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

/**
 * Static methods helpful throughout the program.
 */
public class UiUtils {

    public static void showToast(final Activity activity, final String message) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast toast = Toast.makeText(activity.getApplicationContext(),
                        message, Toast.LENGTH_SHORT);
                toast.setGravity(Gravity.CENTER_VERTICAL | Gravity.BOTTOM, 0, 80);
                toast.show();
            }
        });
    }

    public static void makeChildrenInvisible(ViewGroup view) {
        for (int i = 0; i < view.getChildCount(); i++) {
            view.getChildAt(i).setVisibility(View.GONE);
        }
    }

    public static void enableAllChildren(View view) {
        if (view instanceof ViewGroup) {
            ViewGroup viewGroup = (ViewGroup) view;
            for (int i = 0; i < viewGroup.getChildCount(); i++) {
                View child = viewGroup.getChildAt(i);
                child.setEnabled(true);
                enableAllChildren(child);
            }
        }
    }

    public static void disableAllChildren(View view) {
        if (view instanceof ViewGroup) {
            ViewGroup viewGroup = (ViewGroup) view;
            for (int i = 0; i < viewGroup.getChildCount(); i++) {
                View child = viewGroup.getChildAt(i);
                child.setEnabled(false);
                disableAllChildren(child);
            }
        }
    }
}
