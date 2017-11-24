#include <stdio.h>

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

#include "renderer.cpp"

#ifndef LOG_TAG
    #include "logger.h"
    #define LOG_TAG "EglRuntime"
#endif


static ANativeWindow *window = 0;

static char *app_folder = "/data/data/u.r.p3d";
static char app_ptr[32]= {0};
static char *argv[] = {"."};

#include "/data/data/u.root/usr/src/projects/python-cffi/cpython3Xm.cpp"
#include "/data/data/u.root/usr/src/projects/python-cffi/cpython_thread.cpp"

extern "C" {
    /*
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStart( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnResume( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnPause( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStop( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeSetSurface( JNIEnv* jenv, jobject obj, jobject surface );
    */

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStart(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnStart ==================== ");
        //renderer = new Renderer();
        if (!Python_Ready){
            LOG_INFO("py_not_ready");
            return;
        }
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnResume(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnResume ==================== ");
        if (!Python_Ready){
            LOG_INFO("py_not_ready");
            return;
        }
        renderer_instance->start();
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnPause(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnPause ==================== ");
        if (!Python_Ready){
            LOG_INFO("py_not_ready");
            return;
        }

        renderer_instance->stop();
        return;
    }

    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStop(JNIEnv* jenv, jobject obj)
    {
        LOG_INFO("    ===================  nativeOnStop ==================== ");
        if (!Python_Ready){
            LOG_INFO("py_not_ready");
            return;
        }
        delete renderer_instance;
        renderer_instance = 0;
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

            //renderer_instance->setWindow(window);
            // let python do ri_setWindow();



        } else {
            LOG_INFO("Releasing window");
            ANativeWindow_release(window);
        }

        return;
    }

};
