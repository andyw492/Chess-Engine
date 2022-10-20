#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <stack>
#include <vector>
#include <unordered_map>

#include "helper.h"
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


	U64 zobristTable[13][64];
	U64 startingHashValue = 0ULL;
	unordered_map<U64, vector<vector<U64>>> nextPositionsCache;
	unordered_map<U64, vector<U64>> positionsCache;

	vector<U64> lastEnginePosition;

	int foundInCache = 0;

	vector<float> values;
	bool workerError = false;

	int maxDepth = 0;
	string bestMove = "";

	int nodeId = 0;

	bool windowPrint = false;
	bool enginePrint = false;
	bool workerPrint = false;
	bool workerRandomize = false;

	string gameResult = "";
};

#endif