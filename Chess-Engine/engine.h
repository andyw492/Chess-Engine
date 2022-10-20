#ifndef engine_h
#define engine_h

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <iomanip>

#include "parameters.h"
#include "helper.h"
#include "positionnode.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::iterator;
using std::max;
using std::clock_t;
using std::min;

class Engine
{
public:

	Engine(bool enginePrint, U64 zobristTable[13][64]);

	void startClock();
	void printTime();

	U64 getStartingHashValue(vector<U64> position);
	int getStartingEvaluation(vector<U64> position);

	void deletePositionTree(PositionNode* node);

	UINT start(LPVOID pParam);

private:

	U64 zobristTable[13][64];

	bool dpr = false;
	clock_t clockStart;
	float lastTime = 0;
};

#endif