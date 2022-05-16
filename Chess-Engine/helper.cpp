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

map<string, vector<string>> helper::getLegalMoves(string fen, bool whiteToMove)
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

	// load the board
	char board[8][8];
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			board[i][j] = processedParts[i][j];
		}
	}

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

					// move one square
					if (board[i - 1][j] == ' ') { pieceMoves.push_back(to_string(i - 1) + to_string(j)); }

					// move two squares
					if (i == 6 && board[i - 1][j] == ' ' && board[i - 2][j] == ' ') { pieceMoves.push_back(to_string(i - 2) + to_string(j)); }

					// capture left
					if (j > 0 && board[i - 1][j - 1] >= 97) { pieceMoves.push_back(to_string(i - 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && board[i - 1][j + 1] >= 97) { pieceMoves.push_back(to_string(i - 1) + to_string(j + 1)); }

					break;
				}

				case 'N':
				{
					// add 8 candidate squares
					vector<vector<int>> to;
					to.push_back({ i - 2, j - 1 }); to.push_back({ i - 2, j + 1 });
					to.push_back({ i + 2, j - 1 }); to.push_back({ i + 2, j + 1 });
					to.push_back({ i - 1, j - 2 }); to.push_back({ i + 1, j - 2 });
					to.push_back({ i - 1, j + 2 }); to.push_back({ i + 1, j + 2 });

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

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						x--;
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

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 97) { break; }
						x--;
					}

					break;
				}

				case 'K':
				{
					// top left
					int y = i - 1;
					int x = j - 1;
					if (y >= 0 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
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

					// bottom left
					y = i + 1;
					x = j - 1;
					if (y <= 7 && x >= 0 && (board[y][x] < 65 || board[y][x] > 90))
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

					// top
					y = i - 1;
					x = j;
					if (y >= 0 && (board[y][x] < 65 || board[y][x] > 90))
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

					// bottom
					y = i + 1;
					x = j;
					if (y <= 7 && (board[y][x] < 65 || board[y][x] > 90))
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

					// move one square
					if (board[i + 1][j] == ' ') { pieceMoves.push_back(to_string(i + 1) + to_string(j)); }

					// move two squares
					if (i == 1 && board[i + 1][j] == ' ' && board[i + 2][j] == ' ') { pieceMoves.push_back(to_string(i + 2) + to_string(j)); }

					// capture left
					if (j > 0 && board[i + 1][j - 1] >= 65 && board[i + 1][j - 1] <= 90) { pieceMoves.push_back(to_string(i + 1) + to_string(j - 1)); }

					// capture right
					if (j < 7 && board[i + 1][j + 1] >= 65 && board[i + 1][j + 1] <= 90) { pieceMoves.push_back(to_string(i + 1) + to_string(j + 1)); }

					break;
				}

				case 'n':
				{
					// add 8 candidate squares
					vector<vector<int>> to;
					to.push_back({ i - 2, j - 1 }); to.push_back({ i - 2, j + 1 });
					to.push_back({ i + 2, j - 1 }); to.push_back({ i + 2, j + 1 });
					to.push_back({ i - 1, j - 2 }); to.push_back({ i + 1, j - 2 });
					to.push_back({ i - 1, j + 2 }); to.push_back({ i + 1, j + 2 });

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
					// top left
					int y = i - 1;
					int x = j - 1;
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

					// bottom left
					y = i + 1;
					x = j - 1;
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

					break;
				}

				case 'r':
				{
					// top
					int y = i - 1;
					int x = j;
					while (y >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						// if an enemy piece is hit, then stop
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						y--;
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

					// bottom
					y = i + 1;
					x = j;
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

					break;
				}

				case 'q':
				{
					// top left
					int y = i - 1;
					int x = j - 1;
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

					// bottom left
					y = i + 1;
					x = j - 1;
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

					// right
					y = i;
					x = j + 1;
					while (x <= 7 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						x++;
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

					// left
					y = i;
					x = j - 1;
					while (x >= 0 && board[y][x] < 97)
					{
						pieceMoves.push_back(to_string(y) + to_string(x));
						if (board[y][x] >= 65 && board[y][x] <= 90) { break; }
						x--;
					}

					break;
				}

				case 'k':
				{
					// top left
					int y = i - 1;
					int x = j - 1;
					if (y >= 0 && x >= 0 && board[y][x] < 97)
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

					// bottom left
					y = i + 1;
					x = j - 1;
					if (y <= 7 && x >= 0 && board[y][x] < 97)
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

					// top
					y = i - 1;
					x = j;
					if (y >= 0 && board[y][x] < 97)
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

					// bottom
					y = i + 1;
					x = j;
					if (y <= 7 && board[y][x] < 97)
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

	return legalMoves;
}
