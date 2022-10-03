#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <stack>
#include <vector>

#include "position.h"

using std::string;
using std::stack;
using std::vector;

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

	//char* toEvaluate;
	//char* evaluated;
	stack<char*> toEvaluate;
	vector<float> values;
	bool evaluatorError = false;

	int maxDepth = 0;
	string bestMove = "";

	bool windowPrint = false;
	bool enginePrint = false;
	bool evaluatorPrint = false;

	string gameResult = "";
};

#endif