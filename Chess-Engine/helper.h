#ifndef helper_h
#define helper_h

#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <algorithm>
#include <iostream>

#include "position.h"

using std::vector;
using std::string;
using std::to_string;
using std::map;
using std::remove;
using std::cout;
using std::endl;
using std::find;

namespace helper
{
	vector<string> splitToVector(string str, char del);

	void fenToMatrix(string fen, char matrix[8][8]);

	Position getNewPosition(Position before, string from, string to);

	map<string, vector<string>> getLegalMoves(Position position, bool whiteToMove);
}

#endif