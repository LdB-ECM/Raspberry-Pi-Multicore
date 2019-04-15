
# xRTOS ... PI 3 AARCH64 and PI 2,3 AARCH32
As per usual you can simply copy the files in the DiskImg directory onto a formatted SD card and place in Pi to test 
>
If you need assistance with how to compile the code
>
https://github.com/LdB-ECM/Docs_and_Images/blob/master/Documentation/Multicore_Build.md
>
The current code is technically just 4 cores indpendantly running although it shares some characteristics of what system that would be called SMP and BMP systems. All 4 cores are running on the same address space executing code (there is only one copy of the task switcher in memory) which is a characteristic of SMP systems. Tasks are manually assigned to each core and they are forever bound to that core which is a characteristic of BMP systems. You could make it take on AMP characteristics by copying the execution code to multiple memory locations and get each core to run in it's own memory area. The point being made is you can adapt the switcher itself to any of the more common multicores schemes or even hybrids between them.
>
So in this example we have 4 cores running round robin schedulers each with there own independant scheduler.
>
![](https://github.com/LdB-ECM/Docs_and_Images/blob/master/Images/xRTOS-Schedulers.jpg?raw=true)
>
In later examples we will play with a single centralized scheduler which should be obious and also a more complex L1/L2 scheduler which is shown in the diagram below
>
![](https://github.com/LdB-ECM/Docs_and_Images/blob/master/Images/xRTOS_L1_and_L2_scheduler.jpg?raw=true)
>
Now currently the task switcher is operating on a fixed tick cycle but it is important to realize it does not have to be that way. At the end of each interrupt the EL0 timer is set to interrupt by a delay period value. If you know your task characteristics you can load the value with the expected task duration and run in what is usually called a dynamic tick switcher. So that would involve writing a different delay value in EL0_Timer_Set in the code below
>
~~~
void xTickISR(void)
{
	xTaskIncrementTick();                   // Run the timer tick
	xSchedule();				// Run scheduler selecting next task 
	EL0_Timer_Set(m_nClockTicksPerHZTick);	// Set EL0 timer again for timer tick period
}
~~~
>
The task creation and execution should be fairly obvious but what may not be as obvious is xTaskDelay which suspends or holds a task from executing for a given period. The key to it's operation is to realize that the round robin scheduler only works on the tasks in the readyTasks list. What xTaskDelay does is take the task from that list and instead places the task in the delayedTasks list effectively meaning it does not get any processor time to run as it is not in the readyTasks list. Along with placing the task in the delayedTasks list the code writes a time based on the system tick timer at which to release the task. The system tick handler code when each tick occurs checks if there is any delaytasks and if so checks if it needs to release them. To release a task from delayed it simply removes the task from the delayTasks list and puts it back in the readyTasks list. So on the code a task is either in the readyTasks list or in the delayedTasks list but never both.

Finally I need to talk about the graphics and how I stopped each of the tasks interferring with each other since we have as yet no synchronization primitives. To do that I copied a windows trick and introduced a thing called a Device Context (DC). Each DC carries a small block of data which is those things that would get corrupted if another task interrupted it and itself started drawing on the screen. Thus on each task in the code sample it takes a unique DC that it uses for all drawing processes.
