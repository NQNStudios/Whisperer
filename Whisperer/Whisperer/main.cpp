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

#include "Surface.h"

#include "Style.h"

const char* kWindowTitle = "The Whisperer in Darkness";

const int kWindowWidth = 100;
const int kWindowHeight = 40;

ascii::Game* game;
std::deque<std::string> linesToExecute;
bool readyToContinue = true;

std::map<std::string, ascii::Surface*> surfaces;
std::map<std::string, Style*> styles;
std::map<std::string, std::string> text;

bool updateGraphics = false;
int msToWait = 0;

bool waitingForInput = false;

bool tweening = false;
ascii::Surface* tweeningSurface;
int tweenX;
int tweenY;
int tweenElapsedMS = 0;
int tweenMS;
int tweenSrcX;
int tweenSrcY;
int tweenDestX;
int tweenDestY;
int tweenStepX = 0;
int tweenStepY = 0;
std::string tweenStepScript;

bool scrollingText = false;
std::string textToScroll;
std::stringstream scrollStream;
std::string textScrolled;
int revealedChars;
int charsToReveal;
int textElapsedMS;
int textMS;
ascii::Rectangle textRect;
std::string textStepScript;
Style* textStyle;

int selectedEntry = 0;
bool mainMenu = false;
bool* chapterOpen;
const int kMainMenuX = 40;
const int kMainMenuY = 5;
const int kMainMenuWidth = 20;
const int kMainMenuHeight = 22;
const int kFirstEntryX  = 3;
const int kFirstEntryY = 6;

bool inStepScript = false;

void writeOpenChapters()
{
	std::ofstream ofile("Data/save.wsp");

	for (int i = 0; i < 8; ++i)
	{
		ofile << chapterOpen[i] << std::endl;
	}

	ofile.close();
}

void loadOpenChapters()
{
	chapterOpen = new bool[8];
	chapterOpen[0] = true;
	chapterOpen[1] = false;
	chapterOpen[2] = false;
	chapterOpen[3] = false;
	chapterOpen[4] = false;
	chapterOpen[5] = false;
	chapterOpen[6] = false;
	chapterOpen[7] = false;

	std::ifstream ifile("Data/save.wsp");

	if (ifile)
	{
		for (int i = 0; i < 8; ++i)
		{
			ifile >> chapterOpen[i];
		}

		ifile.close();
	}
	else
	{
		writeOpenChapters();
	}
}

inline void Ready()
{
	if (!inStepScript)
	{
		readyToContinue = true;
	}
}

void RunScript(const char* path);

void loadText(const char* path)
{
	std::ifstream file(path);

	std::string key;
	std::string line;
	while (std::getline(file, key))
	{
		key = key.substr(1);

		std::getline(file, line);

		text[key] = line;
	}

	file.close();
}

void tweenSurface(ascii::Surface* surface, int x1, int y1, int x2, int y2, int ms, std::string stepScript)
{
	tweening = true;

	tweeningSurface = surface;
	tweenSrcX = x1;
	tweenSrcY = y1;
	tweenDestX = x2;
	tweenDestY = y2;
	tweenMS = ms;
	tweenStepScript = stepScript;
	tweenElapsedMS = 0;
	tweenX = tweenSrcX;
	tweenY = tweenSrcY;

	tweenStepX = 0;
	tweenStepY = 0;

	if (tweenSrcX < tweenDestX)
	{
		tweenStepX = 1;
	}
	else if (tweenSrcX > tweenDestX)
	{
		tweenStepX = -1;
	}
	else if (tweenSrcY < tweenDestY)
	{
		tweenStepY = 1;
	}
	else if (tweenSrcY > tweenDestY)
	{
		tweenStepY = -1;
	}
}

void RunLine(const char* line)
{
	readyToContinue = false;

	std::stringstream sstream(line);

	std::string command;
	sstream >> command;

	if (!command.compare("UnlockChapter"))
	{
		std::string chapter;

		sstream >> chapter;

		chapterOpen[atoi(chapter.c_str()) - 1] = true;

		writeOpenChapters();
	}

	if (!command.compare("AwaitInput"))
	{
		waitingForInput = true;
	}

	if (!command.compare("RunMainMenu"))
	{
		mainMenu = true;
		selectedEntry = 0;
	}

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

	if (!command.compare("ClearOpaque"))
	{
		game->graphics()->clearOpaque();
		Ready();

		return;
	}

	if (!command.compare("ClearGlyphs"))
	{
		game->graphics()->clearGlyphs();
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

		if (!surfaces[key])
		{
			surfaces[key] = ascii::Surface::FromFile(path.c_str());
		}

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

		game->imageCache()->loadTexture(key, path.c_str());

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

		game->graphics()->addBackgroundImage(key, textureKey, atoi(x.c_str()), atoi(y.c_str()));

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

		game->graphics()->addForegroundImage(key, textureKey, atoi(x.c_str()), atoi(y.c_str()));

		Ready();
		return;
	}

	if (!command.compare("RemoveBackgroundImage"))
	{
		std::string key;

		sstream >> key;

		game->graphics()->removeBackgroundImage(key);

		Ready();
		return;
	}

	if (!command.compare("RemoveForegroundImage"))
	{
		std::string key;

		sstream >> key;

		game->graphics()->removeForegroundImage(key);

		Ready();
		return;
	}

	if (!command.compare("ClearImages"))
	{
		game->graphics()->clearImages();

		Ready();
		return;
	}

	if (!command.compare("RunScript"))
	{
		std::string path;

		sstream >> path;

		RunScript(path.c_str());

		Ready();
		return;
	}

	if (!command.compare("TweenSurface"))
	{
		std::string key;
		std::string x1;
		std::string y1;
		std::string x2;
		std::string y2;
		std::string ms;
		std::string stepScript;

		sstream >> key;
		sstream >> x1;
		sstream >> y1;
		sstream >> x2;
		sstream >> y2;
		sstream >> ms;
		sstream >> stepScript;

		tweenSurface(surfaces[key], atoi(x1.c_str()), atoi(y1.c_str()), atoi(x2.c_str()), atoi(y2.c_str()), atoi(ms.c_str()), stepScript);
	}

	if (!command.compare("LoadSound"))
	{
		std::string key;
		std::string path;

		sstream >> key;
		sstream >> path;

		game->soundManager()->loadSound(key, path.c_str());
		Ready();
		return;
	}

	if (!command.compare("PlaySound"))
	{
		std::string key;

		sstream >> key;

		game->soundManager()->playSound(key);
		Ready();
		return;
	}

	if (!command.compare("LoadTrack"))
	{
		std::string key;
		std::string path;

		sstream >> key;
		sstream >> path;

		game->soundManager()->loadTrack(key, path.c_str());
		Ready();
		return;
	}

	if (!command.compare("PlayTrack"))
	{
		std::string key;

		sstream >> key;

		game->soundManager()->playTrack(key);
		Ready();
		return;
	}

	if (!command.compare("PauseTrack"))
	{
		std::string key;

		sstream >> key;

		game->soundManager()->pauseTrack();
		Ready();
		return;
	}

	if (!command.compare("FadeInTrack"))
	{
		std::string key;
		std::string ms;

		sstream >> key;
		sstream >> ms;

		game->soundManager()->fadeInTrack(key, atoi(ms.c_str()));

		Ready();
		return;
	}

	if (!command.compare("FadeOutTrack"))
	{
		std::string ms;
		
		sstream >> ms;

		game->soundManager()->fadeOutTrack(atoi(ms.c_str()));

		Ready();
		return;
	}

	if (!command.compare("FreeImage"))
	{
		std::string key;
		
		sstream >> key;

		game->imageCache()->freeTexture(key);

		Ready();
		return;
	}

	if (!command.compare("FreeSound"))
	{
		std::string key;

		sstream >> key;

		game->soundManager()->freeSound(key);

		Ready();
		return;
	}

	if (!command.compare("FreeTrack"))
	{
		std::string key;

		sstream >> key;

		game->soundManager()->freeTrack(key);

		Ready();
		return;
	}

	if (!command.compare("FreeSurface"))
	{
		std::string key;

		sstream >> key;

		delete surfaces[key];
		surfaces.erase(key);

		Ready();
		return;
	}

	if (!command.compare("LoadStyle"))
	{
		std::string key;
		std::string path;

		sstream >> key;
		sstream >> path;

		styles[key] = loadStyle(path.c_str());

		Ready();
		return;
	}

	if (!command.compare("FreeStyle"))
	{
		std::string key;

		sstream >> key;

		delete styles[key];

		styles.erase(key);

		Ready();
		return;
	}

	if (!command.compare("MakeDialog"))
	{
		std::string textKey;
		std::string style;
		std::string x;
		std::string y;
		std::string width;
		std::string height;
		std::string ms;
		std::string stepScriptPath;

		sstream >> textKey;
		sstream >> style;
		sstream >> x;
		sstream >> y;
		sstream >> width;
		sstream >> height;
		sstream >> ms;
		sstream >> stepScriptPath;

		ascii::Surface* bubble = makeBubble(styles[style], atoi(width.c_str()), atoi(height.c_str()));

		game->graphics()->blitSurface(bubble, atoi(x.c_str()), atoi(y.c_str()));

		scrollingText = true;
		textToScroll = text[textKey];
		scrollStream = std::stringstream(textToScroll);
		textScrolled = "";
		revealedChars = 0;
		charsToReveal = 0;
		textElapsedMS = 0;
		textMS = atoi(ms.c_str());
		textRect = ascii::Rectangle(atoi(x.c_str()), atoi(y.c_str()), atoi(width.c_str()), atoi(height.c_str()));
		textStepScript = stepScriptPath;
		textStyle = styles[style];
	}

	if (!command.compare("LoadText"))
	{
		std::string path;

		sstream >> path;

		loadText(path.c_str());

		Ready();
		return;
	}

	if (!command.compare("ClearText"))
	{
		text.clear();

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

void RunStepScript(const char* path)
{
	inStepScript = true;

	std::ifstream file(path);

	std::vector<std::string> lines;

	std::string line;

	while (std::getline(file, line))
	{
		lines.push_back(line);
	}

	for (auto it = lines.begin(); it != lines.end(); ++it)
	{
		//iterate through, immediately executing these lines
		if (it->size() > 0)
		{
			RunLine(it->c_str());
		}
	}

	file.close();

	inStepScript = false;
}

void StartChapter(int chapter)
{

}

void LoadContent(ascii::ImageCache* cache, ascii::SoundManager* soundManager)
{
	loadOpenChapters();

	RunScript("Data/menustart.wsp");
}

void Update(ascii::Game* game, int deltaMS)
{
	if (msToWait > 0)
	{
		//handle Wait calls
		msToWait -= deltaMS;

		if (msToWait <= 0)
		{
			msToWait = 0;
			Ready();
		}
	}

	if (tweening)
	{
		//handle TweenSurface calls
		tweenElapsedMS += deltaMS;

		if (tweenElapsedMS >= tweenMS)
		{
			tweenElapsedMS -= tweenMS;

			tweenX += tweenStepX;
			tweenY += tweenStepY;

			//step
			game->graphics()->clear();

			if (tweenStepScript.length() > 0)
			{
				RunStepScript(tweenStepScript.c_str());
			}

			game->graphics()->blitSurface(tweeningSurface, tweenX, tweenY);
			game->graphics()->update();

			if (tweenX == tweenDestX && tweenY == tweenDestY)
			{
				tweening = false;
				Ready(); //the tween is over
			}
		}
	}

	if (scrollingText)
	{
		textElapsedMS += deltaMS;

		if (textMS < 0)
		{
			//instantly show all
			textScrolled = textToScroll;

			addText(game->graphics(), textScrolled.c_str(), textStyle, textRect);
			game->graphics()->update();

			RunStepScript(textStepScript.c_str());

			scrollingText = false;
			Ready(); //done scrolling
		}

		else if (textElapsedMS > textMS)
		{
			textElapsedMS -= textMS;
			
			//step
			if (revealedChars == charsToReveal)
			{

				std::string nextWord;
				scrollStream >> nextWord;

				if (charsToReveal != 0)
				{
					textScrolled = textScrolled.append(" "); //then it follows a word and needs a space
				}
				textScrolled = textScrolled.append(nextWord);

				revealedChars = 0;
				charsToReveal = nextWord.length();

			}
			else
			{
				++revealedChars;
			}

			RunStepScript(textStepScript.c_str());

			if (!textScrolled.compare(textToScroll) && revealedChars == charsToReveal)
			{
				scrollingText = false;
				Ready(); //done scrolling
			}

			addText(game->graphics(), textScrolled.c_str(), textStyle, textRect);

			ascii::Rectangle trueRect = ascii::Rectangle(
				textRect.x + textStyle->xPadding, textRect.y + textStyle->yPadding, 
				textRect.width - textStyle->xPadding * 2, textRect.height - textStyle->yPadding * 2);

			int endX = game->graphics()->measureStringMultilineX(textScrolled.c_str(), trueRect);
			int endY = game->graphics()->measureStringMultilineY(textScrolled.c_str(), trueRect);
			int cutoffX = endX - charsToReveal + revealedChars;
			for (int x = endX; x >= cutoffX && cutoffX != endX; --x)
			{
				game->graphics()->setCharacter(x, endY, ' ');
			}

			game->graphics()->update();
		}
	}

	if (mainMenu)
	{
		int selectedY = kMainMenuY + kFirstEntryY + selectedEntry;
		if (selectedEntry > 0)
		{
			++selectedY;
		}
		if (selectedEntry > 8)
		{
			++selectedY;
		}

		for (int y = kMainMenuY + kFirstEntryY; y < kMainMenuY + kMainMenuHeight; ++y)
		{
			for (int x = kMainMenuX; x < kMainMenuX + kMainMenuWidth; ++x)
			{
				ascii::Color color = (y == selectedY ? ascii::Color::Red : ascii::Color::Black);

				int levelY = y - kMainMenuY - kFirstEntryY - 1;

				if (!chapterOpen[levelY - 1])
				{
					color = ascii::Color::Gray;
				}

				game->graphics()->setCharacterColor(x, y, color);

				if (levelY > 0 && levelY < 9)
				{
					//draw the >
					char character = (y == selectedY ? '>' : ' ');

					game->graphics()->setCharacter(kMainMenuX + kFirstEntryX, y, character);
				}
			}
		}

		game->graphics()->update();
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
	if (waitingForInput)
	{
		if (input.anyKeyPressed() || input.mouseButtonClicked(ascii::LEFT))
		{
			waitingForInput = false;
			Ready();
		}
	}

	if (mainMenu)
	{
		int selectedX = input.mouseX() / game->graphics()->charWidth();
		int selectedY = input.mouseY() / game->graphics()->charHeight();

		if (selectedX > kMainMenuX && selectedX < kMainMenuX + kMainMenuWidth)
		{
			int selected = selectedY - kMainMenuY - kFirstEntryY;

			if (selected > 0)
			{
				--selected;
			}

			if (selected > 8)
			{
				--selected;
			}

			if (selected >= 0 && selected <= 9)
			{
				int levelEntry = selected - 1;
				if (levelEntry >= 0 && levelEntry < 8)
				{
					if (chapterOpen[levelEntry])
					{
						selectedEntry = selected;
					}
					else
					{
						selectedEntry = -1;
					}
				}
				else
				{
					selectedEntry = selected;
				}
			}
			else
			{
				selectedEntry = -1;
			}
		}
		else
		{
			selectedEntry = -1;
		}

		if (input.mouseButtonClicked(ascii::LEFT) && selectedEntry != -1)
		{
			if (selectedEntry == 0)
			{
				RunScript("Data/introduction.wsp");
			}
			else if (selectedEntry == 9)
			{
				RunScript("Data/credits.wsp");
			}
			else
			{
				StartChapter(selectedEntry);
			}

			Ready();
			mainMenu = false;
		}
	}
}

void Draw(ascii::Graphics& graphics)
{
	if (updateGraphics)
	{
		graphics.update();
		updateGraphics = false;
		Ready();
	}
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