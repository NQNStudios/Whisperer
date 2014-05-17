#include "Game.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <map>

#include <SDL_mixer.h>
#include <SDL_image.h>

const char* kWindowTitle = "The Whisperer in Darkness";

const int kWindowWidth = 100;
const int kWindowHeight = 40;

ascii::Game* game;
std::deque<std::string> linesToExecute;
bool readyToContinue = true;

std::map<std::string, ascii::Surface*> surfaces;

bool updateGraphics = false;
int msToWait = 0;

inline void Ready()
{
	readyToContinue = true;
}

const char* CString(std::string string)
{
	char* cstr = new char [string.length()+1];
	std::strcpy (cstr, string.c_str());

	return cstr;
}

void RunLine(const char* line)
{
	readyToContinue = false;

	std::stringstream sstream(line);

	std::string command;
	sstream >> command;

	if (!command.compare("Clear"))
	{
		game->graphics()->clear();
		Ready();

		return;
	}

	if (!command.compare("ClearTransparent"))
	{
		game->graphics()->clearTransparent();
		Ready();

		return;
	}

	if (!command.compare("Update"))
	{
		updateGraphics = true;

		return;
	}

	if (!command.compare("LoadSurface"))
	{
		std::string key;
		std::string path;

		sstream >> key;
		sstream >> path;

		surfaces[key] = ascii::Surface::FromFile(path.c_str());

		Ready();
		return;
	}

	if (!command.compare("BlitSurface"))
	{
		std::string key;
		std::string x;
		std::string y;

		sstream >> key;
		sstream >> x;
		sstream >> y;

		game->graphics()->blitSurface(surfaces[key], atoi(x.c_str()), atoi(y.c_str()));
		Ready();
		return;
	}

	if (!command.compare("CopySurface"))
	{
		std::string key;
		std::string x;
		std::string y;

		sstream >> key;
		sstream >> x;
		sstream >> y;

		game->graphics()->copySurface(surfaces[key], atoi(x.c_str()), atoi(y.c_str()));
		Ready();
		return;
	}

	if (!command.compare("Wait"))
	{
		std::string amt;

		sstream >> amt;

		int ms = atoi(amt.c_str());

		msToWait = ms;

		return;
	}

	if (!command.compare("LoadImage"))
	{
		std::string key;
		std::string path;

		sstream >> key;
		sstream >> path;

		if (strcmp(CString(path), "Data/UntitledArtwork.png"))
		{
			std::cout << "Could be the problem";
		}

		game->imageCache()->loadTexture(CString(key), CString(path));

		Ready();
		return;
	}

	if (!command.compare("AddBackgroundImage"))
	{
		std::string key;
		std::string textureKey;
		std::string x;
		std::string y;

		sstream >> key;
		sstream >> textureKey;
		sstream >> x;
		sstream >> y;

		if (strcmp(CString(textureKey), "woods"))
		{
			std::cout << "Could be the problem";
		}

		game->graphics()->addBackgroundImage(CString(key), CString(textureKey), atoi(x.c_str()), atoi(y.c_str()));

		Ready();
		return;
	}

	if (!command.compare("AddForegroundImage"))
	{
		std::string key;
		std::string textureKey;
		std::string x;
		std::string y;

		sstream >> key;
		sstream >> textureKey;
		sstream >> x;
		sstream >> y;

		game->graphics()->addForegroundImage(CString(key), CString(textureKey), atoi(x.c_str()), atoi(y.c_str()));

		Ready();
		return;
	}
}

void RunScript(const char* path)
{
	std::ifstream file(path);

	std::vector<std::string> lines;

	std::string line;

	while (std::getline(file, line))
	{
		lines.push_back(line);
	}

	for (auto it = lines.rbegin(); it != lines.rend(); ++it)
	{
		//iterate through backwards in order to add them to linesToExecute in order.
		if (it->size() > 0)
		{
			linesToExecute.push_front(*it);
		}
	}

	file.close();
}

void LoadContent(ascii::ImageCache* cache, ascii::SoundManager* soundManager)
{
	RunScript("Data/testscript.wsp");
}

void Update(ascii::Game* game, int deltaMS)
{
	if (msToWait > 0)
	{
		msToWait -= deltaMS;

		if (msToWait <= 0)
		{
			msToWait = 0;
			Ready();
		}
	}

	if (readyToContinue && linesToExecute.size() > 0)
	{
		std::string line = linesToExecute.front();
		linesToExecute.pop_front();

		RunLine(line.c_str());
	}
}

void HandleInput(ascii::Game* game, ascii::Input& input)
{
}

void Draw(ascii::Graphics& graphics)
{
	if (updateGraphics)
	{
		
		//graphics.addBackgroundImage("woodsss", "woods", 0, 0);

		graphics.update();
		updateGraphics = false;
		Ready();
	}

	graphics.update();
}

int main(int argc, char** argv)
{
	game = new ascii::Game(kWindowTitle, kWindowWidth, kWindowHeight, LoadContent, Update, HandleInput, Draw);

	game->Run();
	
	return 0;
}

void LogFPS(int deltaMS)
{
	std::cout << "FPS: " << 1.0f / ((float)deltaMS / 1000.0f);
}