#include "window.h"

Window::Window(bool windowPrint)
{
	dpr = windowPrint;

	// initialize board square and piece positions
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			squarePos[y][x] = sf::Vector2f(x * 70 + 30.f, y * 70 + 30.f);
			piecePos[y][x] = sf::Vector2f(x * 70 + 34.5f, y * 70 + 36.5f);
		}
	}
}

Window::~Window()
{
	for (int i = 0; i < textures.size(); i++)
	{
		delete textures[i];
	}
	
	for (auto i : objects)
	{
		delete i.second;
	}
}

void Window::dprint(LPVOID pParam, string s)
{
	Parameters *p = ((Parameters*)pParam);
	WaitForSingleObject(p->mutex, INFINITE);
	cout << s << endl;
	ReleaseMutex(p->mutex);
}

void Window::getMouseSquare(sf::Vector2i mousePos, int squareCoords[2])
{
	squareCoords[0] = floor((mousePos.y - 30) / 70);
	squareCoords[1] = floor((mousePos.x - 30) / 70);
}

vector<string> Window::setPiecePositions(char board[8][8])
{
	vector<string> available = { "b1", "b2", "B1", "B2", "k", "K", "n1", "n2", "N1", "N2", "r1", "r2", "R1", "R2" };
	for (int i = 1; i < 9; i++)
	{
		available.push_back("p" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		available.push_back("P" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		available.push_back("q" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		available.push_back("Q" + to_string(i));
	}
	vector<string> pieceNames;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (board[y][x] != ' ')
			{
				char pieceChar = board[y][x];
				string pieceName = "";
				for (int i = 0; i < available.size(); i++)
				{
					if (available[i][0] == pieceChar)
					{
						pieceName = available[i];
						break;
					}
				}

				assert(pieceName != "");

				available.erase(std::remove(available.begin(), available.end(), pieceName), available.end());
				((sf::Sprite*)objects[pieceName])->setPosition(piecePos[y][x].x, piecePos[y][x].y);
				pieceNames.push_back(pieceName);
			}
		}
	}

	return pieceNames;
}

UINT Window::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("windowThread %d started\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	display(pParam);

	// tell engine that window is closed
	WaitForSingleObject(p->mutex, INFINITE);
	p->windowClosed = true;
	ReleaseMutex(p->mutex);

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("windowThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;
}

void Window::initializeObjects(LPVOID pParam, char board[8][8])
{
	Parameters *p = ((Parameters*)pParam);

	vector<string> objectNames{ "board_white", "b1", "b2", "B1", "B2", "k", "K", "n1", "n2", "N1", "N2", "r1", "r2", "R1", "R2" };
	for (int i = 1; i < 9; i++)
	{
		objectNames.push_back("p" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		objectNames.push_back("P" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		objectNames.push_back("q" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		objectNames.push_back("Q" + to_string(i));
	}

	//for (int i = 0; i < objectNames.size(); i++)
	//{
	//	dprint(pParam, objectNames[i]);
	//}

	// initialize sprites, textures
	for(int i = 0; i < objectNames.size(); i++)
	{
		string pieceFileName = "";
		if (objectNames[i] == "board_white")
		{
			pieceFileName = "board_white";
		}
		else
		{
			// convert object name to file name
			// need to convert names b/c cant name one file 'b' and another 'B'
			switch (objectNames[i][0])
			{
			case 'r':
				pieceFileName = "r_black"; break;
			case 'n':
				pieceFileName = "n_black"; break;
			case 'b':
				pieceFileName = "b_black"; break;
			case 'q':
				pieceFileName = "q_black"; break;
			case 'k':
				pieceFileName = "k_black"; break;
			case 'p':
				pieceFileName = "p_black"; break;

			case 'R':
				pieceFileName = "r_white"; break;
			case 'N':
				pieceFileName = "n_white"; break;
			case 'B':
				pieceFileName = "b_white"; break;
			case 'Q':
				pieceFileName = "q_white"; break;
			case 'K':
				pieceFileName = "k_white"; break;
			case 'P':
				pieceFileName = "p_white"; break;

			default:
				dprint(pParam, "error invalid piece");
			}
		}

		textures.push_back(new sf::Texture());
		string fileName = "img/" + pieceFileName + ".png";
		textures[i]->loadFromFile(fileName);
		textures[i]->setSmooth(true);

		sprites.push_back(new sf::Sprite());
		sprites[i]->setTexture(*textures[i]);
		objects[objectNames[i]] = sprites[i];

		if (pieceFileName == "board_white")
		{
			int x = textures[i]->getSize().x;
			int y = textures[i]->getSize().y;
		}
	}

	((sf::Sprite*)(objects)["board_white"])->setScale(sf::Vector2f(0.76087f, 0.76087f));
	((sf::Sprite*)(objects)["board_white"])->setPosition(sf::Vector2f(30.f, 30.f));

	// set the pieces to their squares in the initial fen
	map<string, int> pieceIndexes = { {"r",1}, {"n",1}, {"b",1}, {"p",1}, {"q",1}, {"R",1}, {"N",1}, {"B",1}, {"P",1}, {"Q",1} };
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			string objectName = "";
			switch (board[y][x])
			{
			case 'r':
				objectName = "r" + to_string(pieceIndexes["r"]); pieceIndexes["r"]++; break;
			case 'n':
				objectName = "n" + to_string(pieceIndexes["n"]); pieceIndexes["n"]++; break;
			case 'b':
				objectName = "b" + to_string(pieceIndexes["b"]); pieceIndexes["b"]++; break;
			case 'q':
				objectName = "q" + to_string(pieceIndexes["q"]); pieceIndexes["q"]++; break;
			case 'k':
				objectName = "k"; break;
			case 'p':
				objectName = "p" + to_string(pieceIndexes["p"]); pieceIndexes["p"]++; break;

			case 'R':
				objectName = "R" + to_string(pieceIndexes["R"]); pieceIndexes["R"]++; break;
			case 'N':
				objectName = "N" + to_string(pieceIndexes["N"]); pieceIndexes["N"]++; break;
			case 'B':
				objectName = "B" + to_string(pieceIndexes["B"]); pieceIndexes["B"]++; break;
			case 'Q':
				objectName = "Q" + to_string(pieceIndexes["Q"]); pieceIndexes["Q"]++; break;
			case 'K':
				objectName = "K"; break;
			case 'P':
				objectName = "P" + to_string(pieceIndexes["P"]); pieceIndexes["P"]++; break;
			
			case ' ':
				break;

			// number
			default:
				//dprint(pParam, "char" + to_string(char(parts[i][j])));
				//dprint(pParam, "number" + to_string(char(parts[i][j] - '0')));
				//j += int(partsString[j] - '0');
				//dprint(pParam, "j moved to " + to_string(j));
				dprint(pParam, "error bad char " + to_string(board[y][x]));
				break;
			}

			if (objectName == "")
			{
				continue;
			}

			((sf::Sprite*)(objects)[objectName])->setPosition(piecePos[y][x]);
		}
		
	}

	//int xIndex = 0;
	//int yIndex = 0;
	//for (auto i : *objects)
	//{
	//	if (i.first == "board_white")
	//	{
	//		((sf::Sprite*)i.second)->setScale(sf::Vector2f(0.8f, 0.8f));
	//		((sf::Sprite*)i.second)->setPosition(sf::Vector2f(30.f, 30.f));
	//	}
	//	else
	//	{
	//		((sf::Sprite*)i.second)->setPosition(sf::Vector2f(xIndex * 73.4 + 37.f, yIndex * 73.4 + 37.f));

	//		xIndex++;
	//		if (xIndex == 8)
	//		{
	//			yIndex++;
	//			xIndex = 0;
	//		}
	//	}

	//}

	// polymorphism yay
	//sf::CircleShape* shape = new sf::CircleShape(10.f);
	//shape->setFillColor(sf::Color::Blue);
	//shape->setPosition(sf::Vector2f(500.f, 500.f));
	//(*objects)["circle"] = shape;

	sf::RectangleShape* rectangle = new sf::RectangleShape();
	rectangle->setSize(sf::Vector2f(70, 70));
	rectangle->setOutlineColor(sf::Color::Blue);
	rectangle->setFillColor(sf::Color::Transparent);
	rectangle->setOutlineThickness(-3);
	rectangle->setPosition(0, 0);
	objects["selectionRectangle"] = rectangle;
}

void Window::display(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// construct the board from the initial fen
	string initialFen = "";
	WaitForSingleObject(p->mutex, INFINITE);
	initialFen = p->initialFen;
	ReleaseMutex(p->mutex);

	sf::Font font;
	font.loadFromFile("ModernSans-Light.otf");

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::Black);
	text.setPosition(sf::Vector2f(650.f, 50.f));

	char board[8][8];
	helper::fenToMatrix(initialFen, board);

	WaitForSingleObject(p->mutex, INFINITE);
	memcpy(p->board, board, 64 * sizeof(char));
	ReleaseMutex(p->mutex);

	initializeObjects(pParam, board);

	sf::RenderWindow window(sf::VideoMode(1000, 660), "Chess Engine");

	// inside the main loop, between window.clear() and window.display()
	window.draw(text);

	// draw initial objects
	window.clear(sf::Color::White);
	window.draw(*objects["board_white"]);

	for (auto i : objects)
	{
		if (i.first == "board_white" || i.first == "selectionRectangle") { continue; }
		window.draw(*i.second);
	}

	window.draw(text);

	window.display();

	int selection[2] = { -1, -1 };
	vector<string> piecesToDraw = setPiecePositions(board);

	// rectangles that indicate legal moves
	// stored outside objects map because a vector is more appropriate
	vector<sf::RectangleShape> legalRectangles;
	for (int i = 0; i < 64; i++)
	{
		sf::RectangleShape rectangle;
		rectangle.setSize(sf::Vector2f(70, 70));
		rectangle.setOutlineColor(sf::Color(150, 191, 255, 170));
		rectangle.setFillColor(sf::Color::Transparent);
		rectangle.setOutlineThickness(-3);
		rectangle.setPosition(0, 0);
		legalRectangles.push_back(rectangle);
	}

	int printedLegal = false;

	//--------------------MAIN WINDOW LOOP--------------------------

	while (window.isOpen())
	{
		// draw the current board's objects to window
#pragma region
		WaitForSingleObject(p->mutex, INFINITE);
		memcpy(board, p->board, 64 * sizeof(char));
		ReleaseMutex(p->mutex);

		piecesToDraw = setPiecePositions(board);

		window.clear(sf::Color::White);

		window.draw(*objects["board_white"]);
		if (((sf::RectangleShape*)objects["selectionRectangle"])->getPosition().x > 0)
		{
			window.draw(*objects["selectionRectangle"]);
		}

		for (int i = 0; i < legalRectangles.size(); i++)
		{
			if (legalRectangles[i].getPosition().x > 0)
			{
				window.draw(legalRectangles[i]);
			}
		}

		for (int i = 0; i < piecesToDraw.size(); i++)
		{
			window.draw(*objects[piecesToDraw[i]]);
		}

		window.display();

#pragma endregion

		// check if the game ended
#pragma region
		bool whiteKingAlive = false;
		bool blackKingAlive = false;
		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				if (board[y][x] == 'K') { whiteKingAlive = true; }
				if (board[y][x] == 'k') { blackKingAlive = true; }
			}
		}
		assert(whiteKingAlive || blackKingAlive);

		if (!blackKingAlive)
		{
			text.setString("you won!!");

			// get the positions of the pieces to draw from the current board
			piecesToDraw = setPiecePositions(board);

			window.clear(sf::Color::White);
			window.draw(*objects["board_white"]);
			if (((sf::RectangleShape*)objects["selectionRectangle"])->getPosition().x > 0)
			{
				window.draw(*objects["selectionRectangle"]);
			}

			for (int i = 0; i < piecesToDraw.size(); i++)
			{
				window.draw(*objects[piecesToDraw[i]]);
			}

			window.display();

			window.draw(text);
			window.display();
			Sleep(2000);
			window.close();
		}
		if (!whiteKingAlive)
		{
			text.setString("you lost!!");

			// get the positions of the pieces to draw from the current board
			piecesToDraw = setPiecePositions(board);

			window.clear(sf::Color::White);
			window.draw(*objects["board_white"]);
			if (((sf::RectangleShape*)objects["selectionRectangle"])->getPosition().x > 0)
			{
				window.draw(*objects["selectionRectangle"]);
			}

			for (int i = 0; i < piecesToDraw.size(); i++)
			{
				window.draw(*objects[piecesToDraw[i]]);
			}

			window.display();

			window.draw(text);
			window.display();
			Sleep(2000);
			window.close();
		}
#pragma endregion

		//-------------------POLL EVENTS----------------------------
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (dpr)
			{
				bool playerToMove = false;
				WaitForSingleObject(p->mutex, INFINITE);
				playerToMove = p->playerToMove;
				ReleaseMutex(p->mutex);
				if (playerToMove && !printedLegal)
				{
					printedLegal = true;
					bool castling[4];
					string enpassant = "";
					WaitForSingleObject(p->mutex, INFINITE);
					memcpy(castling, p->castling, 4 * sizeof(bool));
					enpassant = p->enpassant;
					ReleaseMutex(p->mutex);

					auto begin = std::chrono::high_resolution_clock::now();
					map<string, vector<string>> legalMoves = helper::getLegalMoves(board, true, castling, enpassant, true);
					auto end = std::chrono::high_resolution_clock::now();

					cout << "----------------- WINDOW -----------------\n" << endl;

					for (int i = 0; i < 8; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							cout << board[i][j] << " ";
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

					printf("legal moves found in %.3f ms\n", float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
				}
			}


			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);

				int squareY = floor((mousePos.y - 30) / 70);
				int squareX = floor((mousePos.x - 30) / 70);

				bool playerToMove = false;
				WaitForSingleObject(p->mutex, INFINITE);
				playerToMove = p->playerToMove;
				ReleaseMutex(p->mutex);

				bool clickedInBoard = max(mousePos.x, mousePos.y) < 590 && min(mousePos.x, mousePos.y) >= 30;

				// used to decide if it is engines turn after mouse click
				bool madeMove = false;

				// only process a click in the board if players turn
				if (clickedInBoard && playerToMove)
				{
					// get the current board from parameters
					WaitForSingleObject(p->mutex, INFINITE);
					memcpy(board, p->board, 64 * sizeof(char));
					ReleaseMutex(p->mutex);

					int targetSquare[2];
					getMouseSquare(mousePos, targetSquare);

					//if (dpr)
					//{
					//	cout << "target square " << targetSquare[0] << " " << targetSquare[1] << endl;
					//	cout << "selection " << selection << endl;
					//}

					// select the players piece if selection string is empty
					if(selection[0] == -1 && board[targetSquare[0]][targetSquare[1]] >= 65 && board[targetSquare[0]][targetSquare[1]] <= 90)
					{
						((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(squarePos[squareY][squareX]);
						selection[0] = squareY;
						selection[1] = squareX;

						// set legal rectangle positions
						string from = to_string(selection[0]) + to_string(selection[1]);
						bool castling[4];
						string enpassant = "";
						WaitForSingleObject(p->mutex, INFINITE);
						memcpy(castling, p->castling, 4 * sizeof(bool));
						enpassant = p->enpassant;
						ReleaseMutex(p->mutex);
						vector<string> legalMoves = helper::getLegalMoves(board, true, castling, enpassant, true)[from];

						for(int i = 0; i < legalMoves.size(); i++)
						{
							int legalY = legalMoves[i][0] - 48;
							int legalX = legalMoves[i][1] - 48;
							legalRectangles[i].setPosition(squarePos[legalY][legalX]);
						}
					}

					// attempt to move the players piece if selection string is full
					else
					{
						bool validMove = true;
						bool sameSquareSelected = false;

						// if the target square is the same as the selection square, then deselect
						if (selection[0] == targetSquare[0] && selection[1] == targetSquare[1])
						{
							sameSquareSelected = true;
							validMove = false;
							((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(0, 0);
							for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }
							selection[0] = -1;
						}

						// check if the target square is a legal move for the selected piece
						string from = to_string(selection[0]) + to_string(selection[1]);
						string to = to_string(targetSquare[0]) + to_string(targetSquare[1]);
						
						// deselect if not legal
						bool castling[4];
						string enpassant = "";
						WaitForSingleObject(p->mutex, INFINITE);
						memcpy(castling, p->castling, 4 * sizeof(bool));
						enpassant = p->enpassant;
						ReleaseMutex(p->mutex);
						vector<string> legalMoves = helper::getLegalMoves(board, true, castling, enpassant, true)[from];
						if (find(legalMoves.begin(), legalMoves.end(), to) == legalMoves.end())
						{
							validMove = false;
							((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(0, 0);
							for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }
							selection[0] = -1;
						}

						if (validMove)
						{
							// check for castling
							// 0 == normal move
							// 1 == white castles kingside
							// 2 == white castles queenside
							// 3 == black castles kingside
							// 4 == black castles queenside

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
								char targetPrev = board[targetSquare[0]][targetSquare[1]];
								char movedPiece = board[selection[0]][selection[1]];
								board[selection[0]][selection[1]] = ' ';
								board[targetSquare[0]][targetSquare[1]] = movedPiece;

								// if a pawn is on a promoting square, then replace it with a queen
								for (int x = 0; x < 8; x++)
								{
									if (board[0][x] == 'P') { board[0][x] = 'Q'; }
									if (board[7][x] == 'p') { board[7][x] = 'q'; }
								}

								// if a pawn moved forward two squares, then set an en passant square
								// otherwise, clear the en passant square
								if (board[targetSquare[0]][targetSquare[1]] == 'P' && selection[0] - targetSquare[0] == 2)
								{
									enpassant = to_string(targetSquare[0] + 1) + to_string(targetSquare[1]);
								}
								else if (board[targetSquare[0]][targetSquare[1]] == 'p' && selection[0] - targetSquare[0] == -2)
								{
									enpassant = to_string(targetSquare[0] - 1) + to_string(targetSquare[1]);
								}
								else
								{
									enpassant = "";
								}

								// if a pawn moved to an empty square, then clear the square behind it (to handle en passant captures)
								if (board[targetSquare[0]][targetSquare[1]] == 'P' && targetPrev == ' ')
								{
									board[targetSquare[0] + 1][targetSquare[1]] = ' ';
								}
								if (board[targetSquare[0]][targetSquare[1]] == 'p' && targetPrev == ' ')
								{
									board[targetSquare[0] - 1][targetSquare[1]] = ' ';
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
							p->enpassant = enpassant;
							ReleaseMutex(p->mutex);

							// clear selection and legal rectangles
							((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(0, 0);
							for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }
							selection[0] = -1;

							// indicate that it is the engine's turn
							madeMove = true;
						}

						// if the move is invalid (i.e. not castled) and a different player piece was clicked
						// then move the selection square to that piece
						if (!validMove && board[targetSquare[0]][targetSquare[1]] >= 65 && board[targetSquare[0]][targetSquare[1]] <= 90 && !sameSquareSelected)
						{
							((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(squarePos[squareY][squareX]);
							selection[0] = squareY;
							selection[1] = squareX;

							// clear and set legal rectangle positions
							for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }

							string from = to_string(selection[0]) + to_string(selection[1]);
							bool castling[4];
							string enpassant = "";
							WaitForSingleObject(p->mutex, INFINITE);
							memcpy(castling, p->castling, 4 * sizeof(bool));
							enpassant = p->enpassant;
							ReleaseMutex(p->mutex);
							vector<string> legalMoves = helper::getLegalMoves(board, true, castling, enpassant, true)[from];

							for (int i = 0; i < legalMoves.size(); i++)
							{
								int legalY = legalMoves[i][0] - 48;
								int legalX = legalMoves[i][1] - 48;
								legalRectangles[i].setPosition(squarePos[legalY][legalX]);
							}
						}

					}
				}

				// if mouse clicked outside the board
				else if(!clickedInBoard)
				{
					((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(0, 0);
					for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }
					selection[0] = -1;
				}

				char printBoard[8][8];
				WaitForSingleObject(p->mutex, INFINITE);
				memcpy(printBoard, p->board, 64 * sizeof(char));
				ReleaseMutex(p->mutex);

				if (dpr && madeMove)
				{
					for (int i = 0; i < 8; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							cout << printBoard[i][j] << " ";
						}
						cout << endl;
					}
					cout << endl;
				}



				// switch to engine's turn if a move was made
				if (madeMove)
				{
					WaitForSingleObject(p->mutex, INFINITE);
					p->playerToMove = false;
					ReleaseMutex(p->mutex);
				}

				if (dpr && madeMove) { printedLegal = false; }
			}
		}
	}
}