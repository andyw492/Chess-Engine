#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>

#include "parameters.h"
#include "window.h"
#include "engine.h"

#pragma comment(lib, "ws2_32.lib")

using std::cout;
using std::endl;
using std::string;

UINT engineThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	bool enginePrint = false;
	WaitForSingleObject(p->mutex, INFINITE);
	enginePrint = p->enginePrint;
	ReleaseMutex(p->mutex);

	Engine e(enginePrint);
	return e.start(pParam);
}

UINT windowThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	bool windowPrint = false;
	WaitForSingleObject(p->mutex, INFINITE);
	windowPrint = p->windowPrint;
	ReleaseMutex(p->mutex);

	Window w(windowPrint);
	return w.start(pParam);
}

UINT quitThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->finished, INFINITE);
	WaitForSingleObject(p->finished, INFINITE);

	SetEvent(p->eventQuit);

	return 0;
}

int main(void)
{
	HANDLE *handles = new HANDLE[3];
	Parameters p;

	int threadCount = 2;

	p.mutex = CreateMutex(NULL, 0, NULL);
	p.finished = CreateSemaphore(NULL, 0, threadCount, NULL);
	p.eventQuit = CreateEvent(NULL, true, false, NULL);

	p.initialFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
	for (int i = 0; i < 4; i++) { p.castling[i] = false; }

	// debug
	int debugVal = 60;
	p.windowPrint = debugVal % 2 == 0;
	p.enginePrint = debugVal % 3 == 0;

	handles[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)engineThread, &p, 0, NULL);
	handles[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)windowThread, &p, 0, NULL);
	handles[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)quitThread, &p, 0, NULL);

	for (int i = 0; i < threadCount + 1; i++)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	return 0;
}
