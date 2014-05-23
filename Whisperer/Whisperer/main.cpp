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
bool waitingForEnter = false;
bool keyPressed = false;

const int kLettersPerKey = 2;

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

bool exploring = false;
ascii::Surface* mapSurface;
ascii::Surface* playerSurface;
std::string footstepGroup;

int playerX = 0;
int playerY = 0;
float playerFloatX = 0.0f;
float playerFloatY = 0.0f;
bool goingFast = false;
const float kPlayerSpeed = 2.0f;
const float kPlayerFastSpeed = 4.0f;

bool movingLeft = false;
bool movingRight = false;
bool movingUp = false;
bool movingDown = false;

enum Direction
{
	UP,
	RIGHT,
	LEFT,
	DOWN
};

Direction facing = DOWN;
bool dark = false;

void StartChapter(int chapter);

void LogFPS(int deltaMS)
{
	std::cout << "FPS: " << 1.0f / ((float)deltaMS / 1000.0f);
}

int toMS(std::string speed)
{
	if (!speed.compare("Slow"))
	{
		return 333;
	}
	if (!speed.compare("Normal"))
	{
		return 40;
	}
	if (!speed.compare("Fast"))
	{
		return 20;
	}

	if (!speed.compare("Instant"))
	{
		return -1;
	}

	if (!speed.compare("Typed"))
	{
		return 0;
	}

	return -1;
}

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
void RunStepScript(const char* path);

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

void tweenSurface(ascii::Surface* surface, int x1, int y1, int x2, int y2, int ms , std::string stepScript)
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

	if (!command.compare("LoadGroupSound"))
	{
		std::string group;
		std::string path;

		sstream >> group;
		sstream >> path;

		game->soundManager()->loadGroupSound(group, path.c_str());

		Ready();
		return;
	}
	
	if (!command.compare("FreeSoundGroup"))
	{
		std::string group;

		sstream >> group;

		game->soundManager()->freeSoundGroup(group);

		Ready();
		return;
	}

	if (!command.compare("PlaySoundGroup"))
	{
		std::string group;

		sstream >> group;

		game->soundManager()->playSoundGroup(group);

		Ready();
		return;
	}

	if (!command.compare("LoopSoundGroup"))
	{
		std::string group;

		sstream >> group;

		game->soundManager()->loopSoundGroup(group);

		Ready();
		return;
	}

	if (!command.compare("StopLoopingGroup"))
	{
		std::string group;

		if (sstream >> group)
		{
			game->soundManager()->stopLoopingGroup(group);
		}
		else
		{
			game->soundManager()->stopLoopingGroup();
		}

		Ready();
		return;
	}

	if (!command.compare("StartChapter"))
	{
		std::string chapter;

		sstream >> chapter;

		std::stringstream cmdstream;

		cmdstream << "RunScript Data/Chapter" << chapter << "/ch" << chapter << ".wsp";

		char* cstr = new char [cmdstream.str().length()+1];
		std::strcpy (cstr, cmdstream.str().c_str());

		linesToExecute.push_back(cstr);
		Ready();
		return;
	}

	if (!command.compare("Explore"))
	{
		std::string mapPath;
		std::string playerPath;
		std::string darkStr;
		std::string fastStr;

		sstream >> mapPath;
		sstream >> playerPath;
		sstream >> footstepGroup;
		sstream >> darkStr;
		sstream >> fastStr;

		exploring = true;
		mapSurface = ascii::Surface::FromFile(mapPath.c_str());
		playerSurface = ascii::Surface::FromFile(playerPath.c_str());

		for (int x = 0; x < mapSurface->width(); ++x)
		{
			for (int y = 0; y < mapSurface->height(); ++y)
			{
				if (!mapSurface->getSpecialInfo(x, y).compare("Start"))
				{
					playerX = x;
					playerY = 1 + y - playerSurface->height();
					playerFloatX = x;
					playerFloatY = playerY;
				}
			}
		}

		dark = !darkStr.compare("Dark");

		goingFast = !fastStr.compare("Fast");
	}

	if (!command.compare("StopExploring"))
	{
		exploring = false;

		delete mapSurface;
		delete playerSurface;

		Ready();
		return;
	}

	if (!command.compare("PauseExploring"))
	{
		exploring = false;
		Ready();
		return;
	}

	if (!command.compare("ResumeExploring"))
	{
		exploring = true;
		readyToContinue = false;
	}

	if (!command.compare("UnlockChapter"))
	{
		std::string chapter;

		sstream >> chapter;

		chapterOpen[atoi(chapter.c_str()) - 1] = true;

		writeOpenChapters();

		Ready();
		return;
	}

	if (!command.compare("AwaitInput"))
	{
		waitingForInput = true;
	}
	if (!command.compare("AwaitEnter"))
	{
		waitingForEnter = true;
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

		if (!styles[key])
		{
			styles[key] = loadStyle(path.c_str());
		}

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
		std::string speed;
		std::string stepScriptPath;

		sstream >> textKey;
		sstream >> style;
		sstream >> x;
		sstream >> y;
		sstream >> width;
		sstream >> height;
		sstream >> speed;
		sstream >> stepScriptPath;

		if (styles[style]->bubble)
		{
			ascii::Surface* bubble = makeBubble(styles[style], atoi(width.c_str()), atoi(height.c_str()));

			game->graphics()->blitSurface(bubble, atoi(x.c_str()), atoi(y.c_str()));
		}

		scrollingText = true;
		textToScroll = text[textKey];
		scrollStream = std::stringstream(textToScroll);
		textScrolled = "";
		revealedChars = 0;
		charsToReveal = 0;
		textElapsedMS = 0;
		textMS = toMS(speed);
		textRect = ascii::Rectangle(atoi(x.c_str()), atoi(y.c_str()), atoi(width.c_str()), atoi(height.c_str()));
		textStepScript = stepScriptPath;
		textStyle = styles[style];

		if (textMS < 0)
		{
			//instantly show all
			textScrolled = textToScroll;

			addText(game->graphics(), textScrolled.c_str(), textStyle, textRect);

			RunStepScript(textStepScript.c_str());

			scrollingText = false;
			Ready(); //done scrolling
			return;
		}
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



	std::cout << "ERROR: Invalid script command used: " << command << std::endl;
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

void RunEventScript(const char* path)
{
	std::ifstream file(path);

	std::vector<std::string> lines;

	std::string line;

	RunLine("PauseExploring");

	while (std::getline(file, line))
	{
		lines.push_back(line);
	}

	lines.push_back("ResumeExploring");

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
	RunScript("Data/startchapter.wsp");

	std::stringstream sstream;

	sstream << "RunScript Data/Chapter" << chapter << "/ch" << chapter << ".wsp";

	char* cstr = new char [sstream.str().length()+1];
	std::strcpy (cstr, sstream.str().c_str());

	linesToExecute.push_back(cstr);
}

void LoadContent(ascii::ImageCache* cache, ascii::SoundManager* soundManager)
{
	loadOpenChapters();

	RunScript("Data/menustart.wsp");
}

void stepDialog()
{
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

		if (!textScrolled.compare(textToScroll) && revealedChars == charsToReveal)
		{
			scrollingText = false;
			Ready(); //done scrolling
		}


	}
	else
	{
		++revealedChars;

		if (!textScrolled.compare(textToScroll) && revealedChars == charsToReveal)
		{
			scrollingText = false;
			Ready(); //done scrolling
		}

		if (textMS == 0 && revealedChars == charsToReveal)
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
	}

	
	RunStepScript(textStepScript.c_str());

	addText(game->graphics(), textScrolled.c_str(), textStyle, textRect);

	ascii::Rectangle trueRect = ascii::Rectangle(
		textRect.x + textStyle->xPadding, textRect.y + textStyle->yPadding, 
		textRect.width - textStyle->xPadding * 2, textRect.height - textStyle->yPadding * 2);

	int endX = game->graphics()->stringMultilineEndX(textScrolled.c_str(), trueRect);
	int endY = trueRect.y + game->graphics()->measureStringMultilineY(textScrolled.c_str(), trueRect) - 1;
	int cutoffX = endX - charsToReveal + revealedChars;
	for (int x = endX; x >= cutoffX && cutoffX != endX; --x)
	{
		game->graphics()->setCharacter(x, endY, ' ');
	}
}

void Update(ascii::Game* game, int deltaMS)
{
	//LogFPS(deltaMS);

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

	if (exploring)
	{

		float elapsedSec = (float)deltaMS / 1000.0f;

		float dx = 0;
		float dy = 0;

		if (movingLeft)
			dx = -1;

		if (movingRight)
			dx = 1;

		if (movingUp)
			dy = -1;

		if (movingDown)
			dy = 1;
		
		float mag = sqrt(pow(dx, 2) + pow(dy, 2));

		if (mag > 0)
		{
			dx /= mag;
			dy /= mag;
		}

		float speed = (goingFast ? kPlayerFastSpeed : kPlayerSpeed);

		playerFloatX += dx * speed * elapsedSec;
		playerFloatY += dy * speed * elapsedSec;


		int newX = (int)playerFloatX;
		int newY = (int)playerFloatY;

		int y = newY + playerSurface->height() - 1;
		
		std::string specInfo = mapSurface->getSpecialInfo(newX, y);

		if (specInfo.length() > 0)
		{
			if (!specInfo.compare("Wall"))
			{
				playerFloatX = playerX;
				playerFloatY = playerY;
				return;
			}
			else if (!specInfo.compare("Stop"))
			{
				//end the exploration
				RunLine("StopExploring");

				exploring = false;
				return;
			}
			else
			{
				
				//it's a script! Run the script
				RunEventScript(specInfo.c_str());

				//and now remove it from the level
				for (int x = 0; x < mapSurface->width(); ++x)
				{
					for (int y = 0; y < mapSurface->height(); ++y)
					{
						std::string spec = mapSurface->getSpecialInfo(x, y);

						if (!spec.compare(specInfo))
						{
							mapSurface->setSpecialInfo(x, y, "");
						}
					}
				}
			}
		}

		if (newX != playerX || newY != playerY)
		{
			if (footstepGroup.compare(""))
			{
				game->soundManager()->playSoundGroup(footstepGroup);
			}
			playerX = newX;
			playerY = newY;
		}
	}

	if (tweening)
	{
		//handle TweenSurface calls
		tweenElapsedMS += deltaMS;

		if (tweenElapsedMS >= tweenMS)
		{
			tweenElapsedMS -= tweenMS;

			if (tweenX == tweenDestX && tweenY == tweenDestY)
			{
				tweening = false;

				game->graphics()->clear();

				if (tweenStepScript.length() > 0)
				{
					RunStepScript(tweenStepScript.c_str());
				}

				game->graphics()->blitSurface(tweeningSurface, tweenX, tweenY);
				game->graphics()->update();

				Ready(); //the tween is over
				return;
			}

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
			return;
		}

		else if (textMS == 0)
		{
			if (keyPressed)
			{
				keyPressed = false;

				for (int i = 0; i < kLettersPerKey && !(!textScrolled.compare(textToScroll) && (charsToReveal == revealedChars)); ++i)
				{
					stepDialog();
				}
			}

			game->graphics()->update();
		}

		else if (textElapsedMS > textMS)
		{
			textElapsedMS -= textMS;


			//step
			stepDialog();

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

void logMousePos(ascii::Input& input)
{
	if (input.mouseButtonClicked(ascii::LEFT))
	{
		int selectedX = input.mouseX() / game->graphics()->charWidth();
		int selectedY = input.mouseY() / game->graphics()->charHeight();

		std::cout << "Mouse Pos: " << selectedX << ", " << selectedY << std::endl;
	}
}

void handleSkip(ascii::Input& input)
{
	if (input.mouseButtonClicked(ascii::LEFT) || input.anyKeyPressed())
	{
		if (scrollingText && textMS != 0)
		{
			textMS = -1;
		}

		if (msToWait > 0)
		{
			msToWait = 1;
		}

		if (tweening)
		{
			tweenX = tweenDestX;
			tweenY = tweenDestY;
		}

		if (exploring && !input.anyKeyPressed())
		{
			exploring = false;

			delete mapSurface;
			delete playerSurface;

			Ready();
			return;
		}
	}
}

void HandleInput(ascii::Game* game, ascii::Input& input)
{
	logMousePos(input);

	if (waitingForInput)
	{
		if (input.anyKeyPressed() || input.mouseButtonClicked(ascii::LEFT))
		{
			waitingForInput = false;
			Ready();
		}

		return;
	}

	if (waitingForEnter)
	{
		if (input.wasKeyPressed(SDLK_RETURN))
		{
			waitingForEnter = false;
			Ready();
		}

		return;
	}

	keyPressed = input.anyKeyPressed();

	handleSkip(input);

	movingLeft = false;
	movingRight = false;
	movingUp = false;
	movingDown = false;

	if (exploring)
	{
		if (input.isKeyHeld(SDLK_LEFT))
		{
			movingLeft = true;
			facing = LEFT;
		} 
		else if (input.isKeyHeld(SDLK_RIGHT))
		{
			movingRight = true;
			facing = RIGHT;
		}
		else if (input.isKeyHeld(SDLK_UP))
		{
			movingUp = true;
			facing = UP;
		} 
		else if (input.isKeyHeld(SDLK_DOWN))
		{
			movingDown = true;
			facing = DOWN;
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

	if (exploring)
	{
		graphics.clear();
		graphics.blitSurface(mapSurface, 0, 0);
		graphics.blitSurface(playerSurface, playerX, playerY);

		if (dark)
		{
			graphics.clearTransparent();
			
			const int kViewDistX = 16;
			const int kViewDistY = 10;
			const int kViewIncDistX = 2;
			const int kViewIncDistY = 1 ;

			switch (facing)
			{
			case LEFT:
				for (int y = playerY; y < playerY + playerSurface->height(); ++y)
				{
					graphics.setCellOpacity(playerX, y, true);
				}
				for (int x = playerX; x > playerX - kViewDistX; --x)
				{
					int height = 1 + (playerX - x) / kViewIncDistX;
					for (int y = playerY - height / 2; y <= playerY + height / 2; ++y)
					{
						graphics.setCellOpacity(x, y, true);
					}
				}
				break;
			case RIGHT:
				for (int y = playerY; y < playerY + playerSurface->height(); ++y)
				{
					graphics.setCellOpacity(playerX, y, true);
				}
				for (int x = playerX; x < playerX + kViewDistX; ++x)
				{
					int height = 1 + (x - playerX) / kViewIncDistX;
					for (int y = playerY - height / 2; y <= playerY + height / 2; ++y)
					{
						graphics.setCellOpacity(x, y, true);
					}
				}
				break;
			case UP:
				for (int y = playerY; y < playerY + playerSurface->height(); ++y)
				{
					graphics.setCellOpacity(playerX, y, true);
				}
				for (int y = playerY; y > playerY - kViewDistY; --y)
				{
					int width = 1 + (playerY - y) / kViewIncDistY;
					for (int x = playerX - width / 2; x <= playerX + width / 2; ++x)
					{
						graphics.setCellOpacity(x, y, true);
					}
				}
				break;
			case DOWN:
				for (int y = playerY; y < playerY + kViewDistY; ++y)
				{
					int width = 1 + (y - playerY) / kViewIncDistY;
					for (int x = playerX - width / 2; x <= playerX + width / 2; ++x)
					{
						graphics.setCellOpacity(x, y, true);
					}
				}
				break;
			}
		}

		graphics.update();
	}
}

int main(int argc, char** argv)
{
	game = new ascii::Game(kWindowTitle, kWindowWidth, kWindowHeight, LoadContent, Update, HandleInput, Draw);

	game->Run();
	
	return 0;
}