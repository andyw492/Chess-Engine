#ifndef evaluator_h
#define evaluator_h

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>

#include "parameters.h"
#include "helper.h"
#include "positionnode.h"

using std::cout;
using std::endl;
using std::string;
using std::min;
using std::max;

class Evaluator
{
public:

	Evaluator(bool evaluatorPrint);

	UINT start(LPVOID pParam);

private:

	bool dpr = false;
};

#endif