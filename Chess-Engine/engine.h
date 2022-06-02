#ifndef engine_h
#define engine_h

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <chrono>

#include "parameters.h"
#include "helper.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::iterator;
using std::max;
using std::clock_t;

class Engine
{
public:

	Engine(bool enginePrint);

	UINT start(LPVOID pParam);

private:

	bool dpr = false;
};

#endif