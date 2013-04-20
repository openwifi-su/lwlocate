package com.vwp.owmini;

import android.os.*;
import android.preference.*;
 
public class OWLMapPrefs extends PreferenceActivity {
 
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.prefs);
    }
 
}

