#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>

using std::string;

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;

	bool windowPrint = false;
	bool enginePrint = false;

	string initialFen = "";

	bool playerToMove = true;
	char board[8][8];

	// {white kingside allowed, white queenside allowed, black kingside allowed, black queenside allowed}
	bool castling[4] = { true, true, true, true };

	string enpassant = "";

	bool windowClosed = false;
};

#endif