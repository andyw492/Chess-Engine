#include "window.h"

Window::Window()
{
	// initialize board square positions
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			boardSquares[i][j] = sf::Vector2f(j * 70 + 34.5f, i * 70 + 36.5f);
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

UINT Window::start(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->mutex, INFINITE);
	printf("windowThread %d started\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	display(pParam);

	WaitForSingleObject(p->mutex, INFINITE);
	p->windowClosed = true;
	ReleaseMutex(p->mutex);

	ReleaseSemaphore(p->finished, 1, NULL);
	WaitForSingleObject(p->eventQuit, INFINITE);

	// print we're about to exit
	WaitForSingleObject(p->mutex, INFINITE);
	printf("windowThread %d quitting on event\n", GetCurrentThreadId());
	ReleaseMutex(p->mutex);

	return 0;
}

void Window::initializeObjects(LPVOID pParam, string initialFen)
{
	Parameters *p = ((Parameters*)pParam);

	vector<string> objectNames{ "board_white", "b1", "b2", "B1", "B2", "k", "K", "n1", "n2", "N1", "N2", "q", "Q", "r1", "r2", "R1", "R2" };
	string pawnString = "";
	for (int i = 1; i < 9; i++)
	{
		objectNames.push_back("p" + to_string(i));
	}
	for (int i = 1; i < 9; i++)
	{
		objectNames.push_back("P" + to_string(i));
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
			WaitForSingleObject(p->mutex, INFINITE);
			cout << "objectNames[i][0]: " << objectNames[i][0] << endl;
			ReleaseMutex(p->mutex);
			//dprint(pParam, "objectNames[i][0]:" + to_string(char(objectNames[i][0])));

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

			dprint(pParam, "board size " + to_string(x) + " " + to_string(y));
		}
	}

	((sf::Sprite*)(objects)["board_white"])->setScale(sf::Vector2f(0.76087f, 0.76087f));
	((sf::Sprite*)(objects)["board_white"])->setPosition(sf::Vector2f(30.f, 30.f));

	// set the pieces to their squares in the initial fen
	vector<string> parts = helper::splitToVector(initialFen, '/');
	map<string, int> pieceIndexes = {{"r",1}, {"n",1}, {"b",1}, {"p",1}, {"R",1}, {"N",1}, {"B",1}, {"P",1}};
	for (int i = 0; i < parts.size(); i++)
	{
		string partsString = parts[i];
		dprint(pParam, partsString);

		// replace numbers with spaces, e.g. "4" -> "    " (" " * 4)
		string processedParts = "";
		for (int j = 0; j < partsString.length(); j++)
		{
			if (isdigit(partsString[j]))
			{
				int spaceCount = partsString[j] - '0';
				string spaceString = "        ";
				processedParts += spaceString.substr(0, spaceCount);
			}
			else
			{
				processedParts += partsString[j];
			}
		}

		dprint(pParam, "new parts string: " + processedParts);
		dprint(pParam, "---");

		assert(processedParts.length() == 8);
		for (int j = 0; j < processedParts.length(); j++)
		{
			string objectName = "";
			switch (processedParts[j])
			{
			case 'r':
				objectName = "r" + to_string(pieceIndexes["r"]); pieceIndexes["r"]++; break;
			case 'n':
				objectName = "n" + to_string(pieceIndexes["n"]); pieceIndexes["n"]++; break;
			case 'b':
				objectName = "b" + to_string(pieceIndexes["b"]); pieceIndexes["b"]++; break;
			case 'q':
				objectName = "q"; break;
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
				objectName = "Q"; break;
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
				dprint(pParam, "error bad char");
				break;
			}

			if (objectName == "")
			{
				continue;
			}
			((sf::Sprite*)(objects)[objectName])->setPosition(boardSquares[i][j]);
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

	string initialFen = "";
	WaitForSingleObject(p->mutex, INFINITE);
	initialFen = p->initialFen;
	ReleaseMutex(p->mutex);

	initializeObjects(pParam, initialFen);

	sf::RenderWindow window(sf::VideoMode(1000, 660), "Chess Engine");

	// draw initial objects
	window.clear(sf::Color::White);
	window.draw(*objects["board_white"]);

	for (auto i : objects)
	{
		if (i.first == "board_white" || i.first == "selectionRectangle") { continue; }
		window.draw(*i.second);
	}

	window.display();

	//--------------------MAIN WINDOW LOOP--------------------------

	while (window.isOpen())
	{
		//-------------------POLL EVENTS----------------------------
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				dprint(pParam, "mouse position " + to_string(mousePos.x) + " " + to_string(mousePos.y));

				int squarePosX = floor((mousePos.x - 30) / 70) * 70 + 30;
				int squarePosY = floor((mousePos.y - 30) / 70) * 70 + 30;

				// only set new position if button press is inside the board
				if (std::max(squarePosX, squarePosY) < 590 && squarePosX > 0)
				{
					((sf::RectangleShape*)objects["selectionRectangle"])->setPosition(squarePosX, squarePosY);
				}

				// need conversion between mouse position -> board square -> object on that square
				// string called "selection" would store the piece that is currently selected, e.g. "N2"

				// when the mouse clicks on a square, check if selection string has a piece
				//	if yes, then check if there is an enemy piece on that square
				//		if yes, that is a capture, implement later
				//		if no, then move the piece to that square and send that move to the engine

				map<string, vector<string>> legalMoves = helper::getLegalMoves(initialFen, true);
				for (auto i : legalMoves)
				{
					cout << i.first << ": ";
					for (int j = 0; j < i.second.size(); j++)
					{
						cout << i.second[j] << " ";
					}
					cout << endl;
				}

				((sf::Sprite*)objects["N2"])->setPosition(boardSquares[5][5]);

				window.clear(sf::Color::White);

				window.draw(*objects["board_white"]);

				for (auto i : objects)
				{
					if (i.first == "board_white" || i.first == "selectionRectangle")
					{
						continue;
					}
					window.draw(*i.second);
				}

				if (((sf::RectangleShape*)objects["selectionRectangle"])->getPosition().x > 0)
				{
					window.draw(*objects["selectionRectangle"]);
				}
				
					

				//sf::Texture texture;
				//texture.loadFromFile("p_black.png");

				//sf::Sprite sprite2;
				//sprite2.setTexture(texture);
				//sprite2.setPosition(sf::Vector2f(100.f, 50.f));
				//map<string, sf::Sprite> objects2;
				//objects2["a"] = sprite2;
				//window.draw(objects2["a"]);

				window.display();

				// proceed if move is empty
				string move = "";
				WaitForSingleObject(p->mutex, INFINITE);
				move = p->move;
				ReleaseMutex(p->mutex);
				if (move != "")
				{
					continue;
				}

				WaitForSingleObject(p->mutex, INFINITE);
				cout << "window: sending ";
				move = "p";
				//std::getline(cin, move);
				p->move = move;
				ReleaseMutex(p->mutex);

				// if the move is "q", then stop
				if (move == "q")
				{
					window.close();
				}
			}

		}
	}
}