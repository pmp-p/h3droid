#include <stdio.h>
#include <stdint.h>
#include <jni.h>


#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
//#include <GLES/gl.h>

#include "logger.h"



static char *app_folder = "/data/data/u.r.p3d";
static int PyAPI_Ready = 0;

/* standard python3+ skeleton for an interpreter */
#include "/data/data/u.root/usr/src/projects/python-cffi/cpython3Xm.cpp"

/** support for faking a main() for interpreter and use pthread to run it
  * just before entering Py_Initialize "interpreter_prepare" will be called from thead
  *
  */
#include "/data/data/u.root/usr/src/projects/python-cffi/cpython_thread.cpp"


#undef LOG_TAG
#define LOG_TAG "PythonAPI-EGL"

#define OPENGLES_1 1
#undef OPENGLES_2
#include "panda3d/config_androiddisplay.h"


static ANativeWindow *window = 0;
static char app_ptr[32]= {0};
static char *argv[] = {"."};

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;

extern EXPORT_CLASS struct android_app* global_app;

static JavaVM *global_jvm = NULL;

jclass    jni_PandaActivity;
jmethodID jni_PandaActivity_readBitmapSize;
jmethodID jni_PandaActivity_readBitmap;

jclass   jni_BitmapFactory_Options;
jfieldID jni_BitmapFactory_Options_outWidth;
jfieldID jni_BitmapFactory_Options_outHeight;


/**
 * Returns a JNIEnv object for the current thread.  If it doesn't already
 * exist, attaches the JVM to this thread.
 */
JNIEnv *get_jni_env() {
  nassertr(global_jvm != NULL, NULL);
  JNIEnv *env = NULL;
  int status = global_jvm->GetEnv((void**) &env, JNI_VERSION_1_4);

  if (status < 0 || env == NULL) {
    LOG_ERROR("JVM is not available in this thread!\n");
    return NULL;
  }

  return env;
}

/**
 * Called by Java when loading this library.  Initializes the global class
 * references and the method IDs.
 */

jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    global_jvm = jvm;
    LOG_INFO("           ================ JNI_OnLoad ==============");
    JNIEnv *env = get_jni_env();
    assert(env != NULL);

    jni_PandaActivity = env->FindClass("u/r/p3d/EglRuntime");
    jni_PandaActivity = (jclass) env->NewGlobalRef(jni_PandaActivity);

    jni_PandaActivity_readBitmapSize = env->GetStaticMethodID(jni_PandaActivity,
                   "readBitmapSize", "(J)Landroid/graphics/BitmapFactory$Options;");

    jni_PandaActivity_readBitmap = env->GetStaticMethodID(jni_PandaActivity,
                   "readBitmap", "(JI)Landroid/graphics/Bitmap;");

    jni_BitmapFactory_Options = env->FindClass("android/graphics/BitmapFactory$Options");
    jni_BitmapFactory_Options = (jclass) env->NewGlobalRef(jni_BitmapFactory_Options);

    jni_BitmapFactory_Options_outWidth = env->GetFieldID(jni_BitmapFactory_Options, "outWidth", "I");
    jni_BitmapFactory_Options_outHeight = env->GetFieldID(jni_BitmapFactory_Options, "outHeight", "I");
    return JNI_VERSION_1_4;
}


/**
 * Called by Java when unloading this library.  Destroys the global class
 * references.
 */
void JNI_OnUnload(JavaVM *jvm, void *reserved) {
    LOG_INFO("           ================ JNI_OnUnLoad ==============");
    JNIEnv *env = get_jni_env();
    env->DeleteGlobalRef(jni_PandaActivity);
    env->DeleteGlobalRef(jni_BitmapFactory_Options);
}




extern "C" {

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStart(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnStart ==================== ");
        //renderer = new Renderer();
        if (!PyAPI_Ready){
            LOG_INFO("py_not_ready");
            return;
        }
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnResume(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnResume ==================== ");
        if (!PyAPI_Ready){
            LOG_INFO("py_not_ready");
            return;
        }
        //renderer_instance->start();
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnPause(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnPause ==================== ");
        if (!PyAPI_Ready){
            LOG_INFO("py_not_ready");
            return;
        }

        //renderer_instance->stop();
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStop(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnStop ==================== ");
        if (!PyAPI_Ready){
            LOG_INFO("py_not_ready");
            return;
        }
        //delete renderer_instance;
        //renderer_instance = 0;
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface)
    {
        LOG_INFO("    ===================  nativeSetSurface ==================== ");
        if (surface != 0) {

            window = ANativeWindow_fromSurface(jenv, surface);
            LOG_INFO("   @@@@@@@@@@@@ Got window %p  @@@@@@@@@@@", window);

            snprintf(app_ptr, 16, "%p", (void * )window );
            setenv("PANDA_NATIVE_WINDOW", app_ptr, 1);

            interpreter_launch(1,argv);

        } else {
            LOG_INFO("Releasing window");
            ANativeWindow_release(window);
        }

        return;
    }

    extern int interpreter_prepare()
    {
        void* window=0;
        char app_ptr[32]= {0};

        LOG_ERROR("#FIXME: PANDA_PRC_DIR / PANDA_PRC_PATH have not effect !");
        setenv("PANDA_PRC_DIR", "/data/data/u.r/etc", 1);
        setenv("PANDA_PRC_PATH", "/data/data/u.r/etc", 1);

        //RECEIVE
        sscanf( getenv("PANDA_NATIVE_WINDOW"), "%p", &window );
        LOG_INFO(" >>>>> PANDA_NATIVE_WINDOW pointer [ %p ] <<<<< ", window);

        EGLNativeWindowType _window = (EGLNativeWindowType)window;
        if ( window ){

            //renderer_instance->setWindow( (ANativeWindow*) window);
            LOG_INFO("Initializing context");
            const EGLint attribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_NONE
            };

            EGLConfig config;
            EGLint numConfigs;
            EGLint format;
            EGLint width;
            EGLint height;
            //GLfloat ratio;

            ANativeWindow_setBuffersGeometry(_window, 0, 0, format);

            if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
                LOG_ERROR("eglGetDisplay() returned error %d", eglGetError());
                return -1;
            }

            if (!eglInitialize(display, 0, 0)) {
                LOG_ERROR("eglInitialize() returned error %d", eglGetError());
                return -1;
            }

            LOG_INFO(" >>>>> display pointer set to %p <<<<< ", display);


            if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
                LOG_ERROR("eglChooseConfig() returned error %d", eglGetError());
                return -1;
            }

            if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
                LOG_ERROR("eglGetConfigAttrib() returned error %d", eglGetError());
                return -1;
            }


            //TRANSMIT
            snprintf(app_ptr, 16, "%p", (void * )display );
            setenv("PANDA_NATIVE_DISPLAY", app_ptr, 1);

            if (!(surface = eglCreateWindowSurface(display, config, _window, 0))) {
                LOG_ERROR("eglCreateWindowSurface() returned error %d", eglGetError());
                goto fail;
            }

            //TRANSMIT
            snprintf(app_ptr, 16, "%p", (void * )surface );
            setenv("PANDA_NATIVE_SURFACE", app_ptr, 1);

            if (!(context = eglCreateContext(display, config, 0, 0))) {
                LOG_ERROR("eglCreateContext() returned error %d", eglGetError());
                goto fail;
            }

            if (!eglMakeCurrent(display, surface, surface, context)) {
                LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
                goto fail;
            }

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
                LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
                goto fail;
            }

            //TRANSMIT
            snprintf(app_ptr, 16, "%p", (void * )context );
            setenv("PANDA_NATIVE_CONTEXT", app_ptr, 1);

            /*
            glDisable(GL_DITHER);
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
            glClearColor(0, 0, 0, 0);
            glEnable(GL_CULL_FACE);
            glShadeModel(GL_SMOOTH);
            glEnable(GL_DEPTH_TEST);

            glViewport(85, 60, width-85, height-60);

            ratio = (GLfloat) width / height;
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustumf(-ratio, ratio, -1, 1, 1, 10);
*/
            //PyAPI_Ready = 1;

            /**
             * Initializes the library.  This must be called at least once before any of
             * the functions or classes in this library can be used.  Normally, this is
             * called by JNI_OnLoad.
             */
            /*
              PNMFileTypeRegistry *tr = PNMFileTypeRegistry::get_global_ptr();
              PNMFileTypeAndroid::init_type();
              PNMFileTypeAndroid::register_with_read_factory();
              tr->register_type(new PNMFileTypeAndroid);
            */

            return 1;
fail:
        LOG_INFO("Destroying context");
        if (display){
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (context)
                eglDestroyContext(display, context);
            if (surface)
                eglDestroySurface(display, surface);
            eglTerminate(display);
        }
        return -1;

        }
    }
};
