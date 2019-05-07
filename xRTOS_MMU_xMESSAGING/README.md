
# xRTOS ... PI 3 AARCH64 and PI 2,3 AARCH32
As per usual you can simply copy the files in the DiskImg directory onto a formatted SD card and place in Pi to test
>
Okay we have added only two functions but we will now have a lot happening in this example.

So our first function we have added is
##### void xTaskWaitOnMessage (const RegType_t userMessageID);
So with this call we can ask a task to wait via a unique message ID we will provide. The task will remove itself from the ready list and insert itself in the "waiting for message" list. In doing so it stops receiving any CPU time so consumes no CPU time while it waits.

Our second function we have added is
##### void xTaskReleaseMessage(const RegType_t userMessageID);

This function first looks in the current core "waiting for message" list and if it finds a task with that unique ID it will return that task to the ready list to again resume processing. If the task is not found it repeats the same test in the other core "waiting for message" lists. So this is our first example of a real cross core communication.

So on the example we define two unique ID's
~~~
#define WAIT_TASK1  0x1
#define WAIT_TASK2  0x2
~~~
On the Red bar on core 0 it draws it's position and then calls
~~~
xTaskWaitOnMessage(WAIT_TASK1);
~~~
So it now simply waits consuming no CPU power. The light blue bar Task1A draws it's position, does it's timed wait and then releases the RED task via
~~~
xTaskReleaseMessage(WAIT_TASK1);
~~~
So the result is the RED bar moves at exactly the same rate as the light blue bar.

#### This is an example of task to task messaging on the same core.

Now task2 which is the dark blue bar on core1 acts similar to the red bar it draws it's position and then calls
~~~
xTaskWaitOnMessage(WAIT_TASK2);
~~~
Now we use task 3 which is the green bar task on core 2 to this time release the task after it has drawn itself and waited a fixed time with
~~~
xTaskReleaseMessage(WAIT_TASK2);
~~~
So now the blue bar and the green bar move in lock step.

#### This is an example of task to task messaging on the different cores.

It should be obvious we can now at least synchronize tasks both on the same core and across cores.

There is actually a problem with the current setup that the core releasing a task is accessing the data of another core so it needs to be lockless or primitive synched. It will be fine at the moment simply because there is only one message lock on any given core. What we really want to do is get the other core itself to search and release the task by unique message ID. That way no core accesses another cores data which is a nice partition to have.

So to do that we are going to use the 32bit hardware intercore mailbox as described in the errata datasheet QA7
https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf

So we will pick that concept up in the next example and then having got inter core communication established we will look at an L1/L2 scheduler as the cores can synchronize when required.
