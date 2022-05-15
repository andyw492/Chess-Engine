#ifndef engine_h
#define engine_h

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "parameters.h"

using std::cout;
using std::endl;
using std::string;

class Engine
{
public:

	Engine();

	UINT start(LPVOID pParam);

private:

	string fen;
};

#endif