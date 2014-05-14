#include "Game.h"

#include <iostream>

#include <SDL_mixer.h>

const char* kWindowTitle = "The Whisperer in Darkness";

const int kWindowWidth = 100;
const int kWindowHeight = 40;

ascii::Surface* gSurface;

void LoadContent(ascii::ImageCache* cache)
{
	gSurface = ascii::Surface::FromFile("MySurface.srf");
	
	cache->loadTexture("forest", "Untitled artwork.png", ascii::Color::Maroon);
}

void Update(ascii::Game* game, int deltaMS)
{
	std::cout << "FPS: " << 1.0f / ((float)deltaMS / 1000.0f);
}

void HandleInput(ascii::Game* game, ascii::Input& input)
{
	
}

void Draw(ascii::Graphics& graphics)
{
	graphics.clear();

	graphics.blitSurface(gSurface, 0, 0);
	//graphics.addBackgroundImage("forest", "forest", 0, 0);
	
	graphics.update();
}

int main(int argc, char** argv)
{
	ascii::Game game(kWindowTitle, kWindowWidth, kWindowHeight, LoadContent, Update, HandleInput, Draw);

	game.Run();
	
	return 0;
}

