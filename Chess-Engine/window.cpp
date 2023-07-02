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

void Window::windowSetup(Parameters* p, sf::Font& font, vector<sf::Text>& statsText, Position& currentPosition)
{
	if (!font.loadFromFile("ModernSans-Light.otf"))
	{
		cout << "font failed to load" << endl;
	}


	float statsYPosition = 30.f;
	for (int i = 0; i < 12; i++)
	{
		sf::Text t;
		t.setFont(font);
		t.setCharacterSize(18);
		t.setFillColor(sf::Color::Black);
		t.setPosition(sf::Vector2f(650.f, statsYPosition));
		statsText.push_back(t);

		if (i == 2 || i == 4 || i == 6 || i == 8)
		{
			statsYPosition += 50.f;
		}
		else
		{
			statsYPosition += 30.f;
		}
	}

	// construct the board from the initial fen
	string initialFen = "";
	WaitForSingleObject(p->mutex, INFINITE);
	initialFen = p->initialFen;
	ReleaseMutex(p->mutex);

	char board[8][8];
	helper::fenToMatrix(helper::splitToVector(initialFen, ' ')[0], board);

	bool initialCastling[4] = { false, false, false, false };
	string castlingString = helper::splitToVector(initialFen, ' ')[2];
	for (char c : castlingString)
	{
		switch (c)
		{
		case 'K': initialCastling[0] = true; break;
		case 'Q': initialCastling[1] = true; break;
		case 'k': initialCastling[2] = true; break;
		case 'q': initialCastling[3] = true; break;
		default: break;
		}
	}

	string gameResult = "";

	WaitForSingleObject(p->mutex, INFINITE);
	memcpy(p->currentPosition.board, board, 64 * sizeof(char));
	memcpy(p->currentPosition.castling, initialCastling, 4 * sizeof(bool));
	currentPosition = p->currentPosition;
	ReleaseMutex(p->mutex);

	// rectangles that indicate legal moves
	// stored outside objects map because a vector is more appropriate
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
}

void Window::printBoard(Parameters* p, Position currentPosition)
{
	auto begin = std::chrono::high_resolution_clock::now();
	map<string, vector<string>> legalMoves = helper::getLegalMoves(currentPosition, true);
	auto end = std::chrono::high_resolution_clock::now();

	WaitForSingleObject(p->mutex, INFINITE);
	PRINT("----------------- WINDOW -----------------\n\n", 0);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (currentPosition.board[i][j] != ' ')
			{
				PRINT(string() + currentPosition.board[i][j] + " ", 0);
			}
			else
			{
				PRINT(". ", 0);
			}
		}
		PRINT("\n", 0);
	}
	PRINT("\n", 0);

	for (auto i : legalMoves)
	{
		PRINT(i.first + ": ", 0);

		for (int j = 0; j < i.second.size(); j++)
		{
			PRINT(i.second[j] + " ", 0);
		}

		PRINT("\n", 0);
	}
	PRINT("\n", 0);

	PRINT("legal moves found in " + helper::roundFloat(float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6, 3) + " ms\n", 0);
	ReleaseMutex(p->mutex);
}

void Window::getMouseSquare(sf::Vector2i mousePos, int squareCoords[2])
{
	squareCoords[0] = floor((mousePos.y - 30) / 70);
	squareCoords[1] = floor((mousePos.x - 30) / 70);
}

vector<string> Window::getPiecePositions(char board[8][8])
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

void Window::drawObjects(Parameters* p, sf::RenderWindow& window, Position currentPosition)
{
	vector<string> piecesToDraw = getPiecePositions(currentPosition.board);

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
}

vector<sf::Text> Window::getStatsText(Parameters* p, sf::Font& font, vector<sf::Text> statsText, std::chrono::steady_clock::time_point engineStart)
{
	vector<string> stats;

	for (int i = 0; i < 12; i++)
	{
		stats.push_back("0");
	}

	WaitForSingleObject(p->mutex, INFINITE);

	auto currentTime = std::chrono::high_resolution_clock::now();
	stats[0] = to_string(float(std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - engineStart).count()) / 1e9);
	stats[1] = to_string(p->depth0Progress) + " / " + to_string(p->depth0Children);
	stats[2] = to_string(p->depth1Progress) + " / " + to_string(p->depth1Children);
	stats[3] = to_string(p->positionsEvaluated);
	stats[4] = to_string(p->totalPositionsEvaluated);
	stats[5] = to_string(p->nodesCreated);
	stats[6] = to_string(p->nodesDeleted);
	stats[9] = to_string(p->nextPositionsCache.size());
	stats[10] = to_string(p->foundInCache);

	ReleaseMutex(p->mutex);

	for (int i = 0; i < stats.size(); i++)
	{
		size_t found = stats[i].find(".");
		if (found != string::npos)
		{
			stats[i] = stats[i].substr(0, found + 4);
		}

		string statString = "";
		switch (i)
		{
		case 0:
			statString = "Elapsed: " + stats[i] + " seconds";
			break;
		case 1:
			statString = "Depth 0: " + stats[i];
			break;
		case 2:
			statString = "Depth 1: " + stats[i];
			break;
		case 3:
			statString = "Positions evaluated: " + stats[i];
			break;
		case 4:
			statString = "(Total: " + stats[i] + ")";
			break;
		case 5:
			statString = "Nodes created: " + stats[i];
			break;
		case 6:
			statString = "Nodes deleted: " + stats[i];
			break;
		case 7:
			statString = "Nodes pruned: " + stats[i];
			break;
		case 8:
			statString = "(Time saved: " + stats[i] + " seconds)";
			break;
		case 9:
			statString = "Next position cache size: " + stats[i];
			break;
		case 10:
			statString = "Found in cache: " + stats[i];
			break;
		case 11:
			statString = "(Time saved: " + stats[i] + " seconds)";
			break;

		default:
			break;
		}

		statsText[i].setString(statString);
	}

	return statsText;
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

	sf::RectangleShape* rectangle = new sf::RectangleShape();
	rectangle->setSize(sf::Vector2f(70, 70));
	rectangle->setOutlineColor(sf::Color::Blue);
	rectangle->setFillColor(sf::Color::Transparent);
	rectangle->setOutlineThickness(-3);
	rectangle->setPosition(0, 0);
	objects["selectionRectangle"] = rectangle;
}

void Window::updateWindow(Parameters* p, sf::RenderWindow& window, vector<sf::Text>& statsText, sf::Font& font, std::chrono::steady_clock::time_point engineStart, Position& currentPosition)
{
	// get the current board and update stats
	bool playerToMove = false;
	WaitForSingleObject(p->mutex, INFINITE);
	playerToMove = p->playerToMove;
	ReleaseMutex(p->mutex);

	if (!playerToMove)
	{
		statsText = getStatsText(p, font, statsText, engineStart);
	}

	// draw the current board's objects to window
	window.clear(sf::Color::White);
	drawObjects(p, window, currentPosition);

	for (int i = 0; i < statsText.size(); i++)
	{
		window.draw(statsText[i]);
	}

	window.display();

	if (playerToMove)
	{
		WaitForSingleObject(p->mutex, INFINITE);
		currentPosition = p->currentPosition;
		string gameResult = p->gameResult;
		ReleaseMutex(p->mutex);

		// check if the game ended
		if (gameResult.length() > 0)
		{
			Sleep(2000);
			window.close();
		}
	}
}

bool Window::handleBoardClick(Parameters* p, Position& currentPosition, int (&selection)[2], sf::Vector2i mousePos)
{
	int squareY = floor((mousePos.y - 30) / 70);
	int squareX = floor((mousePos.x - 30) / 70);

	int targetSquare[2];
	getMouseSquare(mousePos, targetSquare);

	//if (dpr)
	//{
	//	cout << "target square " << targetSquare[0] << " " << targetSquare[1] << endl;
	//	cout << "selection " << selection << endl;
	//}

	// select the players piece if selection string is empty
	if (selection[0] == -1 && currentPosition.board[targetSquare[0]][targetSquare[1]] >= 65 && currentPosition.board[targetSquare[0]][targetSquare[1]] <= 90)
	{
		((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(squarePos[squareY][squareX]);
		selection[0] = squareY;
		selection[1] = squareX;

		// set legal rectangle positions
		string from = to_string(selection[0]) + to_string(selection[1]);
		bool castling[4];
		//string enpassant = "";
		WaitForSingleObject(p->mutex, INFINITE);
		Position position = p->currentPosition;
		memcpy(castling, p->currentPosition.castling, 4 * sizeof(bool));
		//enpassant = p->currentPosition.enpassant;
		ReleaseMutex(p->mutex);
		vector<string> legalMoves = helper::getLegalMoves(position, true)[from];

		for (int i = 0; i < legalMoves.size(); i++)
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
		//string enpassant = "";

		vector<string> legalMoves = helper::getLegalMoves(currentPosition, true)[from];
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
				char targetPrev = currentPosition.board[targetSquare[0]][targetSquare[1]];
				char movedPiece = currentPosition.board[selection[0]][selection[1]];
				currentPosition.board[selection[0]][selection[1]] = ' ';
				currentPosition.board[targetSquare[0]][targetSquare[1]] = movedPiece;

				// if a pawn is on a promoting square, then replace it with a queen
				for (int x = 0; x < 8; x++)
				{
					if (currentPosition.board[0][x] == 'P') { currentPosition.board[0][x] = 'Q'; }
					if (currentPosition.board[7][x] == 'p') { currentPosition.board[7][x] = 'q'; }
				}

				// if a pawn moved forward two squares, then set an en passant square
				// otherwise, clear the en passant square
				//if (board[targetSquare[0]][targetSquare[1]] == 'P' && selection[0] - targetSquare[0] == 2)
				//{
				//	enpassant = to_string(targetSquare[0] + 1) + to_string(targetSquare[1]);
				//}
				//else if (board[targetSquare[0]][targetSquare[1]] == 'p' && selection[0] - targetSquare[0] == -2)
				//{
				//	enpassant = to_string(targetSquare[0] - 1) + to_string(targetSquare[1]);
				//}
				//else
				//{
				//	enpassant = "";
				//}

				// if a pawn moved to an empty square, then clear the square behind it (to handle en passant captures)
				if (currentPosition.board[targetSquare[0]][targetSquare[1]] == 'P' && targetPrev == ' ')
				{
					currentPosition.board[targetSquare[0] + 1][targetSquare[1]] = ' ';
				}
				if (currentPosition.board[targetSquare[0]][targetSquare[1]] == 'p' && targetPrev == ' ')
				{
					currentPosition.board[targetSquare[0] - 1][targetSquare[1]] = ' ';
				}

				break;
			}
			case 1:
			{
				currentPosition.board[7][4] = ' ';
				currentPosition.board[7][5] = 'R';
				currentPosition.board[7][6] = 'K';
				currentPosition.board[7][7] = ' ';
				break;
			}
			case 2:
			{
				currentPosition.board[7][0] = ' ';
				currentPosition.board[7][2] = 'K';
				currentPosition.board[7][3] = 'R';
				currentPosition.board[7][4] = ' ';
				break;
			}
			case 3:
			{
				currentPosition.board[0][4] = ' ';
				currentPosition.board[0][5] = 'r';
				currentPosition.board[0][6] = 'k';
				currentPosition.board[0][7] = ' ';
				break;
			}
			case 4:
			{
				currentPosition.board[0][0] = ' ';
				currentPosition.board[0][2] = 'k';
				currentPosition.board[0][3] = 'r';
				currentPosition.board[0][4] = ' ';
				break;
			}
			}

			// modify castling permissions
			if (from == "74" || from == "77") { currentPosition.castling[0] = false; }
			if (from == "74" || from == "70") { currentPosition.castling[1] = false; }
			if (from == "04" || from == "07") { currentPosition.castling[2] = false; }
			if (from == "04" || from == "00") { currentPosition.castling[3] = false; }

			// clear selection and legal rectangles
			((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(0, 0);
			for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }
			selection[0] = -1;

			// indicate that it is the engine's turn
			return true;
		}

		// if the move is invalid (i.e. not castled) and a different player piece was clicked
		// then move the selection square to that piece
		if (!validMove && currentPosition.board[targetSquare[0]][targetSquare[1]] >= 65 && currentPosition.board[targetSquare[0]][targetSquare[1]] <= 90 && !sameSquareSelected)
		{
			((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(squarePos[squareY][squareX]);
			selection[0] = squareY;
			selection[1] = squareX;

			// clear and set legal rectangle positions
			for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }

			string from = to_string(selection[0]) + to_string(selection[1]);
			bool castling[4];
			//string enpassant = "";
			WaitForSingleObject(p->mutex, INFINITE);
			memcpy(castling, p->currentPosition.castling, 4 * sizeof(bool));
			Position position = p->currentPosition;
			//enpassant = p->currentPosition.enpassant;
			ReleaseMutex(p->mutex);
			vector<string> legalMoves = helper::getLegalMoves(position, true)[from];

			for (int i = 0; i < legalMoves.size(); i++)
			{
				int legalY = legalMoves[i][0] - 48;
				int legalX = legalMoves[i][1] - 48;
				legalRectangles[i].setPosition(squarePos[legalY][legalX]);
			}
		}

	}

	return false;
}

void Window::display(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	sf::RenderWindow window(sf::VideoMode(1000, 660), "Chess Engine");

	sf::Font font;
	vector<sf::Text> statsText;

	Position currentPosition;
	string gameResult = "";
	int selection[2] = { -1, -1 };

	windowSetup(p, font, statsText, currentPosition);

	initializeObjects(pParam, currentPosition.board);

	std::chrono::steady_clock::time_point engineStart = std::chrono::high_resolution_clock::now();

	// debug
	bool printed = false;

	// main window loop
	while (window.isOpen())
	{
		updateWindow(p, window, statsText, font, engineStart, currentPosition);

		WaitForSingleObject(p->mutex, INFINITE);
		bool playerToMove = p->playerToMove;
		ReleaseMutex(p->mutex);
		if (playerToMove && !printed)
		{
			printBoard(p, currentPosition);
			printed = true;
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);

				bool clickedInBoard = max(mousePos.x, mousePos.y) < 590 && min(mousePos.x, mousePos.y) >= 30;

				// used to decide if it is engines turn after mouse click
				bool madeMove = false;

				// check if game is over
				string gameResult = "";
				WaitForSingleObject(p->mutex, INFINITE);
				gameResult = p->gameResult;
				ReleaseMutex(p->mutex);

				bool playerToMove = false;
				WaitForSingleObject(p->mutex, INFINITE);
				playerToMove = p->playerToMove;
				ReleaseMutex(p->mutex);

				// only process a click in the board if players turn
				if (clickedInBoard && playerToMove && gameResult.length() == 0)
				{
					madeMove = handleBoardClick(p, currentPosition, selection, mousePos);
				}

				// if mouse clicked outside the board
				else if(!clickedInBoard)
				{
					((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(0, 0);
					for (int i = 0; i < 64; i++) { legalRectangles[i].setPosition(0, 0); }
					selection[0] = -1;
				}

				// switch to engine's turn and reset stats if a move was made
				if (madeMove)
				{
					WaitForSingleObject(p->mutex, INFINITE);
					p->playerToMove = false;
					p->currentPosition = currentPosition;
					p->nodesCreated = 0;
					p->nodesDeleted = 0;
					p->positionsEvaluated = 0;
					p->foundInCache = 0;
					ReleaseMutex(p->mutex);

					engineStart = std::chrono::high_resolution_clock::now();

					printed = false;
				}
			}
		}
	}
}

UINT Window::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	//WaitForSingleObject(p->mutex, INFINITE);
	//printf("windowThread %d started\n", GetCurrentThreadId());
	//ReleaseMutex(p->mutex);

	display(pParam);

	// tell engine that window is closed
	WaitForSingleObject(p->mutex, INFINITE);
	p->windowClosed = true;
	ReleaseMutex(p->mutex);

	// thread exit
	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	//WaitForSingleObject(p->mutex, INFINITE);
	//printf("windowThread %d quitting on event\n", GetCurrentThreadId());
	//ReleaseMutex(p->mutex);

	return 0;
}