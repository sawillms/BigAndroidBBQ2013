/*
* 2013 Android Code Kitchen reference solution
* Seth Williams
* swilliams@nvidia.com
*/

#include <jni.h>
#include <android/log.h>

#include <GL/Regal.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#define  LOG_TAG "Dreamtri"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C"
{
  JNIEXPORT void JNICALL Java_com_regal_dreamtri_DreamtriLib_init(JNIEnv *env, jobject obj,  jint width, jint height);
  JNIEXPORT void JNICALL Java_com_regal_dreamtri_DreamtriLib_step(JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_regal_dreamtri_DreamtriLib_direction(JNIEnv *env, jobject obj, jint direction);
};

enum { LEFT, RIGHT };

float g_x;

JNIEXPORT void JNICALL Java_com_regal_dreamtri_DreamtriLib_direction(JNIEnv *env, jobject obj, jint direction)
{
  if (direction == LEFT)
  {
    g_x = -0.5f;
  } else if (direction == RIGHT) {
    g_x = 0.5f; 
  }
}

JNIEXPORT void JNICALL Java_com_regal_dreamtri_DreamtriLib_init(JNIEnv *env, jobject obj,  jint width, jint height)
{
  // Configure Regal to log to file in /sdcard (disabled by default in this sample)
  // Note that for your own projects WRITE_EXTERNAL_STORAGE permission needs to be added to your AndroidManifest.xml
  setenv("REGAL_LOG_ALL", "0", 1);
  setenv("REGAL_LOG_ERROR", "1", 1);
  setenv("REGAL_LOG_FILE", "/sdcard/dreamri.log", 1);

  // Because teh EGL context was created by a Java thread, we need to register it with Regal.
  RegalMakeCurrent(eglGetCurrentContext());

  g_x = 0.0f;

  // Avoid unlikely divide by 0
  if (height==0)
    height=1;

  float aspect = float(width)/float(height);

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glViewport(0, 0, width, height);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  if( aspect > 1 ) {
    glFrustum( -0.1 * aspect, 0.1 * aspect, -0.1, 0.1, 0.1, 100.0 );
  } else {
    glFrustum( -0.1, 0.1, -0.1 / aspect, 0.1 / aspect, 0.1, 100.0 );
  }
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

JNIEXPORT void JNICALL Java_com_regal_dreamtri_DreamtriLib_step(JNIEnv *env, jobject obj)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();
  glTranslatef(g_x,0.0f,-1.0f);
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f( 0.0f,  1.0f, 0.0f);
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-1.0f, -1.0f, 0.0f);
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f( 1.0f, -1.0f, 0.0f);
  glEnd();
}
