#include "Game.h"

#include <iostream>

#include <SDL_mixer.h>

const char* kWindowTitle = "The Whisperer in Darkness";

const int kWindowWidth = 100;
const int kWindowHeight = 40;

ascii::Surface* gSurface;

int gSurfaceX = 0;
int gSurfaceY = 0;

bool gDrawn = false;

void LoadContent(ascii::ImageCache* cache, ascii::SoundManager* soundManager)
{
	gSurface = ascii::Surface::FromFile("Data/caption.srf");
	
	cache->loadTexture("forest", "Data/Untitled artwork.png", ascii::Color::Maroon);

	soundManager->addSound("fireball", "Data/fireball.wav");
	soundManager->addSound("splash", "Data/sunsplash.wav");
	soundManager->addTrack("space", "Data/Space Fighter Loop.mp3");
}

void Update(ascii::Game* game, int deltaMS)
{
	std::cout << "FPS: " << 1.0f / ((float)deltaMS / 1000.0f);
}

void HandleInput(ascii::Game* game, ascii::Input& input)
{
	if (input.wasKeyPressed(SDLK_SPACE))
	{
		if (game->soundManager()->trackPlaying())
		{
			game->soundManager()->fadeOutTrack(2000);
		}
		else
		{
			game->soundManager()->fadeInTrack("space", -1, 2000);
		}
	}

	if (input.mouseButtonDown(ascii::LEFT))
	{
		gSurfaceX = input.mouseX() / game->graphics()->charWidth();
		gSurfaceY = input.mouseY() / game->graphics()->charHeight();

		gDrawn = false;
	}
}

void Draw(ascii::Game* game)
{
	if (!gDrawn)
	{
		gDrawn = true;

		game->graphics()->clearTransparent();
		game->graphics()->blitSurface(gSurface, gSurfaceX, gSurfaceY);
		//game->graphics()->addBackgroundImage("forest", "forest", 0, 0);
	
		game->graphics()->update();
	}
	
}

int main(int argc, char** argv)
{
	ascii::Game game(kWindowTitle, kWindowWidth, kWindowHeight, LoadContent, Update, HandleInput, Draw);

	game.Run();
	
	return 0;
}

