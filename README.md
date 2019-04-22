# Raspberry-Pi-Multicore
This is a series of test code with multicore task schedulers and concepts. This code is designed specifically for multicore Raspberry Pi 2 & 3's it will not work on a Pi 1. On Pi3 the choice of AARCH32 or AARCH64 is available.

The first step is xRTOS our start point with a 4 core preemptive switcher with simple round robin task schedule. Each core is manually loaded with 2 tasks and will create an idle task when started. The 2 tasks per core are simple moving the bars on screen at this stage.
>
More detail is in the actual directory
>
https://github.com/LdB-ECM/Raspberry-Pi-Multicore/tree/master/xRTOS
>
![](https://github.com/LdB-ECM/Docs_and_Images/blob/master/Images/xRTOS.jpg?raw=true)
![](https://github.com/LdB-ECM/Docs_and_Images/blob/master/Images/xRTOS-Schedulers.jpg?raw=true)
