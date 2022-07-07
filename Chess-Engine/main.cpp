#define NOMINMAX
#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <stdio.h>
#include <iostream>


#include <stdlib.h>
#include <crtdbg.h>

#include "parameters.h"
#include "engine.h"
#include "evaluator.h"
#include "window.h"

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

UINT evaluatorThreads(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	bool evaluatorPrint = false;
	WaitForSingleObject(p->mutex, INFINITE);
	evaluatorPrint = p->evaluatorPrint;
	ReleaseMutex(p->mutex);

	Evaluator e(evaluatorPrint);
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
	clock_t c = clock();
	int j = 0;
	for (int i = 0; i < 1000000000; i++)
	{
		j += 1;
	}

	cout << 1000.0 * (clock() - c) / CLOCKS_PER_SEC << " ms" << endl;
	return 0;


	HANDLE *handles = new HANDLE[3];
	Parameters p;

	int threadCount = 3;

	p.mutex = CreateMutex(NULL, 0, NULL);
	p.finished = CreateSemaphore(NULL, 0, threadCount, NULL);
	p.eventQuit = CreateEvent(NULL, true, false, NULL);

	string fen = "";
	int fenOption = 3;

	switch (fenOption)
	{
	case 0:
		// standard starting position
		fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		break;
	case 1:
		// depth 2 test
		fen = "k1K5/8/1P1p1p2/8/8/2P5/8/8 w - - 0 1";
		break;
	case 2:
		// depth 3 test
		fen = "5prk/5ppb/5ppp/5ppn/8/8/7K/5P2 w - - 0 1";
		break;
	case 3:
		// endgame test
		fen = "4k3/4r3/8/8/8/8/3PPP2/4K3 w - - 0 1";
		break;
	}


	p.initialFen = helper::splitToVector(fen, ' ')[0];
	p.maxDepth = 3;

	// debug
	int debugVal = 3;
	p.windowPrint = debugVal % 2 == 0;
	p.enginePrint = debugVal % 3 == 0;
	p.evaluatorPrint = debugVal % 5 == 0;

	handles[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)engineThread, &p, 0, NULL);
	handles[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)evaluatorThreads, &p, 0, NULL);
	handles[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)windowThread, &p, 0, NULL);
	handles[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)quitThread, &p, 0, NULL);

	for (int i = 0; i < threadCount + 1; i++)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	//_CrtDumpMemoryLeaks();

	return 0;
}
