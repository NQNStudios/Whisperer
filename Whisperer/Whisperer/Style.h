#pragma once

#include "Surface.h"

struct Style
{
	~Style();

	ascii::Surface* topLeft;
	ascii::Surface* top;
	ascii::Surface* topRight;
	ascii::Surface* left;
	ascii::Surface* center;
	ascii::Surface* right;
	ascii::Surface* bottomLeft;
	ascii::Surface* bottom;
	ascii::Surface* bottomRight;

	ascii::Color textColor;

	int xPadding;
	int yPadding;
	
	bool bubble;
};

Style* loadStyle(const char* path);
ascii::Surface* makeBubble(Style* style, int width, int height);

void addText(ascii::Surface* surface, const char* text, Style* style, ascii::Rectangle bounds);