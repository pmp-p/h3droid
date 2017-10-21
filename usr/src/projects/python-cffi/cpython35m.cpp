
#define PYTHON_PATH "/data/data/u.r/usr/lib/python3.5"
static char *root_folder = "/data/data/u.r";

#define LIB_PYTHON "libpython3.5m.so"



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


//static void* lib_python = 0;
//static char lib_python_path[] = "libpython3.5m.so";
//typedef void (*PyDict_NewPtr)();
//PyDict_NewPtr LD_PyDict_New = 0;

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


static struct PyModuleDef androidembed = {PyModuleDef_HEAD_INIT, "androidembed",
                                          "", -1, AndroidEmbedMethods};

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
    char *env_logname = "Python3.5";
    char str_cp[513];
    int ret = 0;
    FILE *fd;

    str_cp[512]=0;

  /* AND: Several filepaths are hardcoded here, these must be made
     configurable */
  /* AND: P4A uses env vars...not sure what's best */
    LOGP("Initialize Python for Android");

    setenv("PANDA_PRC_PATH", "/data/data/u.r/etc", 1);
    setenv("XDG_CONFIG_HOME", "/data/data/u.r/XDG_CONFIG_HOME", 1);
    setenv("XDG_CACHE_HOME", "/data/data/u.r/XDG_CACHE_HOME", 1);
    setenv("PYTHON_NAME", "python", 1);
    setenv("PYTHONPATH","/data/data/u.root/usr/share/python3",1);


    Py_SetProgramName(L"python3.5");

 /* our logging module for android */

    PyImport_AppendInittab("androidembed", initandroidembed);

    LOGP("Preparing to initialize python");

    snprintf(str_cp, 256, "%s/usr/lib/python3.5", root_folder );

    if (!dir_exists(str_cp)) {
        LOGP("stdlib not found in:");
        LOGP( str_cp);
        return -1;
    }


    LOGP("crystax python folder exists");
    char paths[256];
    snprintf(paths, 512,
         "%s/stdlib.zip:%s:%s/lib-dynload:%s/site-packages:%s/usr/lib/python3.5:%s/usr/lib/python3",
         PYTHON_PATH,PYTHON_PATH, PYTHON_PATH, PYTHON_PATH, PYTHON_PATH, root_folder);
    LOGP("calculated paths to be...");
    LOGP(paths);



    wchar_t *wchar_paths = Py_DecodeLocale(paths, NULL);

    LOGP("set wchar paths...");

    Py_SetPath(wchar_paths);


    Py_Initialize();

    LOGP("Initialized python");

    /* ensure threads will work. */
    LOGP("<InitThreads>");
    PyEval_InitThreads();
    LOGP("</InitThreads>");

    PyRun_SimpleString("import androidembed\nandroidembed.log('testing python print redirection')");


    /* inject our bootstrap code to redirect python stdin/stdout replace sys.path with our path */



    PyRun_SimpleString(
        "import sys\n"
        "sys.path.insert(0,'.')\n"
        "sys.argv=[]\n"
    );

    snprintf(str_cp, 256, "sys.path.append('''%s/usr/lib/python3''')", root_folder);
    PyRun_SimpleString(str_cp);

    snprintf(str_cp, 256, "sys.path.append('''%s/site-packages''')", PYTHON_PATH );
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
        return -1;
    }

    /* exec the file */
    fd = fopen( str_cp, "r");
    if (fd == NULL) {
        LOGP("faild to open the entrypoint :");
        LOGP(str_cp);
        return -1;
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

  /* close everything  */
    Py_Finalize();
    fclose(fd);

    LOGP("Python for android ended.");

    return ret;

}


#ifdef INTERPRETER
int main(int argc,char *argv[]){
    return interpreter_main(argc,argv);
}
#endif











