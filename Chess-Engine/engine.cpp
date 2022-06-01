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

		string from = "";
		string to = "";

		bool castling[4];
		WaitForSingleObject(p->mutex, INFINITE);
		memcpy(castling, p->castling, 4 * sizeof(bool));
		ReleaseMutex(p->mutex);

		clock_t legalMovesStart = clock();
		map<string, vector<string>> legalMoves = helper::getLegalMoves(board, false, castling, false);
		clock_t legalMovesEnd = clock();

		if (dpr)
		{
			cout << "----------------- ENGINE -----------------\n" << endl;

			for (int y = 0; y < 8; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					cout << board[y][x] << " ";
				}
				cout << endl;
			}
			cout << endl;

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

			cout << "legal moves found in " << 1000.0 * (legalMovesEnd - legalMovesStart) / CLOCKS_PER_SEC << " ms\n" << endl;
		}

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
			//cout << "engine from " << from << " to " << to << endl;
		}

		// check for special moves (castling, promotion, en passant)
		// 0 == normal move
		// 1 == white castles kingside
		// 2 == white castles queenside
		// 3 == black castles kingside
		// 4 == black castles queenside
		// 5 == promotion (destination square in legal moves formatted like "e8=Q")
		// 6 == en passant

		int moveType = 0;
		if (from == "74" && to == "76") { moveType = 1; }
		if (from == "74" && to == "72") { moveType = 2; }
		if (from == "04" && to == "06") { moveType = 3; }
		if (from == "04" && to == "02") { moveType = 4; }

		// modify the board and send to parameters
		switch (moveType)
		{
		case 0:
		{
			char movedPiece = board[from[0] - 48][from[1] - 48];
			board[from[0] - 48][from[1] - 48] = ' ';
			board[to[0] - 48][to[1] - 48] = movedPiece;
			break;
		}
		case 1:
		{
			board[7][4] = ' ';
			board[7][5] = 'R';
			board[7][6] = 'K';
			board[7][7] = ' ';
			break;
		}
		case 2:
		{
			board[7][0] = ' ';
			board[7][2] = 'K';
			board[7][3] = 'R';
			board[7][4] = ' ';
			break;
		}
		case 3:
		{
			board[0][4] = ' ';
			board[0][5] = 'r';
			board[0][6] = 'k';
			board[0][7] = ' ';
			break;
		}
		case 4:
		{
			board[0][0] = ' ';
			board[0][2] = 'k';
			board[0][3] = 'r';
			board[0][4] = ' ';
			break;
		}
		}

		WaitForSingleObject(p->mutex, INFINITE);
		memcpy(p->board, board, 64 * sizeof(char));
		ReleaseMutex(p->mutex);

		// modify castling permissions and send to parameters
		if (from == "74" || from == "77") { castling[0] = false; }
		if (from == "74" || from == "70") { castling[1] = false; }
		if (from == "04" || from == "07") { castling[2] = false; }
		if (from == "04" || from == "00") { castling[3] = false; }

		WaitForSingleObject(p->mutex, INFINITE);
		memcpy(p->castling, castling, 4 * sizeof(bool));
		ReleaseMutex(p->mutex);

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
			cout << endl;
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