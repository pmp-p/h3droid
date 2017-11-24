#include <pthread.h>


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

    void* core = 0;

    char corepath[] = LIB_PYTHON;
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




