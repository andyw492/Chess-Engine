#ifndef helper_h
#define helper_h

#include <vector>
#include <string>
#include <map>

using std::vector;
using std::string;
using std::to_string;
using std::map;

namespace helper
{
	vector<string> splitToVector(string str, char del);

	map<string, vector<string>> getLegalMoves(string fen, bool whiteToMove);
}

#endif