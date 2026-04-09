#include "threading.h"

#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* args = (struct thread_data *) thread_param;
    sleep(args->wait_to_obtain_ms);
    int rc = pthread_mutex_lock(args->mutex);
    if ( rc != 0 ) {
        ERROR_LOG("pthread_mutex_lock failed with %d\n",rc);
    } else {
        sleep(args->wait_to_release_ms);
        args->thread_complete_success = true;
        rc = pthread_mutex_unlock(args->mutex);
        if ( rc != 0 ) {
            ERROR_LOG("pthread_mutex_unlock failed with %d\n",rc);
            args->thread_complete_success = false;
        }
    }
    DEBUG_LOG("threadfunc - thread_complete_success %d\n", args->thread_complete_success);
    return args;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if (data == NULL) {
        return false;
    }
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->mutex = mutex;
    data->thread_complete_success = false;

    int rc = pthread_create(thread, NULL, threadfunc, data);
    if ( rc != 0 ) {
        ERROR_LOG("pthread_create failed with error %d creating thread %lu", rc, (unsigned long)thread);
        free(data);
        return false;
    }
    DEBUG_LOG("Thread started with id %lu", (unsigned long)thread);

    return true;
}

