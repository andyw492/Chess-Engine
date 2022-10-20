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

void Engine::printTime()
{
	if (!dpr) { return; }

	float total = 1000.0 * (clock() - clockStart) / CLOCKS_PER_SEC;
	cout << std::setw(3) << std::setfill(' ') << total - lastTime;
	cout << " ms elapsed; " << total << " ms total" << endl;
	lastTime = total;
}

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

void Engine::deletePositionTree(PositionNode* node)
{
	if (node == NULL) { return; }

	for (PositionNode* child : node->children)
	{
		deletePositionTree(child);
	}

	delete node;
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

UINT Engine::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	srand((unsigned int)time(NULL));

	WaitForSingleObject(p->mutex, INFINITE);
	printf("engineThread %d started\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	while(true)
	{
#pragma region

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
		
		// get info from parameters
		int maxDepth = 0;
		Position currentPosition;

		WaitForSingleObject(p->mutex, INFINITE);
		maxDepth = p->maxDepth;
		currentPosition = p->currentPosition;
		ReleaseMutex(p->mutex);

		// check if engine lost
		map<string, vector<string>> legalMoves = helper::getLegalMoves(currentPosition, false);
		if (legalMoves.size() == 0)
		{
			if (helper::inCheck(currentPosition.board, false))
			{
				WaitForSingleObject(p->mutex, INFINITE);
				p->gameResult = "you won!!";
				ReleaseMutex(p->mutex);
			}
			else
			{
				WaitForSingleObject(p->mutex, INFINITE);
				p->gameResult = "stalemate...";
				ReleaseMutex(p->mutex);
			}
		}

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

			for(int i = 0; i < positionsFromLastEnginePosition.size(); i++)
			{
				//printf("from last engine position\n");
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

		//printf("start value %d\n", getStartingEvaluation(startPosition));

		PositionNode* root = new PositionNode;
		root->position = startPosition;
		root->min = true;
		root->value = FLT_MAX;
		root->id = -1;
		root->nextPositions = helper::getNextPositions(root->position, false, zobristTable);
		root->alpha = -FLT_MAX;
		root->beta = FLT_MAX;
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

			int id = -1;
			WaitForSingleObject(p->mutex, INFINITE);
			id = p->nodeId;
			p->nodeId++;
			ReleaseMutex(p->mutex);

			newNode->id = id;
			root->unvisited.push(id);

			//if (getBit(newNode->position[BLACKKNIGHT], 18))
			//{
			//	printf("position has id %d\n", newNode->id);
			//	helper::printBoard(nextPosition);
			//	printf("----------------\n");
			//}

			root->children.push_back(newNode);
		}

		//if (dpr)
		//{
		//	system("cls");
		//	cout << "----------------- ENGINE -----------------\n" << endl;
		//	for (auto i : legalMoves)
		//	{
		//		cout << i.first << ": ";
		//		for (int j = 0; j < i.second.size(); j++)
		//		{
		//			cout << i.second[j] << " ";
		//		}
		//		cout << endl;
		//	}
		//	cout << endl;
		//}

#pragma endregion

		vector<U64> bestPosition;

		float totalFirstVisit = 0;
		float totalLargeWaited = 0;
		float totalSmallWaited = 0;
		float totalFindValue = 0;
		float totalMiscValue = 0;
		int smallWaitedCount = 0;
		int largeWaitedCount = 0;

		auto begin = std::chrono::high_resolution_clock::now();

		PositionNode* currentNode = root;
		int id = 0;

		float totalWorkerTime = 0;
		float totalTime = 0;

		while (currentNode != NULL)
		{
			WaitForSingleObject(p->mutex, INFINITE);
			int stackSize = p->toExpand.size();
			int expandedSize = p->expanded.size();
			ReleaseMutex(p->mutex);

			auto point1 = std::chrono::high_resolution_clock::now();

			WaitForSingleObject(p->mutex, INFINITE);
			windowClosed = p->windowClosed;
			gameResult = p->gameResult;
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

					if (dpr && currentNode->depth <= 0)
					{
						//printf("toExpand size: %d expanded size: %d\n", stackSize, expandedSize);
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

				//if (pruneChildren)
				//{
				//	if (isContains(currentNode->unvisited, 10))
				//	{
				//		printf("----------------\n");
				//	}
				//}

				//if (pruneChildren)
				//{
				//	printf("from position min node %d alpha %f beta %f\n", currentNode->min, currentNode->alpha, currentNode->beta);
				//	helper::printBoard(currentNode->position);
				//	printf("pruned children:\n");
				//	for (PositionNode* child : currentNode->children)
				//	{
				//		if (isContains(currentNode->unvisited, child->id))
				//		{
				//			helper::printBoard(child->position);
				//			printf("----------------\n");
				//		}
				//	}
				//}
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

				//if (currentNode->id == 12)
				//{
				//	printf("min node %d depth %d value %f\n", currentNode->min, currentNode->depth, currentNode->value);
				//	helper::printBoard(currentNode->position);
				//	printf("child values:\n");
				//	for (PositionNode* child : currentNode->children)
				//	{
				//		printf("%f ", child->value);

				//		if (child->value == 1)
				//		{
				//			printf("value 1 position\n");
				//			helper::printBoard(child->position);
				//			printf("value 1 children\n");
				//			for (PositionNode* grandchild : child->children)
				//			{
				//				printf("min node %d depth %d value %f\n", grandchild->min, grandchild->depth, grandchild->value);
				//				helper::printBoard(grandchild->position);
				//			}
				//			printf("");
				//		}
				//	}
				//	printf("\n");
				//}


				//if (currentNode->id == 10)
				//{
				//	for (PositionNode* child : currentNode->children)
				//	{
				//		if (child->value > -2)
				//		{
				//			printf("> -2 position\n");
				//			helper::printBoard(child->position);
				//		}
				//	}
				//}

				currentNode = currentNode->parent;

				//if (currentNode != NULL && currentNode->id == 12)
				//{
				//	PositionNode* ptr = currentNode;
				//	for (int i = ptr->depth; i < maxDepth; i++)
				//	{
				//		if (ptr == NULL) { break; }

				//		printf("depth %d min %d best value %f\n", i, ptr->min, ptr->value);// best value %f id %d bestchildid %d\n", ptr->depth, bestValue, ptr->id, bestChildId);
				//		printf("children: ");
				//		for (PositionNode* child : ptr->children)
				//		{
				//			printf("%f ", child->value);
				//		}
				//		printf("\n");
				//		helper::printBoard(ptr->position);
				//		printf("---------------------------\n");

				//		for (PositionNode* child : ptr->children)
				//		{
				//			if (child->id == ptr->bestChildId)
				//			{
				//				ptr = child;
				//			}
				//		}
				//	}
				//}
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

		if (dpr)
		{
			//printf("best child id %d\n", root->bestChildId);
			//printf("best child value %d\n", root->children[root->bestChildId]->value);
			//for (auto child : root->children)
			//{
			//	printf("root child id %d\n", child.first);
			//}
		}

		if (dpr)
		{
			//printf("small %f ms; large %f ms\n", totalSmallWaited, totalLargeWaited);
			//printf("small count %d; large count %d\n", smallWaitedCount, largeWaitedCount);
			//printf("firstvisit %f\n", totalFirstVisit);
			//printf("findvalue %f\n", totalFindValue);
			//printf("miscvalue %f\n", totalMiscValue);

			printf("engine time %.3f ms\n", totalTime - totalWorkerTime);
			printf("worker time %.3f ms\n", totalWorkerTime);
		}

		int foundcount = 0;
		WaitForSingleObject(p->mutex, INFINITE);
		foundcount = p->foundInCache;
		p->foundInCache = 0;
		ReleaseMutex(p->mutex);

		printf("found in cache: %d\n", foundcount);

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

		if (dpr || true)
		{
			printf("engine move found in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		}

		begin = std::chrono::high_resolution_clock::now();

		//printTree(root);

		deletePositionTree(root);

		WaitForSingleObject(p->mutex, INFINITE);
		while (!p->toExpand.empty()) { p->toExpand.pop(); }
		p->expanded.clear();
		p->nodeId = 0;
		ReleaseMutex(p->mutex);

		end = std::chrono::high_resolution_clock::now();

		if (dpr)
		{
			printf("engine cleanup done in %.3f ms\n\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		}

		WaitForSingleObject(p->mutex, INFINITE);
		p->lastEnginePosition = bestPosition;
		ReleaseMutex(p->mutex);

		Position bestPositionWindow = helper::U64ToPosition(bestPosition);

		// check if player lost
		legalMoves = helper::getLegalMoves(bestPositionWindow, true);

		if (legalMoves.size() == 0)
		{
			if (helper::inCheck(bestPositionWindow.board, true))
			{
				WaitForSingleObject(p->mutex, INFINITE);
				p->gameResult = "you lost!!";
				ReleaseMutex(p->mutex);
			}
			else
			{
				WaitForSingleObject(p->mutex, INFINITE);
				p->gameResult = "stalemate...";
				ReleaseMutex(p->mutex);
			}
			cout << "";
		}

		//if (dpr)
		//{
		//	cout << "sending best position:" << endl;
		//	for (int a = 0; a < 8; a++)
		//	{
		//		for (int b = 0; b < 8; b++)
		//		{
		//			cout << bestPositionWindow.board[a][b] << " ";
		//		}
		//		cout << endl;
		//	}
		//}

		// send the resulting position to parameters
		WaitForSingleObject(p->mutex, INFINITE);
		p->currentPosition = bestPositionWindow;
		ReleaseMutex(p->mutex);

		// indicate that it is the player's move
		WaitForSingleObject(p->mutex, INFINITE);
		p->playerToMove = true;
		ReleaseMutex(p->mutex);
	}

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("engineThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;

}