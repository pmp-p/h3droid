#ifndef JNIAPI_H
#define JNIAPI_H

extern "C" {
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStart( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnResume( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnPause( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeOnStop( JNIEnv* jenv, jobject obj );
    JNIEXPORT void JNICALL Java_u_r_p3d_EglRuntime_nativeSetSurface( JNIEnv* jenv, jobject obj, jobject surface );
};

#endif // JNIAPI_H
