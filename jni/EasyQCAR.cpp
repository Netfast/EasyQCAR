/*==============================================================================
@author
    Vinjn Zhang ( vinjn.z@gmail.com) 
@file 
    EasyQCAR.cpp

@brief
    Wrapper for easy use of QCAR

==============================================================================*/


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
// #include "Texture.h"
// #include "CubeShaders.h"
// #include "Teapot.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Screen dimensions:
unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode   = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

// Constants:
// static const float kObjectScale = 3.f;

QCAR::DataSet* g_dataSet    = 0;
  

JNIEXPORT int JNICALL
Java_com_vmstudio_EasyQCAR_getOpenGlEsVersionNative(JNIEnv *, jobject)
{
#ifdef USE_OPENGL_ES_1_1        
    return 1;
#else
    return 2;
#endif
}

// JNIEXPORT void JNICALL
// Java_com_vmstudio_EasyQCAR_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
// {
    // isActivityInPortraitMode = isPortrait;
// }
  

JNIEXPORT int JNICALL
Java_com_vmstudio_EasyQCAR_create(JNIEnv* env, jobject, jstring trackerXml)
{
    LOGI("EasyQCAR_create");
    
    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                    trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL)
    {
        imageTracker = static_cast<QCAR::ImageTracker*>(trackerManager.initTracker(QCAR::Tracker::IMAGE_TRACKER));
        if (imageTracker == NULL)
        {
            LOGE("Failed to initialize ImageTracker.");
            return 0;
        }

        LOGI("Successfully initialized ImageTracker.");
    }

    // Create the data sets:
    g_dataSet = imageTracker->createDataSet();
    if (g_dataSet == 0)
    {
        LOGE("Failed to create a new tracking data.");
        return 0;
    }
    // Load the data sets:
    const char* c_xml = env->GetStringUTFChars(trackerXml, 0);    
    if (!g_dataSet->load(c_xml, QCAR::DataSet::STORAGE_APPRESOURCE))
    {
        LOGE("Failed to load data set %s.", c_xml);
        return 0;
    }
    env->ReleaseStringUTFChars(trackerXml, c_xml);

    // Activate the data set:
    if (!imageTracker->activateDataSet(g_dataSet))
    {
        LOGE("Failed to activate data set %s.", c_xml);
        return 0;
    }

    LOGI("Successfully loaded and activated data set %s.", c_xml);
    return 1;
}


JNIEXPORT int JNICALL
Java_com_vmstudio_EasyQCAR_destroy(JNIEnv *, jobject)
{
    LOGI("EasyQCAR_destroy");

    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
        trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL)
    {
        LOGE("Failed to destroy the tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }

    if (g_dataSet != 0)
    {
        if (!imageTracker->deactivateDataSet(g_dataSet))
        {
            LOGE("Failed to destroy the tracking data set because the data set "
                "could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(g_dataSet))
        {
            LOGE("Failed to destroy the tracking data set.");
            return 0;
        }

        LOGI("Successfully destroyed the data set.");
        g_dataSet = 0;
        return 1;
    }
    
    //Destroy the image tracker
    trackerManager.deinitTracker(QCAR::Tracker::IMAGE_TRACKER);

    LOGI("No tracker data set to destroy.");
    return 0;
}

QCAR::State g_state;
bool qcar_begun = false;

JNIEXPORT void JNICALL
Java_com_vmstudio_EasyQCAR_begin(JNIEnv *, jobject)
{
    // LOGI("+++++++++++++++++++++++++++++++++++++++");
    // Get the state from QCAR and mark the beginning of a rendering section
    g_state = QCAR::Renderer::getInstance().begin();
    
    qcar_begun = true;
}

JNIEXPORT void JNICALL
Java_com_vmstudio_EasyQCAR_drawBackground(JNIEnv *, jobject)
{  
    // LOGI("----------------------------------------");
    // Explicitly render the Video Background
    QCAR::Renderer::getInstance().drawVideoBackground();   
}

JNIEXPORT void JNICALL
Java_com_vmstudio_EasyQCAR_end(JNIEnv *, jobject)
{
    if (qcar_begun)
    {
        QCAR::Renderer::getInstance().end();
        qcar_begun = false;
    }
    else
    {
        LOGD("call beginAndDrawBackground() first.");
    }
}

JNIEXPORT int JNICALL
Java_com_vmstudio_EasyQCAR_getNumActiveTrackables(JNIEnv *, jobject)
{
    if (qcar_begun)
        return g_state.getNumActiveTrackables();
    else
        return 0;
}

const int MAT_SIZE = 16;
    
JNIEXPORT jfloatArray JNICALL
Java_com_vmstudio_EasyQCAR_getProjectioMatrix(JNIEnv* env, jobject)
{
    jfloatArray array = env->NewFloatArray(MAT_SIZE);
    env->SetFloatArrayRegion(array, 0, MAT_SIZE, projectionMatrix.data);
    return array;
}

JNIEXPORT jfloatArray JNICALL
Java_com_vmstudio_EasyQCAR_getModelViewMatrix(JNIEnv* env, jobject, jint tIdx)
{
    if (!qcar_begun)
        return NULL;
    const QCAR::Trackable* trackable = g_state.getActiveTrackable(tIdx);
    QCAR::Matrix44F modelViewMatrix =
        QCAR::Tool::convertPose2GLMatrix(trackable->getPose());
    jfloatArray array = env->NewFloatArray(MAT_SIZE);
    env->SetFloatArrayRegion(array, 0, MAT_SIZE, modelViewMatrix.data);
    return array;
}

JNIEXPORT jstring JNICALL
Java_com_vmstudio_EasyQCAR_getName(JNIEnv* env, jobject, jint tIdx)
{
    if (!qcar_begun)
        return NULL;
    const QCAR::Trackable* trackable = g_state.getActiveTrackable(tIdx);
    const char* name = trackable->getName();
    return env->NewStringUTF(name);
}

void
configureVideoBackground()
{
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.
                                getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;
    
    if (isActivityInPortraitMode)
    {
        LOGI("configureVideoBackground PORTRAIT");
        config.mSize.data[0] = videoMode.mHeight
                                * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;

        if(config.mSize.data[0] < screenWidth)
        {
            LOGI("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenWidth;
            config.mSize.data[1] = screenWidth * 
                              (videoMode.mWidth / (float)videoMode.mHeight);
        }
    }
    else
    {
        LOGI("configureVideoBackground LANDSCAPE");
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
                            * (screenWidth / (float)videoMode.mWidth);

        if(config.mSize.data[1] < screenHeight)
        {
            LOGI("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenHeight
                                * (videoMode.mWidth / (float)videoMode.mHeight);
            config.mSize.data[1] = screenHeight;
        }
    }

    LOGI("Configure Video Background : Video (%d,%d), Screen (%d,%d), mSize (%d,%d)", videoMode.mWidth, videoMode.mHeight, screenWidth, screenHeight, config.mSize.data[0], config.mSize.data[1]);

    // Set the config:
    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}


JNIEXPORT void JNICALL
Java_com_vmstudio_EasyQCAR_setScreenSize(
                            JNIEnv* env, jobject obj, jint width, jint height)
{
    LOGI("EasyQCAR_setScreenSize (%d, %d)", width, height); 
    // Store screen dimensions
    screenWidth = width;
    screenHeight = height;
}

JNIEXPORT void JNICALL
Java_com_vmstudio_EasyQCAR_resume(JNIEnv *, jobject)
{
    LOGI("EasyQCAR_resume");

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

    // Start the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->start();
 
    // Cache the projection matrix:
    const QCAR::CameraCalibration& cameraCalibration =
                                QCAR::CameraDevice::getInstance().getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f,
                                            2000.0f);
}


JNIEXPORT void JNICALL
Java_com_vmstudio_EasyQCAR_pause(JNIEnv *,jobject)
{
    LOGI("EasyQCAR_pause");

    // Stop the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->stop();
    
    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}

JNIEXPORT jboolean JNICALL
Java_com_vmstudio_EasyQCAR_toggleFlash(JNIEnv*, jobject, jboolean flash)
{
    return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_vmstudio_EasyQCAR_autofocus(JNIEnv*, jobject)
{
	return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_vmstudio_EasyQCAR_setFocusMode(JNIEnv*, jobject, jint mode)
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
Java_com_vmstudio_EasyQCAR_surfaceChanged(
                        JNIEnv* env, jobject obj, jint width, jint height)
{
    LOGI("EasyQCAR_surfaceChanged");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
}


#ifdef __cplusplus
}
#endif
