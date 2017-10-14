
static char *root_folder = "/data/data/u.r";


#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include <pthread.h>

#include <dlfcn.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <jni.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "android/log.h"
#define LOG(n, x) __android_log_write(ANDROID_LOG_INFO, (n), (x))
#define LOGP(x) LOG("python", (x))

#define ENTRYPOINT_MAXLEN 128


// This will be used to pass some data to the sub interpreter
struct thread_data_args
{
    pthread_t thread_id;
    int  vint;
    bool vbool;
    char *pchar;
};


/*
============================================================================================
*/

static PyObject *androidembed_log(PyObject *self, PyObject *args) {
    char *logstr = NULL;
    if (!PyArg_ParseTuple(args, "s", &logstr)) {
        return NULL;
    }
    LOG(getenv("PYTHON_NAME"), logstr);
    Py_RETURN_NONE;
}

static PyMethodDef AndroidEmbedMethods[] = {
    {"log", androidembed_log, METH_VARARGS, "Log on android platform"},
    {NULL, NULL, 0, NULL}};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef androidembed = {PyModuleDef_HEAD_INIT, "androidembed",
                                          "", -1, AndroidEmbedMethods};

PyMODINIT_FUNC initandroidembed(void) {
    return PyModule_Create(&androidembed);
}
#else
PyMODINIT_FUNC initandroidembed(void) {
    (void)Py_InitModule("androidembed", AndroidEmbedMethods);
}
#endif

int dir_exists(char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        if (S_ISDIR(st.st_mode))
          return 1;
        }
    return 0;
}

int file_exists(const char *filename) {
    FILE *file;
    if (file = fopen(filename, "r")) {
        fclose(file);
        return 1;
    }
    return 0;
}

int interpreter_main(int argc, char *argv[] ){
    char *env_logname = "Python3.5";
    int ret = 0;
    FILE *fd;

  /* AND: Several filepaths are hardcoded here, these must be made
     configurable */
  /* AND: P4A uses env vars...not sure what's best */
    LOGP("Initialize Python for Android");


    setenv("PYTHON_NAME", "python", 1);

    chdir(root_folder);

    Py_SetProgramName(L"python3.5");

 /* our logging module for android */

    PyImport_AppendInittab("androidembed", initandroidembed);

    LOGP("Preparing to initialize python");

    if (dir_exists("./usr/lib/python3.5")) {
        LOGP("crystax python folder exists");
        char paths[256];
        snprintf(paths, 256,
                 "%s/usr/lib/python3.5/stdlib.zip:%s/usr/lib/python3.5/modules",
                 root_folder, root_folder);
        LOGP("calculated paths to be...");
        LOGP(paths);

        wchar_t *wchar_paths = Py_DecodeLocale(paths, NULL);
        Py_SetPath(wchar_paths);

        LOGP("set wchar paths...");
    } else {
        LOGP("crystax_python does not exist");
    }

    Py_Initialize();

    LOGP("Initialized python");

  /* ensure threads will work. */
    LOGP("<InitThreads>");
    PyEval_InitThreads();
    LOGP("</InitThreads>");

    PyRun_SimpleString("import androidembed\nandroidembed.log('testing python print redirection')");

  /* inject our bootstrap code to redirect python stdin/stdout replace sys.path with our path */
    PyRun_SimpleString("import sys, posix\n");


    if (dir_exists("./usr/lib/python3.5")) {
        char add_site_packages_dir[256];
        snprintf(add_site_packages_dir, 256, "sys.path.append('%s/./usr/lib/python3.5/site-packages')", root_folder);

        PyRun_SimpleString("import sys\n"
                           "sys.argv = ['notaninterpreterreally']\n"
                           "from os.path import realpath, join, dirname");

        PyRun_SimpleString(add_site_packages_dir);
        /* "sys.path.append(join(dirname(realpath(__file__)), 'site-packages'))") */
        PyRun_SimpleString("sys.path = ['.'] + sys.path");
    }

    PyRun_SimpleString(
      "class LogFile(object):\n"
      "    def __init__(self):\n"
      "        self.buffer = ''\n"
      "    def write(self, s):\n"
      "        s = self.buffer + s\n"
      "        lines = s.split(\"\\n\")\n"
      "        for l in lines[:-1]:\n"
      "            androidembed.log(l)\n"
      "        self.buffer = lines[-1]\n"
      "    def flush(self):\n"
      "        return\n"
      "sys.stdout = sys.stderr = LogFile()\n"
      "print('Android path', sys.path)\n"
      "import os\n"
      "print('os.environ is', os.environ)\n"
      "print('python bootstrap done. __name__ is', __name__)");

    LOGP("AND: Ran string");

 /* run it !  */

    LOGP("Run user program, change dir and execute entrypoint");

 /* Get the entrypoint */

    fd = fopen("main.py", "r");
    if (fd == NULL) {
        LOGP("Open the entrypoint failed");
        goto abort_main;
    }

  /* run python !   */

    ret = PyRun_SimpleFile(fd, "main.py" );

    if (PyErr_Occurred() != NULL) {
        ret = 1;
        PyErr_Print(); /* This exits with the right code if SystemExit. */
        PyObject *f = PySys_GetObject("stdout");

         /* python2 used Py_FlushLine, but this no longer exists */
        if (PyFile_WriteString("\n", f))
            PyErr_Clear();
    }

  /* close everything  */
    Py_Finalize();
    fclose(fd);

    LOGP("Python for android ended.");

abort_main:
    return ret;
  //  pthread_exit(0);
}



/*
============================================================================================
*/

void * interpreter_thread(void *args) {
    struct thread_data_args *thread_args = (struct thread_data_args*)args;

    if (thread_args->vbool == true)
    {
        printf("VALUE= %i", thread_args->vint);
    }

    interpreter_main( thread_args->vint , &thread_args->pchar );

abort_thread:
    pthread_exit(0);
}







/*
============================================================================================
*/

int interpreter_launch(int argc, char *argv[]){

    struct thread_data_args *thread_args;
    thread_args = (thread_data_args*)malloc(sizeof(*thread_args));

    thread_args->vint  = 1;
    thread_args->vbool = true;
    thread_args->pchar = (char *)argv;

    // data/data/u.root/usr/lib/python3.5/stdlib.zip";
    void* core = 0;


    char corepath[] = "libpython3.5m.so";

    core = dlopen(corepath, RTLD_LAZY);

    if (core == 0) {
        const char* lasterr = dlerror();
        LOGP( "Fatal Python error: cannot load library") ;
    } else {
        LOGP("interpreter_launch: starting 1 python instance");
        pthread_create(&thread_args->thread_id, NULL, interpreter_thread, (void*)thread_args);
    }
    return 0;
}














