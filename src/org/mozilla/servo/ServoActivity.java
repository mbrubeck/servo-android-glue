/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.servo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

public class ServoActivity extends Activity {
    private static final String LOGTAG = "ServoActivity";

    private static final int LOCAL_EGL_OPENGL_ES2_BIT = 4;

    private static final int[] CONFIG_SPEC_24BPP = {
        EGL10.EGL_RED_SIZE, 8,
        EGL10.EGL_GREEN_SIZE, 8,
        EGL10.EGL_BLUE_SIZE, 8,
        EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
        EGL10.EGL_RENDERABLE_TYPE, LOCAL_EGL_OPENGL_ES2_BIT,
        EGL10.EGL_NONE
    };

    static {
        System.loadLibrary("ServoAndroid");
    }

    private EGL10 mEGL;
    private EGLDisplay mEGLDisplay;
    private EGLConfig mEGLConfig;
    private EGLSurface mEGLSurfaceForCompositor;
    private SurfaceView mSurfaceView;

    public native void loadUrl(String url);
    public native void initDisplay(Object surface);

    public void initEGL() {
        mEGL = (EGL10)EGLContext.getEGL();
        mEGLDisplay = mEGL.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
        if (mEGLDisplay == EGL10.EGL_NO_DISPLAY) {
            Log.w(LOGTAG, "Can't get EGL display!");
            return;
        }

        int[] returnedVersion = new int[2];
        if (!mEGL.eglInitialize(mEGLDisplay, returnedVersion)) {
            Log.w(LOGTAG, "eglInitialize failed");
            return;
        }
        mEGLConfig = chooseConfig();
        mEGLSurfaceForCompositor = mEGL.eglCreateWindowSurface(mEGLDisplay, mEGLConfig, mSurfaceView.getHolder(), null);
        initDisplay(mEGLSurfaceForCompositor);
    }

    private EGLConfig chooseConfig() {
        int[] desiredConfig;
        int rSize, gSize, bSize;
        int[] numConfigs = new int[1];

        desiredConfig = CONFIG_SPEC_24BPP;
        rSize = gSize = bSize = 8;

        if (!mEGL.eglChooseConfig(mEGLDisplay, desiredConfig, null, 0, numConfigs) ||
                numConfigs[0] <= 0) {
            throw new RuntimeException("No available EGL configurations " + mEGL.eglGetError());
        }

        EGLConfig[] configs = new EGLConfig[numConfigs[0]];
        if (!mEGL.eglChooseConfig(mEGLDisplay, desiredConfig, configs, numConfigs[0], numConfigs)) {
            throw new RuntimeException("No EGL configuration for that specification " + mEGL.eglGetError());
        }

        // Select the first configuration that matches the screen depth.
        int[] red = new int[1], green = new int[1], blue = new int[1];
        for (EGLConfig config : configs) {
            mEGL.eglGetConfigAttrib(mEGLDisplay, config, EGL10.EGL_RED_SIZE, red);
            mEGL.eglGetConfigAttrib(mEGLDisplay, config, EGL10.EGL_GREEN_SIZE, green);
            mEGL.eglGetConfigAttrib(mEGLDisplay, config, EGL10.EGL_BLUE_SIZE, blue);
            if (red[0] == rSize && green[0] == gSize && blue[0] == bSize) {
                return config;
            }
        }

        throw new RuntimeException("No suitable EGL configuration found");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSurfaceView = new SurfaceView(this);
        setContentView(mSurfaceView);

        final Intent intent = getIntent();
        if (intent.getAction().equals(Intent.ACTION_VIEW)) {
            final String url = intent.getDataString();
            Log.d(LOGTAG, "Received url "+url);
            //loadUrl(url);
        } else {
            //loadUrl("/mnt/sdcard/html/about-mozilla.html");
        }
    }

    @Override
    protected void onStart() {
        initEGL();
        loadUrl("/mnt/sdcard/html/about-mozilla.html");
    }
}
