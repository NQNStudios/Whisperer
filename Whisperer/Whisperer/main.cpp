#include "Game.h"

const char* kWindowTitle = "The Whisperer in Darkness";

const int kWindowWidth = 80;
const int kWindowHeight = 25;

ascii::Surface* gSurface;

void LoadContent(ascii::ImageCache* cache)
{
	gSurface = ascii::Surface::FromFile("ART.txt", cache);
}

void Update(ascii::Game* game, int deltaMS)
{
	
}

void HandleInput(ascii::Game* game, ascii::Input& input)
{
	
}

void Draw(ascii::Graphics& graphics)
{
	graphics.clear();

	graphics.blitSurface(gSurface, 0, 0);

	graphics.update();
}

int main(int argc, char** argv)
{
	ascii::Game game(kWindowTitle, kWindowWidth, kWindowHeight, LoadContent, Update, HandleInput, Draw);

	game.Run();
	
	return 0;
}

