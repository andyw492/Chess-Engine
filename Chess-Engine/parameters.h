#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <stack>
#include <vector>
#include <unordered_map>

#include "position.h"

using std::string;
using std::stack;
using std::vector;
using std::unordered_map;

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
	stack<char*> toExpand;
	unordered_map<int, char*> expanded;

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