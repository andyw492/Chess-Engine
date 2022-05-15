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

class Window
{
public:

	Window();
	~Window();

	void dprint(LPVOID pParam, string s);

	UINT start(LPVOID pParam);

	void initializeObjects(LPVOID pParam, string initialFen);

	void display(LPVOID pParam);

private:
	// heap memory
	vector<sf::Texture*> textures;
	vector<sf::Sprite*> sprites;
	map<string, sf::Drawable*> objects;

	// x and y coords for each board square
	sf::Vector2f boardSquares[8][8];
};

#endif