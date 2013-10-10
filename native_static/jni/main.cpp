/*
 * 2013 Android Code Kitchen reference solution
 * Seth Williams
 * swilliams@nvidia.com
 */

#define INPUT 1 // input handling code isolated so you can see what to add in a discrete step

#include <jni.h>
#include <errno.h>
#include <stdlib.h>

#include <GL/Regal.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#if INPUT
// AMotionEvent_getAxisValue is not exposed via the NDK yet so we have to manually load it
#include <dlfcn.h>

extern float AMotionEvent_getAxisValue(const AInputEvent* motion_event, int32_t axis, size_t pointer_index);
static typeof(AMotionEvent_getAxisValue) *p_AMotionEvent_getAxisValue;
#define AMotionEvent_getAxisValue (*p_AMotionEvent_getAxisValue)
#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "dreamtri", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "dreamtri", __VA_ARGS__))

/**
 * Our saved state data.
 */
struct saved_state {
    float x;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

void dreamTriInit(int width, int height)
{
  // Configure Regal to log to file in /sdcard (disabled by default in this sample)
  // Note that for your own projects WRITE_EXTERNAL_STORAGE permission needs to be added to your AndroidManifest.xml
  setenv("REGAL_LOG_ALL", "0", 1);
  setenv("REGAL_LOG_ERROR", "1", 1);
  setenv("REGAL_LOG_FILE", "/sdcard/dreamri.log", 1);

  // This is not necessary when the EGL context is created natively, but is here as a reminder
  // to do register the EGL context with Regal when it is created by a Java thread. 
  RegalMakeCurrent(eglGetCurrentContext());

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

  // avoid unlikely divide by zero
  if (height==0)
  {
    height=1;
  }

  float aspect = float(width)/float(height);

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

void dreamTriStep(struct engine* engine)
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glLoadIdentity();
  glTranslatef(engine->state.x,0.0f,-1.0f);
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f( 0.0f,  1.0f, 0.0f);
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-1.0f, -1.0f, 0.0f);
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f( 1.0f, -1.0f, 0.0f);
  glEnd();
}

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);

    EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    context = eglCreateContext(display, config, NULL, ctxAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    dreamTriInit(w,h);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    dreamTriStep(engine);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

#if INPUT
/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {

    // these values correspond to the queriable axes indicated
    enum {
        AXIS_X        = 0,  // left joystick, left to right
        AXIS_Y        = 1,  // left joystick, up to down 
        AXIS_Z        = 11, // right joystick, left to right 
        AXIS_RZ       = 14, // right joystick, up to down
        AXIS_HAT_X    = 15, // DPAD left to right
        AXIS_HAT_Y    = 16, // DPAD up to down
        AXIS_LTRIGGER = 17, // left trigger 
        AXIS_RTRIGGER = 18  // right trigger
    };

    struct engine* engine = (struct engine*)app->userData;

    if (AInputEvent_getSource(event) & AINPUT_SOURCE_CLASS_POINTER)
        return 0; // ignore touch events

    int type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_KEY) {
        int key = AKeyEvent_getKeyCode(event);
        switch (key) {
            case AKEYCODE_BUTTON_L1:
                LOGI("Pressed L1");
                engine->state.x = -0.5;
                break;
            case AKEYCODE_BUTTON_R1:
                LOGI("Pressed R1");
                engine->state.x = 0.5;
                break;
            case AKEYCODE_DPAD_LEFT:
                LOGI("Pressed DPADL");
                engine->state.x = -0.5;
                break;
            case AKEYCODE_DPAD_RIGHT:
                LOGI("Pressed DPADR");
                engine->state.x = 0.5;
                break;
            case AKEYCODE_BUTTON_THUMBL:
                LOGI("Pressed LAS");
                engine->state.x = -0.5;
                break;
            case AKEYCODE_BUTTON_THUMBR:
                LOGI("Pressed RAS");
                engine->state.x = 0.5;
                break;
            default:
                break;
        }
        return 1;
    } else if (type == AINPUT_EVENT_TYPE_MOTION) {

        float las_l2r = AMotionEvent_getAxisValue(event, AXIS_X, 0);
        if (las_l2r == -1.0)
            engine->state.x = -0.5;
        else if (las_l2r == 1.0)
            engine->state.x = 0.5;

        float ras_l2r = AMotionEvent_getAxisValue(event, AXIS_Z, 0);
        if (ras_l2r == -1.0)
            engine->state.x = -0.5;
        else if (ras_l2r == 1.0)
            engine->state.x = 0.5;

        float dpad_l2r = AMotionEvent_getAxisValue(event, AXIS_HAT_X, 0);
        if (dpad_l2r == -1.0)
            engine->state.x = -0.5;
        else if (dpad_l2r == 1.0)
            engine->state.x = 0.5;

        float ltrigger = AMotionEvent_getAxisValue(event, AXIS_LTRIGGER, 0);
        if (ltrigger == 1.0)
            engine->state.x = -0.5;

        float rtrigger = AMotionEvent_getAxisValue(event, AXIS_RTRIGGER, 0);
        if (rtrigger == 1.0)
            engine->state.x = 0.5;

        LOGI("Motion event LAS:%f RAS:%f DPAD:%f LTRIG:%f RTRIG:%f", las_l2r, ras_l2r, dpad_l2r, ltrigger, rtrigger);
        return 1;
    }

    return 0;
}
#endif

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    engine.app = state;
#if INPUT
    state->onInputEvent = engine_handle_input;
    p_AMotionEvent_getAxisValue = reinterpret_cast<float (*)(const AInputEvent* motion_event, int32_t axis, size_t pointer_index)>(dlsym(RTLD_DEFAULT, "AMotionEvent_getAxisValue"));
#endif

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // Loop until all events are read, then continue to draw the next frame of animation.
        while ((ident=ALooper_pollAll(0, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        engine_draw_frame(&engine);
    }
}
