
# xRTOS ... PI 3 AARCH64 and PI 2,3 AARCH32
As per usual you can simply copy the files in the DiskImg directory onto a formatted SD card and place in Pi to test
>
![](https://github.com/LdB-ECM/Docs_and_Images/blob/master/Images/xRTOS_SEMS.jpg?raw=true)
>
So in this code we begin our work with synchronizing primitives, we start here with a simple Binary Semaphore.

A binary semaphore is a synchronization object that can have only two states:
1. Not taken.
2. Taken.

Taking a binary semaphore brings it in the “taken” state. Trying to take a semaphore that is already taken will suspend the calling task indefinitely until the semaphore is "given" back. "Giving" a semaphore that is in the not taken state has no effect it will harmless pass thru.
>
So we extend our start example so each task1 on each core will increment a simple counter. We wish to use printf to display that count but only 1 core at a time can use printf otherwise it scrambles the various outputs.
>
So we create a screen semaphore at the very start of main.
>
```
static SemaphoreHandle_t screenSem;
screenSem = xSemaphoreCreateBinary();
```  
>
Now before each task prints, the task takes the screen Semaphore. Thus if more than one core goes for the semaphore at the same time, one core will be granted access the other will queue up waiting for the semaphore to be give back. The task that has the semaphore moves to the screen position and prints it's count. Finally that task will give the semaphore back, at which point the waiting core will be able to take the semaphore and so it enters it routine and prints.
>
The net result is each core can use the printf function to display it's count without conflict.
>
Now there is an obvious problem that any task waiting to take the binary semaphore is still in the readylist and thus it using CPU power to basically sit in a loop waiting for the semaphore to be given back. As we only have 1 printf line of a simple integer the wait processing power is barely noticeable. However on complex samples that CPU time wasting could be significant. What we really want is a task waiting to take a binary semaphore to be taken from the readylist so it cost no CPU load (like vTaskDelay does). The task that has the binary semaphore as it gives the binary semaphore back should signal the waiting task effectively putting it back in the readyList so it can then run it's printf.
>
So that is our next step to organize synchronization primitives that includes signaling.
>
You will note we have at this stage still left out task priority. If we are going to have priority to the tasks then we will also need priority to the synchronization primitives. If multiple tasks are waiting for a resource then it would usually follow the highest priority task waiting should be given it first. An alternative approach might be when requesting a resource an independent resource priority is given to allow task priority and resource priority to differ. Whatever the case the moment we introduce priority we must consider it everywhere.
