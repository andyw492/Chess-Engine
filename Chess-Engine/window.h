#ifndef window_h
#define window_h

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ctype.h>
#include <stdlib.h>
#include <cassert>
#include <algorithm>
#include <chrono>

#include "parameters.h"
#include "helper.h"

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::map;
using std::unique_ptr;
using std::string;
using std::to_string;
using std::memcpy;
using std::find;
using std::max;
using std::min;

class Window
{
public:

	Window(bool windowPrint);
	~Window();

	void dprint(LPVOID pParam, string s);

	void getMouseSquare(sf::Vector2i mousePos, int squareCoords[2]);

	vector<string> setPiecePositions(char board[8][8]);

	UINT start(LPVOID pParam);

	void initializeObjects(LPVOID pParam, char board[8][8]);

	void display(LPVOID pParam);

private:

	// debug print
	bool dpr = false;

	// heap memory
	vector<sf::Texture*> textures;
	vector<sf::Sprite*> sprites;
	map<string, sf::Drawable*> objects;

	// x and y coords for each board square
	sf::Vector2f squarePos[8][8];

	// x and y coords for each piece on board squares
	sf::Vector2f piecePos[8][8];
};

#endif