
# xRTOS ... PI 3 AARCH64 and PI 2,3 AARCH32
As per usual you can simply copy the files in the **DiskImg** directory onto a formatted SD card and place in Pi to test 
>
If you need assistance with how to compile the code
>
https://github.com/LdB-ECM/Docs_and_Images/blob/master/Documentation/Multicore_Build.md
>
So the change on this directory is simply we have turned on the MMU 
>
No the processor load monitor has not died ... the MMU speeds up the Raspberry Pi by over 20-30 fold and so the processor loads drops to less than 1% and as it rounds we get 0%.
>
If you doubt it change the tick speed in xRTOS.h to 10000 up from 1000 (which is a horribly high context switch speed) but it will make the bars move a lot faster and you should get a CPU load reading back.
>
All the MMU is doing at this stage is providing a basic 1:1 map on the standard memory map from 0x0 to 0x80000000
>
In a later excercise we will play with different aspects of virtualization and protection which the MMU offers that bring more safety and even a bit more speed.
