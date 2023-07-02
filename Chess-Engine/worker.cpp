#include "worker.h"

Worker::Worker(bool workerPrint, U64 zobristTable[13][64], bool randomize)
{
	dpr = workerPrint;
	memcpy(this->zobristTable, zobristTable, 13 * 64 * sizeof(U64));
	this->randomize = randomize;
}

double Worker::getBestValue(PositionNode* node)
{
	double bestValue = (node->min ? FLT_MAX : -FLT_MAX);

	for (vector<U64> nextPosition : node->nextPositions)
	{
		int evaluationInteger = helper::getIntFromBits(nextPosition[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10);
		int evaluationDecimal = helper::getIntFromBits(nextPosition[BOARDEXTRA], EVALUATIONLSB + 10, EVALUATIONLSB + 17);
		double evaluation = (double)evaluationInteger + ((double)evaluationDecimal / 100);
		if (getBit(nextPosition[BOARDEXTRA], NEGATIVEEVALUATION))
		{
			evaluation *= -1;
		}

		bestValue = (node->min ? min(bestValue, evaluation) : max(bestValue, evaluation));
	}

	// slightly randomize value to prevent repeating moves
	if (randomize)
	{
		bestValue += ((double)rand() / RAND_MAX) / 100;
	}

	return bestValue;
}

void Worker::evaluateNode(Parameters* p, PositionNode* node)
{
	int maxDepth = 0;
	WaitForSingleObject(p->mutex, INFINITE);
	maxDepth = p->maxDepth;
	ReleaseMutex(p->mutex);

	auto workerPoint1 = std::chrono::high_resolution_clock::now();

	// find the next legal positions from the current node
	vector<U64> position = node->position;
	bool whiteToMove = (node->min ? false : true);

	// check if node->nextpositions can be found from hash table
	// hash value is computed from parent position
	U64 hashValue = position[BOARDHASHVALUE];

	bool inCache = false;
	WaitForSingleObject(p->mutex, INFINITE);
	if (p->nextPositionsCache.find(hashValue) != p->nextPositionsCache.end())
	{
		inCache = true;
		node->nextPositions = p->nextPositionsCache[hashValue];
		p->foundInCache++;
	}
	ReleaseMutex(p->mutex);

	if (!inCache)
	{
		node->nextPositions = helper::getNextPositions(position, whiteToMove, zobristTable);

		//WaitForSingleObject(p->mutex, INFINITE);
		//printf("position evaluated with hash value %llu gives positions:\n", hashValue);
		//for (vector<U64> nextPosition : node->nextPositions)
		//{
		//	helper::printBoard(nextPosition);
		//	printf("----------------\n");
		//}
		//ReleaseMutex(p->mutex);

		// put node->nextpositions in hash table
		WaitForSingleObject(p->mutex, INFINITE);
		p->nextPositionsCache[hashValue] = node->nextPositions;
		ReleaseMutex(p->mutex);
	}
	//else if(false)
	//{
	//	vector<vector<U64>> assertNextPositions = helper::getNextPositions(position, whiteToMove, zobristTable);

	//	bool badHash = false;
	//	for (int i = 0; i < assertNextPositions.size(); i++)
	//	{
	//		for (int j = 0; j < assertNextPositions[i].size(); j++)
	//		{
	//			if (assertNextPositions[i][j] != node->nextPositions[i][j])
	//			{
	//				badHash = true;
	//			}
	//		}
	//	}

	//	if (badHash)
	//	{
	//		WaitForSingleObject(p->mutex, INFINITE);
	//		printf("positions expected from hash value %llu:\n", hashValue);
	//		for (vector<U64> nextPosition : assertNextPositions)
	//		{
	//			helper::printBoard(nextPosition);
	//			printf("extra == %llu\n", nextPosition[BOARDEXTRA]);
	//			printf("----------------\n");
	//		}

	//		printf("positions received from hash value %llu:\n", hashValue);
	//		for (vector<U64> nextPosition : node->nextPositions)
	//		{
	//			helper::printBoard(nextPosition);
	//			printf("extra == %llu\n", nextPosition[BOARDEXTRA]);
	//			printf("----------------\n");
	//		}

	//		printf("current position:\n");
	//		helper::printBoard(position);
	//		printf("-------------\n");

	//		printf("position from hash:\n");
	//		helper::printBoard(p->positionsCache[hashValue]);
	//		printf("-------------\n");

	//		ReleaseMutex(p->mutex);

	//		assert(false);

	//	}
	//}


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
		auto workerPoint2 = std::chrono::high_resolution_clock::now();
		//printf("worker stalemate/checkmate done in %f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(workerPoint2 - workerPoint1).count()) / 1e6);

	}

	// if current node is at depth limit, find the node's value and move to parent
	else if (node->depth == maxDepth - 1)
	{
		//bool a = false;

		//if (a && false)
		//{
		//	WaitForSingleObject(p->mutex, INFINITE);
		//	printf("evaluating position for min=%d\n", node->min);
		//	helper::printBoard(node->position);
		//}

		node->value = getBestValue(node);

		WaitForSingleObject(p->mutex, INFINITE);
		p->positionsEvaluated += node->nextPositions.size();
		p->totalPositionsEvaluated += node->nextPositions.size();
		ReleaseMutex(p->mutex);

		//if (a)
		//{
		//	if (node->value == -2)
		//	{
		//		WaitForSingleObject(p->mutex, INFINITE);
		//		printf("evaluated position for min=%d\n", node->min);
		//		helper::printBoard(node->position);
		//		printf("value is %f\n", node->value);
		//		ReleaseMutex(p->mutex);
		//	}

		//}


		auto workerPoint2 = std::chrono::high_resolution_clock::now();
		//printf("worker evaluate at maxdepth - 1 done in %f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(workerPoint2 - workerPoint1).count()) / 1e6);

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

			WaitForSingleObject(p->mutex, INFINITE);
			PRINTNOLOCK("[", 1);
			for (int i = 0; i < 4; i++)
			{
				PRINTNOLOCK(to_string(p->amountAtDepth[i]) + " ", 1);
			}
			PRINTNOLOCK("], new depth " + to_string(node->depth + 1) + "\n", 1);
			ReleaseMutex(p->mutex);

			int id = -1;
			WaitForSingleObject(p->mutex, INFINITE);
			id = p->nodeId;
			p->nodeId++;
			p->nodesCreated++;
			ReleaseMutex(p->mutex);

			newNode->id = id;
			node->unvisited.push(id);

			node->children.push_back(newNode);
		}
		auto workerPoint2 = std::chrono::high_resolution_clock::now();

		//printf("worker create child nodes done in %f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(workerPoint2 - workerPoint1).count()) / 1e6);
	}

	WaitForSingleObject(p->mutex, INFINITE);
	p->expanded[node->id] = (char*)node;
	ReleaseMutex(p->mutex);
}

void Worker::deleteNode(Parameters* p, PositionNode* node)
{
	if (node == NULL) { return; }

	for (PositionNode* child : node->children)
	{
		deleteNode(p, child);
	}

	WaitForSingleObject(p->mutex, INFINITE);
	PRINTNOLOCK("[", 1);
	for (int i = 0; i < 4; i++)
	{
		PRINTNOLOCK(to_string(p->amountAtDepth[i]) + " ", 1);
	}
	PRINTNOLOCK("], delete depth " + to_string(node->depth) + "\n", 1);
	ReleaseMutex(p->mutex);

	delete node;

	WaitForSingleObject(p->mutex, INFINITE);
	p->nodesDeleted += 1;
	ReleaseMutex(p->mutex);
}

UINT Worker::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	//WaitForSingleObject(p->mutex, INFINITE);
	//printf("workerThread %d started\n", GetCurrentThreadId());
	//ReleaseMutex(p->mutex);

	while (true)
	{
		// if delete signal is on, signal back that worker is ready to delete
		WaitForSingleObject(p->mutex, INFINITE);
		if (p->deleteSignal)
		{
			p->readyToDelete.insert(GetCurrentThreadId());
		}
		ReleaseMutex(p->mutex);

		// check if window closed or game ended
		bool windowClosed = false;
		string gameResult = "";

		// wait for a node to evaluate or delete
		char* toEvaluate = nullptr;
		char* toDelete = nullptr;

		PositionNode* node = nullptr;

		WaitForSingleObject(p->mutex, INFINITE);
		if (!p->deleteSignal && !p->toExpand.empty())
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
			bool doClear = false;
			WaitForSingleObject(p->mutex, INFINITE);
			if (!p->toDelete.empty())
			{
				if (!p->startedClear)
				{
					p->startedClear = true;
					doClear = true;
				}
				else
				{
					toDelete = p->toDelete.top();
					p->toDelete.pop();
				}
			}
			ReleaseMutex(p->mutex);

			if (doClear)
			{
				p->nextPositionsCache.clear();
			}

			if (toDelete != nullptr)
			{
				node = (PositionNode*)toDelete;
			}
		}

		if (toEvaluate != nullptr)
		{
			evaluateNode(p, node);
		}
		else if (toDelete != nullptr)
		{
			deleteNode(p, node);
		}

	}

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	//WaitForSingleObject(p->mutex, INFINITE);
	//printf("workerThread %d quitting on event\n", GetCurrentThreadId());
	//ReleaseMutex(p->mutex);

	return 0;
}