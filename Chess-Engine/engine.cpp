#include "engine.h"

Engine::Engine(bool enginePrint, U64 zobristTable[13][64])
{
	dpr = enginePrint;
	memcpy(this->zobristTable, zobristTable, 13 * 64 * sizeof(U64));
}

void Engine::startClock()
{
	clockStart = clock();
}

//void Engine::printTime(Parameters* p)
//{
//	if (!dpr) { return; }
//
//	float total = 1000.0 * (clock() - clockStart) / CLOCKS_PER_SEC;
//	PRINT(helper::roundFloat(total - lastTime, 0) + " ms elapsed; " + helper::roundFloat(total, 0) + " ms total\n");
//	lastTime = total;
//}

int Engine::getStartingEvaluation(vector<U64> position)
{
	int value = 0;

	for (int i = WHITEPAWN; i < BLACKKING + 1; i++)
	{
		if (i == WHITEKING || i == BLACKKING) { continue; }

		for (int square = 0; square < 64; square++)
		{
			if (getBit(position[i], square)) { value += helper::pieceValues[i]; }
		}
	}

	return value;
}

void printTree(PositionNode* node)
{
	if (node == NULL) { return; }

	printf("depth %d min %d bestChildId %d", node->depth, node->min, node->bestChildId);
	printf("\nposition for node id %d has value %.0f\n", node->id, node->value);
	helper::printBoard(node->position);
	printf("\n");

	for (PositionNode* child : node->children)
	{
		printTree(child);
	}
}

bool isContains(queue<int> q, int x) {
	while (!q.empty()) {
		if (q.front() == x)
			return true;
		q.pop();
	}
	return false;
}

// source: https://www.geeksforgeeks.org/minimax-algorithm-in-game-theory-set-5-zobrist-hashing/
U64 Engine::getStartingHashValue(vector<U64> position)
{
	U64 h = 0;

	for (int square = 0; square < 64; square++)
	{
		for (int piece = WHITEPAWN; piece < BOARDEXTRA + 1; piece++)
		{
			h ^= zobristTable[piece][square];
		}
	}

	return h;
}

bool Engine::checkGameEnded(Parameters* p, Position currentPosition, bool engineToMove)
{
	map<string, vector<string>> legalMoves = helper::getLegalMoves(currentPosition, !engineToMove);
	if (legalMoves.size() == 0)
	{
		if (helper::inCheck(currentPosition.board, !engineToMove))
		{
			WaitForSingleObject(p->mutex, INFINITE);
			p->gameResult = (engineToMove ? "you won!!" : "you lost!!");
			ReleaseMutex(p->mutex);
		}
		else
		{
			WaitForSingleObject(p->mutex, INFINITE);
			p->gameResult = "stalemate...";
			ReleaseMutex(p->mutex);
		}

		WaitForSingleObject(p->mutex, INFINITE);
		p->playerToMove = true;
		ReleaseMutex(p->mutex);

		return true;
	}

	return false;
}

vector<U64> Engine::findBestPosition(Parameters* p, Position currentPosition, PositionNode* root)
{
	int maxDepth = 0;
	WaitForSingleObject(p->mutex, INFINITE);
	maxDepth = p->maxDepth;
	ReleaseMutex(p->mutex);

	// initialize engine vector<U64> if necessary (remove this probably)
	bool hasLastEnginePosition = false;
	WaitForSingleObject(p->mutex, INFINITE);
	hasLastEnginePosition = (p->lastEnginePosition.size() > 0);
	ReleaseMutex(p->mutex);

	vector<U64> startPosition;
	if (!hasLastEnginePosition)
	{
		startPosition = helper::positionToU64(currentPosition);
		U64 startHashValue = getStartingHashValue(startPosition);

		int startEvaluation = getStartingEvaluation(startPosition);
		if (startEvaluation >= 0)
		{
			startPosition[BOARDEXTRA] = helper::setBitsFromInt(startPosition[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, startEvaluation);
			popBit(startPosition[BOARDEXTRA], NEGATIVEEVALUATION);
		}
		else
		{
			startPosition[BOARDEXTRA] = helper::setBitsFromInt(startPosition[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, startEvaluation * -1);
			setBit(startPosition[BOARDEXTRA], NEGATIVEEVALUATION);
		}

		startPosition[BOARDHASHVALUE] = startHashValue;
	}
	else
	{
		// get hash value of the position after the player moved
		vector<U64> lastEnginePosition;
		WaitForSingleObject(p->mutex, INFINITE);
		lastEnginePosition = p->lastEnginePosition;
		ReleaseMutex(p->mutex);

		vector<vector<U64>> positionsFromLastEnginePosition = helper::getNextPositions(lastEnginePosition, true, zobristTable);
		vector<U64> playerPosition = helper::positionToU64(currentPosition);

		PRINT("last engine\n\n", 2);
		WaitForSingleObject(p->mutex, INFINITE);
		if (p->debugPrint[2])
		{
			helper::printBoard(lastEnginePosition);
		}
		ReleaseMutex(p->mutex);
		

		PRINT("player\n\n", 2);
		WaitForSingleObject(p->mutex, INFINITE);
		if (p->debugPrint[2])
		{
			helper::printBoard(playerPosition);
		}
		ReleaseMutex(p->mutex);
		PRINT("candidates\n\n", 2);

		for (int i = 0; i < positionsFromLastEnginePosition.size(); i++)
		{
			//helper::printBoard(positionsFromLastEnginePosition[i]);

			bool match = true;
			for (int j = 0; j < BOARDEXTRA; j++)
			{
				if (positionsFromLastEnginePosition[i][j] != playerPosition[j])
				{
					match = false;
					break;
				}
			}

			for (int j = 0; j < 5; j++)
			{
				if (getBit(positionsFromLastEnginePosition[i][BOARDEXTRA], j) != getBit(playerPosition[BOARDEXTRA], j))
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				startPosition = positionsFromLastEnginePosition[i];
				break;
			}
		}

		assert(startPosition.size() > 0);
	}

	WaitForSingleObject(p->mutex, INFINITE);
	if (p->debugPrint[0])
	{
		PRINT("----------------- ENGINE -----------------\n\n", 0);
		helper::printBoard(startPosition);
	}
	ReleaseMutex(p->mutex);

	vector<vector<U64>> nextPositions = helper::getNextPositions(startPosition, false, zobristTable);
	WaitForSingleObject(p->mutex, INFINITE);
	for (int i = 0; i < nextPositions.size(); i++)
	{
		if (p->debugPrint[0])
		{
			helper::printBoard(nextPositions[i]);
		}		
	}
	ReleaseMutex(p->mutex);

	root->position = startPosition;
	root->min = true;
	root->value = FLT_MAX;
	root->id = -1;
	root->nextPositions = helper::getNextPositions(root->position, false, zobristTable);
	root->alpha = -FLT_MAX;
	root->beta = FLT_MAX;

	WaitForSingleObject(p->mutex, INFINITE);
	p->amountAtDepth[0] += 1;
	ReleaseMutex(p->mutex);

	WaitForSingleObject(p->mutex, INFINITE);
	PRINTNOLOCK("[", 1);
	for (int i = 0; i < 4; i++)
	{
		PRINTNOLOCK(to_string(p->amountAtDepth[i]) + " ", 1);
	}
	PRINTNOLOCK("], new depth 0\n", 1);
	ReleaseMutex(p->mutex);

	WaitForSingleObject(p->mutex, INFINITE);
	p->nodesCreated++;
	ReleaseMutex(p->mutex);

	//root->hashValue = getStartingHashValue(p, root->position);

	//printf("starting position next positions:\n");

	for (vector<U64> nextPosition : root->nextPositions)
	{
		PositionNode* newNode = new PositionNode;
		newNode->depth = root->depth + 1;
		newNode->min = !(root->min);
		newNode->position = nextPosition;
		newNode->parent = root;
		newNode->value = (newNode->min ? FLT_MAX : -FLT_MAX);

		WaitForSingleObject(p->mutex, INFINITE);
		PRINTNOLOCK("[", 1);
		for (int i = 0; i < 4; i++)
		{
			PRINTNOLOCK(to_string(p->amountAtDepth[i]) + " ", 1);
		}
		PRINTNOLOCK("], new depth " + to_string(root->depth + 1) + "\n", 1);
		ReleaseMutex(p->mutex);

		int id = -1;
		WaitForSingleObject(p->mutex, INFINITE);
		id = p->nodeId;
		p->nodeId++;
		p->nodesCreated++;
		ReleaseMutex(p->mutex);

		newNode->id = id;
		root->unvisited.push(id);

		root->children.push_back(newNode);
	}

#pragma endregion

	vector<U64> bestPosition;

	float totalFirstVisit = 0;
	float totalLargeWaited = 0;
	float totalSmallWaited = 0;
	float totalFindValue = 0;
	float totalMiscValue = 0;
	int smallWaitedCount = 0;
	int largeWaitedCount = 0;

	// stats
	int depth0Progress = 0;
	int depth0Children = root->children.size();
	int depth1Progress = 0;
	int depth1Children = 0;
	int nodesPruned = 0;

	auto begin = std::chrono::high_resolution_clock::now();

	PositionNode* currentNode = root;
	int id = 0;

	float totalWorkerTime = 0;
	float totalTime = 0;

	while (currentNode != NULL)
	{
		//WaitForSingleObject(p->mutex, INFINITE);
		//int stackSize = p->toExpand.size();
		//int expandedSize = p->expanded.size();
		//ReleaseMutex(p->mutex);

		auto point1 = std::chrono::high_resolution_clock::now();

		WaitForSingleObject(p->mutex, INFINITE);
		bool windowClosed = p->windowClosed;
		string gameResult = p->gameResult;
		p->depth0Progress = depth0Progress;
		p->depth0Children = depth0Children;
		p->depth1Progress = depth1Progress;
		p->depth1Children = depth1Children;
		ReleaseMutex(p->mutex);

		if (windowClosed || gameResult.length() > 0) { break; }

		bool pruneChildren = false;

		if (currentNode->depth == maxDepth - 1)
		{
			if (currentNode->depth > 0 && ((currentNode->parent->min && currentNode->value < currentNode->parent->value) ||
				(!currentNode->parent->min && currentNode->value > currentNode->parent->value)))
			{
				currentNode->parent->value = currentNode->value;
				currentNode->parent->bestChildId = currentNode->id;
			}
			currentNode = currentNode->parent;
		}

		// if the node has no value, then this is the first visit to this node and its children are not ready to visit
		if (currentNode->value == FLT_MAX || currentNode->value == -FLT_MAX)
		{
			for (PositionNode* child : currentNode->children)
			{
				WaitForSingleObject(p->mutex, INFINITE);
				p->toExpand.push((char*)child);
				int stackSize = p->toExpand.size();
				int expandedSize = p->expanded.size();
				ReleaseMutex(p->mutex);

				if (currentNode->depth == 1)
				{
					depth1Progress = 1;
				}
			}
		}

		// if the node has a value, then it has visited a child who set a value for this node
		else
		{
			if (!currentNode->min)
			{
				currentNode->alpha = max(currentNode->alpha, currentNode->value);
			}
			else
			{
				currentNode->beta = min(currentNode->beta, currentNode->value);
			}

			pruneChildren = currentNode->beta <= currentNode->alpha;
		}

		// if all children have been visited or pruned
		if (currentNode->unvisited.size() == 0 || pruneChildren)
		{
			if (currentNode->depth > 0 && ((currentNode->parent->min && currentNode->value < currentNode->parent->value) ||
				(!currentNode->parent->min && currentNode->value > currentNode->parent->value)))
			{
				currentNode->parent->value = currentNode->value;
				currentNode->parent->bestChildId = currentNode->id;
			}

			if (currentNode->depth > 0 && currentNode->parent->depth == 0)
			{
				depth0Progress += 1;
			}
			if (currentNode->depth > 0 && currentNode->parent->depth == 1)
			{
				depth1Progress += 1;
				depth1Children = currentNode->parent->children.size();
			}

			currentNode = currentNode->parent;
		}


		// otherwise, evaluate next child
		else if (currentNode->unvisited.size() > 0)
		{
			int nextNodeId = currentNode->unvisited.front();
			currentNode->unvisited.pop();

			PositionNode* nextNode = nullptr;

			auto workerPoint1 = std::chrono::high_resolution_clock::now();
			while (nextNode == nullptr)
			{
				WaitForSingleObject(p->mutex, INFINITE);
				nextNode = (PositionNode*)p->expanded[nextNodeId];
				windowClosed = p->windowClosed;
				gameResult = p->gameResult;
				ReleaseMutex(p->mutex);

				if (windowClosed || gameResult.length() > 0) { break; }
			}
			if (windowClosed || gameResult.length() > 0) { continue; }

			auto workerPoint2 = std::chrono::high_resolution_clock::now();
			totalWorkerTime += float(std::chrono::duration_cast<std::chrono::nanoseconds>(workerPoint2 - workerPoint1).count()) / 1e6;

			WaitForSingleObject(p->mutex, INFINITE);
			p->expanded.erase(nextNodeId);
			ReleaseMutex(p->mutex);

			nextNode->alpha = currentNode->alpha;
			nextNode->beta = currentNode->beta;
			currentNode = nextNode;
		}

		auto point2 = std::chrono::high_resolution_clock::now();
		totalTime += float(std::chrono::duration_cast<std::chrono::nanoseconds>(point2 - point1).count()) / 1e6;
	}

	//if (dpr)
	//{
	//	printf("best child id %d\n", root->bestChildId);
	//	printf("best child value %d\n", root->children[root->bestChildId]->value);
	//	for (auto child : root->children)
	//	{
	//		printf("root child id %d\n", child.first);
	//	}
	//}

	//if (dpr)
	//{
	//	printf("small %f ms; large %f ms\n", totalSmallWaited, totalLargeWaited);
	//	printf("small count %d; large count %d\n", smallWaitedCount, largeWaitedCount);
	//	printf("firstvisit %f\n", totalFirstVisit);
	//	printf("findvalue %f\n", totalFindValue);
	//	printf("miscvalue %f\n", totalMiscValue);

	//	printf("engine time %.3f ms\n", totalTime - totalWorkerTime);
	//	printf("worker time %.3f ms\n", totalWorkerTime);
	//}

	int foundcount = 0;
	WaitForSingleObject(p->mutex, INFINITE);
	foundcount = p->foundInCache;
	ReleaseMutex(p->mutex);

	//printf("found in cache: %d\n", foundcount);

	bestPosition = root->children[root->bestChildId]->position;

	//currentNode = root;
	//for (int i = 0; i < maxDepth; i++)
	//{
	//	if (currentNode == NULL) { break; }

	//	printf("depth %d min %d best value %f\n", i, currentNode->min, currentNode->value);// best value %f id %d bestchildid %d\n", currentNode->depth, bestValue, currentNode->id, bestChildId);
	//	printf("children: ");
	//	for (PositionNode* child : currentNode->children)
	//	{
	//		printf("%f ", child->value);
	//	}
	//	printf("\n");
	//	helper::printBoard(currentNode->position);
	//	printf("---------------------------\n");

	//	for (PositionNode* child : currentNode->children)
	//	{
	//		if (child->id == currentNode->bestChildId)
	//		{
	//			currentNode = child;
	//		}
	//	}
	//}

	//currentNode = root;
	//for (PositionNode* child : currentNode->children)
	//{
	//	printf("id %d value %d\n", child->id, child->value);
	//	helper::printBoard(child->position);
	//}

	//currentNode = root;
	//for (int i = 0; i < currentNode->children.size(); i++)
	//{
	//	if (currentNode->children[i]->id == 12)
	//	{
	//		currentNode = currentNode->children[i];
	//		helper::printBoard(currentNode->position);

	//		for (PositionNode* child : currentNode->children)
	//		{
	//			printf("id %d value %d\n", child->id, child->value);
	//			helper::printBoard(child->position);
	//		}
	//	}
	//}

	auto end = std::chrono::high_resolution_clock::now();

	return bestPosition;
}

void Engine::deletePositionTree(Parameters* p, PositionNode* node)
{
	if (node == NULL) { return; }

	if (node->depth < 2)
	{
		for (PositionNode* child : node->children)
		{
			deletePositionTree(p, child);
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

	if (node->depth == 2)
	{
		WaitForSingleObject(p->mutex, INFINITE);
		p->toDelete.push((char*)node);
		ReleaseMutex(p->mutex);
	}
}

void Engine::endEngineTurn(Parameters* p, vector<U64> bestPosition)
{
	while (true)
	{
		WaitForSingleObject(p->mutex, INFINITE);
		if (p->nodesDeleted == p->nodesCreated)
		{
			ReleaseMutex(p->mutex);
			break;
		}
		else
		{
			bool windowClosed = p->windowClosed;
			string gameResult = p->gameResult;
			if (windowClosed || gameResult.length() > 0) { ReleaseMutex(p->mutex); break; }
			ReleaseMutex(p->mutex);
		}
	}

	WaitForSingleObject(p->mutex, INFINITE);
	while (!p->toExpand.empty()) { p->toExpand.pop(); }
	p->expanded.clear();
	p->nodeId = 0;
	ReleaseMutex(p->mutex);

	auto end = std::chrono::high_resolution_clock::now();

	//if (dpr)
	//{
	//	printf("engine cleanup done in %.3f ms\n\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
	//}

	WaitForSingleObject(p->mutex, INFINITE);
	p->lastEnginePosition = bestPosition;
	ReleaseMutex(p->mutex);

	Position bestPositionWindow = helper::U64ToPosition(bestPosition);

	checkGameEnded(p, bestPositionWindow, false);

	// send the resulting position to parameters
	WaitForSingleObject(p->mutex, INFINITE);
	p->currentPosition = bestPositionWindow;
	ReleaseMutex(p->mutex);

	// indicate that it is the player's move
	WaitForSingleObject(p->mutex, INFINITE);
	p->playerToMove = true;
	ReleaseMutex(p->mutex);
}

UINT Engine::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	srand((unsigned int)time(NULL));

	WaitForSingleObject(p->mutex, INFINITE);
	for (int i = 0; i < p->maxDepth; i++)
	{
		p->amountAtDepth.push_back(0);
	}
	ReleaseMutex(p->mutex);

	// main engine loop
	while (true)
	{
		// check if window closed or game ended
		bool windowClosed = false;
		string gameResult = "";

		// check if engine's turn
		bool engineToMove = false;

		WaitForSingleObject(p->mutex, INFINITE);
		engineToMove = !p->playerToMove;
		windowClosed = p->windowClosed;
		gameResult = p->gameResult;
		ReleaseMutex(p->mutex);

		if (windowClosed || gameResult.length() > 0) { break; }

		if (!engineToMove)
		{
			continue;
		}

		WaitForSingleObject(p->mutex, INFINITE);
		p->deleteSignal = false;
		p->readyToDelete.clear();
		p->startedClear = false;
		ReleaseMutex(p->mutex);

		Position currentPosition;
		WaitForSingleObject(p->mutex, INFINITE);
		currentPosition = p->currentPosition;
		ReleaseMutex(p->mutex);

		if (checkGameEnded(p, currentPosition, true))
		{
			continue;
		}

		auto begin = std::chrono::high_resolution_clock::now();

		PositionNode* root = new PositionNode;
		vector<U64> bestPosition = findBestPosition(p, currentPosition, root);

		auto end = std::chrono::high_resolution_clock::now();

		//if (dpr)
		//{
		//	printf("engine move found in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		//}

		WaitForSingleObject(p->mutex, INFINITE);
		p->deleteSignal = true;
		ReleaseMutex(p->mutex);

		while (true)
		{
			WaitForSingleObject(p->mutex, INFINITE);
			windowClosed = p->windowClosed;
			ReleaseMutex(p->mutex);
			if (windowClosed) { break; }

			WaitForSingleObject(p->mutex, INFINITE);
			if (p->readyToDelete.size() == p->workerCount)
			{
				ReleaseMutex(p->mutex);
				break;
			}
			else
			{
				ReleaseMutex(p->mutex);
			}
		}

		begin = std::chrono::high_resolution_clock::now();

		deletePositionTree(p, root);

		endEngineTurn(p, bestPosition);

		end = std::chrono::high_resolution_clock::now();

		//if (dpr)
		//{
		//	printf("engine cleanup done in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		//}
	}

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	return 0;

}