#include "evaluator.h"

Evaluator::Evaluator(bool enginePrint)
{
	dpr = enginePrint;
}

UINT Evaluator::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("evaluatorThread %d started\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	while (true)
	{
		// check if window closed or game ended
		bool windowClosed = false;
		string gameResult = "";

		// wait for a node to evaluate
		char* toEvaluate = nullptr;
		PositionNode* node = nullptr;

		WaitForSingleObject(p->mutex, INFINITE);
		if (!p->toExpand.empty())
		{
			toEvaluate = p->toExpand.top();
			p->toExpand.pop();
		}
		windowClosed = p->windowClosed;
		gameResult = p->gameResult;
		ReleaseMutex(p->mutex);

		if (windowClosed || gameResult.length() > 0) { break; }

		if (toEvaluate != nullptr)
		{
			node = (PositionNode*)toEvaluate;
		}
		else
		{
			continue;
		}

		vector<float> values; // debug

		// find the most min/max value of the node's position's legal moves
		Position position = node->position;
		bool whiteToMove = (node->min ? false : true);

		auto begin = std::chrono::high_resolution_clock::now();
		node->legalMoves = helper::getLegalMoves(position, whiteToMove);
		auto end = std::chrono::high_resolution_clock::now();

		if (dpr)
		{
			printf("legal moves found in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		}

		WaitForSingleObject(p->mutex, INFINITE);
		p->expanded[node->id] = (char*)node;
		ReleaseMutex(p->mutex);
		
	}

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("evaluatorThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;
}