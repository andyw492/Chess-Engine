#include "engine.h"

Engine::Engine(bool enginePrint)
{
	dpr = enginePrint;
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
		WaitForSingleObject(p->mutex, INFINITE);
		windowClosed = p->windowClosed;
		ReleaseMutex(p->mutex);
		if (windowClosed) { break; }

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
		if (dpr)
		{
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

		// find the resulting position from the best move
		// pseudocode: https://pastebin.com/MNrCY7eu
		while (true)
		{
			// check if window is still open
			bool windowClosed = false;
			WaitForSingleObject(p->mutex, INFINITE);
			windowClosed = p->windowClosed;
			ReleaseMutex(p->mutex);
			if (windowClosed) { break; }

			bool allEvaluated = true;

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

			while (currentNode->children.size() == 0 || !allEvaluated)
			{
				if (currentNode->children.size() == 0)
				{
					if (dpr)
					{
						cout << "\ndepth " << currentNode->depth << " board, creating children" << endl;
						for (int i = 0; i < 8; i++)
						{
							for (int j = 0; j < 8; j++)
							{
								cout << currentNode->position.board[i][j] << " ";
							}
							cout << endl;
						}
					}

					Position position = currentNode->position;
					map<string, vector<string>> legalMoves = helper::getLegalMoves(position, !currentNode->min);
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
					if (!currentNode->children[i]->evaluated) { currentNode = currentNode->children[i]; break; }
				}

				if (currentNode->depth == maxDepth - 1)
				{
					WaitForSingleObject(p->mutex, INFINITE);
					p->toEvaluate = (char*)currentNode;
					p->evaluated = nullptr;
					ReleaseMutex(p->mutex);

					// wait for current node to evaluate
					bool evaluated = false;
					while (!evaluated)
					{
						// check if window is still open
						bool windowClosed = false;
						WaitForSingleObject(p->mutex, INFINITE);
						windowClosed = p->windowClosed;
						ReleaseMutex(p->mutex);
						if (windowClosed) { break; }

						WaitForSingleObject(p->mutex, INFINITE);
						evaluated = (p->evaluated != nullptr);
						ReleaseMutex(p->mutex);
					}

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
		
		// send the resulting position to parameters
		WaitForSingleObject(p->mutex, INFINITE);
		p->currentPosition = bestPosition;
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