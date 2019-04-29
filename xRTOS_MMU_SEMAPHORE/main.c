#include <stdint.h>
#include <string.h>
#include "rpi-smartstart.h"
#include "emb-stdio.h"
#include "xRTOS.h"
#include "task.h"
#include "windows.h"
#include "semaphore.h"

void DoProgress(HDC dc, int step, int total, int x, int y, int barWth, int barHt,  COLORREF col)
{

	// minus label len
	int pos = (step * barWth) / total;

	// Draw the colour bar
	COLORREF orgBrush = SetDCBrushColor(dc, col);
	Rectangle(dc, x, y, x+pos, y+barHt);

	// Draw the no bar section 
	SetDCBrushColor(dc, 0);
	Rectangle(dc, x+pos, y, x+barWth, y+barHt);

	SetDCBrushColor(dc, orgBrush);

}

static SemaphoreHandle_t screenSem;
static unsigned int Counts[4] = { 0 };

void task1(void *pParam) {
	HDC Dc = CreateExternalDC(1);
	COLORREF col = 0xFFFF0000;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}

		DoProgress(Dc, step, total, 10, 100, GetScreenWidth()-20, 20, col);
		xTaskDelay(20);
		Counts[0]++;
		xSemaphoreTake(screenSem);
		GotoXY(0, 10);
		printf("Core 0 count %u\n", Counts[0]);
		xSemaphoreGive(screenSem);
	}
}

void task2(void *pParam) {
	HDC Dc = CreateExternalDC(2);
	COLORREF col = 0xFF0000FF;
	int total = 1000;
	volatile int step = 0;
	volatile int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 200, GetScreenWidth() - 20, 20, col);
		xTaskDelay(22);
		Counts[1]++;
		xSemaphoreTake(screenSem);
		GotoXY(0, 16);
		printf("Core 1 count %u\n", Counts[1]);
		xSemaphoreGive(screenSem);
	}
}

void task3(void *pParam) {
	HDC Dc = CreateExternalDC(3);
	COLORREF col = 0xFF00FF00;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 300, GetScreenWidth() - 20, 20, col);
		xTaskDelay(24);
		Counts[2]++;
		xSemaphoreTake(screenSem);
		GotoXY(0, 22);
		printf("Core 2 count %u\n", Counts[2]);
		xSemaphoreGive(screenSem);
	}
}

void task4 (void* pParam) {
	HDC Dc = CreateExternalDC(4);
	COLORREF col = 0xFFFFFF00;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 400, GetScreenWidth() - 20, 20, col);
		xTaskDelay(26);
		Counts[3]++;
		xSemaphoreTake(screenSem);
		GotoXY(0, 28);
		printf("Core 3 count %u\n", Counts[3]);
		xSemaphoreGive(screenSem);
	}
}

void task1A(void* pParam) {
	char buf[32];
	HDC Dc = CreateExternalDC(5);
	COLORREF col = 0xFF00FFFF;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 125, GetScreenWidth() - 20, 20, col);
		xTaskDelay(35);
		sprintf(&buf[0], "Core 0 Load: %3i%% Task count: %2i", xLoadPercentCPU(), xTaskGetNumberOfTasks());
		TextOut(Dc, 20, 80, &buf[0], strlen(&buf[0]));
	}
}


void task2A(void* pParam) {
	char buf[32];
	HDC Dc = CreateExternalDC(6);
	COLORREF col = 0xFFFFFFFF;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 225, GetScreenWidth() - 20, 20, col);
		xTaskDelay(37);
		sprintf(&buf[0], "Core 1 Load: %3i%% Task count: %2i", xLoadPercentCPU(), xTaskGetNumberOfTasks());
		TextOut(Dc, 20, 180, &buf[0], strlen(&buf[0]));
	}
}

void task3A(void* pParam) {
	char buf[32];
	HDC Dc = CreateExternalDC(7);
	COLORREF col = 0xFF7F7F7F;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 325, GetScreenWidth() - 20, 20, col);
		xTaskDelay(39);
		sprintf(&buf[0], "Core 2 Load: %3i%% Task count: %2i", xLoadPercentCPU(), xTaskGetNumberOfTasks());
		TextOut(Dc, 20, 280, &buf[0], strlen(&buf[0]));
	}
}

void task4A(void* pParam) {
	char buf[32];
	HDC Dc = CreateExternalDC(8);
	COLORREF col = 0xFFFF00FF;
	int total = 1000;
	int step = 0;
	int dir = 1;
	while (1) {
		step += dir;
		if ((step == total) || (step == 0))
		{
			dir = -dir;
		}
		DoProgress(Dc, step, total, 10, 425, GetScreenWidth() - 20, 20, col);
		xTaskDelay(41);
		sprintf(&buf[0], "Core 3 Load: %3i%% Task count: %2i", xLoadPercentCPU(), xTaskGetNumberOfTasks());
		TextOut(Dc, 20, 380, &buf[0], strlen(&buf[0]));
	}
}


void main (void)
{
	Init_EmbStdio(WriteText);										// Initialize embedded stdio
	PiConsole_Init(0, 0, 0, printf);								// Auto resolution console, message to screen
	displaySmartStart(printf);										// Display smart start details
	ARM_setmaxspeed(printf);										// ARM CPU to max speed
	printf("Task tick rate: %u\n", configTICK_RATE_HZ);
	
	xRTOS_Init();													// Initialize the xRTOS system .. done before any other xRTOS call

	screenSem = xSemaphoreCreateBinary();

	/* Core 0 tasks */
	xTaskCreate(0, task1, "Core0-1", 512, NULL, 4, NULL);
	xTaskCreate(0, task1A, "Core0-2", 512, NULL, 2, NULL);

	/* Core 1 tasks */
	xTaskCreate(1, task2, "Core1-1", 512, NULL, 2, NULL);
	xTaskCreate(1, task2A, "Core1-2", 512, NULL, 2, NULL);
	
	/* Core 2 tasks */
	xTaskCreate(2, task3, "Core2-1", 512, NULL, 2, NULL);
	xTaskCreate(2, task3A, "Core2-2", 512, NULL, 2, NULL);

	/* Core 3 tasks */
	xTaskCreate(3, task4, "Core3-1", 512, NULL, 2, NULL);
	xTaskCreate(3, task4A, "Core3-2", 512, NULL, 2, NULL);

	/* Start scheduler */
	xTaskStartScheduler();
	/*
	 *	We should never get here, but just in case something goes wrong,
	 *	we'll place the CPU into a safe loop.
	 */
	while (1) {
	}
}
