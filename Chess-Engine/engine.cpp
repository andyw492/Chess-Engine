#include "engine.h"

Engine::Engine()
{
	// starting position
	fen = "";
}

UINT Engine::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("engineThread %d started\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	while(true)
	{
		// check if window is still open
		bool windowClosed = false;
		WaitForSingleObject(p->mutex, INFINITE);
		windowClosed = p->windowClosed;
		ReleaseMutex(p->mutex);

		if (windowClosed)
		{
			break;
		}

		// proceed if move is full
		string move = "";
		WaitForSingleObject(p->mutex, INFINITE);
		move = p->move;
		ReleaseMutex(p->mutex);
		if (move == "")
		{
			continue;
		}

		WaitForSingleObject(p->mutex, INFINITE);
		cout << "processing..." << endl;
		ReleaseMutex(p->mutex);

		Sleep(500);

		WaitForSingleObject(p->mutex, INFINITE);
		cout << "engine: received " << move << endl;
		p->move = ""; // indicate that move was received
		ReleaseMutex(p->mutex);

		// if the move is "q", then stop
		if (move == "q")
		{
			break;
		}
	}

	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	// print we're about to exit
	WaitForSingleObject(p->mutex, INFINITE);
	printf("engineThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;
}