#include <stdio.h>

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

#include "jniapi.h"
#include "logger.h"
#include "renderer.h"

#define LOG_TAG "EglRuntime"


static ANativeWindow *window = 0;
static Renderer *renderer = 0;

static char *app_folder = "/data/data/u.r.p3d";

#include "cpython35m.h"


JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStart(JNIEnv* jenv, jobject obj)
{
    char *argv[] = {"."};

    LOG_INFO("    ===================  nativeOnStart ==================== ");
    interpreter_launch(1,argv);

exit:
    renderer = new Renderer();
    return;
}

JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnResume(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnResume");
    renderer->start();
    return;
}

JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnPause(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnPause");
    renderer->stop();
    return;
}

JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStop(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnStop");
    delete renderer;
    renderer = 0;
    return;
}

JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface)
{
    if (surface != 0) {



        window = ANativeWindow_fromSurface(jenv, surface);
        LOG_INFO("Got window %p", window);
        renderer->setWindow(window);



    } else {
        LOG_INFO("Releasing window");
        ANativeWindow_release(window);
    }

    return;
}

