package com.vmstudio.ArSample;

import javax.microedition.khronos.opengles.GL10;

import processing.core.PApplet;
import processing.core.PMatrix3D;
import processing.opengl.PGraphicsOpenGL;
import android.os.Bundle;

import com.vmstudio.DebugLog;
import com.vmstudio.EasyQCAR;

public class SampleActivity extends PApplet {

	EasyQCAR qcar = new EasyQCAR();

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		orientation(LANDSCAPE);

		qcar.setup(this, displayWidth, displayHeight);
		qcar.create("StonesAndChips.xml");
	}

	public String sketchRenderer() {
		return OPENGL;
	}

	public void setup() {

	}

	public void resume() {
		qcar.onSurfaceCreated();
		qcar.onResume();
	}
	
	public String sketchColordepth() {
		return MEDIUM_COLOR_DEPTH;
	}

//	public void handleDraw()
//	{
//	    ((PGraphicsOpenGL)g).pgl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
//		qcar.begin();
//		qcar.drawBackground();
//		qcar.end();	
//	}
	
	public void draw() {
		fill(255, 200);
		rect(0,0,400,400);
		
		qcar.begin();
		qcar.drawBackground();

		float[] prj = qcar.getProjectioMatrix();

		for (int i = 0; i < qcar.getNumActiveTrackables(); i++) {
			DebugLog.LOGD(qcar.getName(i));
			float[] modelView = qcar.getModelViewMatrix(i);
			DebugLog.LOGD(modelView.toString());
			pushMatrix();
			{
				PMatrix3D matModelView = new PMatrix3D();
				matModelView.set(modelView);
				setMatrix(matModelView);

				translate(width/2, height/2);
				fill(255);
				noStroke();
				box(100,100,100);
//				box(5, 5, 5);
			}
			popMatrix();
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