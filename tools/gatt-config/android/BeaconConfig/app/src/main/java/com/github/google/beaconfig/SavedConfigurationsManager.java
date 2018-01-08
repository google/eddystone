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

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import com.google.gson.Gson;

import java.util.ArrayList;

/**
 * This class is used from the BeaconConfig activity for saving the beacon's current configuration.
 * It uses SharedPreferences to save or recover beacon configurations.
 *
 * It accepts a BeaconConfiguration object to save a configuration and serializes it to a String
 * which it puts in the SharedPreferences.
 *
 * It returns a BeaconConfiguration object when a configuration with a certain name is requested.
 */
public class SavedConfigurationsManager {
    private static final String TAG = SavedConfigurationsManager.class.getSimpleName();
    private final Activity activity;
    private final Gson gson;

    public SavedConfigurationsManager(Activity activity) {
        this.activity = activity;
        this.gson = new Gson();
    }

    /**
     * This function must be called before any other call to this class. It initialises the list of
     * configuration names
     */
    public void initialiseConfigurationSaving() {
        ArrayList<String> configurationNames = new ArrayList<>();
        SharedPreferences.Editor spe = activity.getSharedPreferences(Constants.SAVED_CONFIGURATIONS,
                Context.MODE_PRIVATE).edit();
        spe.putString(Constants.CONFIG_NAMES, gson.toJson(configurationNames));
        spe.apply();
    }

    /**
     * This method serializes the BeaconConfiguration object to a String which it puts in a shared
     * preferences file with name "SAVED_CONFIGURATIONS". The new configuration is saved with key
     * the name of the configuration (configuration.getName()).
     *
     * @param configuration the configuration which we want to save
     */
    public void saveNewConfiguration(BeaconConfiguration configuration) {
        SharedPreferences.Editor spEditor = activity.getSharedPreferences(
                Constants.SAVED_CONFIGURATIONS, Context.MODE_PRIVATE).edit();

        String serializedConfiguration = gson.toJson(configuration);
        spEditor.putString(configuration.getName(), serializedConfiguration);
        addNewName(configuration.getName());
        spEditor.apply();
    }

    /**
     * At the beginning of the application an array was put in the "SAVED_CONFIGURATIONS" shared
     * preferences file which is a list of all saved configurations' names. This method adds a new
     * name to this list.
     *
     * This method should only be called when a new configuration is saved to the shared preferences
     * file and the configNameView is the key with which the configuration was saved in the shared
     * preferences map.
     *
     * @param configName
     */
    private void addNewName(String configName) {
        ArrayList<String> spConfigNames = getConfigurationNamesList();
        spConfigNames.add(configName);

        SharedPreferences.Editor spe = activity.getSharedPreferences(Constants.SAVED_CONFIGURATIONS,
                Context.MODE_PRIVATE).edit();
        spe.remove(Constants.CONFIG_NAMES);
        spe.putString(Constants.CONFIG_NAMES, gson.toJson(spConfigNames));
        spe.apply();
    }

    /**
     * Retrieves a configuration from the "SAVED_CONFIGURATIONS" shared preferences file with name
     * configNameView. This configuration must have previously been saved to the shared preferences
     * file
     *
     * @param configName the name of the configuration we want to retrieve
     * @return the saved configuration with this name or null if no such configuration was found
     */
    public BeaconConfiguration getConfiguration(String configName) {
        SharedPreferences sp = activity.getSharedPreferences(
                Constants.SAVED_CONFIGURATIONS, Context.MODE_PRIVATE);
        String serializedConfiguration = sp.getString(configName, "");
        BeaconConfiguration configuration
                = gson.fromJson(serializedConfiguration, BeaconConfiguration.class);

        if (configuration == null) {
            Log.d(TAG, "Configuration \"" + configName + "\" not found");
            return null;
        }
        Log.d(TAG, "Retrieved configuration " + configuration.getName());
        return configuration;
    }

    /**
     * This looks up the configuration name which stays in position "position" in the list of
     * configuration names (which is saved in the "SAVED_CONFIGURATIONS" shared preferences file).
     * Then uses this name to retrieve the relevant configuration from this file.
     *
     * @param position position of the configuration in the array of configuration names
     * @return the configuration at position "position" or null if no such configuration was found
     */
    public BeaconConfiguration getConfigurationAtPosition(int position) {
        ArrayList<String> spConfigNames = getConfigurationNamesList();
        String configName = spConfigNames.get(position);
        return getConfiguration(configName);
    }

    public void deleteConfigurationWithName(String name) {
        ArrayList<String> spConfigNames = getConfigurationNamesList();
        spConfigNames.remove(name);
        SharedPreferences.Editor spEditor = activity.getSharedPreferences(
                Constants.SAVED_CONFIGURATIONS, Context.MODE_PRIVATE).edit();
        spEditor.remove(name);
        spEditor.remove(Constants.CONFIG_NAMES);
        spEditor.putString(Constants.CONFIG_NAMES, gson.toJson(spConfigNames));
        spEditor.commit();
    }

    /**
     * @return the list of all names of configurations saved to the "SAVED_CONFIGURATIONS" shared
     * preferences file
     */
    public ArrayList<String> getConfigurationNamesList() {
        SharedPreferences sp = activity.getSharedPreferences(Constants.SAVED_CONFIGURATIONS,
                Context.MODE_PRIVATE);
        String serializedConfigNames = sp.getString(Constants.CONFIG_NAMES, null);
        final ArrayList<String> configNames
                = gson.fromJson(serializedConfigNames, ArrayList.class);
        return configNames;
    }

    public int getNumberOfConfigurations() {
        SharedPreferences sp = activity.getSharedPreferences(
                Constants.SAVED_CONFIGURATIONS, Context.MODE_PRIVATE);
        String serializedConfigNames = sp.getString(Constants.CONFIG_NAMES, null);
        ArrayList<String> spConfigNames
                = gson.fromJson(serializedConfigNames, ArrayList.class);
        return spConfigNames.size();
    }
}
