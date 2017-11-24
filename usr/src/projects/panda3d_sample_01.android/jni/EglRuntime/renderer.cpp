//
// Copyright 2011 Tero Saarni
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <android/native_window.h> // requires ndk r5 or newer
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>

#include <pthread.h>
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>


#ifndef LOG_TAG
    #include "logger.h"
#endif

#define LOG_TAG "EglRenderer"

#define OPENGLES_1
#include "panda3d/config_androiddisplay.h"


class Renderer {

public:
    Renderer();
    virtual ~Renderer();

    // Following methods can be called from any thread.
    // They send message to render thread which executes required actions.
    void start();
    void stop();
    void setWindow(ANativeWindow* window);


private:

    enum RenderThreadMessage {
        MSG_NONE = 0,
        MSG_WINDOW_SET,
        MSG_RENDER_LOOP_EXIT
    };

    pthread_t _threadId;
    pthread_mutex_t _mutex;
    enum RenderThreadMessage _msg;

    // android window, supported by NDK r5 and newer
    ANativeWindow* _window;

    EGLDisplay _display;
    EGLSurface _surface;
    EGLContext _context;
    GLfloat _angle;

    // RenderLoop is called in a rendering thread started in start() method
    // It creates rendering context and renders scene until stop() is called
    void renderLoop();

    bool initialize();
    void destroy();

    void drawFrame();

    // Helper method for starting the thread
    static void* threadStartCallback(void *myself);

};


static GLint vertices[][3] = {
    { -0x10000, -0x10000, -0x10000 },
    {  0x10000, -0x10000, -0x10000 },
    {  0x10000,  0x10000, -0x10000 },
    { -0x10000,  0x10000, -0x10000 },
    { -0x10000, -0x10000,  0x10000 },
    {  0x10000, -0x10000,  0x10000 },
    {  0x10000,  0x10000,  0x10000 },
    { -0x10000,  0x10000,  0x10000 }
};

static GLint colors[][4] = {
    { 0x00000, 0x00000, 0x00000, 0x10000 },
    { 0x10000, 0x00000, 0x00000, 0x10000 },
    { 0x10000, 0x10000, 0x00000, 0x10000 },
    { 0x00000, 0x10000, 0x00000, 0x10000 },
    { 0x00000, 0x00000, 0x10000, 0x10000 },
    { 0x10000, 0x00000, 0x10000, 0x10000 },
    { 0x10000, 0x10000, 0x10000, 0x10000 },
    { 0x00000, 0x10000, 0x10000, 0x10000 }
};

GLubyte indices[] = {
    0, 4, 5,    0, 5, 1,
    1, 5, 6,    1, 6, 2,
    2, 6, 7,    2, 7, 3,
    3, 7, 4,    3, 4, 0,
    4, 7, 6,    4, 6, 5,
    3, 0, 1,    3, 1, 2
};


Renderer::Renderer()
    : _msg(MSG_NONE), _display(0), _surface(0), _context(0), _angle(0)
{
    LOG_INFO("Renderer instance created");
    pthread_mutex_init(&_mutex, 0);
    return;
}

Renderer::~Renderer()
{
    LOG_INFO("Renderer instance destroyed");
    pthread_mutex_destroy(&_mutex);
    return;
}

void Renderer::start()
{
    LOG_INFO("Creating renderer thread");
    pthread_create(&_threadId, 0, threadStartCallback, this);
    return;
}

void Renderer::stop()
{
    LOG_INFO("Stopping renderer thread");

    // send message to render thread to stop rendering
    pthread_mutex_lock(&_mutex);
    _msg = MSG_RENDER_LOOP_EXIT;
    pthread_mutex_unlock(&_mutex);

    pthread_join(_threadId, 0);
    LOG_INFO("Renderer thread stopped");

    return;
}

void Renderer::setWindow(ANativeWindow *window)
{
    // notify render thread that window has changed
    pthread_mutex_lock(&_mutex);
    _msg = MSG_WINDOW_SET;
    //_window = window;
    pthread_mutex_unlock(&_mutex);
    return;
}



void Renderer::renderLoop()
{
    bool renderingEnabled = true;

    LOG_INFO("renderLoop()");

    while (renderingEnabled) {

        pthread_mutex_lock(&_mutex);

        // process incoming messages
        switch (_msg) {

            case MSG_WINDOW_SET:
                initialize();
                break;

            case MSG_RENDER_LOOP_EXIT:
                renderingEnabled = false;
                //destroy();
                break;

            default:
                break;
        }
        _msg = MSG_NONE;

        if (_display) {
            drawFrame();
            if (!eglSwapBuffers(_display, _surface)) {
                LOG_ERROR("eglSwapBuffers() returned error %d", eglGetError());
            }
        }

        pthread_mutex_unlock(&_mutex);
    }

    LOG_INFO("Render loop exits");

    return;
}

bool Renderer::initialize()
{


    return true;
}



void Renderer::drawFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3.0f);
    glRotatef(_angle, 0, 1, 0);
    glRotatef(_angle*0.25f, 1, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glFrontFace(GL_CW);
    glVertexPointer(3, GL_FIXED, 0, vertices);
    glColorPointer(4, GL_FIXED, 0, colors);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

    _angle += 1.2f;
}

void* Renderer::threadStartCallback(void *myself)
{
    Renderer *renderer = (Renderer*)myself;

    renderer->renderLoop();
    pthread_exit(0);

    return 0;
}


static Renderer *renderer_instance = new Renderer();

static int Python_Ready = 0;

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;


extern "C"
{
    extern int interpreter_prepare()
    {
        void* window=0;
        char* wenv;
        char app_ptr[32]= {0};

        wenv= getenv("PANDA_NATIVE_WINDOW");
        LOG_INFO(" >>>>> window env %s found <<<<< ", wenv);
        sscanf( wenv, "%p", &window );
        LOG_INFO(" >>>>> window pointer %p found <<<<< ", window);
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
            GLfloat ratio;



            ANativeWindow_setBuffersGeometry(_window, 0, 0, format);


            if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
                LOG_ERROR("eglGetDisplay() returned error %d", eglGetError());
                return -1;
            }

            if (!eglInitialize(display, 0, 0)) {
                LOG_ERROR("eglInitialize() returned error %d", eglGetError());
                return -1;
            }

            LOG_INFO(" >>>>> display pointer set <<<<< ", display);


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
                return -1;
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
                return -1;
            }

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
                LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
                return -1;
            }

            //TRANSMIT
            snprintf(app_ptr, 16, "%p", (void * )context );
            setenv("PANDA_NATIVE_CONTEXT", app_ptr, 1);

            glDisable(GL_DITHER);
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
            glClearColor(0, 0, 0, 0);
            glEnable(GL_CULL_FACE);
            glShadeModel(GL_SMOOTH);
            glEnable(GL_DEPTH_TEST);

            glViewport(0, 0, width, height);

            ratio = (GLfloat) width / height;
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustumf(-ratio, ratio, -1, 1, 1, 10);

            //init_libandroiddisplay();
            //Python_Ready = 1;
            //renderer_instance->start();


            return 1;
fail:
        LOG_INFO("Destroying context");

        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return -1;

        }
    }
}












































//
