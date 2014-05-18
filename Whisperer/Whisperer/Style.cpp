#include "Style.h"

#include <fstream>
#include <sstream>

Style::~Style()
{
	delete topLeft;
	delete top;
	delete topRight;
	
	delete left;
	delete center;
	delete right;

	delete bottomLeft;
	delete bottom;
	delete bottomRight;
}

Style* loadStyle(const char* path)
{
	std::ifstream file(path);

	std::string bubblePath;
	std::string colorInfo;

	std::getline(file, bubblePath);
	std::getline(file, colorInfo);

	Style* style = new Style();

	if (bubblePath.length() > 0)
	{
		style->bubble = true;

		ascii::Surface* bubbleSurface = ascii::Surface::FromFile(bubblePath.c_str());

		int cornerWidth = (bubbleSurface->width() - 1) / 2;
		int cornerHeight = (bubbleSurface->height() - 1) / 2;

		style->topLeft = new ascii::Surface(cornerWidth, cornerHeight);
		style->topLeft->copySurface(bubbleSurface, ascii::Rectangle(0, 0, cornerWidth, cornerHeight), 0, 0);
		style->top = new ascii::Surface(1, cornerHeight);
		style->top->copySurface(bubbleSurface, ascii::Rectangle(cornerWidth, 0, 1, cornerHeight), 0, 0);
		style->topRight = new ascii::Surface(cornerWidth, cornerHeight);
		style->topRight->copySurface(bubbleSurface, ascii::Rectangle(cornerWidth + 1, 0, cornerWidth, cornerHeight), 0, 0);

		style->left = new ascii::Surface(cornerWidth, 1);
		style->left->copySurface(bubbleSurface, ascii::Rectangle(0, cornerHeight, cornerWidth, 1), 0, 0);
		style->center = new ascii::Surface(1, 1);
		style->center->copySurface(bubbleSurface, ascii::Rectangle(cornerWidth, cornerHeight, 1, 1), 0, 0);
		style->right = new ascii::Surface(cornerWidth, 1);
		style->right->copySurface(bubbleSurface, ascii::Rectangle(cornerWidth + 1, cornerHeight, cornerWidth, 1), 0, 0);

		style->bottomLeft = new ascii::Surface(cornerWidth, cornerHeight);
		style->bottomLeft->copySurface(bubbleSurface, ascii::Rectangle(0, cornerHeight + 1, cornerWidth, cornerHeight), 0, 0);
		style->bottom = new ascii::Surface(1, cornerHeight);
		style->bottom->copySurface(bubbleSurface, ascii::Rectangle(cornerWidth, cornerHeight + 1, 1, cornerHeight), 0, 0);
		style->bottomRight = new ascii::Surface(cornerWidth, cornerHeight);
		style->bottomRight->copySurface(bubbleSurface, ascii::Rectangle(cornerWidth + 1, cornerHeight + 1, cornerWidth, cornerHeight), 0, 0);

		delete bubbleSurface;

	}
	else
	{
		style->bubble = false;
	}

	std::stringstream cstream(colorInfo);

	std::string r;
	std::string g;
	std::string b;

	cstream >> r;
	cstream >> g;
	cstream >> b;

	style->textColor = ascii::Color(atoi(r.c_str()), atoi(g.c_str()), atoi(b.c_str()));

	std::string paddingInfo;

	std::getline(file, paddingInfo);

	std::stringstream pstream(paddingInfo);

	std::string x;
	std::string y;
	pstream >> x;
	pstream >> y;

	style->xPadding = atoi(x.c_str());
	style->yPadding = atoi(y.c_str());

	file.close();

	return style;
}

ascii::Surface* makeBubble(Style* style, int width, int height)
{
	ascii::Surface* surface = new ascii::Surface(width, height);
	
	int cornerWidth = style->topLeft->width();
	int cornerHeight = style->topLeft->height();
	int centerWidth = width - cornerWidth * 2;
	int centerHeight = height - cornerHeight * 2;

	surface->copySurface(style->topLeft, 0, 0);
	for (int x = cornerWidth; x < width - cornerWidth; ++x)
	{
		surface->copySurface(style->top, x, 0);
	}
	surface->copySurface(style->topRight, width - cornerWidth, 0);

	for (int y = cornerHeight; y < height - cornerHeight; ++y)
	{
		surface->copySurface(style->left, 0, y);
		for (int x = cornerWidth; x < width - cornerWidth; ++x)
		{
			surface->copySurface(style->center, x, y);
		}
		surface->copySurface(style->right, width - cornerWidth, y);
	}

	surface->copySurface(style->bottomLeft, 0, height - cornerHeight);
	for (int x = cornerWidth; x < width - cornerWidth; ++x)
	{
		surface->copySurface(style->bottom, x, height - cornerHeight);
	}
	surface->copySurface(style->bottomRight, width - cornerWidth, height - cornerHeight);

	return surface;
}

void addText(ascii::Surface* surface, const char* text, Style* style, ascii::Rectangle bounds)
{
	surface->blitStringMultiline(text, style->textColor, ascii::Rectangle(bounds.x + style->xPadding, bounds.y + style->yPadding, bounds.width - style->xPadding * 2, bounds.height - style->yPadding * 2));
}