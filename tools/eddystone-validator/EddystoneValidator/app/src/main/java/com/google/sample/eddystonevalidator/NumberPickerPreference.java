// Copyright 2015 Google Inc. All rights reserved.
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

package com.google.sample.eddystonevalidator;

import android.content.Context;
import android.content.res.TypedArray;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.NumberPicker;

public class NumberPickerPreference  extends DialogPreference {
  private static final int DEFAULT_VALUE = 5;

  private final int min;
  private final int max;

  private Integer tunedDefault;
  private int initialValue;
  private NumberPicker numberPicker;

  public NumberPickerPreference(Context context, AttributeSet attrs) {
    super(context, attrs);

    TypedArray numberPickerType = context.obtainStyledAttributes(attrs,
        R.styleable.NumberPickerPreference, 0, 0);
    min = numberPickerType.getInt(R.styleable.NumberPickerPreference_min, 0);
    max = numberPickerType.getInt(R.styleable.NumberPickerPreference_max, 10);
    numberPickerType.recycle();
    setDialogLayoutResource(R.layout.numberpicker_dialog);
  }

  @Override
  protected void onSetInitialValue(boolean restorePersistedValue, Object defaultValueObject) {
    Integer defaultValue = (Integer) defaultValueObject;
    if (restorePersistedValue) {
      int defaultInt = tunedDefault != null ? tunedDefault : DEFAULT_VALUE;
      initialValue = this.getPersistedInt(defaultInt);
    } else {
      initialValue = defaultValue;
      persistInt(initialValue);
    }
  }

  @Override
  protected Object onGetDefaultValue(TypedArray a, int index) {
    tunedDefault = a.getInteger(index, DEFAULT_VALUE);
    return tunedDefault;
  }

  @Override
  public void onBindDialogView(View view) {
    numberPicker = (NumberPicker) view.findViewById(R.id.numberPicker);
    numberPicker.setMinValue(min);
    numberPicker.setMaxValue(max);
    numberPicker.setValue(initialValue);
    numberPicker.setWrapSelectorWheel(false);
    numberPicker.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);

    super.onBindDialogView(view);
  }

  @Override
  protected void onDialogClosed(boolean positiveResult) {
    if (positiveResult) {
      int value = numberPicker.getValue();
      initialValue = value;
      persistInt(value);
    }
  }
}
