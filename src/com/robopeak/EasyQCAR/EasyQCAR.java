package com.robopeak.EasyQCAR;

import android.app.Activity;

import com.qualcomm.QCAR.QCAR;
import com.qualcomm.ar.pl.DebugLog;

public class EasyQCAR {
	/** Native tracker initialization and deinitialization. */
	// public native int initTracker();
	// public native void deinitTracker();
	/** Tells native code whether we are in portait or landscape mode */
	// private native void setActivityPortraitMode(boolean isPortrait);
	/** Native function to update the renderer. */
	private native void surfaceChanged(int width, int height);

	// tracking related
	public native void begin();
	public native void end();
	
	public native void drawBackground();

	public native int getNumActiveTrackables();

	public native float[] getProjectioMatrix();

	public native float[] getModelViewMatrix(int tIdx);

	public native String getName(int tIdx);

	/** Native functions to load and destroy tracking data. */
	public native int create(String trackerXml);

	public native int destroy();

	// public native void destroyTrackerData();

	/** Native methods for starting and stoping the camera. */
	private native void resume();

	private native void pause();

	/** Native function to initialize the application. */
	private native void setScreenSize(int width, int height);

	/** Native function to deinitialize the application. */
	// private native void deinitApplicationNative();

	/** Static initializer block to load native libraries on start-up. */
	static {
		loadLibrary("QCAR");
		loadLibrary("EasyQCAR");
	}

	final static String tag = "EasyQCAR";
	
	//TODO: add to onResume()
	public void onSurfaceCreated() {
		// Call QCAR function to (re)initialize rendering after first use
		// or after OpenGL ES context was lost (e.g. after onPause/onResume):
		QCAR.onSurfaceCreated();
	}

	/** A helper for loading native libraries stored in "libs/armeabi*". */
	private static boolean loadLibrary(String nLibName) {
		try {
			System.loadLibrary(nLibName);
			DebugLog.LOGI(tag, "Native library lib" + nLibName + ".so loaded");
			return true;
		} catch (UnsatisfiedLinkError ulee) {
			DebugLog.LOGE(tag, "The library lib" + nLibName
					+ ".so could not be loaded");
		} catch (SecurityException se) {
			DebugLog.LOGE(tag, "The library lib" + nLibName
					+ ".so was not allowed to be loaded");
		}

		return false;
	}

	public boolean setup(Activity host, int width, int height) {
		int mQCARFlags = QCAR.GL_11;
		int mProgressValue = -1;

		QCAR.setInitParameters(host, mQCARFlags);

		do {
			// QCAR.init() blocks until an initialization step is complete,
			// then it proceeds to the next step and reports progress in
			// percents (0 ... 100%)
			// If QCAR.init() returns -1, it indicates an error.
			// Initialization is done when progress has reached 100%.
			mProgressValue = QCAR.init();

			DebugLog.LOGD(tag, "QCAR.init Progress [" + mProgressValue + "/100]");

			// We check whether the task has been canceled in the meantime
			// (by calling AsyncTask.cancel(true))
			// and bail out if it has, thus stopping this thread.
			// This is necessary as the AsyncTask will run to completion
			// regardless of the status of the component that started is.
		} while (mProgressValue >= 0 && mProgressValue < 100);

		// NOTE: Check if initialization failed because the device is
		// not supported. At this point the user should be informed
		// with a message.
		if (mProgressValue == QCAR.INIT_DEVICE_NOT_SUPPORTED) {
			DebugLog.LOGD(tag, "Failed to initialize QCAR because this "
					+ "device is not supported.");
		} else if (mProgressValue == QCAR.INIT_ERROR) {
			DebugLog.LOGD(tag, "Failed to initialize QCAR.");
		}

		setScreenSize(width, height);

		return true;
	}

	public void onSurfaceChanged(int width, int height) {
		surfaceChanged(width, height);
		QCAR.onSurfaceChanged(width, height);
	}

	public void onResume() {
		resume();
		QCAR.onResume();
	}

	public void onPause() {
		pause();
		QCAR.onPause();
	}

	public void onDestroy() {
		destroy();
		QCAR.deinit();
	}
}
