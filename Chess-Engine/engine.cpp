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

		vector<U64> board = helper::positionToU64(currentPosition);
		vector<vector<U64>> nextPositions = helper::getNextPositions(board, false);

		vector<U64> bestPosition = nextPositions[0];

		Position bestPositionWindow = helper::U64ToPosition(bestPosition);

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