#ifndef position_h
#define position_h

#include <string>

using std::string;

class Position
{
public:
	char board[8][8];
	//bool castling[4] = { true, true, true, true }; // {white kingside allowed, white queenside allowed, black kingside allowed, black queenside allowed}
	bool castling[4] = { false, false, false, false };
	string enpassant = "";
};

#endif

