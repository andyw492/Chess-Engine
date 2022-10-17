#include "engine.h"

Engine::Engine(bool enginePrint)
{
	dpr = enginePrint;
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



void Engine::deletePositionTree(PositionNode* node)
{
	if (node == NULL) { return; }

	for (PositionNode* child : node->children)
	{
		deletePositionTree(child);
	}

	delete node;
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

		PositionNode* root = new PositionNode;
		root->position = helper::positionToU64(currentPosition);
		root->min = true;
		root->value = FLT_MAX;
		root->id = -1;
		root->nextPositions = helper::getNextPositions(root->position, false);
		root->alpha = -FLT_MAX;
		root->beta = FLT_MAX;

		// check if engine lost
		if (root->nextPositions.size() == 0)
		{
			if (helper::inCheck(root->position, false))
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

			break;
		}

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

					if (dpr && currentNode->depth == 1)
					{
						printf("toExpand size: %d expanded size: %d\n", stackSize, expandedSize);
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

		bestPosition = root->children[root->bestChildId]->position;

		currentNode = root;
		for (int i = 0; i < maxDepth; i++)
		{
			if (currentNode == NULL) { break; }

			printf("depth %d\n", i);// best value %f id %d bestchildid %d\n", currentNode->depth, bestValue, currentNode->id, bestChildId);
			helper::printBoard(currentNode->position);
			printf("---------------------------\n");

			for (PositionNode* child : currentNode->children)
			{
				if (child->id == currentNode->bestChildId)
				{
					currentNode = child;
				}
			}
		}

		auto end = std::chrono::high_resolution_clock::now();

		if (dpr || true)
		{
			printf("engine move found in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		}

		begin = std::chrono::high_resolution_clock::now();

		deletePositionTree(root);

		WaitForSingleObject(p->mutex, INFINITE);
		while (!p->toExpand.empty()) { p->toExpand.pop(); }
		p->expanded.clear();
		p->nodeId = 0;
		ReleaseMutex(p->mutex);

		end = std::chrono::high_resolution_clock::now();

		if (dpr)
		{
			printf("engine cleanup done in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		}

		Position bestPositionWindow = helper::U64ToPosition(bestPosition);

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

		// check if player lost
		vector<vector<U64>> nextPositionsFromBestPosition = helper::getNextPositions(bestPosition, true);

		if (nextPositionsFromBestPosition.size() == 0)
		{
			if (helper::inCheck(bestPosition, true))
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
		}

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