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
	SDL_Quit();
}

void Game::Run()
{
	ascii::Graphics graphics(kWindowTitle, kWindowWidth, kWindowHeight);

	LoadContent(graphics);

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

void Game::LoadContent(ascii::Graphics& graphics)
{
	image = graphics.loadTexture("glyph.bmp");
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

	graphics.drawBorder(' ', ascii::Color::Yellow, ascii::Color::Black);

	graphics.blitTexture(image, 1, 1);
	
	graphics.update();
}