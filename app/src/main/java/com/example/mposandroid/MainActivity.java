package com.example.mposandroid;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity implements SurfaceHolder.Callback, View.OnTouchListener {
    static {
        System.loadLibrary("mpos_android_spike");
    }

    private final Handler handler = new Handler(Looper.getMainLooper());
    private final Runnable framePump = new Runnable() {
        @Override
        public void run() {
            nativeTick();
            handler.postDelayed(this, 16);
        }
    };

    private SurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        surfaceView = new SurfaceView(this);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setFocusable(true);
        surfaceView.setFocusableInTouchMode(true);
        surfaceView.setOnTouchListener(this);
        setContentView(surfaceView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        nativeResume();
        handler.post(framePump);
    }

    @Override
    protected void onPause() {
        handler.removeCallbacks(framePump);
        nativePause();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        nativeShutdown();
        super.onDestroy();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Surface surface = holder.getSurface();
        nativeSurfaceCreated(surface);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        nativeSurfaceChanged(holder.getSurface(), width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        nativeSurfaceDestroyed();
    }

    @Override
    public boolean onTouch(View view, MotionEvent event) {
        int action = event.getActionMasked();
        boolean pressed = action != MotionEvent.ACTION_UP
                && action != MotionEvent.ACTION_CANCEL
                && event.getPointerCount() > 0;

        float x = pressed ? event.getX(0) : 0.0f;
        float y = pressed ? event.getY(0) : 0.0f;
        nativeTouch(x, y, pressed);
        return true;
    }

    private static native void nativeSurfaceCreated(Surface surface);
    private static native void nativeSurfaceChanged(Surface surface, int width, int height);
    private static native void nativeSurfaceDestroyed();
    private static native void nativeTouch(float x, float y, boolean pressed);
    private static native void nativeTick();
    private static native void nativeResume();
    private static native void nativePause();
    private static native void nativeShutdown();
}
