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

string helper::roundFloat(float f, int r)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(r) << f;
	return stream.str();
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
	
	memcpy(newPosition.castling, before.castling, 4 * sizeof(bool));

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
			//newPosition.enpassant = to_string(to[0] - 48 + 1) + to_string(to[1] - 48);
		}
		else if (board[to[0] - 48][to[1] - 48] == 'p' && (from[0] - 48) - (to[0] - 48) == -2)
		{
			//newPosition.enpassant = to_string(to[0] - 48 - 1) + to_string(to[1] - 48);
		}
		else
		{
			//newPosition.enpassant = "";
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
	bool changed = false;
	if (from == "74" || from == "77") { newPosition.castling[0] = false; changed = true; }
	if (from == "74" || from == "70") { newPosition.castling[1] = false; changed = true; }
	if (from == "04" || from == "07") { newPosition.castling[2] = false; changed = true; }
	if (from == "04" || from == "00") { newPosition.castling[3] = false; changed = true; }

	memcpy(newPosition.board, board, 64 * sizeof(char));

	return newPosition;
}

map<string, vector<string>> helper::getLegalMoves(Position position, bool whiteToMove)
{
	char board[8][8];
	bool castling[4];
	//string enpassant;
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

					//int enpassantY = -1;
					//int enpassantX = -1;
					//if (enpassant.length() > 0)
					//{
					//	enpassantY = enpassant[0] - 48;
					//	enpassantX = enpassant[1] - 48;
					//}

					// capture left
					if (j > 0 && (board[i - 1][j - 1] >= 97/* || (i - 1 == enpassantY && j - 1 == enpassantX)*/)) { pieceMoves.push_back(to_string(i - 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && (board[i - 1][j + 1] >= 97/* || (i - 1 == enpassantY && j + 1 == enpassantX)*/)) { pieceMoves.push_back(to_string(i - 1) + to_string(j + 1)); }

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

					//int enpassantY = -1;
					//int enpassantX = -1;
					//if (enpassant.length() > 0)
					//{
					//	enpassantY = enpassant[0] - 48;
					//	enpassantX = enpassant[1] - 48;
					//}

					// capture left
					if (j > 0 && ((board[i + 1][j - 1] >= 65 && board[i + 1][j - 1] <= 90)/* || (i + 1 == enpassantY && j - 1 == enpassantX)*/)) { pieceMoves.push_back(to_string(i + 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && ((board[i + 1][j + 1] >= 65 && board[i + 1][j + 1] <= 90)/* || (i + 1 == enpassantY && j + 1 == enpassantX)*/)) { pieceMoves.push_back(to_string(i + 1) + to_string(j + 1)); }

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
 
// (copied from getNextPositions())
// if whiteToMove == true, then returns true if white is in check
bool helper::inCheck(vector<U64> board, bool whiteToMove)
{
	U64 dangerMap = 0ULL;

	// find ally/enemy piece positions
	U64 allyPieces = 0ULL;
	U64 enemyPieces = 0ULL;
	if (whiteToMove)
	{
		for (int j = 0; j < 6; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(allyPieces, square);
				}
			}
		}

		for (int j = 6; j < 12; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(enemyPieces, square);
				}
			}
		}
	}
	else
	{
		for (int j = 6; j < 12; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(allyPieces, square);
				}
			}
		}

		for (int j = 0; j < 6; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(enemyPieces, square);
				}
			}
		}
	}

	int pieceStart = (whiteToMove ? 0 : 6);
	int pieceEnd = (whiteToMove ? 6 : 12);

	// find all possible moves (legal or not) from the current position
	for (int i = pieceStart; i < pieceEnd; i++)
	{
		char currentPiece = helper::pieces[i];
		switch (currentPiece)
		{
		case 'P':
		case 'p':
		{
			U64 enemyPieceBitBoard = (currentPiece == 'P' ? board[BLACKPAWN] : board[WHITEPAWN]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[4] = { -7, -9 };

					if (!whiteToMove)
					{
						for (int j = 0; j < 4; j++)
						{
							moveAmount[j] *= -1;
						}
					}

					//int enpassantSquare = getIntFromBits(board[BOARDEXTRA], ENPASSANTLSB, ENPASSANTLSB + 7);

					if ((whiteToMove && getBit(COL7BITBOARD, startSquare)) || (!whiteToMove && getBit(COL0BITBOARD, startSquare))) { moveAmount[2] = 0; }
					if ((whiteToMove && getBit(COL0BITBOARD, startSquare)) || (!whiteToMove && getBit(COL7BITBOARD, startSquare))) { moveAmount[3] = 0; }

					for (int j = 2; j < 4; j++)
					{
						int moveSquare = startSquare + moveAmount[j];
						if (moveAmount[j] == 0) { continue; }

						setBit(dangerMap, moveSquare);
					}
				}
			}

			break;
		}

		case 'N':
		case 'n':
		{
			U64 enemyPieceBitBoard = (currentPiece == 'N' ? board[BLACKKNIGHT] : board[WHITEKNIGHT]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(enemyPieceBitBoard, startSquare))
				{
					int toMove[8] = { startSquare - 17, startSquare - 15, startSquare - 10, startSquare - 6, startSquare + 6, startSquare + 10, startSquare + 15, startSquare + 17 };

					int knightPos = 0;

					if (getBit(COL0BITBOARD, startSquare)) { knightPos += (1 << 0); }
					if (getBit(COL1BITBOARD, startSquare)) { knightPos += (1 << 1); }
					if (getBit(COL6BITBOARD, startSquare)) { knightPos += (1 << 2); }
					if (getBit(COL7BITBOARD, startSquare)) { knightPos += (1 << 3); }
					if (getBit(ROW0BITBOARD, startSquare)) { knightPos += (1 << 4); }
					if (getBit(ROW1BITBOARD, startSquare)) { knightPos += (1 << 5); }
					if (getBit(ROW6BITBOARD, startSquare)) { knightPos += (1 << 6); }
					if (getBit(ROW7BITBOARD, startSquare)) { knightPos += (1 << 7); }

					if (getBit(knightPos, 0) || getBit(knightPos, 4) || getBit(knightPos, 5)) { toMove[0] = -1; }
					if (getBit(knightPos, 3) || getBit(knightPos, 4) || getBit(knightPos, 5)) { toMove[1] = -1; }
					if (getBit(knightPos, 0) || getBit(knightPos, 1) || getBit(knightPos, 4)) { toMove[2] = -1; }
					if (getBit(knightPos, 2) || getBit(knightPos, 3) || getBit(knightPos, 4)) { toMove[3] = -1; }
					if (getBit(knightPos, 0) || getBit(knightPos, 1) || getBit(knightPos, 7)) { toMove[4] = -1; }
					if (getBit(knightPos, 2) || getBit(knightPos, 3) || getBit(knightPos, 7)) { toMove[5] = -1; }
					if (getBit(knightPos, 0) || getBit(knightPos, 6) || getBit(knightPos, 7)) { toMove[6] = -1; }
					if (getBit(knightPos, 3) || getBit(knightPos, 6) || getBit(knightPos, 7)) { toMove[7] = -1; }

					for (int moveSquare : toMove)
					{
						if (moveSquare == -1) { continue; }

						setBit(dangerMap, moveSquare);
					}
				}
			}

			break;
		}

		case 'B':
		case 'b':
		{
			U64 enemyPieceBitBoard = (currentPiece == 'B' ? board[BLACKBISHOP] : board[WHITEBISHOP]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[4] = { -9, -7, 7, 9 };

					for (int j = 0; j < 4; j++)
					{
						int moveSquare = startSquare;

						while (true)
						{
							if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
							if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }

							moveSquare += moveAmount[j];

							setBit(dangerMap, moveSquare);
						}
					}


				}
			}

			break;
		}

		case 'R':
		case 'r':
		{
			U64 enemyPieceBitBoard = (currentPiece == 'R' ? board[BLACKROOK] : board[WHITEROOK]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[4] = { -8, -1, 1, 8 };

					for (int j = 0; j < 4; j++)
					{
						int moveSquare = startSquare;

						while (true)
						{
							if (j == 0 && getBit(ROW0BITBOARD, moveSquare)) { break; }
							if (j == 1 && getBit(COL0BITBOARD, moveSquare)) { break; }
							if (j == 2 && getBit(COL7BITBOARD, moveSquare)) { break; }
							if (j == 3 && getBit(ROW7BITBOARD, moveSquare)) { break; }

							moveSquare += moveAmount[j];

							setBit(dangerMap, moveSquare);
						}
					}


				}
			}

			break;
		}

		case 'Q':
		case 'q':
		{
			U64 enemyPieceBitBoard = (currentPiece == 'Q' ? board[BLACKQUEEN] : board[WHITEQUEEN]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[8] = { -9, -7, 7, 9, -8, -1, 1, 8 };

					for (int j = 0; j < 8; j++)
					{
						int moveSquare = startSquare;

						while (true)
						{
							if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
							if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
							if (j == 4 && getBit(ROW0BITBOARD, moveSquare)) { break; }
							if (j == 5 && getBit(COL0BITBOARD, moveSquare)) { break; }
							if (j == 6 && getBit(COL7BITBOARD, moveSquare)) { break; }
							if (j == 7 && getBit(ROW7BITBOARD, moveSquare)) { break; }

							moveSquare += moveAmount[j];

							setBit(dangerMap, moveSquare);

						}
					}


				}
			}

			break;
		}


		default:
			break;

		}

	}

	int kingSquare = -1;
	for (int i = 0; i < 64; i++)
	{
		if ((whiteToMove && getBit(board[WHITEKING], i)) || (!whiteToMove && getBit(board[BLACKKING], i)))
		{
			kingSquare = i;
		}
	}

	return getBit(dangerMap, kingSquare);
}

// another version for char array board
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

vector<U64> helper::positionToU64(Position position)
{
	char positionBoard[8][8];
	bool castling[4];
	//string enpassant = position.enpassant;
	memcpy(positionBoard, position.board, 64 * sizeof(char));
	memcpy(castling, position.castling, 4 * sizeof(bool));

	vector<U64> board;

	for (int i = WHITEPAWN; i < BLACKKING + 1; i++)
	{
		char currentPiece = helper::pieces[i];
		U64 currentPieceBitBoard = 0ULL;

		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				if (positionBoard[y][x] == currentPiece)
				{
					setBit(currentPieceBitBoard, y * 8 + x);
				}
			}
		}

		board.push_back(currentPieceBitBoard);
	}

	U64 extra = 1ULL;
	for (int i = 1; i < 5; i++)
	{
		if (castling[i - 1])
		{
			setBit(extra, i);
		}
	}

	//if (enpassant.length() > 0)
	//{
	//	int enpassantY = enpassant[0] - 48;
	//	int enpassantX = enpassant[1] - 48;

	//	extra = helper::setBitsFromInt(extra, ENPASSANTLSB, ENPASSANTLSB + 7, enpassantY * 8 + enpassantX);
	//}
	//else
	//{
	//	setBit(extra, ENPASSANTLSB + 7);
	//}

	board.push_back(extra);

	// hash value
	board.push_back(0);

	return board;
}

Position helper::U64ToPosition(vector<U64> board)
{
	Position position;

	char positionBoard[8][8];
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			positionBoard[y][x] = ' ';
		}
	}

	for (int i = WHITEPAWN; i < BLACKKING + 1; i++)
	{
		char currentPiece = helper::pieces[i];

		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				if (getBit(board[i], y * 8 + x))
				{
					assert(positionBoard[y][x] == ' ');
					positionBoard[y][x] = currentPiece;
				}
			}
		}
	}
	memcpy(position.board, positionBoard, 64 * sizeof(char));

	bool castling[4] = { true, true, true, true };
	for (int i = 1; i < 5; i++)
	{
		if (!getBit(board[BOARDEXTRA], i))
		{
			castling[i - 1] = false;
		}
	}
	memcpy(position.castling, castling, 4 * sizeof(bool));

	//int enpassantSquare = helper::getIntFromBits(board[BOARDEXTRA], ENPASSANTLSB, ENPASSANTLSB + 7);

	//if (enpassantSquare < 64)
	//{
	//	int enpassantY = enpassantSquare / 8;
	//	int enpassantX = enpassantSquare % 8;

	//	string enpassantString = "";
	//	enpassantString += char(enpassantY + 48);
	//	enpassantString += char(enpassantX + 48);
	//	position.enpassant = enpassantString;
	//}
	//else
	//{
	//	position.enpassant = "";
	//}

	return position;
}

void helper::printBoard(vector<U64> board)
{
	char charBoard[8][8];

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			charBoard[i][j] = '.';
		}
	}

	for (int i = 0; i < 12; i++)
	{
		U64 currentBoard = board[i];

		char currentPiece = helper::pieces[i];

		int square = 0;
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				square = j * 8 + k;
				if (getBit(currentBoard, square))
				{
					assert(charBoard[j][k] == '.');
					charBoard[j][k] = currentPiece;
				}
			}
		}
	}

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%c ", charBoard[i][j]);
		}
		printf("\n");
	}

	U64 additional = board[12];
	string additionalString = "";

	additionalString += (getBit(additional, 0) ? "White to move\n" : "Black to move\n");

	additionalString += "Castling: ";

	string castlingString = "";
	castlingString = (getBit(additional, 1) ? "K" : "") + castlingString;
	castlingString = (getBit(additional, 2) ? "Q" : "") + castlingString;
	castlingString = (getBit(additional, 3) ? "k" : "") + castlingString;
	castlingString = (getBit(additional, 4) ? "q" : "") + castlingString;
	additionalString += castlingString + "\n";

	printf("%s", additionalString.c_str());

	printf("-----------------------\n");
}

int helper::getIntFromBits(U64 bits, int start, int end)
{
	U64 temp = 0ULL;
	int i = 0;
	for (int j = start; j < end; j++)
	{
		if (getBit(bits, j)) { setBit(temp, i); }
		i++;
	}

	return (int)temp;
}

U64 helper::setBitsFromInt(U64 bits, int start, int end, int num)
{
	U64 temp = (U64)num;
	int i = 0;
	for (int j = start; j < end; j++)
	{
		popBit(bits, j);
		if (getBit(temp, i)) { setBit(bits, j); }
		i++;
	}

	return bits;
}

vector<vector<U64>> helper::getNextPositions(vector<U64> board, bool whiteToMove, U64 zobristTable[13][64])
{
	board[BOARDEXTRA] ^= 1ULL;
	board[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][0];
	assert(whiteToMove == getBit(board[BOARDEXTRA], 0));

	//int enpassantSquare = getIntFromBits(board[BOARDEXTRA], ENPASSANTLSB, ENPASSANTLSB + 7);

	int baseEvaluationInteger = getIntFromBits(board[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10);
	int baseEvaluationDecimal = getIntFromBits(board[BOARDEXTRA], EVALUATIONLSB + 10, EVALUATIONLSB + 17);
	double baseEvaluation = (double)baseEvaluationInteger + ((double)baseEvaluationDecimal / 100);
	if (getBit(board[BOARDEXTRA], NEGATIVEEVALUATION))
	{
		baseEvaluation *= -1;
	}

	if (baseEvaluation > 1000 | baseEvaluation < -1000)
	{
		printf("");
	}

	int capturedPiece = -1;
	int newEvaluation = 0;

	// default metadata for board
	for (int i = LASTKINGMOVE; i <= LASTCASTLEWHITEKINGSIDE; i++)
	{
		popBit(board[BOARDEXTRA], i);
	}

	vector<vector<U64>> nextPositions[6];

	int pieceStart = (whiteToMove ? 0 : 6);
	int pieceEnd = (whiteToMove ? 6 : 12);

	// used later for checking legality of moves
	U64 dangerMap = 0ULL;

	// find ally/enemy piece positions
	U64 allyPieces = 0ULL;
	U64 enemyPieces = 0ULL;
	if (whiteToMove)
	{
		for (int j = 0; j < 6; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(allyPieces, square);
				}
			}
		}

		for (int j = 6; j < 12; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(enemyPieces, square);
				}
			}
		}
	}
	else
	{
		for (int j = 6; j < 12; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(allyPieces, square);
				}
			}
		}

		for (int j = 0; j < 6; j++)
		{
			U64 currentBoard = board[j];
			for (int square = 0; square < 64; square++)
			{
				if (getBit(currentBoard, square))
				{
					setBit(enemyPieces, square);
				}
			}
		}
	}

	// find all possible moves (legal or not) from the current position
	for (int i = pieceStart; i < pieceEnd; i++)
	{
		char currentPiece = helper::pieces[i];
		switch (currentPiece)
		{
		case 'P':
		case 'p':
		{
			U64 allyPieceBitBoard = (currentPiece == 'P' ? board[WHITEPAWN] : board[BLACKPAWN]);
			U64 enemyPieceBitBoard = (currentPiece == 'P' ? board[BLACKPAWN] : board[WHITEPAWN]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(allyPieceBitBoard, startSquare) || getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[4] = { -8, -16, -7, -9 };

					if (!whiteToMove)
					{
						for (int j = 0; j < 4; j++)
						{
							moveAmount[j] *= -1;
						}
					}

					if ((whiteToMove && !getBit(ROW6BITBOARD, startSquare)) || (!whiteToMove && !getBit(ROW1BITBOARD, startSquare))) { moveAmount[1] = 0; }

					if ((whiteToMove && getBit(COL7BITBOARD, startSquare)) || (!whiteToMove && getBit(COL0BITBOARD, startSquare))) { moveAmount[2] = 0; }
					if ((whiteToMove && getBit(COL0BITBOARD, startSquare)) || (!whiteToMove && getBit(COL7BITBOARD, startSquare))) { moveAmount[3] = 0; }

					if (!getBit(enemyPieces, startSquare + moveAmount[2])/* && (startSquare + moveAmount[2] != enpassantSquare) */) { moveAmount[2] = 0; }
					if (!getBit(enemyPieces, startSquare + moveAmount[3])/* && (startSquare + moveAmount[3] != enpassantSquare) */) { moveAmount[3] = 0; }

					if (getBit(allyPieceBitBoard, startSquare))
					{
						for (int j = 0; j < 4; j++)
						{
							int moveSquare = startSquare + moveAmount[j];

							bool pawnPromotion = (whiteToMove && getBit(ROW1BITBOARD, startSquare)) || (!whiteToMove && getBit(ROW6BITBOARD, startSquare));
							int newPiece = 0;
							if (currentPiece == 'P')
							{
								newPiece = (pawnPromotion ? WHITEQUEEN : WHITEPAWN);
							}
							else
							{
								newPiece = (pawnPromotion ? BLACKQUEEN : BLACKPAWN);
							}

							if (moveAmount[j] == 0) { continue; }

							if (getBit(allyPieces, moveSquare)) { continue; }

							// capture enemy piece (including enpassant)
							else if ((getBit(enemyPieces, moveSquare)/* || moveSquare == enpassantSquare*/) && j >= 2)
							{
								vector<U64> newBoard = board;

								if (currentPiece == 'P')
								{
									int enpassantoffset = 0;// (moveSquare == enpassantSquare ? 8 : 0);

									popBit(newBoard[WHITEPAWN], startSquare);
									setBit(newBoard[newPiece], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEPAWN][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[newPiece][moveSquare];

									for (int k = 6; k < 12; k++)
									{
										if (getBit(newBoard[k], moveSquare + enpassantoffset))
										{
											popBit(newBoard[k], moveSquare + enpassantoffset);
											newBoard[BOARDHASHVALUE] ^= zobristTable[k][moveSquare + enpassantoffset];

											newEvaluation = baseEvaluation - pieceValues[k];
											if (newEvaluation >= 0)
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
												popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											else
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
												setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}

											capturedPiece = k % 6;
											break;
										}
									}
								}
								else
								{
									int enpassantoffset = 0;// (moveSquare == enpassantSquare ? -8 : 0);

									popBit(newBoard[BLACKPAWN], startSquare);
									setBit(newBoard[newPiece], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKPAWN][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[newPiece][moveSquare];

									for (int k = 0; k < 6; k++)
									{
										if (getBit(newBoard[k], moveSquare + enpassantoffset))
										{
											popBit(newBoard[k], moveSquare + enpassantoffset);
											newBoard[BOARDHASHVALUE] ^= zobristTable[k][moveSquare + enpassantoffset];

											newEvaluation = baseEvaluation - pieceValues[k];
											if (newEvaluation >= 0)
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
												popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											else
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
												setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											capturedPiece = k % 6;
											break;
										}
									}
								}

								nextPositions[capturedPiece].push_back(newBoard);
							}

							// move one or two squares
							else if (!getBit(enemyPieces, moveSquare) && j < 2)
							{
								if ((moveAmount[j] == -16 && (getBit(allyPieces, moveSquare + 8) || getBit(enemyPieces, moveSquare + 8)) ||
									moveAmount[j] == 16 && (getBit(allyPieces, moveSquare - 8) || getBit(enemyPieces, moveSquare - 8)))) {
									continue;
								}

								vector<U64> newBoard = board;

								if (currentPiece == 'P')
								{
									popBit(newBoard[WHITEPAWN], startSquare);
									setBit(newBoard[newPiece], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEPAWN][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[newPiece][moveSquare];
								}
								else
								{
									popBit(newBoard[BLACKPAWN], startSquare);
									setBit(newBoard[newPiece], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKPAWN][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[newPiece][moveSquare];
								}

								// set enpassant square if pawn moved two squares
								//if (moveAmount[j] == -16)
								//{
								//	newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], ENPASSANTLSB, ENPASSANTLSB + 7, moveSquare + 8);

								//	for (int k = ENPASSANTLSB; k < ENPASSANTLSB + 7; k++)
								//	{
								//		if (getBit(newBoard[BOARDEXTRA], k))
								//		{
								//			newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][k];
								//		}
								//	}
								//}
								//else if (moveAmount[j] == 16)
								//{
								//	newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], ENPASSANTLSB, ENPASSANTLSB + 7, moveSquare - 8);

								//	for (int k = ENPASSANTLSB; k < ENPASSANTLSB + 7; k++)
								//	{
								//		if (getBit(newBoard[BOARDEXTRA], k))
								//		{
								//			newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][k];
								//		}
								//	}
								//}

								nextPositions[0].push_back(newBoard);
							}
						}
					}

					if (getBit(enemyPieceBitBoard, startSquare))
					{
						if (currentPiece == 'P')
						{
							setBit(dangerMap, startSquare + 7);
							setBit(dangerMap, startSquare + 9);
						}
						else
						{
							setBit(dangerMap, startSquare - 7);
							setBit(dangerMap, startSquare - 9);
						}
					}
				}
			}

			break;
		}

		case 'N':
		case 'n':
		{
			U64 allyPieceBitBoard = (currentPiece == 'N' ? board[WHITEKNIGHT] : board[BLACKKNIGHT]);
			U64 enemyPieceBitBoard = (currentPiece == 'N' ? board[BLACKKNIGHT] : board[WHITEKNIGHT]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(allyPieceBitBoard, startSquare) || getBit(enemyPieceBitBoard, startSquare))
				{
					int toMove[8] = { startSquare - 17, startSquare - 15, startSquare - 10, startSquare - 6, startSquare + 6, startSquare + 10, startSquare + 15, startSquare + 17 };

					int knightPos = 0;

					if (getBit(COL0BITBOARD, startSquare)) { knightPos += (1 << 0); }
					if (getBit(COL1BITBOARD, startSquare)) { knightPos += (1 << 1); }
					if (getBit(COL6BITBOARD, startSquare)) { knightPos += (1 << 2); }
					if (getBit(COL7BITBOARD, startSquare)) { knightPos += (1 << 3); }
					if (getBit(ROW0BITBOARD, startSquare)) { knightPos += (1 << 4); }
					if (getBit(ROW1BITBOARD, startSquare)) { knightPos += (1 << 5); }
					if (getBit(ROW6BITBOARD, startSquare)) { knightPos += (1 << 6); }
					if (getBit(ROW7BITBOARD, startSquare)) { knightPos += (1 << 7); }

					if (getBit(knightPos, 0) || getBit(knightPos, 4) || getBit(knightPos, 5)) { toMove[0] = -1; }
					if (getBit(knightPos, 3) || getBit(knightPos, 4) || getBit(knightPos, 5)) { toMove[1] = -1; }
					if (getBit(knightPos, 0) || getBit(knightPos, 1) || getBit(knightPos, 4)) { toMove[2] = -1; }
					if (getBit(knightPos, 2) || getBit(knightPos, 3) || getBit(knightPos, 4)) { toMove[3] = -1; }
					if (getBit(knightPos, 0) || getBit(knightPos, 1) || getBit(knightPos, 7)) { toMove[4] = -1; }
					if (getBit(knightPos, 2) || getBit(knightPos, 3) || getBit(knightPos, 7)) { toMove[5] = -1; }
					if (getBit(knightPos, 0) || getBit(knightPos, 6) || getBit(knightPos, 7)) { toMove[6] = -1; }
					if (getBit(knightPos, 3) || getBit(knightPos, 6) || getBit(knightPos, 7)) { toMove[7] = -1; }

					if (getBit(allyPieceBitBoard, startSquare))
					{
						for (int moveSquare : toMove)
						{
							if (moveSquare == -1) { continue; }

							if (getBit(allyPieces, moveSquare)) { continue; }

							else if (getBit(enemyPieces, moveSquare))
							{
								vector<U64> newBoard = board;

								if (currentPiece == 'N')
								{
									popBit(newBoard[WHITEKNIGHT], startSquare);
									setBit(newBoard[WHITEKNIGHT], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKNIGHT][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKNIGHT][moveSquare];

									for (int j = 6; j < 12; j++)
									{
										if (getBit(newBoard[j], moveSquare))
										{
											popBit(newBoard[j], moveSquare);
											newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

											newEvaluation = baseEvaluation - pieceValues[j];
											if (newEvaluation >= 0)
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
												popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											else
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
												setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											capturedPiece = j % 6;
											break;
										}
									}
								}
								else
								{
									popBit(newBoard[BLACKKNIGHT], startSquare);
									setBit(newBoard[BLACKKNIGHT], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKNIGHT][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKNIGHT][moveSquare];

									for (int j = 0; j < 6; j++)
									{
										if (getBit(newBoard[j], moveSquare))
										{
											popBit(newBoard[j], moveSquare);
											newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

											newEvaluation = baseEvaluation - pieceValues[j];
											if (newEvaluation >= 0)
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
												popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											else
											{
												newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
												setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
											}
											capturedPiece = j % 6;
											break;
										}
									}
								}

								nextPositions[capturedPiece].push_back(newBoard);
							}
							else
							{
								vector<U64> newBoard = board;

								if (currentPiece == 'N')
								{
									popBit(newBoard[WHITEKNIGHT], startSquare);
									setBit(newBoard[WHITEKNIGHT], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKNIGHT][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKNIGHT][moveSquare];
								}
								else
								{
									popBit(newBoard[BLACKKNIGHT], startSquare);
									setBit(newBoard[BLACKKNIGHT], moveSquare);

									newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKNIGHT][startSquare];
									newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKNIGHT][moveSquare];
								}

								nextPositions[0].push_back(newBoard);
							}
						}
					}

					if (getBit(enemyPieceBitBoard, startSquare))
					{
						for (int moveSquare : toMove)
						{
							if (moveSquare == -1) { continue; }

							setBit(dangerMap, moveSquare);
						}
					}

				}
			}

			break;
		}

		case 'B':
		case 'b':
		{
			U64 allyPieceBitBoard = (currentPiece == 'B' ? board[WHITEBISHOP] : board[BLACKBISHOP]);
			U64 enemyPieceBitBoard = (currentPiece == 'B' ? board[BLACKBISHOP] : board[WHITEBISHOP]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(allyPieceBitBoard, startSquare) || getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[4] = { -9, -7, 7, 9 };

					for (int j = 0; j < 4; j++)
					{
						int moveSquare = startSquare;

						while (true)
						{
							if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
							if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }

							moveSquare += moveAmount[j];

							if (getBit(allyPieceBitBoard, startSquare))
							{
								if (getBit(allyPieces, moveSquare)) { break; }
								else if (getBit(enemyPieces, moveSquare))
								{
									vector<U64> newBoard = board;

									if (currentPiece == 'B')
									{
										popBit(newBoard[WHITEBISHOP], startSquare);
										setBit(newBoard[WHITEBISHOP], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEBISHOP][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEBISHOP][moveSquare];

										for (int j = 6; j < 12; j++)
										{
											if (getBit(newBoard[j], moveSquare))
											{
												popBit(newBoard[j], moveSquare);
												newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

												newEvaluation = baseEvaluation - pieceValues[j];
												if (newEvaluation >= 0)
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
													popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												else
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
													setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												capturedPiece = j % 6;
												break;
											}
										}
									}
									else
									{
										popBit(newBoard[BLACKBISHOP], startSquare);
										setBit(newBoard[BLACKBISHOP], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKBISHOP][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKBISHOP][moveSquare];

										for (int j = 0; j < 6; j++)
										{
											if (getBit(newBoard[j], moveSquare))
											{
												popBit(newBoard[j], moveSquare);
												newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

												newEvaluation = baseEvaluation - pieceValues[j];
												if (newEvaluation >= 0)
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
													popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												else
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
													setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												capturedPiece = j % 6;
												break;
											}
										}
									}

									nextPositions[capturedPiece].push_back(newBoard);
									break;
								}
								else
								{
									vector<U64> newBoard = board;

									if (currentPiece == 'B')
									{
										popBit(newBoard[WHITEBISHOP], startSquare);
										setBit(newBoard[WHITEBISHOP], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEBISHOP][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEBISHOP][moveSquare];
									}
									else
									{
										popBit(newBoard[BLACKBISHOP], startSquare);
										setBit(newBoard[BLACKBISHOP], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKBISHOP][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKBISHOP][moveSquare];
									}

									nextPositions[0].push_back(newBoard);
								}
							}

							if (getBit(enemyPieceBitBoard, startSquare))
							{
								setBit(dangerMap, moveSquare);
							}
						}
					}


				}
			}

			break;
		}

		case 'R':
		case 'r':
		{
			U64 allyPieceBitBoard = (currentPiece == 'R' ? board[WHITEROOK] : board[BLACKROOK]);
			U64 enemyPieceBitBoard = (currentPiece == 'R' ? board[BLACKROOK] : board[WHITEROOK]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(allyPieceBitBoard, startSquare) || getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[4] = { -8, -1, 1, 8 };

					for (int j = 0; j < 4; j++)
					{
						int moveSquare = startSquare;

						while (true)
						{
							if (j == 0 && getBit(ROW0BITBOARD, moveSquare)) { break; }
							if (j == 1 && getBit(COL0BITBOARD, moveSquare)) { break; }
							if (j == 2 && getBit(COL7BITBOARD, moveSquare)) { break; }
							if (j == 3 && getBit(ROW7BITBOARD, moveSquare)) { break; }

							moveSquare += moveAmount[j];

							if (getBit(allyPieceBitBoard, startSquare))
							{
								if (getBit(allyPieces, moveSquare)) { break; }
								else if (getBit(enemyPieces, moveSquare))
								{
									vector<U64> newBoard = board;

									if (currentPiece == 'R')
									{
										popBit(newBoard[WHITEROOK], startSquare);
										setBit(newBoard[WHITEROOK], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][moveSquare];

										for (int j = 6; j < 12; j++)
										{
											if (getBit(newBoard[j], moveSquare))
											{
												popBit(newBoard[j], moveSquare);
												newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

												newEvaluation = baseEvaluation - pieceValues[j];
												if (newEvaluation >= 0)
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
													popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												else
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
													setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												capturedPiece = j % 6;
												break;
											}
										}

										if (startSquare == 63 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEKINGSIDE];
										}

										if (startSquare == 56 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEQUEENSIDE];
										}
									}
									else
									{
										popBit(newBoard[BLACKROOK], startSquare);
										setBit(newBoard[BLACKROOK], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][moveSquare];

										for (int j = 0; j < 6; j++)
										{
											if (getBit(newBoard[j], moveSquare))
											{
												popBit(newBoard[j], moveSquare);
												newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

												newEvaluation = baseEvaluation - pieceValues[j];
												if (newEvaluation >= 0)
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
													popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												else
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
													setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												capturedPiece = j % 6;
												break;
											}
										}

										if (startSquare == 7 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKKINGSIDE];
										}

										if (startSquare == 0 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKQUEENSIDE];
										}
									}

									nextPositions[capturedPiece].push_back(newBoard);
									break;
								}
								else
								{
									vector<U64> newBoard = board;

									if (currentPiece == 'R')
									{
										popBit(newBoard[WHITEROOK], startSquare);
										setBit(newBoard[WHITEROOK], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][moveSquare];

										if (startSquare == 63 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEKINGSIDE];
										}

										if (startSquare == 56 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEQUEENSIDE];
										}
									}
									else
									{
										popBit(newBoard[BLACKROOK], startSquare);
										setBit(newBoard[BLACKROOK], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][moveSquare];

										if (startSquare == 7 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKKINGSIDE];
										}

										if (startSquare == 0 && getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE))
										{
											popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
											newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKQUEENSIDE];
										}
									}

									nextPositions[0].push_back(newBoard);
								}
							}

							if (getBit(enemyPieceBitBoard, startSquare))
							{
								setBit(dangerMap, moveSquare);
							}
						}
					}


				}
			}

			break;
		}

		case 'Q':
		case 'q':
		{
			U64 allyPieceBitBoard = (currentPiece == 'Q' ? board[WHITEQUEEN] : board[BLACKQUEEN]);
			U64 enemyPieceBitBoard = (currentPiece == 'Q' ? board[BLACKQUEEN] : board[WHITEQUEEN]);

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(allyPieceBitBoard, startSquare) || getBit(enemyPieceBitBoard, startSquare))
				{
					int moveAmount[8] = { -9, -7, 7, 9, -8, -1, 1, 8 };

					for (int j = 0; j < 8; j++)
					{
						int moveSquare = startSquare;

						while (true)
						{
							if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
							if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
							if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
							if (j == 4 && getBit(ROW0BITBOARD, moveSquare)) { break; }
							if (j == 5 && getBit(COL0BITBOARD, moveSquare)) { break; }
							if (j == 6 && getBit(COL7BITBOARD, moveSquare)) { break; }
							if (j == 7 && getBit(ROW7BITBOARD, moveSquare)) { break; }

							moveSquare += moveAmount[j];

							if (getBit(allyPieceBitBoard, startSquare))
							{
								if (getBit(allyPieces, moveSquare)) { break; }
								else if (getBit(enemyPieces, moveSquare))
								{
									vector<U64> newBoard = board;

									if (currentPiece == 'Q')
									{
										popBit(newBoard[WHITEQUEEN], startSquare);
										setBit(newBoard[WHITEQUEEN], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEQUEEN][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEQUEEN][moveSquare];

										for (int j = 6; j < 12; j++)
										{
											if (getBit(newBoard[j], moveSquare))
											{
												popBit(newBoard[j], moveSquare);
												newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

												newEvaluation = baseEvaluation - pieceValues[j];
												if (newEvaluation >= 0)
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
													popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												else
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
													setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												capturedPiece = j % 6;
												break;
											}
										}
									}
									else
									{
										popBit(newBoard[BLACKQUEEN], startSquare);
										setBit(newBoard[BLACKQUEEN], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKQUEEN][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKQUEEN][moveSquare];

										for (int j = 0; j < 6; j++)
										{
											if (getBit(newBoard[j], moveSquare))
											{
												popBit(newBoard[j], moveSquare);
												newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

												newEvaluation = baseEvaluation - pieceValues[j];
												if (newEvaluation >= 0)
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
													popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												else
												{
													newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
													setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
												}
												capturedPiece = j % 6;
												break;
											}
										}
									}

									nextPositions[capturedPiece].push_back(newBoard);
									break;
								}
								else
								{
									vector<U64> newBoard = board;

									if (currentPiece == 'Q')
									{
										popBit(newBoard[WHITEQUEEN], startSquare);
										setBit(newBoard[WHITEQUEEN], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEQUEEN][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEQUEEN][moveSquare];
									}
									else
									{
										popBit(newBoard[BLACKQUEEN], startSquare);
										setBit(newBoard[BLACKQUEEN], moveSquare);

										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKQUEEN][startSquare];
										newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKQUEEN][moveSquare];
									}

									nextPositions[0].push_back(newBoard);
								}
							}

							if (getBit(enemyPieceBitBoard, startSquare))
							{
								setBit(dangerMap, moveSquare);
							}
						}
					}


				}
			}

			break;
		}

		case 'K':
		case 'k':
		{
			U64 currentPieceBitBoard;
			if (currentPiece == 'K') { currentPieceBitBoard = board[WHITEKING]; }
			else { currentPieceBitBoard = board[BLACKKING]; }

			for (int startSquare = 0; startSquare < 64; startSquare++)
			{
				if (getBit(currentPieceBitBoard, startSquare))
				{
					// castling
					if (currentPiece == 'K' && startSquare == 60)
					{
						bool kingsideLegal = getBit(board[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
						for (int castleSquare = 61; castleSquare < 63; castleSquare++)
						{
							kingsideLegal = (kingsideLegal && !getBit(allyPieces, castleSquare) && !getBit(enemyPieces, castleSquare));
						}

						if (kingsideLegal)
						{
							vector<U64> newBoard = board;

							popBit(newBoard[WHITEKING], startSquare);
							setBit(newBoard[WHITEKING], 62);
							popBit(newBoard[WHITEROOK], 63);
							setBit(newBoard[WHITEROOK], 61);

							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][startSquare];
							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][62];
							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][63];
							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][61];

							popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
							popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
							setBit(newBoard[BOARDEXTRA], LASTCASTLEWHITEKINGSIDE);

							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEKINGSIDE];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEQUEENSIDE];

							setBit(newBoard[BOARDEXTRA], LASTKINGMOVE);
							nextPositions[0].push_back(newBoard);
						}

						bool queensideLegal = getBit(board[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
						for (int castleSquare = 57; castleSquare < 60; castleSquare++)
						{
							queensideLegal = (queensideLegal && !getBit(allyPieces, castleSquare) && !getBit(enemyPieces, castleSquare));
						}

						if (queensideLegal)
						{
							vector<U64> newBoard = board;

							popBit(newBoard[WHITEKING], startSquare);
							setBit(newBoard[WHITEKING], 58);
							popBit(newBoard[WHITEROOK], 56);
							setBit(newBoard[WHITEROOK], 59);

							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][startSquare];
							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][58];
							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][56];
							newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEROOK][59];

							popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
							popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
							setBit(newBoard[BOARDEXTRA], LASTCASTLEWHITEQUEENSIDE);

							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEKINGSIDE];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEQUEENSIDE];

							setBit(newBoard[BOARDEXTRA], LASTKINGMOVE);
							nextPositions[0].push_back(newBoard);
						}
					}

					if (currentPiece == 'k' && startSquare == 4)
					{
						bool kingsideLegal = getBit(board[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
						for (int castleSquare = 5; castleSquare < 7; castleSquare++)
						{
							kingsideLegal = (kingsideLegal && !getBit(allyPieces, castleSquare) && !getBit(enemyPieces, castleSquare));
						}

						if (kingsideLegal)
						{
							vector<U64> newBoard = board;

							popBit(newBoard[BLACKKING], startSquare);
							setBit(newBoard[BLACKKING], 6);
							popBit(newBoard[BLACKROOK], 7);
							setBit(newBoard[BLACKROOK], 5);

							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][startSquare];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][6];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][7];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][5];

							popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
							popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
							setBit(newBoard[BOARDEXTRA], LASTCASTLEBLACKKINGSIDE);

							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKKINGSIDE];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKQUEENSIDE];

							setBit(newBoard[BOARDEXTRA], LASTKINGMOVE);
							nextPositions[0].push_back(newBoard);
						}

						bool queensideLegal = getBit(board[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
						for (int castleSquare = 1; castleSquare < 4; castleSquare++)
						{
							queensideLegal = (queensideLegal && !getBit(allyPieces, castleSquare) && !getBit(enemyPieces, castleSquare));
						}

						if (queensideLegal)
						{
							vector<U64> newBoard = board;

							popBit(newBoard[BLACKKING], startSquare);
							setBit(newBoard[BLACKKING], 2);
							popBit(newBoard[BLACKROOK], 0);
							setBit(newBoard[BLACKROOK], 3);

							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][startSquare];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][2];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][0];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKROOK][3];

							popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
							popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
							setBit(newBoard[BOARDEXTRA], LASTCASTLEBLACKQUEENSIDE);

							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKKINGSIDE];
							newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKQUEENSIDE];

							setBit(newBoard[BOARDEXTRA], LASTKINGMOVE);
							nextPositions[0].push_back(newBoard);
						}
					}

					// regular movement
					int moveAmount[8] = { -9, -7, 7, 9, -8, -1, 1, 8 };

					for (int j = 0; j < 8; j++)
					{
						int moveSquare = startSquare;

						if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { continue; }
						if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { continue; }
						if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { continue; }
						if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { continue; }
						if (j == 4 && getBit(ROW0BITBOARD, moveSquare)) { continue; }
						if (j == 5 && getBit(COL0BITBOARD, moveSquare)) { continue; }
						if (j == 6 && getBit(COL7BITBOARD, moveSquare)) { continue; }
						if (j == 7 && getBit(ROW7BITBOARD, moveSquare)) { continue; }

						moveSquare += moveAmount[j];

						if (getBit(allyPieces, moveSquare)) { continue; }
						else if (getBit(enemyPieces, moveSquare))
						{
							vector<U64> newBoard = board;

							if (currentPiece == 'K')
							{
								popBit(newBoard[WHITEKING], startSquare);
								setBit(newBoard[WHITEKING], moveSquare);

								newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][startSquare];
								newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][moveSquare];

								for (int j = 6; j < 12; j++)
								{
									if (getBit(newBoard[j], moveSquare))
									{
										popBit(newBoard[j], moveSquare);
										newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

										newEvaluation = baseEvaluation - pieceValues[j];
										if (newEvaluation >= 0)
										{
											newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
											popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
										}
										else
										{
											newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
											setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
										}
										capturedPiece = j % 6;
										break;
									}
								}

								if (startSquare == 60)
								{
									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEKINGSIDE];
									}

									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEQUEENSIDE];
									}
								}
							}
							else
							{
								popBit(newBoard[BLACKKING], startSquare);
								setBit(newBoard[BLACKKING], moveSquare);

								newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][startSquare];
								newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][moveSquare];

								for (int j = 0; j < 6; j++)
								{
									if (getBit(newBoard[j], moveSquare))
									{
										popBit(newBoard[j], moveSquare);
										newBoard[BOARDHASHVALUE] ^= zobristTable[j][moveSquare];

										newEvaluation = baseEvaluation - pieceValues[j];
										if (newEvaluation >= 0)
										{
											newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation);
											popBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
										}
										else
										{
											newBoard[BOARDEXTRA] = setBitsFromInt(newBoard[BOARDEXTRA], EVALUATIONLSB, EVALUATIONLSB + 10, newEvaluation * -1);
											setBit(newBoard[BOARDEXTRA], NEGATIVEEVALUATION);
										}
										capturedPiece = j % 6;
										break;
									}
								}

								if (startSquare == 4)
								{
									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKKINGSIDE];
									}

									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKQUEENSIDE];
									}
								}
							}

							setBit(newBoard[BOARDEXTRA], LASTKINGMOVE);

							nextPositions[capturedPiece].push_back(newBoard);
						}
						else
						{
							vector<U64> newBoard = board;

							if (currentPiece == 'K')
							{
								popBit(newBoard[WHITEKING], startSquare);
								setBit(newBoard[WHITEKING], moveSquare);

								newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][startSquare];
								newBoard[BOARDHASHVALUE] ^= zobristTable[WHITEKING][moveSquare];

								if (startSquare == 60)
								{
									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEKINGSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEKINGSIDE];
									}

									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEWHITEQUEENSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEWHITEQUEENSIDE];
									}
								}
							}
							else
							{
								popBit(newBoard[BLACKKING], startSquare);
								setBit(newBoard[BLACKKING], moveSquare);

								newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][startSquare];
								newBoard[BOARDHASHVALUE] ^= zobristTable[BLACKKING][moveSquare];

								if (startSquare == 4)
								{
									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKKINGSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKKINGSIDE];
									}

									if (getBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE))
									{
										popBit(newBoard[BOARDEXTRA], LEGALCASTLEBLACKQUEENSIDE);
										newBoard[BOARDHASHVALUE] ^= zobristTable[BOARDEXTRA][LEGALCASTLEBLACKQUEENSIDE];
									}
								}
							}

							setBit(newBoard[BOARDEXTRA], LASTKINGMOVE);
							nextPositions[0].push_back(newBoard);
						}
					}


				}
			}

			break;
		}

		default:
			break;

		}

	}

	// check the legality of all the possible moves found
	vector<vector<U64>> legalPositions;

	for (int capturedPiece = 5; capturedPiece >= 0; capturedPiece--)
	{
		for (vector<U64> nextPosition : nextPositions[capturedPiece])
		{
			int kingSquare = -1;
			for (int i = 0; i < 64; i++)
			{
				if ((whiteToMove && getBit(nextPosition[WHITEKING], i)) || (!whiteToMove && getBit(nextPosition[BLACKKING], i)))
				{
					kingSquare = i;
				}
			}

			bool kingInDanger = (getBit(dangerMap, kingSquare) ? true : false);

			// find ally/enemy piece positions (if whiteToMove, then ally == black)
			allyPieces = 0ULL;
			enemyPieces = 0ULL;
			if (!whiteToMove)
			{
				for (int j = 0; j < 6; j++)
				{
					U64 currentBoard = nextPosition[j];
					for (int square = 0; square < 64; square++)
					{
						if (getBit(currentBoard, square))
						{
							setBit(allyPieces, square);
						}
					}
				}

				for (int j = 6; j < 12; j++)
				{
					U64 currentBoard = nextPosition[j];
					for (int square = 0; square < 64; square++)
					{
						if (getBit(currentBoard, square))
						{
							setBit(enemyPieces, square);
						}
					}
				}
			}
			else
			{
				for (int j = 6; j < 12; j++)
				{
					U64 currentBoard = nextPosition[j];
					for (int square = 0; square < 64; square++)
					{
						if (getBit(currentBoard, square))
						{
							setBit(allyPieces, square);
						}
					}
				}

				for (int j = 0; j < 6; j++)
				{
					U64 currentBoard = nextPosition[j];
					for (int square = 0; square < 64; square++)
					{
						if (getBit(currentBoard, square))
						{
							setBit(enemyPieces, square);
						}
					}
				}
			}

			// find pieceStart and pieceEnd
			if (whiteToMove)
			{
				if (!getBit(nextPosition[BOARDEXTRA], LASTKINGMOVE))
				{
					if (!kingInDanger)
					{
						pieceStart = BLACKBISHOP;
						pieceEnd = BLACKQUEEN + 1;
					}
					else
					{
						pieceStart = BLACKPAWN;
						pieceEnd = BLACKQUEEN + 1;
					}
				}
				else
				{
					pieceStart = BLACKPAWN;
					pieceEnd = BLACKKING + 1;
				}
			}
			else
			{
				if (!getBit(nextPosition[BOARDEXTRA], LASTKINGMOVE))
				{
					if (!kingInDanger)
					{
						pieceStart = WHITEBISHOP;
						pieceEnd = WHITEQUEEN + 1;
					}
					else
					{
						pieceStart = WHITEPAWN;
						pieceEnd = WHITEQUEEN + 1;
					}
				}
				else
				{
					pieceStart = WHITEPAWN;
					pieceEnd = WHITEKING + 1;
				}
			}

			bool illegalPosition = false;

			for (int i = pieceStart; i < pieceEnd; i++)
			{
				char currentPiece = helper::pieces[i];
				switch (currentPiece)
				{
				case 'P':
				case 'p':
				{
					U64 currentPieceBitBoard = (currentPiece == 'P' ? nextPosition[WHITEPAWN] : nextPosition[BLACKPAWN]);

					for (int startSquare = 0; startSquare < 64; startSquare++)
					{
						if (getBit(currentPieceBitBoard, startSquare))
						{
							int toMove[8] = { startSquare - 17, startSquare - 15, startSquare - 10, startSquare - 6, startSquare + 6, startSquare + 10, startSquare + 15, startSquare + 17 };

							if (currentPiece == 'P')
							{
								if (startSquare - 7 == kingSquare || startSquare - 9 == kingSquare)
								{
									illegalPosition = true;
								}
							}
							else
							{
								if (startSquare + 7 == kingSquare || startSquare + 9 == kingSquare)
								{
									illegalPosition = true;
								}
							}
						}

						if (illegalPosition) { break; }
					}

					break;
				}

				case 'N':
				case 'n':
				{
					U64 currentPieceBitBoard = (currentPiece == 'N' ? nextPosition[WHITEKNIGHT] : nextPosition[BLACKKNIGHT]);

					for (int startSquare = 0; startSquare < 64; startSquare++)
					{
						if (getBit(currentPieceBitBoard, startSquare))
						{
							int toMove[8] = { startSquare - 17, startSquare - 15, startSquare - 10, startSquare - 6, startSquare + 6, startSquare + 10, startSquare + 15, startSquare + 17 };

							int knightPos = 0;

							if (getBit(COL0BITBOARD, startSquare)) { knightPos += (1 << 0); }
							if (getBit(COL1BITBOARD, startSquare)) { knightPos += (1 << 1); }
							if (getBit(COL6BITBOARD, startSquare)) { knightPos += (1 << 2); }
							if (getBit(COL7BITBOARD, startSquare)) { knightPos += (1 << 3); }
							if (getBit(ROW0BITBOARD, startSquare)) { knightPos += (1 << 4); }
							if (getBit(ROW1BITBOARD, startSquare)) { knightPos += (1 << 5); }
							if (getBit(ROW6BITBOARD, startSquare)) { knightPos += (1 << 6); }
							if (getBit(ROW7BITBOARD, startSquare)) { knightPos += (1 << 7); }

							if (getBit(knightPos, 0) || getBit(knightPos, 4) || getBit(knightPos, 5)) { toMove[0] = -1; }
							if (getBit(knightPos, 3) || getBit(knightPos, 4) || getBit(knightPos, 5)) { toMove[1] = -1; }
							if (getBit(knightPos, 0) || getBit(knightPos, 1) || getBit(knightPos, 4)) { toMove[2] = -1; }
							if (getBit(knightPos, 2) || getBit(knightPos, 3) || getBit(knightPos, 4)) { toMove[3] = -1; }
							if (getBit(knightPos, 0) || getBit(knightPos, 1) || getBit(knightPos, 7)) { toMove[4] = -1; }
							if (getBit(knightPos, 2) || getBit(knightPos, 3) || getBit(knightPos, 7)) { toMove[5] = -1; }
							if (getBit(knightPos, 0) || getBit(knightPos, 6) || getBit(knightPos, 7)) { toMove[6] = -1; }
							if (getBit(knightPos, 3) || getBit(knightPos, 6) || getBit(knightPos, 7)) { toMove[7] = -1; }


							for (int moveSquare : toMove)
							{
								if (moveSquare == -1) { continue; }

								if (moveSquare == kingSquare)
								{
									illegalPosition = true;
									break;
								}
							}
						}

						if (illegalPosition) { break; }
					}

					break;
				}

				case 'B':
				case 'b':
				{
					U64 currentPieceBitBoard = (currentPiece == 'B' ? nextPosition[WHITEBISHOP] : nextPosition[BLACKBISHOP]);

					for (int startSquare = 0; startSquare < 64; startSquare++)
					{
						if (getBit(currentPieceBitBoard, startSquare))
						{
							int moveAmount[4] = { -9, -7, 7, 9 };

							for (int j = 0; j < 4; j++)
							{
								int moveSquare = startSquare;

								while (true)
								{
									if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
									if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
									if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
									if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }

									moveSquare += moveAmount[j];

									if (moveSquare == kingSquare)
									{
										illegalPosition = true;
										break;
									}

									if (getBit(allyPieces, moveSquare) || getBit(enemyPieces, moveSquare)) { break; }
								}

								if (illegalPosition) { break; }
							}
						}

						if (illegalPosition) { break; }
					}

					break;
				}

				case 'R':
				case 'r':
				{
					U64 currentPieceBitBoard = (currentPiece == 'R' ? nextPosition[WHITEROOK] : nextPosition[BLACKROOK]);

					for (int startSquare = 0; startSquare < 64; startSquare++)
					{
						if (getBit(currentPieceBitBoard, startSquare))
						{
							int moveAmount[4] = { -8, -1, 1, 8 };

							for (int j = 0; j < 4; j++)
							{
								int moveSquare = startSquare;

								while (true)
								{
									if (j == 0 && getBit(ROW0BITBOARD, moveSquare)) { break; }
									if (j == 1 && getBit(COL0BITBOARD, moveSquare)) { break; }
									if (j == 2 && getBit(COL7BITBOARD, moveSquare)) { break; }
									if (j == 3 && getBit(ROW7BITBOARD, moveSquare)) { break; }

									moveSquare += moveAmount[j];
									if (moveSquare == kingSquare)
									{
										illegalPosition = true;
										break;
									}

									if (getBit(allyPieces, moveSquare) || getBit(enemyPieces, moveSquare)) { break; }
								}

								if (illegalPosition) { break; }
							}
						}

						if (illegalPosition) { break; }
					}

					break;
				}

				case 'Q':
				case 'q':
				{
					U64 currentPieceBitBoard = (currentPiece == 'Q' ? nextPosition[WHITEQUEEN] : nextPosition[BLACKQUEEN]);

					for (int startSquare = 0; startSquare < 64; startSquare++)
					{
						if (getBit(currentPieceBitBoard, startSquare))
						{
							int moveAmount[8] = { -9, -7, 7, 9, -8, -1, 1, 8 };

							for (int j = 0; j < 8; j++)
							{
								int moveSquare = startSquare;

								while (true)
								{
									if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
									if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { break; }
									if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
									if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { break; }
									if (j == 4 && getBit(ROW0BITBOARD, moveSquare)) { break; }
									if (j == 5 && getBit(COL0BITBOARD, moveSquare)) { break; }
									if (j == 6 && getBit(COL7BITBOARD, moveSquare)) { break; }
									if (j == 7 && getBit(ROW7BITBOARD, moveSquare)) { break; }

									moveSquare += moveAmount[j];
									if (moveSquare == kingSquare)
									{
										illegalPosition = true;
										break;
									}

									if (getBit(allyPieces, moveSquare) || getBit(enemyPieces, moveSquare)) { break; }
								}

								if (illegalPosition) { break; }
							}
						}

						if (illegalPosition) { break; }
					}

					break;
				}

				case 'K':
				case 'k':
				{
					U64 currentPieceBitBoard = (currentPiece == 'K' ? nextPosition[WHITEKING] : nextPosition[BLACKKING]);

					for (int startSquare = 0; startSquare < 64; startSquare++)
					{
						if (getBit(currentPieceBitBoard, startSquare))
						{
							int moveAmount[8] = { -9, -7, 7, 9, -8, -1, 1, 8 };

							for (int j = 0; j < 8; j++)
							{
								int moveSquare = startSquare;

								if (j == 0 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { continue; }
								if (j == 1 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW0BITBOARD, moveSquare))) { continue; }
								if (j == 2 && (getBit(COL0BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { continue; }
								if (j == 3 && (getBit(COL7BITBOARD, moveSquare) || getBit(ROW7BITBOARD, moveSquare))) { continue; }
								if (j == 4 && getBit(ROW0BITBOARD, moveSquare)) { continue; }
								if (j == 5 && getBit(COL0BITBOARD, moveSquare)) { continue; }
								if (j == 6 && getBit(COL7BITBOARD, moveSquare)) { continue; }
								if (j == 7 && getBit(ROW7BITBOARD, moveSquare)) { continue; }

								moveSquare += moveAmount[j];

								if (moveSquare == kingSquare)
								{
									illegalPosition = true;
									break;
								}

								if (getBit(allyPieces, moveSquare) || getBit(enemyPieces, moveSquare)) { break; }
							}
						}

						if (illegalPosition) { break; }
					}

					break;
				}

				default:
					break;

				}

				if (illegalPosition) { break; }
			}

			if (!illegalPosition)
			{
				legalPositions.push_back(nextPosition);
			}
		}

	}

	return legalPositions;
}
