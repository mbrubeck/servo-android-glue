/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.servo;

import android.app.NativeActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class ServoActivity extends NativeActivity {
    private static final String LOGTAG = "ServoActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        final Intent intent = getIntent();
        if (intent.getAction().equals(Intent.ACTION_VIEW)) {
            final String url = intent.getDataString();
            Log.d(LOGTAG, "Received url "+url);
        }
        super.onCreate(savedInstanceState);
    }
}
