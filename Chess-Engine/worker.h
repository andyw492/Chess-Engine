#ifndef worker_h
#define worker_h

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>

#include "parameters.h"
#include "helper.h"
#include "positionnode.h"

using std::cout;
using std::endl;
using std::string;
using std::min;
using std::max;

class Worker
{
public:

	Worker(bool workerPrint, U64 zobristTable[13][64], bool randomize);

	UINT start(LPVOID pParam);

private:

	double getBestValue(PositionNode* node);

	void evaluateNode(Parameters* p, PositionNode* node);

	void deleteNode(Parameters* p, PositionNode* node);

	U64 zobristTable[13][64];

	bool dpr = false;
	bool randomize = false;
};

#endif