/*
#ifndef PYVER
    #include "cpython35m.h"
#endif
*/

#ifndef root_folder
    #warning  ============== using hardcoded app path  ==============
    static char *root_folder = "/data/data/u.r";
#endif

#define PY_SSIZE_T_CLEAN
#include "Python.h"

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

#include <locale.h>

#ifndef INTERPRETER
    extern "C" int interpreter_prepare();
#endif

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
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef androidembed = {PyModuleDef_HEAD_INIT, "androidembed", NULL, -1, AndroidEmbedMethods};

PyMODINIT_FUNC initandroidembed(void) {
    return PyModule_Create(&androidembed);
}

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
    char *env_logname = PY_LOG;
    char str_cp[513];
    int ret = 0;
    FILE *fd = NULL;

    str_cp[512]=0;

#ifndef INTERPRETER
    LOGP("Preparing Embed for Python3 for Android");
    if (interpreter_prepare()<0)
        return -1;
#endif

    LOGP("Initializing Python3 for Android");

    setenv("XDG_CONFIG_HOME", "/data/data/u.r/XDG_CONFIG_HOME", 1);
    setenv("XDG_CACHE_HOME", "/data/data/u.r/XDG_CACHE_HOME", 1);
    setenv("PYTHONDONTWRITEBYTECODE","1",1);
    setenv("PYTHON_NAME", "python-runtime", 1);
    setenv("LD_LIBRARY_PATH","/data/data/u.root/lib-armhf:/data/data/u.r/lib-armhf:/vendor/lib:/system/lib",1);
    setenv("PYTHON_HOME", "/data/data/u.r/usr", 1);
    setenv("PYTHONPATH",PYTHONPATH,1);
    setenv("PYTHONCOERCECLOCALE","1",1);

    Py_SetProgramName(L"./python3");

 /* our logging module for android */

#if PYVER > 36
    //setlocale(LC_ALL, "C.UTF-8");
#endif

    LOGP("ADDING: PyImport_AppendInittab(androidembed, initandroidembed);");
    PyImport_AppendInittab("androidembed", initandroidembed);


    LOGP("Preparing to initialize python");

    if (!dir_exists(PY_LIB)) {
        LOGP("stdlib not found in:");
        LOGP( PY_LIB );
        return -1;
    }


    LOGP("Python stdlib folder found at :");
    LOGP(PY_LIB);
    char paths[256];

    snprintf(paths, 512,
         "%s/stdlib.zip:%s:%s/lib-dynload:%s/site-packages:%s",
         PY_PATH,PY_PATH, PY_PATH, PY_PATH, PY_PATH, PY_LIBS);

    LOGP("calculated paths to be...");
    LOGP(paths);

    wchar_t *wchar_paths = Py_DecodeLocale(paths, NULL);
    LOGP("Setting wchar paths...");
    Py_SetPath(wchar_paths);

    LOGP("Initialize python");
    Py_Initialize();

    LOGP("Initialized python");

    /* ensure threads will work. */
    LOGP("<InitThreads>");
    PyEval_InitThreads();
    LOGP("</InitThreads>");

    LOGP("<LogTest>");
    PyRun_SimpleString("import androidembed\nandroidembed.log('    testing python print redirection')");
    LOGP("</LogTest>");

    /* inject our bootstrap code to redirect python stdin/stdout replace sys.path with our path */

    PyRun_SimpleString(
        "import sys\n"
        "sys.path.insert(0,'.')\n"
        "sys.argv=[]\n"
    );

    snprintf(str_cp, 256, "sys.path.append('''%s/usr/lib/python3''')", root_folder);
    PyRun_SimpleString(str_cp);

    snprintf(str_cp, 256, "sys.path.append('''%s/site-packages''')", PY_PATH );
    PyRun_SimpleString(str_cp);

    int i;
    for (i=0;i<argc;i++){
        snprintf(str_cp, 256, "sys.argv.append('''%s''')", argv[i]);
        PyRun_SimpleString(str_cp);
    }


    /* run python !   */


#ifdef INTERPRETER
    setenv("PANDA_NATIVE_WINDOW","0",1);

    PyRun_SimpleString(
        "print('interpreter:',sys.argv)\n"
    );

    if (argc>1){
        fd = fopen( argv[1], "r");
        if (fd == NULL) {
            LOGP("faild to open the entrypoint :");
            LOGP(argv[1]);
            return -1;
        }

        ret = PyRun_SimpleFile(fd, "__main__" );
    }

#else

    chdir(root_folder);

    PyRun_SimpleString(
      "sys.argv = ['notaninterpreter']\n"
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

    /* Insert the script entrypoint in argv */
    snprintf(str_cp, 512, "sys.argv.insert(1,'''%s/%s''')", root_folder , "main.py");
    PyRun_SimpleString( str_cp );

    LOGP("Run user program, change dir and execute entrypoint :");
    snprintf(str_cp, 512, "%s/%s", root_folder , "main.py");

    LOGP( str_cp);

    if (!file_exists(str_cp)) {
        LOGP("entrypoint script not found !");
        LOGP( str_cp);
        ret = 1;
        goto done;
    }

    /* exec the file */
    fd = fopen( str_cp, "r");
    if (fd == NULL) {
        LOGP("faild to open the entrypoint :");
        LOGP(str_cp);
        ret = 1;
        goto done;
    }

    LOGP("Running entry point :");
    ret = PyRun_SimpleFile(fd, str_cp );
#endif

    if (PyErr_Occurred() != NULL) {
        ret = 1;
        PyErr_Print(); /* This exits with the right code if SystemExit. */
        PyObject *f = PySys_GetObject("stdout");

         /* python2 used Py_FlushLine, but this no longer exists */
        if (PyFile_WriteString("\n", f))
            PyErr_Clear();
    }

done:

  /* close everything  */
    Py_Finalize();
    if (fd)
        fclose(fd);

    LOGP("Python for android ended.");

    return ret;

}


#ifdef INTERPRETER
int main(int argc,char *argv[]){
    return interpreter_main(argc,argv);
}
#endif











