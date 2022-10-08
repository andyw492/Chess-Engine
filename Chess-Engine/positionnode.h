#ifndef positionnode_h
#define positionnode_h

#include <vector>
#include <unordered_set>
#include "position.h"

using std::vector;
using std::unordered_set;

class PositionNode
{
public:

	int depth = 0;
	bool min = false;
	Position position;
	PositionNode* parent = NULL;

	int id = 0;
	map<string, vector<string>> legalMoves;

	bool firstVisit = true;
	map<int, PositionNode*> children;
	unordered_set<int> unvisited;

	float value = 0;
	int bestChildId = 0;

	string prevMove = ""; // debugging

	int deleteIndex = 0; // freeing memory
};

#endif