/*
* 2013 Android Code Kitchen reference solution
* Seth Williams
* swilliams@nvidia.com
*/

package com.regal.dreamtri;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.WindowManager;

public class DreamtriActivity extends Activity {

    private static final String TAG = "DreamtriActivity";
    private static final int LEFT = 0;
    private static final int RIGHT = 1;

    DreamtriView mView;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new DreamtriView(getApplication());
	setContentView(mView);
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        switch (event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                final int keyCode = event.getKeyCode();
                switch (keyCode) {
                    case KeyEvent.KEYCODE_BUTTON_L1:
                    case KeyEvent.KEYCODE_DPAD_LEFT:
                    case KeyEvent.KEYCODE_BUTTON_THUMBL:
                        DreamtriLib.direction(LEFT);
                        return true;
                    case KeyEvent.KEYCODE_BUTTON_R1:
                    case KeyEvent.KEYCODE_DPAD_RIGHT:
                    case KeyEvent.KEYCODE_BUTTON_THUMBR:
                        DreamtriLib.direction(RIGHT);
                        return true;
                    default:
                        break;
                }
                break;
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent event) {
        // Check that the event came from a joystick
        if (((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0)
                && (event.getAction() == MotionEvent.ACTION_MOVE)) {

                float las_l2r = event.getAxisValue(MotionEvent.AXIS_X);
                float ras_l2r = event.getAxisValue(MotionEvent.AXIS_Z);
                float dpad_l2r = event.getAxisValue(MotionEvent.AXIS_HAT_X);
                float ltrigger = event.getAxisValue(MotionEvent.AXIS_LTRIGGER);
                float rtrigger = event.getAxisValue(MotionEvent.AXIS_RTRIGGER);

                if ((las_l2r == -1.0) || (ras_l2r == -1.0) || (dpad_l2r == -1.0) || (ltrigger == 1.0))
                    DreamtriLib.direction(LEFT);

                if ((las_l2r == 1.0) || (ras_l2r == 1.0) || (dpad_l2r == 1.0) || (rtrigger == 1.0))
                    DreamtriLib.direction(RIGHT);

                // return true here so that additional actions don't happen for the input (e.g. right joystick pointer mode)
                return true;
        }
        return super.dispatchGenericMotionEvent(event);
    }
}
