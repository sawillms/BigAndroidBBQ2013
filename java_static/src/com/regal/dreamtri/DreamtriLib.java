/*
* 2013 Android Code Kitchen reference solution
* Seth Williams
* swilliams@nvidia.com
*/

package com.regal.dreamtri;

import android.util.Log;

// Wrapper for native library

public class DreamtriLib {
     private static final String TAG = "Dreamtri";

     static {
         Log.v(TAG, "Loading native DreamTri library...");
         System.loadLibrary("dreamtri");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void init(int width, int height);
     public static native void step();
     public static native void direction(int direction);
}
