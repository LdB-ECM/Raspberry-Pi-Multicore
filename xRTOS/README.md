
# xRTOS ... PI 3 AARCH64 and PI 2,3 AARCH32
As per usual you can simply copy the files in the DiskImg directory onto a formatted SD card and place in Pi to test 
>
The current code is technically just 4 cores indpendantly running although it shares some characteristics of what system that would be called SMP and BMP systems. All 4 cores are running on the same address space executing code (there is only one copy of the task switcher in memory) which is a characteristic of SMP systems. Tasks are manually assigned to each core and they are forever bound to that core which is a characteristic of BMP systems. You could make it take on AMP characteristics by copying the execution code to multiple memory locations and get each core to run in it's own memory area. The point being made is you can adapt the switcher itself to any of the more multicores schemes or even hybrids between them.
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
