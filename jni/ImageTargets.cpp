/*==============================================================================
 Copyright (c) 2010-2011 QUALCOMM Incorporated.
 All Rights Reserved.
 Qualcomm Confidential and Proprietary

 @file
 ImageTargets.cpp

 @brief
 Sample for ImageTargets

 ==============================================================================*/

#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef USE_OPENGL_ES_1_1
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>

#include "SampleUtils.h"
#include "Texture.h"
#include "CubeShaders.h"
#include "Tomcat.h"
#include "PlaneObject.h"

#ifdef __cplusplus
extern "C" {
#endif

// Textures:
int textureCount = 0;
Texture** textures = 0;

int textureIndex = 0;

Texture* planeObjectTexture = 0;

bool isPlaneObject = false;

// OpenGL ES 2.0 specific:
#ifdef USE_OPENGL_ES_2_0
unsigned int shaderProgramID = 0;
GLint vertexHandle = 0;
GLint normalHandle = 0;
GLint textureCoordHandle = 0;
GLint mvpMatrixHandle = 0;
#endif

// Screen dimensions:
unsigned int screenWidth = 0;
unsigned int screenHeight = 0;

float angleX = 0;
float angleY = 0;

float translateX = 0;
float translateY = 0;

QCAR::DataSet* dataSetDefault = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

// Constants:
static const float kObjectScale = 2.f;

bool switchDataSetAsap = false;

// Object to receive update callbacks from QCAR SDK
class ImageTargets_UpdateCallback: public QCAR::UpdateCallback {
	virtual void QCAR_onUpdate(QCAR::State& /*state*/) {
		if (switchDataSetAsap) {
			switchDataSetAsap = false;

			// Get the image tracker:
			QCAR::TrackerManager& trackerManager =
					QCAR::TrackerManager::getInstance();
			QCAR::ImageTracker
					* imageTracker =
							static_cast<QCAR::ImageTracker*> (trackerManager.getTracker(
									QCAR::Tracker::IMAGE_TRACKER));
			if (imageTracker == 0 || dataSetDefault == 0
					|| imageTracker->getActiveDataSet() == 0) {
				LOG("Failed to switch data set.");
				return;
			}

			if (imageTracker->getActiveDataSet() == dataSetDefault) {
				imageTracker->deactivateDataSet(dataSetDefault);
				imageTracker->activateDataSet(dataSetDefault);
			}
		}
	}
};

ImageTargets_UpdateCallback updateCallback;

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_planeObjectTexture(JNIEnv *env, jobject obj)
{
	// Handle to the activity class:
	jclass activityClass = env->GetObjectClass(obj);

	jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
			"getTextureCount", "()I");
	if (getTextureCountMethodID == 0)
	{
		LOG("Function getTextureCount() not found.");
		return;
	}

	textureCount = env->CallIntMethod(obj, getTextureCountMethodID);
	if (!textureCount)
	{
		LOG("getTextureCount() returned zero.");
		return;
	}

	if (planeObjectTexture != NULL)
	{
		delete planeObjectTexture;
		planeObjectTexture = NULL;
	}

	isPlaneObject = true;

	planeObjectTexture = new Texture();

	jmethodID getTextureMethodID = env->GetMethodID(activityClass,
			"getTexture", "(I)Lcom/snda/sdar/Texture;");

	if (getTextureMethodID == 0)
	{
		LOG("Function getTexture() not found.");
		return;
	}

	// Register the textures

	jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, textureCount - 1);
	if (textureObject == NULL)
	{
		LOG("GetTexture() returned zero pointer");
		return;
	}

	planeObjectTexture = Texture::create(env, textureObject);

}

JNIEXPORT int JNICALL
Java_com_snda_sdar_ImageTargets_getOpenGlEsVersionNative(JNIEnv *, jobject)
{
#ifdef USE_OPENGL_ES_1_1        
	return 1;
#else
	return 2;
#endif
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
	isActivityInPortraitMode = isPortrait;
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_onQCARInitializedNative(JNIEnv *, jobject)
{

	QCAR::registerCallback(&updateCallback);

	// Comment in to enable tracking of up to 2 targets simultaneously and
	// split the work over multiple frames:
	QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
	QCAR::setHint(QCAR::HINT_IMAGE_TARGET_MULTI_FRAME_ENABLED, 1);
}

JNIEXPORT int JNICALL
Java_com_snda_sdar_ImageTargets_initTracker(JNIEnv *, jobject)
{
	LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initTracker");

	// Initialize the image tracker:
	QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
	QCAR::Tracker* tracker = trackerManager.initTracker(QCAR::Tracker::IMAGE_TRACKER);
	if (tracker == NULL)
	{
		LOG("Failed to initialize ImageTracker.");
		return 0;
	}

	LOG("Successfully initialized ImageTracker.");
	return 1;
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_deinitTracker(JNIEnv *, jobject)
{
	LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitTracker");

	// Deinit the image tracker:
	QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
	trackerManager.deinitTracker(QCAR::Tracker::IMAGE_TRACKER);
}

JNIEXPORT int JNICALL
Java_com_snda_sdar_ImageTargets_loadTrackerData(JNIEnv *, jobject)
{
	LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_loadTrackerData");

	// Get the image tracker:
	QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
	QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
			trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
	if (imageTracker == NULL)
	{
		LOG("Failed to load tracking data set because the ImageTracker has not"
				" been initialized.");
		return 0;
	}

	// Create the data sets:
	dataSetDefault = imageTracker->createDataSet();
	if (dataSetDefault == 0)
	{
		LOG("Failed to create a new tracking data.");
		return 0;
	}

	// Load the data sets:
	if (!dataSetDefault->load("default.xml", QCAR::DataSet::STORAGE_APPRESOURCE))
	{
		LOG("Failed to load data set.");
		return 0;
	}

	// Activate the data set:
	if (!imageTracker->activateDataSet(dataSetDefault))
	{
		LOG("Failed to activate data set.");
		return 0;
	}

	LOG("Successfully loaded and activated data set.");
	return 1;
}

JNIEXPORT int JNICALL
Java_com_snda_sdar_ImageTargets_destroyTrackerData(JNIEnv *, jobject)
{
	LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_destroyTrackerData");

	// Get the image tracker:
	QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
	QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
			trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
	if (imageTracker == NULL)
	{
		LOG("Failed to destroy the tracking data set because the ImageTracker has not"
				" been initialized.");
		return 0;
	}

	if (dataSetDefault != 0)
	{
		if (imageTracker->getActiveDataSet() == dataSetDefault &&
				!imageTracker->deactivateDataSet(dataSetDefault))
		{
			LOG("Failed to destroy the tracking data set StonesAndChips because the data set "
					"could not be deactivated.");
			return 0;
		}

		if (!imageTracker->destroyDataSet(dataSetDefault))
		{
			LOG("Failed to destroy the tracking data set StonesAndChips.");
			return 0;
		}

		LOG("Successfully destroyed the data set StonesAndChips.");
		dataSetDefault = 0;
	}

	return 1;
}

/*
 Transform a point(column vector) by a 4x4 matrix. Then, out = m * in
 Input: m ----- the 4x4 matrix, in ---- the 4x1 vector
 Output: out ---- the resulting 4x1 vector
 */
static void transform_point(float out[4], const float m[16], const float in[4]) {
#define M(row,col) m[col*4+row]
	out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3)
			* in[3];
	out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3)
			* in[3];
	out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3)
			* in[3];
	out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3)
			* in[3];
#undef M
}

static void col2row(float in[16], float out[16]) {

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[i * 4 + j] = in[j * 4 + i];
		}
	}

}

bool gluProject(float objx, float objy, float objz,
		const float modelMatrix[16], const float projMatrix[16],
		const int viewport[4], float *winx, float *winy, float *winz) {
	// matrice transformation
	float in[4], out[4];
	//initialize matrice and column vector as a transformer
	in[0] = objx;
	in[1] = objy;
	in[2] = objz;
	in[3] = 1.0;
	transform_point(out, modelMatrix, in); //乘以模型视图矩阵
	transform_point(in, projMatrix, out); //乘以投影矩阵
	//齐次向量的第四项不能为0
	if (in[3] == 0.0)
		return false;
	//向量齐次化标准化
	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];
	//视口向量的作用
	*winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
	*winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;
	*winz = (1 + in[2]) / 2;
	return true;
}

QCAR::Vec2F cameraPointToScreenPoint(QCAR::Vec2F cameraPoint, bool isPortrait) {
	QCAR::VideoMode videoMode = QCAR::CameraDevice::getInstance().getVideoMode(
			QCAR::CameraDevice::MODE_DEFAULT);
	QCAR::VideoBackgroundConfig config =
			QCAR::Renderer::getInstance().getVideoBackgroundConfig();

	int xOffset = ((int) screenWidth - config.mSize.data[0]) / 2.0f
			+ config.mPosition.data[0];
	int yOffset = ((int) screenHeight - config.mSize.data[1]) / 2.0f
			- config.mPosition.data[1];

	if (isPortrait) {
		// camera image is rotated 90 degrees
		int rotatedX = videoMode.mHeight - cameraPoint.data[1];
		int rotatedY = cameraPoint.data[0];

		return QCAR::Vec2F(
				rotatedX * config.mSize.data[0] / (float) videoMode.mHeight
						+ xOffset,
				rotatedY * config.mSize.data[1] / (float) videoMode.mWidth
						+ yOffset);
	} else {
		return QCAR::Vec2F(
				cameraPoint.data[0] * config.mSize.data[0]
						/ (float) videoMode.mWidth + xOffset,
				cameraPoint.data[1] * config.mSize.data[1]
						/ (float) videoMode.mHeight + yOffset);
	}
}

JNIEXPORT jboolean JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_renderFrame(JNIEnv *env, jobject obj)
{
	//LOG("Java_com_snda_sdar_ImageTargets_GLRenderer_renderFrame");

	// Clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	jboolean isDetected = false;

	// Render video background:
	QCAR::State state = QCAR::Renderer::getInstance().begin();

	// Explicitly render the Video Background
	QCAR::Renderer::getInstance().drawVideoBackground();

#ifdef USE_OPENGL_ES_1_1
	// Set GL11 flags:
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Did we find any trackables this frame?
	for(int tIdx = 0; tIdx < state.getNumActiveTrackables(); tIdx++)
	{
		isDetected = true;

		// Get the trackable:
		const QCAR::Trackable* trackable = state.getActiveTrackable(tIdx);
		QCAR::Matrix44F modelViewMatrix =
		QCAR::Tool::convertPose2GLMatrix(trackable->getPose());

		// Choose the texture based on the target name:
		//int textureIndex = (!strcmp(trackable->getName(), "stones")) ? 0 : 1;

		const Texture* const thisTexture = textures[textureIndex];
		const Texture* const tagTexture = textures[textureCount - 1];

#ifdef USE_OPENGL_ES_1_1
		// Load projection matrix:
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projectionMatrix.data);

		// Load model view matrix:
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(modelViewMatrix.data);
		glTranslatef(0.f, 0.f, kObjectScale);
		glScalef(kObjectScale, kObjectScale, kObjectScale);

		// Draw object:
		glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
		glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) &planeTexCoords[0]);
		glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) &planeVertices[0]);
		glNormalPointer(GL_FLOAT, 0, (const GLvoid*) &planeNormals[0]);
		glDrawElements(GL_TRIANGLES, NUM_PLANE_OBJECT_INDEX, GL_UNSIGNED_SHORT,
				(const GLvoid*) &planeIndices[0]);

		//		// Load model view matrix:
		//		glMatrixMode(GL_MODELVIEW);
		//		glLoadMatrixf(modelViewMatrix.data);
		//		glTranslatef(50.f, 50.f, kObjectScale);
		//		glScalef(1, 1, 1);
		//
		//		// Draw object:
		//		glBindTexture(GL_TEXTURE_2D, tagTexture->mTextureID);
		//		glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) &planeTexCoords[0]);
		//		glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) &planeVertices[0]);
		//		glNormalPointer(GL_FLOAT, 0, (const GLvoid*) &planeNormals[0]);
		//		glDrawElements(GL_TRIANGLES, NUM_PLANE_OBJECT_INDEX, GL_UNSIGNED_SHORT,
		//				(const GLvoid*) &planeIndices[0]);
#else

		QCAR::Matrix44F modelViewMatrix2;

		for (int i = 0; i < 16; i++)
		{
			modelViewMatrix2.data[i] = modelViewMatrix.data[i];
		}

		QCAR::Matrix44F modelViewProjection;
		QCAR::Matrix44F modelViewProjection2;

		SampleUtils::translatePoseMatrix(translateX, translateY, kObjectScale,
				&modelViewMatrix.data[0]);
		SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
				&modelViewMatrix.data[0]);

		SampleUtils::rotatePoseMatrix(angleX, 0, 1, 0,
				&modelViewMatrix.data[0]);
		SampleUtils::rotatePoseMatrix(angleY, 1, 0, 0,
				&modelViewMatrix.data[0]);

		SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
				&modelViewMatrix.data[0] ,
				&modelViewProjection.data[0]);

		glUseProgram(shaderProgramID);

		glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
				(const GLvoid*) &planeVertices[0]);
		glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
				(const GLvoid*) &planeNormals[0]);
		glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
				(const GLvoid*) &planeTexCoords[0]);

		glEnableVertexAttribArray(vertexHandle);
		glEnableVertexAttribArray(normalHandle);
		glEnableVertexAttribArray(textureCoordHandle);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
		glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
				(GLfloat*)&modelViewProjection.data[0] );
		glDrawElements(GL_TRIANGLES, NUM_PLANE_OBJECT_INDEX, GL_UNSIGNED_SHORT,
				(const GLvoid*) &planeIndices[0]);

		/////////////////////////////////////////////////////////////////////////////

		SampleUtils::translatePoseMatrix(50, 50, kObjectScale,
				&modelViewMatrix2.data[0]);
		SampleUtils::scalePoseMatrix(1, 1, 1,
				&modelViewMatrix2.data[0]);

		SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
				&modelViewMatrix2.data[0] ,
				&modelViewProjection2.data[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tagTexture->mTextureID);
		glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
				(GLfloat*)&modelViewProjection2.data[0] );
		glDrawElements(GL_TRIANGLES, NUM_PLANE_OBJECT_INDEX, GL_UNSIGNED_SHORT,
				(const GLvoid*) &planeIndices[0]);

		jclass renderClass = env->GetObjectClass(obj);

		jmethodID screenPointMethodId = env->GetMethodID(renderClass , "screenPoint", "(FF)V");
		if (!screenPointMethodId)
		{
			LOG("Function screenPoint(float, float) not found.");
			return 0;
		}

		const QCAR::CameraCalibration& cameraCalibration =
		QCAR::CameraDevice::getInstance().getCameraCalibration();
		QCAR::Vec2F cameraPoint = QCAR::Tool::projectPoint(cameraCalibration, trackable->getPose(), QCAR::Vec3F(0, 0, 0));

		QCAR::Vec2F screenPoint = cameraPointToScreenPoint(cameraPoint, true);

		env->CallObjectMethod(obj, screenPointMethodId, screenPoint.data[0], screenPoint.data[1]);

		SampleUtils::checkGlError("ImageTargets renderFrame");
#endif

	}

	glDisable(GL_DEPTH_TEST);

#ifdef USE_OPENGL_ES_1_1        
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
	glDisableVertexAttribArray(vertexHandle);
	glDisableVertexAttribArray(normalHandle);
	glDisableVertexAttribArray(textureCoordHandle);
#endif

	QCAR::Renderer::getInstance().end();

	return isDetected;
}

void configureVideoBackground() {
	// Get the default video mode:
	QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
	QCAR::VideoMode videoMode = cameraDevice. getVideoMode(
			QCAR::CameraDevice::MODE_DEFAULT);

	// Configure the video background
	QCAR::VideoBackgroundConfig config;
	config.mEnabled = true;
	config.mSynchronous = true;
	config.mPosition.data[0] = 0.0f;
	config.mPosition.data[1] = 0.0f;

	if (isActivityInPortraitMode) {
		//LOG("configureVideoBackground PORTRAIT");
		config.mSize.data[0] = videoMode.mHeight * (screenHeight
				/ (float) videoMode.mWidth);
		config.mSize.data[1] = screenHeight;
	} else {
		//LOG("configureVideoBackground LANDSCAPE");
		config.mSize.data[0] = screenWidth;
		config.mSize.data[1] = videoMode.mHeight * (screenWidth
				/ (float) videoMode.mWidth);
	}

	// Set the config:
	QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_initApplicationNative(
		JNIEnv* env, jobject obj, jint width, jint height)
{
	LOG("Java_com_snda_sdar_ImageTargets_initApplicationNative");

	// Store screen dimensions
	screenWidth = width;
	screenHeight = height;

	// Handle to the activity class:
	jclass activityClass = env->GetObjectClass(obj);

	jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
			"getTextureCount", "()I");
	if (getTextureCountMethodID == 0)
	{
		LOG("Function getTextureCount() not found.");
		return;
	}

	textureCount = env->CallIntMethod(obj, getTextureCountMethodID);
	if (!textureCount)
	{
		LOG("getTextureCount() returned zero.");
		return;
	}

	textures = new Texture*[textureCount];

	jmethodID getTextureMethodID = env->GetMethodID(activityClass,
			"getTexture", "(I)Lcom/snda/sdar/Texture;");

	if (getTextureMethodID == 0)
	{
		LOG("Function getTexture() not found.");
		return;
	}

	// Register the textures
	for (int i = 0; i < textureCount; ++i)
	{

		jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, i);
		if (textureObject == NULL)
		{
			LOG("GetTexture() returned zero pointer");
			return;
		}

		textures[i] = Texture::create(env, textureObject);
	}
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_deinitApplicationNative(
		JNIEnv* env, jobject obj)
{
	LOG("Java_com_snda_sdar_ImageTargets_deinitApplicationNative");

	// Release texture resources
	if (textures != 0)
	{
		for (int i = 0; i < textureCount; ++i)
		{
			delete textures[i];
			textures[i] = NULL;
		}

		delete[]textures;
		textures = NULL;

		textureCount = 0;
	}
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_startCamera(JNIEnv *,
		jobject)
{
	LOG("Java_com_snda_sdar_ImageTargets_startCamera");

	// Initialize the camera:
	if (!QCAR::CameraDevice::getInstance().init())
	return;

	// Configure the video background
	configureVideoBackground();

	// Select the default mode:
	if (!QCAR::CameraDevice::getInstance().selectVideoMode(
					QCAR::CameraDevice::MODE_DEFAULT))
	return;

	// Start the camera:
	if (!QCAR::CameraDevice::getInstance().start())
	return;

	// Uncomment to enable flash
	//if(QCAR::CameraDevice::getInstance().setFlashTorchMode(true))
	//	LOG("IMAGE TARGETS : enabled torch");

	// Uncomment to enable infinity focus mode, or any other supported focus mode
	// See CameraDevice.h for supported focus modes
	//if(QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_INFINITY))
	//	LOG("IMAGE TARGETS : enabled infinity focus");

	// Start the tracker:
	QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
	QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
	if(imageTracker != 0)
	imageTracker->start();
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_stopCamera(JNIEnv *,
		jobject)
{
	LOG("Java_com_snda_sdar_ImageTargets_stopCamera");

	// Stop the tracker:
	QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
	QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
	if(imageTracker != 0)
	imageTracker->stop();

	QCAR::CameraDevice::getInstance().stop();
	QCAR::CameraDevice::getInstance().deinit();
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargets_setProjectionMatrix(JNIEnv *, jobject)
{
	// Cache the projection matrix:
	const QCAR::CameraCalibration& cameraCalibration =
	QCAR::CameraDevice::getInstance().getCameraCalibration();
	projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f,
			2000.0f);
}

JNIEXPORT jboolean JNICALL
Java_com_snda_sdar_ImageTargets_toggleFlash(JNIEnv*, jobject, jboolean flash)
{
	return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_snda_sdar_ImageTargets_autofocus(JNIEnv*, jobject)
{
	return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;

}

JNIEXPORT jboolean JNICALL
Java_com_snda_sdar_ImageTargetssetFocusMode(JNIEnv*, jobject, jint mode)
{
	int qcarFocusMode;

	switch ((int)mode)
	{
		case 0:
		qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
		break;

		case 1:
		qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
		break;

		case 2:
		qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
		break;

		case 3:
		qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
		break;

		default:
		return JNI_FALSE;
	}

	return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;

}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_initRendering(
		JNIEnv* env, jobject obj)
{
	LOG("Java_com_snda_sdar_ImageTargets_ImageTargetsRenderer_initRendering");

	// Define clear color
	glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);

	// Now generate the OpenGL texture objects and add settings
	for (int i = 0; i < textureCount; ++i)
	{
		glGenTextures(1, &(textures[i]->mTextureID));
		glBindTexture(GL_TEXTURE_2D, textures[i]->mTextureID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[i]->mWidth,
				textures[i]->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				(GLvoid*) textures[i]->mData);
	}

	if (isPlaneObject && planeObjectTexture != NULL)
	{
		glGenTextures(1, &(planeObjectTexture->mTextureID));
		glBindTexture(GL_TEXTURE_2D, planeObjectTexture->mTextureID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, planeObjectTexture->mWidth,
				planeObjectTexture->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				(GLvoid*) planeObjectTexture->mData);
	}

#ifndef USE_OPENGL_ES_1_1

	shaderProgramID = SampleUtils::createProgramFromBuffer(cubeMeshVertexShader,
			cubeFragmentShader);

	vertexHandle = glGetAttribLocation(shaderProgramID,
			"vertexPosition");
	normalHandle = glGetAttribLocation(shaderProgramID,
			"vertexNormal");
	textureCoordHandle = glGetAttribLocation(shaderProgramID,
			"vertexTexCoord");
	mvpMatrixHandle = glGetUniformLocation(shaderProgramID,
			"modelViewProjectionMatrix");

#endif

}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_updateRendering(
		JNIEnv* env, jobject obj, jint width, jint height)
{
	LOG("Java_com_snda_sdar_ImageTargets_ImageTargetsRenderer_updateRendering");

	// Update screen dimensions
	screenWidth = width;
	screenHeight = height;

	// Reconfigure the video background
	configureVideoBackground();
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_setRotate(
		JNIEnv* env, jobject obj, float x, float y)
{
	angleX = x;
	angleY = y;
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_setTranslate(
		JNIEnv* env, jobject obj, float x, float y)
{
	translateX = x;
	translateY = y;
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_nextIndex(
		JNIEnv* env, jobject obj)
{
	textureIndex++;
	if (textureIndex == textureCount - 1) {
		textureIndex = 0;
	}
}

JNIEXPORT void JNICALL
Java_com_snda_sdar_ImageTargetsRenderer_previousIndex(
		JNIEnv* env, jobject obj)
{
	textureIndex--;
	if (textureIndex == -1) {
		textureIndex = textureCount - 2;
	}
}

#ifdef __cplusplus
}
#endif
