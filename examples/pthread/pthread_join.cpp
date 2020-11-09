

#include <iostream>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void * threadFunc(void * arg)
{
    std::cout << "Thread Function :: Start" << std::endl;
    // Sleep for 2 seconds
    sleep(2);
    std::cout << "Thread Function :: End" << std::endl;
    // Return value from thread
    return new int(6);
}
int main()
{
    // Thread id
    pthread_t threadId;
    // Create a thread that will funtion threadFunc()
    int err = pthread_create(&threadId, NULL, &threadFunc, NULL);
    // Check if thread is created sucessfuly
    if (err)
    {
        std::cout << "Thread creation failed : " << strerror(err);
        return err;
    }
    else
        std::cout << "Thread Created with ID : " << threadId << std::endl;
    // Do some stuff
    void * ptr = NULL;
    std::cout << "Waiting for thread to exit" << std::endl;
    // Wait for thread to exit
    err = pthread_join(threadId, &ptr);
    if (err)
    {
        std::cout << "Failed to join Thread : " << strerror(err) << std::endl;
        return err;
    }
    if (ptr)
        std::cout << " value returned by thread : " << *(int *) ptr
                << std::endl;
    delete (int *) ptr;
    return 0;
}