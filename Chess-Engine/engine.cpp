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

double Engine::evaluate(PositionNode* node)
{
	// find the value for the base position (legal moves are made from this position)
	// calculate value as (white total piece value - black total piece value)
	double baseValue = 0;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			char piece = node->position.board[y][x];
			float pieceValue = (piece < 97 ? 1 : -1);
			switch (piece)
			{
			case 'K':
			case 'k':
				pieceValue *= 10000; break;
	
			case 'Q':
			case 'q':
				pieceValue *= 9; break;
	
			case 'R':
			case 'r':
				pieceValue *= 5; break;
	
			case 'B':
			case 'b':
				pieceValue *= 3.2; break;
	
			case 'N':
			case 'n':
				pieceValue *= 3; break;
	
			case 'P':
			case 'p':
				pieceValue *= 1; break;
	
			case ' ':
				pieceValue = 0; break;
	
			default:
				cout << "invalid piece" << endl; break;
			}
	
			baseValue += pieceValue;
		}
	}

	double bestValue = (node->min ? FLT_MAX : -FLT_MAX);

	// find the best value by simulating each legal move and finding the value change from the move
	for (auto move : node->legalMoves)
	{
		char movingPiece = node->position.board[move.first.at(0) - '0'][move.first.at(1) - '0'];
		bool pawnMove = (movingPiece == 'P') || (movingPiece == 'p');

		for (string dest : move.second)
		{
			double valueChange = (node->min ? -1 : 1);

			int y = dest.at(0) - '0';
			int x = dest.at(1) - '0';
			char piece = node->position.board[y][x];
			switch (piece)
			{
			case 'K':
			case 'k':
				valueChange *= DBL_MAX; break;
				
			case 'Q':
			case 'q':
				valueChange *= 9; break;
				
			case 'R':
			case 'r':
				valueChange *= 5; break;
				
			case 'B':
			case 'b':
				valueChange *= 3.2; break;
				
			case 'N':
			case 'n':
				valueChange *= 3; break;
				
			case 'P':
			case 'p':
				valueChange *= 1; break;

			case ' ':
				// check for en passant
				bool whiteEnPassant = pawnMove && !node->min && node->position.board[y + 1][x] == 'p';
				bool blackEnPassant = pawnMove && node->min && node->position.board[y - 1][x] == 'P';
				if (whiteEnPassant || blackEnPassant)
				{
					valueChange *= 1;
				}
				else
				{
					valueChange = 0;
				}

				break;
			}

			double newValue = baseValue + valueChange;
			bestValue = (node->min ? min(bestValue, newValue) : max(bestValue, newValue));
		}
	}

	// slightly randomize value to prevent repeating moves
	return bestValue + ((double)rand() / RAND_MAX) / 100;
}

//double Engine::evaluate(Position position)
//{
//	// evaluation: calculate value as (white total piece value - black total piece value)
//	double totalValue = 0;
//	for (int y = 0; y < 8; y++)
//	{
//		for (int x = 0; x < 8; x++)
//		{
//			char piece = position.board[y][x];
//			float pieceValue = (piece < 97 ? 1 : -1);
//			switch (piece)
//			{
//			case 'K':
//			case 'k':
//				pieceValue *= 10000; break;
//
//			case 'Q':
//			case 'q':
//				pieceValue *= 9; break;
//
//			case 'R':
//			case 'r':
//				pieceValue *= 5; break;
//
//			case 'B':
//			case 'b':
//				pieceValue *= 3.2; break;
//
//			case 'N':
//			case 'n':
//				pieceValue *= 3; break;
//
//			case 'P':
//			case 'p':
//				pieceValue *= 1; break;
//
//			case ' ':
//				pieceValue = 0; break;
//
//			default:
//				cout << "invalid piece" << endl; break;
//			}
//
//			totalValue += pieceValue;
//		}
//	}
//
//	// slightly randomize value to prevent repeating moves
//	return totalValue + ((double)rand() / RAND_MAX) / 100;
//}

void Engine::deletePositionTree(PositionNode* node)
{
	for (auto child : node->children)
	{
		deletePositionTree(child.second);
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
		root->position = currentPosition;
		root->min = true;
		root->value = FLT_MAX;
		root->id = -1;

		map<string, vector<string>> legalMoves = helper::getLegalMoves(root->position, false);
		root->legalMoves = legalMoves;

		// check if engine lost
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

		if (dpr)
		{
			system("cls");
			cout << "----------------- ENGINE -----------------\n" << endl;
			for (auto i : legalMoves)
			{
				cout << i.first << ": ";
				for (int j = 0; j < i.second.size(); j++)
				{
					cout << i.second[j] << " ";
				}
				cout << endl;
			}
			cout << endl;
		}

#pragma endregion


		Position bestPosition;

		startClock();
		float totalFirstVisit = 0;
		float totalLargeWaited = 0;
		float totalSmallWaited = 0;
		float totalFindValue = 0;
		int smallWaitedCount = 0;
		int largeWaitedCount = 0;

		auto begin = std::chrono::high_resolution_clock::now();

		PositionNode* currentNode = root;
		int id = 0;

		while(currentNode != NULL)
		{
			WaitForSingleObject(p->mutex, INFINITE);
			windowClosed = p->windowClosed;
			gameResult = p->gameResult;
			ReleaseMutex(p->mutex);

			if (windowClosed || gameResult.length() > 0) { break; }

			// if current node is at depth limit, find the node's value and move to parent
			if (currentNode->depth == maxDepth - 1)
			{
				auto point1 = std::chrono::high_resolution_clock::now();

				currentNode->value = evaluate(currentNode);
				currentNode = currentNode->parent;

				auto point2 = std::chrono::high_resolution_clock::now();
				totalFindValue += float(std::chrono::duration_cast<std::chrono::nanoseconds>(point2 - point1).count()) / 1e6;
			}

			// if there are no legal moves at the current node (i.e. the position is a stalemate or checkmate), find the node's value and move to parent
			else if (currentNode->legalMoves.size() == 0)
			{
				if (helper::inCheck(currentNode->position.board, !currentNode->min))
				{
					currentNode->value = -INT_MAX;
				}
				else if (helper::inCheck(currentNode->position.board, currentNode->min))
				{
					currentNode->value = INT_MAX;
				}
				else
				{
					currentNode->value = 0;
				}
				
				currentNode = currentNode->parent;
			}
			
			// otherwise, expand the current node
			else
			{
				if (currentNode->firstVisit)
				{
					auto point1 = std::chrono::high_resolution_clock::now();

					currentNode->firstVisit = false;

					for (auto move : currentNode->legalMoves)
					{
						for (int i = 0; i < move.second.size(); i++)
						{
							PositionNode* newNode = new PositionNode;
							newNode->depth = currentNode->depth + 1;
							newNode->min = !(currentNode->min);
							newNode->position = helper::getNewPosition(currentNode->position, move.first, move.second[i]);
							newNode->parent = currentNode;
							newNode->value = (newNode->min ? FLT_MAX : -FLT_MAX);
							newNode->id = id;

							newNode->prevMove = move.first + " -> " + move.second[i];

							currentNode->children.insert({ id, newNode });
							currentNode->unvisited.insert(id);

							id++;

							WaitForSingleObject(p->mutex, INFINITE);
							p->toExpand.push((char*)newNode);
							int stackSize = p->toExpand.size();
							int expandedSize = p->expanded.size();
							ReleaseMutex(p->mutex);

							if (dpr && currentNode->depth == 2)
							{
								//printf("toExpand size: %d expanded size: %d\n", stackSize, expandedSize);
							}
						}
					}

					auto point2 = std::chrono::high_resolution_clock::now();
					totalFirstVisit += float(std::chrono::duration_cast<std::chrono::nanoseconds>(point2 - point1).count()) / 1e6;
				}

				if (currentNode->unvisited.size() > 0)
				{
					auto it = currentNode->unvisited.begin();
					int nextNodeId = *it;
					currentNode->unvisited.erase(it);

					PositionNode* nextNode = nullptr;

					auto point1 = std::chrono::high_resolution_clock::now();

					while (nextNode == nullptr)
					{
						WaitForSingleObject(p->mutex, INFINITE);
						nextNode = (PositionNode*)p->expanded[nextNodeId];
						windowClosed = p->windowClosed;
						gameResult = p->gameResult;
						ReleaseMutex(p->mutex);

						if (windowClosed || gameResult.length() > 0) { break; }
					}

					auto point2 = std::chrono::high_resolution_clock::now();

					float timeWaited = float(std::chrono::duration_cast<std::chrono::nanoseconds>(point2 - point1).count()) / 1e6;
					
					if (timeWaited < 10)
					{
						totalSmallWaited += timeWaited;
						smallWaitedCount += 1;
					}
					
					if (dpr && timeWaited < 10)
					{
						//printf("waited %.3f ms for node\n", timeWaited);
						//printf("total < 10: %.3f ms\n", totalSmallWaited);
					}

					if (timeWaited > 10)
					{
						totalLargeWaited += timeWaited;
						largeWaitedCount += 1;
					}
					
					if (dpr && timeWaited > 10)
					{
						//printf("waited %.3f ms for node\n", timeWaited);
						//printf("total > 10: %.3f ms\n", totalLargeWaited);
					}

					WaitForSingleObject(p->mutex, INFINITE);
					p->expanded.erase(nextNodeId);
					ReleaseMutex(p->mutex);

					currentNode = nextNode;
				}

				else
				{
					float bestValue = (currentNode->min ? FLT_MAX : -FLT_MAX);
					int bestChildId = 0;

					for (auto child : currentNode->children)
					{
						//if (dpr && currentNode->depth == 0)
						//{
						//	printf("child id %d child value %f best child id %d best value %f\n", child.first, child.second->value, bestChildId, bestValue);
						//}

						float value = child.second->value;

						if ((currentNode->min && value < bestValue) || (!currentNode->min && value > bestValue))
						{
							bestValue = value;
							bestChildId = child.first;
						}
					}

					if (dpr && currentNode->depth <= 1)
					{
						printf("depth %d all children visited, best value == %f\n", currentNode->depth, bestValue);
					}
					
					currentNode->value = bestValue;
					currentNode->bestChildId = bestChildId;
					currentNode = currentNode->parent;
				}
			}
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
			printf("small %f ms; large %f ms\n", totalSmallWaited, totalLargeWaited);
			printf("small count %d; large count %d\n", smallWaitedCount, largeWaitedCount);
			printf("firstvisit %f\n", totalFirstVisit);
			printf("findvalue %f\n", totalFindValue);
		}


		bestPosition = root->children[root->bestChildId]->position;

		auto end = std::chrono::high_resolution_clock::now();

		if (dpr || true)
		{
			printf("engine move found in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
		}
		

		// delete the position node tree
		deletePositionTree(root);

		if (dpr)
		{
			cout << "sending best position:" << endl;
			for (int a = 0; a < 8; a++)
			{
				for (int b = 0; b < 8; b++)
				{
					cout << bestPosition.board[a][b] << " ";
				}
				cout << endl;
			}
		}

		// send the resulting position to parameters
		WaitForSingleObject(p->mutex, INFINITE);
		p->currentPosition = bestPosition;
		ReleaseMutex(p->mutex);

		// check if player lost
		legalMoves = helper::getLegalMoves(bestPosition, true);

		if (legalMoves.size() == 0)
		{
			if (helper::inCheck(bestPosition.board, true))
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

		printTime();

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