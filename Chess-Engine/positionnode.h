#ifndef positionnode_h
#define positionnode_h

#include <vector>
#include "position.h"

using std::vector;

class PositionNode
{
public:

	int depth = 0;

	Position position;

	bool min = false;

	float value = 0;
	bool evaluated = false;

	PositionNode* parent;
	vector<PositionNode*> children;

	string prevMove = ""; // debugging

	int deleteIndex = 0; // freeing memory
};

#endif