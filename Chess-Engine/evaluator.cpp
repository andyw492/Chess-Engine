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
		if (!p->toEvaluate.empty())
		{
			toEvaluate = p->toEvaluate.top();
			p->toEvaluate.pop();
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
		map<string, vector<string>> legalMoves = helper::getLegalMoves(position, whiteToMove);

		if (dpr)
		{
			cout << "\nprev move " << node->prevMove << endl;
			for (auto move : legalMoves)
			{
				cout << "\t" << move.first << ":";
				for (string s : move.second)
				{
					cout << " " << s;
				}
				cout << endl;
			}
		}

		float minMax = (node->min ? FLT_MAX : -FLT_MAX);
		for (auto move : legalMoves)
		{
			for (int i = 0; i < move.second.size(); i++)
			{
				Position newPosition = helper::getNewPosition(position, move.first, move.second[i]);

				// evaluation: calculate value as (white total piece value - black total piece value)
				float totalValue = 0;
				for (int y = 0; y < 8; y++)
				{
					for (int x = 0; x < 8; x++)
					{
						char piece = newPosition.board[y][x];
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

						totalValue += pieceValue;
					}
				}

				minMax = (node->min ? min(minMax, totalValue) : max(minMax, totalValue));
				values.push_back(totalValue);
			}
		}

		node->value = minMax;

		// if no legal moves in the position, then set value to:
		// 0 if stalemate
		// 10000 if white victory
		// -10000 if black victory
		if (legalMoves.size() == 0)
		{
			if (helper::inCheck(position.board, !node->min))
			{
				node->value = -10000;
			}
			else if (helper::inCheck(position.board, node->min))
			{
				node->value = 10000;
			}
			else
			{
				node->value = 0;
			}

			values.push_back(node->value);
		}

		if (dpr)
		{
			string minMaxString = (node->min ? "min" : "max");
			WaitForSingleObject(p->mutex, INFINITE);
			cout << "depth " << node->depth << " " << minMaxString << ": [";
			for (float value : values)
			{
				cout << value << " ";
			}
			cout << "] = " << node->value << endl;
			ReleaseMutex(p->mutex);
		}

		WaitForSingleObject(p->mutex, INFINITE);
		p->values.push_back(node->value);
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