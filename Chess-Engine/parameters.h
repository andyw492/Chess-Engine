#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <unordered_map>

#include "helper.h"
#include "position.h"

using std::string;
using std::stack;
using std::queue;
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

	vector<float> values;
	bool workerError = false;

	int maxDepth = 0;
	string bestMove = "";

	int nodeId = 0;

	stack<char*> toDelete;

	int workerCount;
	bool deleteSignal;
	set<DWORD> readyToDelete;

	bool windowPrint = false;
	bool enginePrint = false;
	bool workerPrint = false;
	bool workerRandomize = false;

	string gameResult = "";

	// stats
	int depth0Progress = 0;
	int depth0Children = 0;
	int depth1Progress = 0;
	int depth1Children = 0;
	int positionsEvaluated = 0;
	int totalPositionsEvaluated = 0;
	int nodesCreated = 0;
	int nodesDeleted = 0;
	int nodesPruned = 0;
	int foundInCache = 0;

	// debug
	vector<int> debugPrint;
	vector<int> amountAtDepth;
};

#endif