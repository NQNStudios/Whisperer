#include "Game.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <SDL.h>

const char* kWindowTitle = "The Whisperer in Darkness";

const int kFPS = 60;
const int kMaxFrameTime = 5 * 1000 / 60;

const int kWindowWidth = 80;
const int kWindowHeight = 25;

Game::Game()
	: mRunning(false)
{
	SDL_Init(SDL_INIT_EVERYTHING);
}

Game::~Game()
{
	delete cache;

	SDL_Quit();
}

void Game::Run()
{
	ascii::Graphics graphics(kWindowTitle, kWindowWidth, kWindowHeight);

	cache = graphics.createImageCache();
	LoadContent(cache);

	ascii::Input input;

	mRunning = true;

	int lastUpdateTime = SDL_GetTicks();

	while (mRunning)
	{
		const int initialTime = SDL_GetTicks();

		input.beginNewFrame();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					Quit();
					return;
				case SDL_KEYDOWN:
					input.keyDownEvent(event);
					break;
				case SDL_KEYUP:
					input.keyUpEvent(event);
					break;
			}
		}

		HandleInput(input);

		const int currentTime = SDL_GetTicks();
		const int elapsedTime = currentTime - lastUpdateTime;

		Update(std::min(elapsedTime, kMaxFrameTime));
		lastUpdateTime = currentTime;

		Draw(graphics);

		const int msPerFrame = 1000 / kFPS;
		const int elapsedTimeMS = SDL_GetTicks() - initialTime;

		if (elapsedTimeMS < msPerFrame)
			SDL_Delay(msPerFrame - elapsedTimeMS);
	}
}

void Game::Quit()
{
	mRunning = false;
}

void Game::LoadContent(ascii::ImageCache* cache)
{
	surface = ascii::Surface::FromFile("ART.txt", cache);

	std::string specialInfo;
	for (int x = 0; x < surface->width(); ++x)
	{
		for (int y = 0; y < surface->height(); ++y)
		{
			specialInfo = surface->getSpecialInfo(x, y);

			if (!specialInfo.empty())
			{
				std::cout << "Special info at (" << x << ", " << y << ": " << specialInfo << std::endl;
			}
		}
	}
}

void Game::Update(int deltaMS)
{
	
}

void Game::HandleInput(ascii::Input& input)
{
	
}

void Game::Draw(ascii::Graphics& graphics)
{
	graphics.clear();

	graphics.blitSurface(surface, 0, 0);

	graphics.update();
}