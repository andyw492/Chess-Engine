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

map<string, vector<string>> helper::getLegalMoves(char board[8][8], bool whiteToMove, bool castling[4], bool playerCheckDetection)
{
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
					// promotion; implement later
					if (i == 0) { break; }

					// capture left
					if (j > 0 && board[i - 1][j - 1] >= 97) { pieceMoves.push_back(to_string(i - 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && board[i - 1][j + 1] >= 97) { pieceMoves.push_back(to_string(i - 1) + to_string(j + 1)); }

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
					// promotion; implement later
					if (i == 7) { break; }

					// capture left
					if (j > 0 && board[i + 1][j - 1] >= 65 && board[i + 1][j - 1] <= 90) { pieceMoves.push_back(to_string(i + 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && board[i + 1][j + 1] >= 65 && board[i + 1][j + 1] <= 90) { pieceMoves.push_back(to_string(i + 1) + to_string(j + 1)); }

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

	map<string, vector<string>> illegalMoves;
	for (auto move : legalMoves)
	{
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
				if (j < 7 && tempBoard[i - 1][j + 1] >= 'p') { pieceIllegal.push_back(to); }

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

		}

		if (pieceIllegal.size() > 0)
		{
			illegalMoves[move.first] = pieceIllegal;
		}
	}

	for (auto move : illegalMoves)
	{
		vector<string> newLegal;
		vector<string> oldLegal = legalMoves[move.first];
		for (int i = 0; i < oldLegal.size(); i++)
		{
			if (find(move.second.begin(), move.second.end(), oldLegal[i]) == move.second.end())
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

float helper::getPieceValue(char piece)
{
	switch (piece)
	{
	case 'K':
	case 'k':
		return 100;

	case 'Q':
	case 'q':
		return 9;

	case 'R':
	case 'r':
		return 5;

	case 'B':
	case 'b':
		return 3.2;

	case 'N':
	case 'n':
		return 3;

	case 'P':
	case 'p':
		return 1;
	}
}