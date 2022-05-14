#ifndef engine_cpp
#define engine_cpp

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "parameters.cpp"

using std::cout;
using std::endl;
using std::string;

class Engine
{
public:

	Engine()
	{
		// starting position
		fen = "";
	}

	UINT start(LPVOID pParam)
	{
		Parameters *p = ((Parameters*)pParam);

		WaitForSingleObject(p->mutex, INFINITE);
		printf("engineThread %d started\n", GetCurrentThreadId());
		ReleaseMutex(p->mutex);

		while(true)
		{
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

private:

	string fen;
};

#endif