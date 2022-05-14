#ifndef window_cpp
#define window_cpp

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "parameters.cpp"

using std::cout;
using std::endl;
using std::cin;

class Window
{
public:

	Window() {}

	UINT start(LPVOID pParam)
	{
		Parameters *p = ((Parameters*)pParam);

		WaitForSingleObject(p->mutex, INFINITE);
		printf("windowThread %d started\n", GetCurrentThreadId());
		ReleaseMutex(p->mutex);

		// send a move to engine
		while (true)
		{
			// proceed if move is empty
			string move = "";
			WaitForSingleObject(p->mutex, INFINITE);
			move = p->move;
			ReleaseMutex(p->mutex);
			if (move != "")
			{
				continue;
			}

			WaitForSingleObject(p->mutex, INFINITE);
			cout << "window: sending ";
			std::getline(cin, move);
			p->move = move;
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
		printf("windowThread %d quitting on event\n", GetCurrentThreadId());
		ReleaseMutex(p->mutex);

		return 0;
	}
};

#endif