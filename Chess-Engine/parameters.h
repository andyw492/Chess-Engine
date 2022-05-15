#ifndef parameters_h
#define parameters_h

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;

	string initialFen = "";

	string move = "";
	map<string, vector<string>> legalMoves;

	bool windowClosed = false;
};

#endif