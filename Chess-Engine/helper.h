#ifndef helper_h
#define helper_h

#define U64 unsigned long long

#define WHITEPAWN 0
#define WHITEKNIGHT 1
#define WHITEBISHOP 2
#define WHITEROOK 3
#define WHITEQUEEN 4
#define WHITEKING 5
#define BLACKPAWN 6
#define BLACKKNIGHT 7
#define BLACKBISHOP 8
#define BLACKROOK 9
#define BLACKQUEEN 10
#define BLACKKING 11
#define BOARDEXTRA 12
#define BOARDHASHVALUE 13

#define WHITETOMOVE 0
#define LEGALCASTLEWHITEKINGSIDE 1
#define LEGALCASTLEWHITEQUEENSIDE 2
#define LEGALCASTLEBLACKKINGSIDE 3
#define LEGALCASTLEBLACKQUEENSIDE 4
#define LASTKINGMOVE 5
#define LASTCASTLEWHITEKINGSIDE 9
#define LASTCASTLEWHITEQUEENSIDE 8
#define LASTCASTLEBLACKKINGSIDE 7
#define LASTCASTLEBLACKQUEENSIDE 6
#define EVALUATIONLSB 10
#define NEGATIVEEVALUATION 27

#define COL0BITBOARD 0x0101010101010101
#define COL1BITBOARD 0x0202020202020202
#define COL6BITBOARD 0x4040404040404040
#define COL7BITBOARD 0x8080808080808080
#define ROW0BITBOARD 0x00000000000000FF
#define ROW1BITBOARD 0x000000000000FF00
#define ROW6BITBOARD 0x00FF000000000000
#define ROW7BITBOARD 0xFF00000000000000

#define setBit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define getBit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define popBit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define PRINT(str, label) if (p->debugPrint[label]) { WaitForSingleObject(p->mutex, INFINITE); cout << str; ReleaseMutex(p->mutex); }
#define PRINTNOLOCK(str, label) if (p->debugPrint[label]) { cout << str; }

#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <set>

#include "position.h"

using std::vector;
using std::string;
using std::to_string;
using std::map;
using std::remove;
using std::cout;
using std::endl;
using std::find;
using std::set;

namespace helper
{
	char pieces[] = { 'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k' };
	double pieceValues[12] = { 1, 3, 3, 5, 9, 0, -1, -3, -3, -5, -9, 0 };

	vector<string> splitToVector(string str, char del);

	string roundFloat(float f, int r);

	void fenToMatrix(string fen, char matrix[8][8]);

	Position getNewPosition(Position before, string from, string to);

	map<string, vector<string>> getLegalMoves(Position position, bool whiteToMove);

	bool inCheck(vector<U64>, bool whiteToMove);
	bool inCheck(char board[8][8], bool whiteToMove);

	vector<U64> positionToU64(Position position);

	Position U64ToPosition(vector<U64> board);

	void printBoard(vector<U64> board);

	int getIntFromBits(U64 bits, int start, int end);

	U64 setBitsFromInt(U64 bits, int start, int end, int num);

	vector<vector<U64>> getNextPositions(vector<U64> board, bool whiteToMove, U64 zobristTable[13][64]);
}

#endif