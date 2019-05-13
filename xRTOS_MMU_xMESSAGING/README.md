
# xRTOS ... PI 3 AARCH64 and PI 2,3 AARCH32
As per usual you can simply copy the files in the DiskImg directory onto a formatted SD card and place in Pi to test
>
Okay we have added only two functions but we will now have a lot happening in this example.

So our first function we have added is
##### void xTaskWaitOnMessage (const RegType_t userMessageID);
So with this call we can ask a task to wait via a unique message ID we will provide. The task will remove itself from the ready list and insert itself in the "waiting for message" list. In doing so it stops receiving any CPU time so consumes no CPU time while it waits. So it acts a lot like xTaskDelay but is released by a message usually from another task rather than a period of time.

Our second function we have added is
##### void xTaskReleaseMessage(const RegType_t userMessageID);

This function first looks in the current core "waiting for message" list and if it finds a task with that unique ID it will return that task to the ready list to again resume processing. If the task is not found it sends an IPC message to other cores for them the check their "waiting for message" lists. So this is our first example of a real cross core communication.

On the Pi as defined via QA7 each core has 4 32bit hardware intercore mailbox hardware as described in the errata datasheet QA7
https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf

 For our IPC message we have connected mailbox 0 to the FIQ interrupt of that core. So the act of writing to mailbox 0 of core 0 will generate an FIQ on core 0, writing to mailbox 0 of core 1 will generate an FIQ on core 1 etc. So xTaskReleaseMessage if it can not find the message in it's core will write the message ID to mailbox0 of the other 3 cores.

To do this we need a semaphore for mailbox0 because multiple tasks could be trying to Release Tasks at the same time. So when a task wishes to send an IPC message it must first take the mailbox0 semaphore for the core it is sending to. Once it has the semaphore it sends the message and exits. The core that recieves the message will give the semaphore back and so any waiting tasks can then post their message.

So here we a have the example of a low level semaphore providing protection for a much higher level IPC communication.


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

So we now have some basic inter core communication established we will next look at an L1/L2 scheduler as the cores can synchronize when required.
