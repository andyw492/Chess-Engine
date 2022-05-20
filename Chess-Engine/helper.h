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

	void fenToMatrix(string fen, char matrix[8][8]);

	map<string, vector<string>> getLegalMoves(char board[8][8], bool whiteToMove, bool castling[4]);

	float getPieceValue(char piece);
}

#endif