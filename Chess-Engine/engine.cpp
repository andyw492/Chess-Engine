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
			// check if window closed or game ended
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
				if (!currentNode->children[i]->evaluated) { allEvaluated = false; break; }
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
				cout << "expanding tree" << endl;
			}

			while (currentNode->depth < maxDepth - 1 && !gameEnd)
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
						if (dpr)
						{
							cout << "\ncurrent depth " << currentNode->depth << ", creating children" << endl;
						}
						
						printTime();
					}

					Position position = currentNode->position;
					map<string, vector<string>> legalMoves = helper::getLegalMoves(position, !currentNode->min);

					// create children
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

					// if no legal moves in the position
					// then set current node's value to 0 if stalemate, 10000 if white victory, -10000 if black victory
					if (legalMoves.size() == 0)
					{
						gameEnd = true;

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

						currentNode->evaluated = true;
					}
					else
					{
						currentNode = currentNode->children[0];
					}
				}

				else if (!allEvaluated)
				{
					// current node = first unevaluated child
					for (int i = 0; i < currentNode->children.size(); i++)
					{
						if (!currentNode->children[i]->evaluated)
						{
							currentNode = currentNode->children[i];
							if (dpr)
							{
								cout << "\nenter depth " << currentNode->depth << endl;
							}
							
							break;
						}
					}
				}

				else
				{
					break;
				}
			}

			if (currentNode->depth == maxDepth - 1 && !gameEnd)
			{
				currentNode = currentNode->parent;

				if (dpr)
				{
					cout << "\nevaluate node at depth " << currentNode->depth << ": " << endl;
				}

				WaitForSingleObject(p->mutex, INFINITE);
				for (int i = 0; i < currentNode->children.size(); i++)
				{
					p->toEvaluate.push((char*)currentNode->children[i]);
				}
				ReleaseMutex(p->mutex);

				// wait for current node to evaluate
				while (true)
				{
					bool windowClosed = false;
					string gameResult = "";
					int valuesDone = 0;
					bool evaluatorError = false;

					WaitForSingleObject(p->mutex, INFINITE);
					windowClosed = p->windowClosed;
					gameResult = p->gameResult;
					valuesDone = p->values.size();
					evaluatorError = p->evaluatorError;
					ReleaseMutex(p->mutex);

					if (windowClosed || gameResult.length() > 0)
					{
						break;
					}

					if (valuesDone == currentNode->children.size())
					{
						break;
					}

					if (evaluatorError)
					{
						cout << "\n\n\nevaluator error\n\n\n" << endl;
						break;
					}
				}

				vector<float> values;
				WaitForSingleObject(p->mutex, INFINITE);
				values = p->values;
				p->values.clear();
				ReleaseMutex(p->mutex);
					
				float minMax = (currentNode->min ? FLT_MAX : -FLT_MAX);
				for (float value : values)
				{
					minMax = (currentNode->min ? min(minMax, value) : max(minMax, value));
				}

				currentNode->value = minMax;
				currentNode->evaluated = true;

				printTime();
			}

			float currentValue = currentNode->value;
			float parentValue = currentNode->parent->value;
			currentNode->parent->value = (currentNode->parent->min ? min(currentValue, parentValue) : max(currentValue, parentValue));
			currentNode->parent->evaluated = true;

			if (dpr)
			{
				if (parentValue != currentNode->parent->value)
				{
					string s = (currentNode->parent->min ? "min" : "max");
					cout << s << " parent at depth=" << currentNode->parent->depth << " value updated from " << parentValue << " to " << currentNode->parent->value << endl;
				}
			}

			currentNode = currentNode->parent;
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