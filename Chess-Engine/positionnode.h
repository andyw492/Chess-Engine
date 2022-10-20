#ifndef positionnode_h
#define positionnode_h

#include <vector>
#include <unordered_set>
#include <queue>
#include "position.h"

using std::vector;
using std::unordered_set;
using std::queue;

class PositionNode
{
public:

	int depth = 0;
	bool min = false;
	vector<U64> position;
	PositionNode* parent = NULL;

	int id = 0;
	U64 hashValue = 0;
	vector<vector<U64>> nextPositions;

	bool firstVisit = true;
	vector<PositionNode*> children;
	queue<int> unvisited;

	int lastVisited = -1;

	float value = 0;
	int bestChildId = -1;

	float alpha = 0;
	float beta = 0;
};

#endif