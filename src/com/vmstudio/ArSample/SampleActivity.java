package com.vmstudio.ArSample;

import processing.core.PApplet;
import android.os.Bundle;

import com.qualcomm.ar.pl.DebugLog;
import com.vmstudio.EasyQCAR;

public class SampleActivity extends PApplet {

	EasyQCAR qcar = new EasyQCAR();

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		orientation(LANDSCAPE);
		
		qcar.setup(this, displayWidth, displayHeight);
		qcar.create("StonesAndChips.xml");
		
		// LayoutParams params = new LayoutParams(LayoutParams.FILL_PARENT,
		// LayoutParams.FILL_PARENT);
		//
		// cameraView = new CameraSurfaceView(this);
		// addContentView(cameraView, params);
	}

	public void setup() {

	}

	public void resume() {
		qcar.onSurfaceCreated();
		qcar.onResume();
	}

	public void draw() {
		qcar.begin();
		qcar.drawBackground();
		float[] prj = qcar.getProjectioMatrix();

		for (int i = 0; i < qcar.getNumActiveTrackables(); i++) {
			DebugLog.LOGD("qcar", qcar.getName(i));
			float[] modelView = qcar.getModelViewMatrix(i);
			DebugLog.LOGD("qcar", modelView.toString());
		}
		qcar.end();
	}

	public void pause() {
		qcar.onPause();
	}

	public void destroy() {
		qcar.onDestroy();
	}

}