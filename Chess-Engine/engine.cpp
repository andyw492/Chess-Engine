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

		if (windowClosed)
		{
			break;
		}

		// check if engine's turn
		bool engineToMove = false;
		WaitForSingleObject(p->mutex, INFINITE);
		engineToMove = !p->playerToMove;
		ReleaseMutex(p->mutex);

		if (!engineToMove)
		{
			continue;
		}
		
		// get the current board from parameters
		char board[8][8];
		WaitForSingleObject(p->mutex, INFINITE);
		memcpy(board, p->board, 64 * sizeof(char));
		ReleaseMutex(p->mutex);

		if (dpr)
		{
			Sleep(500);
			cout << "\nengine's turn:" << endl;
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					cout << board[i][j] << " ";
				}
				cout << endl;
			}
			cout << "--------" << endl;
		}

		map<string, vector<string>> legalMoves = helper::getLegalMoves(board, false);

		if (dpr)
		{
			for (auto i : legalMoves)
			{
				cout << i.first << ": ";
				for (int j = 0; j < i.second.size(); j++)
				{
					cout << i.second[j] << " ";
				}
				cout << endl;
			}
		}

		string from = "";
		string to = "";

		// if an enemy piece is capturable, then capture the most valuable
		// (king > queen > rook > bishop > knight > pawn)
		float maxValue = 0;
		for (auto i : legalMoves)
		{
			for (int j = 0; j < i.second.size(); j++)
			{
				int y = i.second[j][0] - 48;
				int x = i.second[j][1] - 48;

				// can't capture own piece
				if (board[y][x] >= 97 || board[y][x] == ' ')
				{
					continue;
				}

				int pieceValue = helper::getPieceValue(board[y][x]);

				if (dpr)
				{
					cout << "y " << y << " x " << x << " piece value " << pieceValue << endl;
				}

				if (pieceValue > maxValue)
				{
					maxValue = pieceValue;
					from = i.first;
					to = i.second[j];
				}
			}
		}

		// if no piece is capturable, choose a random move
		if (from == "")
		{
			map<string, vector<string>>::iterator it = legalMoves.begin();
			std::advance(it, rand() % legalMoves.size());
			from = it->first;
			to = legalMoves[from][rand() % legalMoves[from].size()];
		}

		if (dpr)
		{
			cout << "engine from " << from << " to " << to << endl;
		}

		// "thinking"
		Sleep(200);

		// modify board
		char movedPiece = board[from[0] - 48][from[1] - 48];
		board[from[0] - 48][from[1] - 48] = ' ';
		board[to[0] - 48][to[1] - 48] = movedPiece;

		if (dpr)
		{
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					cout << board[i][j] << " ";
				}
				cout << endl;
			}
			cout << "--------" << endl;
		}

		// send board to parameters
		WaitForSingleObject(p->mutex, INFINITE);
		memcpy(p->board, board, 64 * sizeof(char));
		ReleaseMutex(p->mutex);

		// skip engine's move
		WaitForSingleObject(p->mutex, INFINITE);
		p->playerToMove = true;
		ReleaseMutex(p->mutex);

	}

	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	// print we're about to exit
	WaitForSingleObject(p->mutex, INFINITE);
	printf("engineThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;
}