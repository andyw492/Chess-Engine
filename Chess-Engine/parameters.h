#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>

#include "position.h"

using std::string;

class Parameters
{
public:
	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;

	bool windowClosed = false;

	string initialFen = "";

	bool playerToMove = true;
	Position currentPosition;

	char* toEvaluate = nullptr;
	char* evaluated = nullptr;

	int maxDepth = 0;
	string bestMove = "";

	bool windowPrint = false;
	bool enginePrint = false;
	bool evaluatorPrint = false;

	string gameResult = "";
};

#endif