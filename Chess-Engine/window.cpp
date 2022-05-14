#ifndef window_cpp
#define window_cpp

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

#include "parameters.cpp"

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::map;
using std::unique_ptr;

class Window
{
public:

	Window() {}

	UINT start(LPVOID pParam)
	{
		Parameters *p = ((Parameters*)pParam);

		WaitForSingleObject(p->mutex, INFINITE);
		printf("windowThread %d started\n", GetCurrentThreadId());
		ReleaseMutex(p->mutex);

		display(pParam);

		ReleaseSemaphore(p->finished, 1, NULL);
		WaitForSingleObject(p->eventQuit, INFINITE);

		// print we're about to exit
		WaitForSingleObject(p->mutex, INFINITE);
		printf("windowThread %d quitting on event\n", GetCurrentThreadId());
		ReleaseMutex(p->mutex);

		return 0;
	}

	void initializeObjects(map<string, sf::Drawable*>* objects)
	{
		vector<string> objectNames{"board_white", "b_black", "b_white", "k_black", "k_white", "n_black", "n_white", "p_black", "p_white", "q_black", "q_white", "r_black", "r_white"};

		for(int i = 0; i < objectNames.size(); i++)
		{
			textures.push_back(new sf::Texture());
			string fileName = "img/" + objectNames[i] + ".png";
			textures[i]->loadFromFile(fileName);

			sprites.push_back(new sf::Sprite());
			sprites[i]->setTexture(*textures[i]);
			sprites[i]->setPosition(sf::Vector2f(i*50.f, 150.f));

			if (objectNames[i] == "board_white")
			{
				sprites[i]->setScale(sf::Vector2f(0.8f, 0.8f));
			}

			(*objects)[objectNames[i]] = sprites[i];
		}

		// polymorphism yay
		sf::CircleShape* shape = new sf::CircleShape(100.f);
		shape->setFillColor(sf::Color::Green);
		shape->setPosition(sf::Vector2f(500.f, 500.f));
		(*objects)["circle"] = shape;
	}

	void display(LPVOID pParam)
	{
		Parameters *p = ((Parameters*)pParam);

		sf::RenderWindow window(sf::VideoMode(900, 900), "Chess Engine");

		map<string, sf::Drawable*> objects;
		initializeObjects(&objects);


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
					window.clear(sf::Color::White);

					window.draw(*objects["board_white"]);
					for (auto i : objects)
					{
						if (i.first == "board_white")
						{
							continue;
						}
						window.draw(*i.second);
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
					std::getline(cin, move);
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

private:
	vector<sf::Texture*> textures;
	vector<sf::Sprite*> sprites;
};

#endif