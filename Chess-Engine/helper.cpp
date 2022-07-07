#include "helper.h"


// source: https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
vector<string> helper::splitToVector(string str, char del)
{
	vector<string> splitResult;

	// declaring temp string to store the curr "word" upto del
	string temp = "";

	for (int i = 0; i < (int)str.size(); i++) {
		// If cur char is not del, then append it to the cur "word", otherwise
		  // you have completed the word, print it, and start a new word.
		if (str[i] != del) {
			temp += str[i];
		}
		else {
			splitResult.push_back(temp);
			temp = "";
		}
	}

	splitResult.push_back(temp);

	return splitResult;
}

void helper::fenToMatrix(string fen, char matrix[8][8])
{
	// replace numbers with spaces, e.g. "4" -> "    " (" " * 4)
	vector<string> parts = helper::splitToVector(fen, '/');
	vector<string> processedParts;
	for (int i = 0; i < parts.size(); i++)
	{
		string partsString = parts[i];
		string processedPartsString = "";
		for (int j = 0; j < partsString.length(); j++)
		{
			if (isdigit(partsString[j]))
			{
				int spaceCount = partsString[j] - '0';
				string spaceString = "        ";
				processedPartsString += spaceString.substr(0, spaceCount);
			}
			else
			{
				processedPartsString += partsString[j];
			}
		}

		processedParts.push_back(processedPartsString);
	}

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			matrix[i][j] = processedParts[i][j];
		}
	}
}

Position helper::getNewPosition(Position before, string from, string to)
{
	Position newPosition;

	char board[8][8];
	memcpy(board, before.board, 64 * sizeof(char));

	// check for special moves (castling, promotion, en passant)
	// 0 == normal move
	// 1 == white castles kingside
	// 2 == white castles queenside
	// 3 == black castles kingside
	// 4 == black castles queenside
	// 5 == promotion (destination square in legal moves formatted like "e8=Q")
	// 6 == en passant

	int moveType = 0;
	if (board[7][4] == 'K' && from == "74" && to == "76") { moveType = 1; }
	if (board[7][4] == 'K' && from == "74" && to == "72") { moveType = 2; }
	if (board[0][4] == 'k' && from == "04" && to == "06") { moveType = 3; }
	if (board[0][4] == 'k' && from == "04" && to == "02") { moveType = 4; }

	// modify the board
	switch (moveType)
	{
	case 0:
	{
		char toPrev = board[to[0] - 48][to[1] - 48];
		char movedPiece = board[from[0] - 48][from[1] - 48];
		board[from[0] - 48][from[1] - 48] = ' ';
		board[to[0] - 48][to[1] - 48] = movedPiece;

		// if a pawn is on a promoting square, then replace it with a queen
		for (int x = 0; x < 8; x++)
		{
			if (board[0][x] == 'P') { board[0][x] = 'Q'; }
			if (board[7][x] == 'p') { board[7][x] = 'q'; }
		}

		// if a pawn moved forward two squares, then set an en passant square
		// otherwise, clear the en passant square
		if (board[to[0] - 48][to[1] - 48] == 'P' && (from[0] - 48) - (to[0] - 48) == 2)
		{
			newPosition.enpassant = to_string(to[0] - 48 + 1) + to_string(to[1] - 48);
		}
		else if (board[to[0] - 48][to[1] - 48] == 'p' && (from[0] - 48) - (to[0] - 48) == -2)
		{
			newPosition.enpassant = to_string(to[0] - 48 - 1) + to_string(to[1] - 48);
		}
		else
		{
			newPosition.enpassant = "";
		}

		// if a pawn moved to an empty square, then clear the square behind it (to handle en passant captures)
		if (board[to[0] - 48][to[1] - 48] == 'P' && toPrev == ' ')
		{
			board[to[0] - 48 + 1][to[1] - 48] = ' ';
		}
		if (board[to[0] - 48][to[1] - 48] == 'p' && toPrev == ' ')
		{
			board[to[0] - 48 - 1][to[1] - 48] = ' ';
		}

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

	// modify castling permissions and send to parameters
	if (from == "74" || from == "77") { newPosition.castling[0] = false; }
	if (from == "74" || from == "70") { newPosition.castling[1] = false; }
	if (from == "04" || from == "07") { newPosition.castling[2] = false; }
	if (from == "04" || from == "00") { newPosition.castling[3] = false; }

	memcpy(newPosition.board, board, 64 * sizeof(char));

	return newPosition;
}

map<string, vector<string>> helper::getLegalMoves(Position position, bool whiteToMove)
{
	char board[8][8];
	bool castling[4];
	string enpassant;
	memcpy(board, position.board, 64 * sizeof(char));
	memcpy(castling, position.castling, 4 * sizeof(bool));
	
	// always look for check
	bool playerCheckDetection = true;
	//bool playerCheckDetection = whiteToMove;

	// load the legal moves; white to move
	map<string, vector<string>> legalMoves;
	if (whiteToMove)
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (board[i][j] < 65 || board[i][j] > 90) { continue; }

				char currentPiece = board[i][j];
				vector<string> pieceMoves;

				switch (currentPiece)
				{
				case 'P':
				{
					// promotion, handled by window/engine
					if (i == 0) { break; }

					int enpassantY = -1;
					int enpassantX = -1;
					if (enpassant.length() > 0)
					{
						enpassantY = enpassant[0] - 48;
						enpassantX = enpassant[1] - 48;
					}

					// capture left
					if (j > 0 && (board[i - 1][j - 1] >= 97 || (i - 1 == enpassantY && j - 1 == enpassantX))) { pieceMoves.push_back(to_string(i - 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && (board[i - 1][j + 1] >= 97 || (i - 1 == enpassantY && j + 1 == enpassantX))) { pieceMoves.push_back(to_string(i - 1) + to_string(j + 1)); }

					// move two squares
					if (i == 6 && board[i - 1][j] == ' ' && board[i - 2][j] == ' ') { pieceMoves.push_back(to_string(i - 2) + to_string(j)); }

					// move one square
					if (board[i - 1][j] == ' ') { pieceMoves.push_back(to_string(i - 1) + to_string(j)); }

					break;
				}

				case 'N':
				{
					// add 8 candidate squares
					vector<vector<int>> to;
					to.push_back({ i - 2, j - 1 }); to.push_back({ i - 2, j + 1 });
					to.push_back({ i - 1, j - 2 }); to.push_back({ i - 1, j + 2 });
					to.push_back({ i + 1, j - 2 }); to.push_back({ i + 1, j + 2 });
					to.push_back({ i + 2, j - 1 }); to.push_back({ i + 2, j + 1 });

					// filter candidate squares
					for (int k = 0; k < to.size(); k++)
					{
						int y = to[k][0];
						int x = to[k][1];
						// board[y][x] < 65 || board[y][x] > 90 means not allied piece
						if (y >= 0 && y <= 7 && x >= 0 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
						{
							pieceMoves.push_back(to_string(y) + to_string(x));
						}
					}

					break;
				}

				case 'B':
				{
					// top left
					int y = i - 1;
					int x = j - 1;
					while (y >= 0 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 97) { break; }
						y--;
						x--;
					}

					// top right
					y = i - 1;
					x = j + 1;
					while (y >= 0 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y--;
						x++;
					}

					// bottom left
					y = i + 1;
					x = j - 1;
					while (y <= 7 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y++;
						x--;
					}

					// bottom right
					y = i + 1;
					x = j + 1;
					while (y <= 7 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y++;
						x++;
					}

					break;
				}

				case 'R':
				{
					// top
					int y = i - 1;
					int x = j;
					while (y >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 97) { break; }
						y--;
					}

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						x--;
					}

					// right
					y = i;
					x = j + 1;
					while (x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						x++;
					}

					// bottom
					y = i + 1;
					x = j;
					while (y <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y++;
					}

					break;
				}

				case 'Q':
				{
					// top left
					int y = i - 1;
					int x = j - 1;
					while (y >= 0 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 97) { break; }
						y--;
						x--;
					}

					// top
					y = i - 1;
					x = j;
					while (y >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 97) { break; }
						y--;
					}

					// top right
					y = i - 1;
					x = j + 1;
					while (y >= 0 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y--;
						x++;
					}

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						x--;
					}

					// right
					y = i;
					x = j + 1;
					while (x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						x++;
					}

					// bottom left
					y = i + 1;
					x = j - 1;
					while (y <= 7 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y++;
						x--;
					}

					// bottom
					y = i + 1;
					x = j;
					while (y <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y++;
					}

					// bottom right
					y = i + 1;
					x = j + 1;
					while (y <= 7 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						y++;
						x++;
					}

					break;
				}

				case 'K':
				{
					// castling
					if (castling[0] && board[7][5] == ' ' && board[7][6] == ' ')
					{
						pieceMoves.push_back("76");
					}
					if (castling[1] && board[7][1] == ' ' && board[7][2] == ' ' && board[7][3] == ' ')
					{
						pieceMoves.push_back("72");
					}

					// top left
					int y = i - 1;
					int x = j - 1;
					if (y >= 0 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// top
					y = i - 1;
					x = j;
					if (y >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// top right
					y = i - 1;
					x = j + 1;
					if (y >= 0 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// left
					y = i;
					x = j - 1;
					if (x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// right
					y = i;
					x = j + 1;
					if (x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// bottom left
					y = i + 1;
					x = j - 1;
					if (y <= 7 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// bottom
					y = i + 1;
					x = j;
					if (y <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// bottom right
					y = i + 1;
					x = j + 1;
					if (y <= 7 && x <= 7 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					break;
				}

				default:
					break;
				}

				if (pieceMoves.size() > 0)
				{
					legalMoves[to_string(i) + to_string(j)] = pieceMoves;
				}
			}
		}
	}

	// load the legal moves; black to move
	else
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (board[i][j] < 97 || board[i][j] > 122) { continue; }

				char currentPiece = board[i][j];
				vector<string> pieceMoves;

				switch (currentPiece)
				{
				case 'p':
				{
					// promotion, handled by window/engine
					if (i == 7) { break; }

					int enpassantY = -1;
					int enpassantX = -1;
					if (enpassant.length() > 0)
					{
						enpassantY = enpassant[0] - 48;
						enpassantX = enpassant[1] - 48;
					}

					// capture left
					if (j > 0 && ((board[i + 1][j - 1] >= 65 && board[i + 1][j - 1] <= 90) || (i + 1 == enpassantY && j - 1 == enpassantX))) { pieceMoves.push_back(to_string(i + 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && ((board[i + 1][j + 1] >= 65 && board[i + 1][j + 1] <= 90) || (i + 1 == enpassantY && j + 1 == enpassantX))) { pieceMoves.push_back(to_string(i + 1) + to_string(j + 1)); }

					// move two squares
					if (i == 1 && board[i + 1][j] == ' ' && board[i + 2][j] == ' ') { pieceMoves.push_back(to_string(i + 2) + to_string(j)); }

					// move one square
					if (board[i + 1][j] == ' ') { pieceMoves.push_back(to_string(i + 1) + to_string(j)); }

					break;
				}

				case 'n':
				{
					// add 8 candidate squares
					vector<vector<int>> to;
					to.push_back({ i + 2, j - 1 }); to.push_back({ i + 2, j + 1 });
					to.push_back({ i + 1, j - 2 }); to.push_back({ i + 1, j + 2 });
					to.push_back({ i - 1, j - 2 }); to.push_back({ i - 1, j + 2 });
					to.push_back({ i - 2, j - 1 }); to.push_back({ i - 2, j + 1 });

					// filter candidate squares
					for (int k = 0; k < to.size(); k++)
					{
						int y = to[k][0];
						int x = to[k][1];
						// board[y][x] < 97 means not allied piece
						if (y >= 0 && y <= 7 && x >= 0 && x <= 7 && board[y][x] < 97)
						{
							pieceMoves.push_back(to_string(y) + to_string(x));
						}
					}

					break;
				}

				case 'b':
				{
					// bottom left
					int y = i + 1;
					int x = j - 1;
					while (y <= 7 && x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y++;
						x--;
					}

					// bottom right
					y = i + 1;
					x = j + 1;
					while (y <= 7 && x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y++;
						x++;
					}

					// top left
					y = i - 1;
					x = j - 1;
					while (y >= 0 && x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
						x--;
					}

					// top right
					y = i - 1;
					x = j + 1;
					while (y >= 0 && x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
						x++;
					}

					break;
				}

				case 'r':
				{
					// bottom
					int y = i + 1;
					int x = j;
					while (y <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y++;
					}

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						x--;
					}

					// right
					y = i;
					x = j + 1;
					while (x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						x++;
					}

					// top
					y = i - 1;
					x = j;
					while (y >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
					}

					break;
				}

				case 'q':
				{
					// bottom left
					int y = i + 1;
					int x = j - 1;
					while (y <= 7 && x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y++;
						x--;
					}

					// bottom
					y = i + 1;
					x = j;
					while (y <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y++;
					}

					// bottom right
					y = i + 1;
					x = j + 1;
					while (y <= 7 && x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y++;
						x++;
					}

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						x--;
					}

					// right
					y = i;
					x = j + 1;
					while (x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						x++;
					}

					// top left
					y = i - 1;
					x = j - 1;
					while (y >= 0 && x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
						x--;
					}

					// top
					y = i - 1;
					x = j;
					while (y >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
					}

					// top right
					y = i - 1;
					x = j + 1;
					while (y >= 0 && x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
						x++;
					}

					break;
				}

				case 'k':
				{
					// castling
					if (castling[2] && board[0][5] == ' ' && board[0][6] == ' ')
					{
						pieceMoves.push_back("06");
					}
					if (castling[3] && board[0][1] == ' ' && board[0][2] == ' ' && board[0][3] == ' ')
					{
						pieceMoves.push_back("02");
					}

					// bottom left
					int y = i + 1;
					int x = j - 1;
					if (y <= 7 && x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// bottom
					y = i + 1;
					x = j;
					if (y <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// bottom right
					y = i + 1;
					x = j + 1;
					if (y <= 7 && x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// left
					y = i;
					x = j - 1;
					if (x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// right
					y = i;
					x = j + 1;
					if (x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// top left
					y = i - 1;
					x = j - 1;
					if (y >= 0 && x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// top
					y = i - 1;
					x = j;
					if (y >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					// top right
					y = i - 1;
					x = j + 1;
					if (y >= 0 && x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
					}

					break;
				}

				default:
					break;
				}

				if (pieceMoves.size() > 0)
				{
					legalMoves[to_string(i) + to_string(j)] = pieceMoves;
				}
			}
		}
	}

	// remove legal moves that would put the king into check
#pragma region
	
	int kingY = -1;
	int kingX = -1;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if ((whiteToMove && board[y][x] == 'K') || (!whiteToMove && board[y][x] == 'k'))
			{
				kingY = y;
				kingX = x;
				y = 8; x = 8; // break from loops
			}
		}
	}

	// if castling is legal, then add additional squares for putting king in check
	map<string, vector<string>> castlingExtra;
	for (auto move : legalMoves)
	{
		if (move.first != "74" && move.first != "04") { continue; }

		vector<string> pieceExtra;
		for (int iter = 0; iter < move.second.size(); iter++)
		{
			string from = move.first;
			string to = move.second[iter];

			if (from == "74" && to == "76" || from == "74" && to == "72")
			{
				vector<string> pieceMoves = move.second;
				pieceMoves.push_back("74");
				legalMoves[move.first] = pieceMoves;
				break;
			}

			if (from == "04" && to == "06" || from == "04" && to == "02")
			{
				vector<string> pieceMoves = move.second;
				pieceMoves.push_back("04");
				legalMoves[move.first] = pieceMoves;
				break;
			}
		}
	}

	// load illegal moves from legal moves
	map<string, vector<string>> illegalMoves;
	for (auto move : legalMoves)
	{
		// for the engine, skip illegal move detection unless it wants to castle
		bool skip = false;
		if (!playerCheckDetection)
		{
			skip = true;
			if (move.first == "04" && board[0][4] == 'k') { skip = false; }
		}

		if (skip) { continue; }

		vector<string> pieceIllegal;
		for (int iter = 0; iter < move.second.size(); iter++)
		{
			char tempBoard[8][8];
			memcpy(tempBoard, board, 64 * sizeof(char));

			string from = move.first;
			string to = move.second[iter];

			char movedPiece = tempBoard[from[0] - 48][from[1] - 48];
			tempBoard[from[0] - 48][from[1] - 48] = ' ';
			tempBoard[to[0] - 48][to[1] - 48] = movedPiece;

			int i = kingY;
			int j = kingX;

			if (movedPiece == 'K' || movedPiece == 'k')
			{
				i = to[0] - 48;
				j = to[1] - 48;
			}

			//for (int y = 0; y < 8; y++)
			//{
			//	for (int x = 0; x < 8; x++)
			//	{
			//		cout << tempBoard[y][x] << " ";
			//	}
			//	cout << endl;
			//}
			//cout << endl;

			// check if the king can be captured by each type of piece
			if (whiteToMove)
			{
				// pawns
				if (j > 0 && tempBoard[i - 1][j - 1] == 'p') { pieceIllegal.push_back(to); }
				if (j < 7 && tempBoard[i - 1][j + 1] == 'p') { pieceIllegal.push_back(to); }

				// knights
				vector<vector<int>> knightPos;
				knightPos.push_back({ i - 2, j - 1 }); knightPos.push_back({ i - 2, j + 1 });
				knightPos.push_back({ i - 1, j - 2 }); knightPos.push_back({ i - 1, j + 2 });
				knightPos.push_back({ i + 1, j - 2 }); knightPos.push_back({ i + 1, j + 2 });
				knightPos.push_back({ i + 2, j - 1 }); knightPos.push_back({ i + 2, j + 1 });
				for (int k = 0; k < knightPos.size(); k++)
				{
					int y = knightPos[k][0];
					int x = knightPos[k][1];
					if (y >= 0 && y <= 7 && x >= 0 && x <= 7 && tempBoard[y][x] == 'n') { pieceIllegal.push_back(to); }
				}

				// bishops, rooks, queen
#pragma region
				// top left
				int y = i - 1;
				int x = j - 1;
				while (y >= 0 && x >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					y--;
					x--;
				}

				// top right
				y = i - 1;
				x = j + 1;
				while (y >= 0 && x <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					y--;
					x++;
				}

				// bottom left
				y = i + 1;
				x = j - 1;
				while (y <= 7 && x >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					y++;
					x--;
				}

				// bottom right
				y = i + 1;
				x = j + 1;
				while (y <= 7 && x <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'b' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					y++;
					x++;
				}

				// top
				y = i - 1;
				x = j;
				while (y >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					y--;
				}

				// left
				y = i;
				x = j - 1;
				while (x >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					x--;
				}

				// right
				y = i;
				x = j + 1;
				while (x <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					x++;
				}

				// bottom
				y = i + 1;
				x = j;
				while (y <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q'))
				{
					if (tempBoard[y][x] == 'r' || tempBoard[y][x] == 'q') { pieceIllegal.push_back(to); break; }
					y++;
				}
#pragma endregion

				// king
#pragma region
				// top left
				y = i - 1;
				x = j - 1;
				if (y >= 0 && x >= 0 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// top
				y = i - 1;
				x = j;
				if (y >= 0 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// top right
				y = i - 1;
				x = j + 1;
				if (y >= 0 && x <= 7 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// left
				y = i;
				x = j - 1;
				if (x >= 0 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// right
				y = i;
				x = j + 1;
				if (x <= 7 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// bottom left
				y = i + 1;
				x = j - 1;
				if (y <= 7 && x >= 0 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// bottom
				y = i + 1;
				x = j;
				if (y <= 7 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }

				// bottom right
				y = i + 1;
				x = j + 1;
				if (y <= 7 && x <= 7 && (tempBoard[y][x] == 'k')) { pieceIllegal.push_back(to); }
#pragma endregion

			}

			else // black to move
			{
				// pawns
				if (j > 0 && tempBoard[i + 1][j - 1] == 'P') { pieceIllegal.push_back(to); }
				if (j < 7 && tempBoard[i + 1][j + 1] == 'P') { pieceIllegal.push_back(to); }

				// knights
				vector<vector<int>> knightPos;
				knightPos.push_back({ i - 2, j - 1 }); knightPos.push_back({ i - 2, j + 1 });
				knightPos.push_back({ i - 1, j - 2 }); knightPos.push_back({ i - 1, j + 2 });
				knightPos.push_back({ i + 1, j - 2 }); knightPos.push_back({ i + 1, j + 2 });
				knightPos.push_back({ i + 2, j - 1 }); knightPos.push_back({ i + 2, j + 1 });
				for (int k = 0; k < knightPos.size(); k++)
				{
					int y = knightPos[k][0];
					int x = knightPos[k][1];
					if (y >= 0 && y <= 7 && x >= 0 && x <= 7 && tempBoard[y][x] == 'N') { pieceIllegal.push_back(to); }
				}

				// bishops, rooks, queen
#pragma region
				// top left
				int y = i - 1;
				int x = j - 1;
				while (y >= 0 && x >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					y--;
					x--;
				}

				// top right
				y = i - 1;
				x = j + 1;
				while (y >= 0 && x <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					y--;
					x++;
				}

				// bottom left
				y = i + 1;
				x = j - 1;
				while (y <= 7 && x >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					y++;
					x--;
				}

				// bottom right
				y = i + 1;
				x = j + 1;
				while (y <= 7 && x <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'B' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					y++;
					x++;
				}

				// top
				y = i - 1;
				x = j;
				while (y >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					y--;
				}

				// left
				y = i;
				x = j - 1;
				while (x >= 0 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					x--;
				}

				// right
				y = i;
				x = j + 1;
				while (x <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					x++;
				}

				// bottom
				y = i + 1;
				x = j;
				while (y <= 7 && (tempBoard[y][x] == ' ' || tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q'))
				{
					if (tempBoard[y][x] == 'R' || tempBoard[y][x] == 'Q') { pieceIllegal.push_back(to); break; }
					y++;
				}
#pragma endregion

				// king
#pragma region
				// top left
				y = i - 1;
				x = j - 1;
				if (y >= 0 && x >= 0 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// top
				y = i - 1;
				x = j;
				if (y >= 0 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// top right
				y = i - 1;
				x = j + 1;
				if (y >= 0 && x <= 7 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// left
				y = i;
				x = j - 1;
				if (x >= 0 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// right
				y = i;
				x = j + 1;
				if (x <= 7 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// bottom left
				y = i + 1;
				x = j - 1;
				if (y <= 7 && x >= 0 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// bottom
				y = i + 1;
				x = j;
				if (y <= 7 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }

				// bottom right
				y = i + 1;
				x = j + 1;
				if (y <= 7 && x <= 7 && (tempBoard[y][x] == 'K')) { pieceIllegal.push_back(to); }
#pragma endregion

			}
		}

		if (pieceIllegal.size() > 0)
		{
			illegalMoves[move.first] = pieceIllegal;
		}
	}

	// find instances of illegal castling (e.g. if castles from 74 to 76, and 74/75 is under attack)
	// (load the illegalCastling vector)
	vector<string> illegalCastling;
	vector<string> illegalToDelete;
	for (auto move : illegalMoves)
	{
		if (move.first != "74" && move.first != "04") { continue; }

		vector<string> pieceExtra;
		for (int iter = 0; iter < move.second.size(); iter++)
		{
			string from = move.first;
			string to = move.second[iter];

			if (from == "74" && to == "74") { illegalCastling.push_back("76"); illegalCastling.push_back("72"); }
			if (from == "74" && to == "75") { illegalCastling.push_back("76"); }
			if (from == "74" && to == "73") { illegalCastling.push_back("72"); }

			if (from == "04" && to == "04") { illegalCastling.push_back("06"); illegalCastling.push_back("02"); }
			if (from == "04" && to == "05") { illegalCastling.push_back("06"); }
			if (from == "04" && to == "03") { illegalCastling.push_back("02"); }

		}
	}

	// remove additional squares from castling in legalMoves (e.g. 74 to 74)
	for (auto move : legalMoves)
	{
		if (move.first != "74" && move.first != "04") { continue; }

		if (find(move.second.begin(), move.second.end(), move.first) != move.second.end())
		{
			vector<string> pieceMoves = move.second;
			pieceMoves.erase(remove(pieceMoves.begin(), pieceMoves.end(), move.first), pieceMoves.end());
			legalMoves[move.first] = pieceMoves;
		}
	}

	// remove illegal moves from legalMoves
	for (auto move : illegalMoves)
	{
		vector<string> newLegal;
		vector<string> oldLegal = legalMoves[move.first];
		for (int i = 0; i < oldLegal.size(); i++)
		{
			if (find(move.second.begin(), move.second.end(), oldLegal[i]) == move.second.end() &&
				find(illegalCastling.begin(), illegalCastling.end(), oldLegal[i]) == illegalCastling.end())
			{
				newLegal.push_back(oldLegal[i]);
			}
		}

		if (newLegal.size() > 0)
		{
			legalMoves[move.first] = newLegal;
		}
		else
		{
			legalMoves.erase(move.first);
		}
	}


#pragma endregion

	return legalMoves;
}
 
// (copied from getLegalMoves())
// if whiteToMove == true, then returns true if white is in check
bool helper::inCheck(char board[8][8], bool whiteToMove)
{
	int i = -1;
	int j = -1;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if ((whiteToMove && board[y][x] == 'K') || (!whiteToMove && board[y][x] == 'k'))
			{
				i = y;
				j = x;
				y = 8; x = 8; // break from loops
			}
		}
	}

	// check if the king can be captured by each type of piece
	if (whiteToMove)
	{
		// pawns
		if (j > 0 && board[i - 1][j - 1] == 'p') { return true; }
		if (j < 7 && board[i - 1][j + 1] == 'p') { return true; }

		// knights
		vector<vector<int>> knightPos;
		knightPos.push_back({ i - 2, j - 1 }); knightPos.push_back({ i - 2, j + 1 });
		knightPos.push_back({ i - 1, j - 2 }); knightPos.push_back({ i - 1, j + 2 });
		knightPos.push_back({ i + 1, j - 2 }); knightPos.push_back({ i + 1, j + 2 });
		knightPos.push_back({ i + 2, j - 1 }); knightPos.push_back({ i + 2, j + 1 });
		for (int k = 0; k < knightPos.size(); k++)
		{
			int y = knightPos[k][0];
			int x = knightPos[k][1];
			if (y >= 0 && y <= 7 && x >= 0 && x <= 7 && board[y][x] == 'n') { return true; }
		}

		// bishops, rooks, queen
#pragma region
				// top left
		int y = i - 1;
		int x = j - 1;
		while (y >= 0 && x >= 0 && (board[y][x] == ' ' || board[y][x] == 'b' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'b' || board[y][x] == 'q') { return true; break; }
			y--;
			x--;
		}

		// top right
		y = i - 1;
		x = j + 1;
		while (y >= 0 && x <= 7 && (board[y][x] == ' ' || board[y][x] == 'b' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'b' || board[y][x] == 'q') { return true; break; }
			y--;
			x++;
		}

		// bottom left
		y = i + 1;
		x = j - 1;
		while (y <= 7 && x >= 0 && (board[y][x] == ' ' || board[y][x] == 'b' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'b' || board[y][x] == 'q') { return true; break; }
			y++;
			x--;
		}

		// bottom right
		y = i + 1;
		x = j + 1;
		while (y <= 7 && x <= 7 && (board[y][x] == ' ' || board[y][x] == 'b' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'b' || board[y][x] == 'q') { return true; break; }
			y++;
			x++;
		}

		// top
		y = i - 1;
		x = j;
		while (y >= 0 && (board[y][x] == ' ' || board[y][x] == 'r' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'r' || board[y][x] == 'q') { return true; break; }
			y--;
		}

		// left
		y = i;
		x = j - 1;
		while (x >= 0 && (board[y][x] == ' ' || board[y][x] == 'r' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'r' || board[y][x] == 'q') { return true; break; }
			x--;
		}

		// right
		y = i;
		x = j + 1;
		while (x <= 7 && (board[y][x] == ' ' || board[y][x] == 'r' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'r' || board[y][x] == 'q') { return true; break; }
			x++;
		}

		// bottom
		y = i + 1;
		x = j;
		while (y <= 7 && (board[y][x] == ' ' || board[y][x] == 'r' || board[y][x] == 'q'))
		{
			if (board[y][x] == 'r' || board[y][x] == 'q') { return true; break; }
			y++;
		}
#pragma endregion

		// king
#pragma region
				// top left
		y = i - 1;
		x = j - 1;
		if (y >= 0 && x >= 0 && (board[y][x] == 'k')) { return true; }

		// top
		y = i - 1;
		x = j;
		if (y >= 0 && (board[y][x] == 'k')) { return true; }

		// top right
		y = i - 1;
		x = j + 1;
		if (y >= 0 && x <= 7 && (board[y][x] == 'k')) { return true; }

		// left
		y = i;
		x = j - 1;
		if (x >= 0 && (board[y][x] == 'k')) { return true; }

		// right
		y = i;
		x = j + 1;
		if (x <= 7 && (board[y][x] == 'k')) { return true; }

		// bottom left
		y = i + 1;
		x = j - 1;
		if (y <= 7 && x >= 0 && (board[y][x] == 'k')) { return true; }

		// bottom
		y = i + 1;
		x = j;
		if (y <= 7 && (board[y][x] == 'k')) { return true; }

		// bottom right
		y = i + 1;
		x = j + 1;
		if (y <= 7 && x <= 7 && (board[y][x] == 'k')) { return true; }
#pragma endregion

	}

	else // black to move
	{
		// pawns
		if (j > 0 && board[i + 1][j - 1] == 'P') { return true; }
		if (j < 7 && board[i + 1][j + 1] == 'P') { return true; }

		// knights
		vector<vector<int>> knightPos;
		knightPos.push_back({ i - 2, j - 1 }); knightPos.push_back({ i - 2, j + 1 });
		knightPos.push_back({ i - 1, j - 2 }); knightPos.push_back({ i - 1, j + 2 });
		knightPos.push_back({ i + 1, j - 2 }); knightPos.push_back({ i + 1, j + 2 });
		knightPos.push_back({ i + 2, j - 1 }); knightPos.push_back({ i + 2, j + 1 });
		for (int k = 0; k < knightPos.size(); k++)
		{
			int y = knightPos[k][0];
			int x = knightPos[k][1];
			if (y >= 0 && y <= 7 && x >= 0 && x <= 7 && board[y][x] == 'N') { return true; }
		}

		// bishops, rooks, queen
#pragma region
				// top left
		int y = i - 1;
		int x = j - 1;
		while (y >= 0 && x >= 0 && (board[y][x] == ' ' || board[y][x] == 'B' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'B' || board[y][x] == 'Q') { return true; break; }
			y--;
			x--;
		}

		// top right
		y = i - 1;
		x = j + 1;
		while (y >= 0 && x <= 7 && (board[y][x] == ' ' || board[y][x] == 'B' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'B' || board[y][x] == 'Q') { return true; break; }
			y--;
			x++;
		}

		// bottom left
		y = i + 1;
		x = j - 1;
		while (y <= 7 && x >= 0 && (board[y][x] == ' ' || board[y][x] == 'B' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'B' || board[y][x] == 'Q') { return true; break; }
			y++;
			x--;
		}

		// bottom right
		y = i + 1;
		x = j + 1;
		while (y <= 7 && x <= 7 && (board[y][x] == ' ' || board[y][x] == 'B' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'B' || board[y][x] == 'Q') { return true; break; }
			y++;
			x++;
		}

		// top
		y = i - 1;
		x = j;
		while (y >= 0 && (board[y][x] == ' ' || board[y][x] == 'R' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'R' || board[y][x] == 'Q') { return true; break; }
			y--;
		}

		// left
		y = i;
		x = j - 1;
		while (x >= 0 && (board[y][x] == ' ' || board[y][x] == 'R' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'R' || board[y][x] == 'Q') { return true; break; }
			x--;
		}

		// right
		y = i;
		x = j + 1;
		while (x <= 7 && (board[y][x] == ' ' || board[y][x] == 'R' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'R' || board[y][x] == 'Q') { return true; break; }
			x++;
		}

		// bottom
		y = i + 1;
		x = j;
		while (y <= 7 && (board[y][x] == ' ' || board[y][x] == 'R' || board[y][x] == 'Q'))
		{
			if (board[y][x] == 'R' || board[y][x] == 'Q') { return true; break; }
			y++;
		}
#pragma endregion

		// king
#pragma region
				// top left
		y = i - 1;
		x = j - 1;
		if (y >= 0 && x >= 0 && (board[y][x] == 'K')) { return true; }

		// top
		y = i - 1;
		x = j;
		if (y >= 0 && (board[y][x] == 'K')) { return true; }

		// top right
		y = i - 1;
		x = j + 1;
		if (y >= 0 && x <= 7 && (board[y][x] == 'K')) { return true; }

		// left
		y = i;
		x = j - 1;
		if (x >= 0 && (board[y][x] == 'K')) { return true; }

		// right
		y = i;
		x = j + 1;
		if (x <= 7 && (board[y][x] == 'K')) { return true; }

		// bottom left
		y = i + 1;
		x = j - 1;
		if (y <= 7 && x >= 0 && (board[y][x] == 'K')) { return true; }

		// bottom
		y = i + 1;
		x = j;
		if (y <= 7 && (board[y][x] == 'K')) { return true; }

		// bottom right
		y = i + 1;
		x = j + 1;
		if (y <= 7 && x <= 7 && (board[y][x] == 'K')) { return true; }
#pragma endregion

	}

	return false;
}