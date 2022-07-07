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
	float total = 1000.0 * (clock() - clockStart) / CLOCKS_PER_SEC;
	cout << std::setw(3) << std::setfill(' ') << total - lastTime;
	cout << " ms elapsed; " << total << " ms total" << endl;
	lastTime = total;
}

void Engine::deletePositionTree(PositionNode* node)
{
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
		// check if window is still open
		bool windowClosed = false;
		string gameResult = "";
		WaitForSingleObject(p->mutex, INFINITE);
		windowClosed = p->windowClosed;
		gameResult = p->gameResult;
		ReleaseMutex(p->mutex);
		if (windowClosed || gameResult.length() > 0) { break; }

		// check if engine's turn
		bool engineToMove = false;
		WaitForSingleObject(p->mutex, INFINITE);
		engineToMove = !p->playerToMove;
		ReleaseMutex(p->mutex);

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
		PositionNode* currentNode = root;

		map<string, vector<string>> legalMoves = helper::getLegalMoves(root->position, false);

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


		Position bestPosition;

		startClock();

		// find the resulting position from the best move
		// pseudocode: https://pastebin.com/MNrCY7eu
		while (true)
		{
			// check if window is still open
			bool windowClosed = false;
			string gameResult = "";
			WaitForSingleObject(p->mutex, INFINITE);
			windowClosed = p->windowClosed;
			gameResult = p->gameResult;
			ReleaseMutex(p->mutex);
			if (windowClosed || gameResult.length() > 0) { break; }

			bool allEvaluated = true;
			bool gameEnd = false;

			if (currentNode->children.size() == 0)
			{
				allEvaluated = false;
			}
			for (int i = 0; i < currentNode->children.size(); i++)
			{
				if (!currentNode->children[i]->evaluated) { allEvaluated = false; }
			}

			if (currentNode->depth == 0 && allEvaluated)
			{
				float minValue = FLT_MAX;
				for (int i = 0; i < currentNode->children.size(); i++)
				{
					if (currentNode->children[i]->value < minValue)
					{
						bestPosition = currentNode->children[i]->position;
						minValue = currentNode->children[i]->value;
					}
				}

				break;
			}

			if (dpr)
			{
				printTime();
				cout << "expanding tree" << endl;
			}

			while ((!allEvaluated || currentNode->children.size() == 0) && !gameEnd)
			{
				if (currentNode->children.size() == 0)
				{
					if (dpr)
					{
						//cout << "\ndepth " << currentNode->depth << " board, creating children" << endl;
						//for (int i = 0; i < 8; i++)
						//{
						//	for (int j = 0; j < 8; j++)
						//	{
						//		cout << currentNode->position.board[i][j] << " ";
						//	}
						//	cout << endl;
						//}
						cout << "\ncurrent depth " << currentNode->depth << ", creating children" << endl;
						printTime();
					}

					Position position = currentNode->position;
					map<string, vector<string>> legalMoves = helper::getLegalMoves(position, !currentNode->min);

					// if no legal moves in the position, then set value to:
					// 0 if stalemate
					// 10000 if white victory
					// -10000 if black victory
					if (legalMoves.size() == 0)
					{
						gameEnd = true;
						currentNode->evaluated = true;

						if (helper::inCheck(position.board, !currentNode->min))
						{
							currentNode->value = -10000;
						}
						else if (helper::inCheck(position.board, currentNode->min))
						{
							currentNode->value = 10000;
						}
						else
						{
							currentNode->value = 0;
						}
						cout << "";
					}

					for (auto move : legalMoves)
					{
						for (int i = 0; i < move.second.size(); i++)
						{
							PositionNode* newNode = new PositionNode;
							newNode->depth = currentNode->depth + 1;
							newNode->position = helper::getNewPosition(position, move.first, move.second[i]);
							newNode->min = !(currentNode->min);
							newNode->value = (newNode->min ? FLT_MAX : -FLT_MAX);
							newNode->parent = currentNode;

							newNode->prevMove = move.first + " -> " + move.second[i];

							currentNode->children.push_back(newNode);
						}
					}
				}

				for (int i = 0; i < currentNode->children.size(); i++)
				{
					if (!currentNode->children[i]->evaluated)
					{
						currentNode = currentNode->children[i];
						cout << "\nenter depth " << currentNode->depth << endl;
						printTime();
						break;

					}
				}

				if (currentNode->depth == maxDepth - 1 && !currentNode->evaluated)
				{
					WaitForSingleObject(p->mutex, INFINITE);
					p->toEvaluate = (char*)currentNode;
					p->evaluated = nullptr;
					ReleaseMutex(p->mutex);

					cout << "evaluate node at depth " << currentNode->depth << endl;
					printTime();

					// wait for current node to evaluate
					bool evaluated = false;
					while (!evaluated)
					{
						// check if window is still open
						bool windowClosed = false;
						string gameResult = "";
						WaitForSingleObject(p->mutex, INFINITE);
						windowClosed = p->windowClosed;
						gameResult = p->gameResult;
						ReleaseMutex(p->mutex);
						if (windowClosed || gameResult.length() > 0) { break; }

						WaitForSingleObject(p->mutex, INFINITE);
						evaluated = (p->evaluated != nullptr);
						ReleaseMutex(p->mutex);
					}

					printTime();

					float value = 0;
					WaitForSingleObject(p->mutex, INFINITE);
					value = ((PositionNode*)p->evaluated)->value;
					ReleaseMutex(p->mutex);

					currentNode->value = value;
					currentNode->evaluated = true;

					if (dpr)
					{
						//cout << "current node evaluated to " << value << endl;
					}

					break;
				}
			}

			float currentValue = currentNode->value;
			float parentValue = currentNode->parent->value;
			currentNode->parent->value = (currentNode->parent->min ? min(currentValue, parentValue) : max(currentValue, parentValue));

			if (dpr)
			{
				if (parentValue != currentNode->parent->value)
				{
					string s = (currentNode->parent->min ? "min" : "max");
					cout << s << " parent at depth=" << currentNode->parent->depth << " value updated from " << parentValue << " to " << currentNode->parent->value << endl;
				}
			}

			currentNode = currentNode->parent;
			currentNode->evaluated = true;

		}

		// delete the position node tree
		deletePositionTree(root);

		WaitForSingleObject(p->mutex, INFINITE);
		cout << "sending best position:" << endl;
		for (int a = 0; a < 8; a++)
		{
			for (int b = 0; b < 8; b++)
			{
				cout << bestPosition.board[a][b] << " ";
			}
			cout << endl;
		}
		ReleaseMutex(p->mutex);
		
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