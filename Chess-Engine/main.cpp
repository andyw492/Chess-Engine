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

// this function is where threadA starts
UINT engineThread(LPVOID pParam)
{
	Engine e;
	return e.start(pParam);
}

// this function is where threadA starts
UINT windowThread(LPVOID pParam)
{
	Window w;
	return w.start(pParam);
}

// this function is where threadB starts
UINT quitThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for both threadA threads to quit
	WaitForSingleObject(p->finished, INFINITE);
	WaitForSingleObject(p->finished, INFINITE);

	// force other threads to quit
	SetEvent(p->eventQuit);

	return 0;
}

int main(void)
{
	// thread handles are stored here; they can be used to check status of threads, or kill them
	HANDLE *handles = new HANDLE[3];
	Parameters p;

	// create a mutex for accessing critical sections (including printf); initial state = not locked
	p.mutex = CreateMutex(NULL, 0, NULL);
	// create a semaphore that counts the number of active threads; initial value = 0, max = 2
	p.finished = CreateSemaphore(NULL, 0, 2, NULL);
	// create a quit event; manual reset, initial state = not signaled
	p.eventQuit = CreateEvent(NULL, true, false, NULL);

	p.initialFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

	// structure p is the shared space between the threads
	handles[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)engineThread, &p, 0, NULL);		// start threadA (instance #1) 
	handles[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)windowThread, &p, 0, NULL);		// start threadA (instance #2)
	handles[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)quitThread, &p, 0, NULL);		// start threadB 

	// make sure this thread hangs here until the other three quit; otherwise, the program will terminate prematurely
	for (int i = 0; i < 3; i++)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	string s = "777777777777777777777777777777777777777777777777777777777777777";

	return 0;
}
