#ifndef worker_h
#define worker_h

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

class Worker
{
public:

	Worker(bool workerPrint);

	double evaluate(PositionNode* node);

	UINT start(LPVOID pParam);

private:

	bool dpr = false;
};

#endif