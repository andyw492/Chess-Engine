#include "worker.h"

Worker::Worker(bool enginePrint)
{
	dpr = enginePrint;
}

double Worker::evaluate(PositionNode* node)
{
	//// if the node's position is stalemate/checkmate, return -inf, 0, or inf as evaluation
	//if (node->nextPositions.size() == 0)
	//{
	//	if (helper::inCheck(node->position, !node->min))
	//	{
	//		return -FLT_MAX;
	//	}
	//	else if (helper::inCheck(node->position, node->min))
	//	{
	//		return FLT_MAX;
	//	}
	//	else
	//	{
	//		return 0;
	//	}
	//}

	double pieceValues[12] = { 1, 3, 3.2, 5, 9, 0, -1, -3, -3.2, -5, -9, 0 };

	// calculate total value of original position, then add/subtract from it using the last captured piece in nextPositions
	double bestValue = 0;
	for (int i = WHITEPAWN; i < BLACKKING + 1; i++)
	{
		if (i == WHITEKING || i == BLACKKING) { continue; }

		for (int square = 0; square < 64; square++)
		{
			if (getBit(node->position[i], square)) { bestValue += pieceValues[i]; }
		}
	}

	for (vector<U64> nextPosition : node->nextPositions)
	{
		int capturedPiece = helper::getIntFromBits(nextPosition[BOARDEXTRA], LASTCAPTURELSB, LASTCAPTURELSB + 3);

		if (capturedPiece != 7)
		{
			bestValue = (node->min ? min(bestValue, bestValue - pieceValues[capturedPiece]) : max(bestValue, bestValue + pieceValues[capturedPiece]));
		}

	}

	// slightly randomize value to prevent repeating moves
	return bestValue + ((double)rand() / RAND_MAX) / 100;
}

UINT Worker::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("workerThread %d started\n", GetCurrentThreadId());
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

		int maxDepth = 0;
		WaitForSingleObject(p->mutex, INFINITE);
		maxDepth = p->maxDepth;
		ReleaseMutex(p->mutex);

		// find the next legal positions from the current node
		vector<U64> position = node->position;
		bool whiteToMove = (node->min ? false : true);
		node->nextPositions = helper::getNextPositions(position, whiteToMove);

		// if the current node can't be expanded due to stalemate/checkmate, find the node's value and move to parent
		if (node->nextPositions.empty())
		{
			if (helper::inCheck(node->position, !node->min))
			{
				node->value = -INT_MAX;
			}
			else if (helper::inCheck(node->position, node->min))
			{
				node->value = INT_MAX;
			}
			else
			{
				node->value = 0;
			}
		}

		// if current node is at depth limit, find the node's value and move to parent
		else if (node->depth == maxDepth - 1)
		{
			node->value = evaluate(node);
		}

		// otherwise, create child nodes
		else
		{
			for (vector<U64> nextPosition : node->nextPositions)
			{
				PositionNode* newNode = new PositionNode;
				newNode->depth = node->depth + 1;
				newNode->min = !(node->min);
				newNode->position = nextPosition;
				newNode->parent = node;
				newNode->value = (newNode->min ? FLT_MAX : -FLT_MAX);

				int id = -1;
				WaitForSingleObject(p->mutex, INFINITE);
				id = p->nodeId;
				p->nodeId++;
				ReleaseMutex(p->mutex);

				newNode->id = id;
				node->unvisited.push(id);

				node->children.push_back(newNode);
			}
		}

		WaitForSingleObject(p->mutex, INFINITE);
		p->expanded[node->id] = (char*)node;
		ReleaseMutex(p->mutex);
		
	}

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("workerThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;
}