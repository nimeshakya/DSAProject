#include <iostream>
#include <string>
#include <sstream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <random>

// window resolution
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SNAKE_WIDTH = 20;
const int FOOD_WIDTH = 20;

// in game texture
class GameTexture
{
public:
	GameTexture(); // initialize members
	~GameTexture(); // deallocate memory
	// free memory
	void free();
	bool loadTextTextureFromFile(std::string textTexture, SDL_Color textColor);
	// renders the texture on screen
	void render(int x, int y);

	// get dimensions
	int getWidth();
	int getHeight();
private:
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
};

// node class (body part)
class BodyPart
{
public:
	SDL_Rect bodyRect;
	BodyPart* nextBody;

	BodyPart()
	{
		this->bodyRect = { 0, 0, SNAKE_WIDTH, SNAKE_WIDTH };
		this->nextBody = NULL;
	}

	BodyPart(int x, int y)
	{
		this->bodyRect = { x, y, SNAKE_WIDTH, SNAKE_WIDTH };
		this->nextBody = NULL;
	}
};

// linked list class (singly / snake)
class Snake
{
public:
	Snake();

	// insert node at end of list
	void insertNode(SDL_Point point);

	// handle user inputs
	void handleInput(SDL_Event e, bool* canInput);

	// updatePos
	void updatePosition();

	// check boundary collision
	bool boundCollision();

	// check if body collision
	bool selfCollision();

	// function to render the bodyParts
	void render();

	// clears memory of body parts
	void clearBody();

	// checks if a position is part of body
	bool isPositionOfPart(int x, int y);

	void countBodyPart();

	// reset snake
	void reset();

	BodyPart* head = NULL;

private:
	int xDir, yDir;
};

class Food
{
public:
	Food();
	void renderFood();
	void updateFoodPos(int x, int y);
	SDL_Rect foodRect;
};

// initialize SDl and subsystems
bool init();
// loads all media
bool loadmedia();
// shuts down SDl and subsystems
void close();

/* GLOBAL VARIABLES */
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
GameTexture gGameOverTextPrompt;
GameTexture gRestartGuidePrompt;
GameTexture gScoreTexture;

// Game Texture Functions

GameTexture::GameTexture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

GameTexture::~GameTexture()
{
	free();
}

void GameTexture::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

bool GameTexture::loadTextTextureFromFile(std::string textTexture, SDL_Color textColor)
{
	free();
	// texture surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textTexture.c_str(), textColor);
	if (textSurface == NULL)
	{
		std::cout << "Unable to render text surface! SDL_ttf ERROR: " << TTF_GetError() << '\n';
	}
	else
	{
		// create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			std::cout << "Unable to create texture from rendered text! SDL ERROR: " << SDL_GetError() << '\n';
		}
		else
		{
			// get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		// get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	return (mTexture != NULL);
}

void GameTexture::render(int x, int y)
{
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

int GameTexture::getWidth()
{
	return mWidth;
}

int GameTexture::getHeight()
{
	return mHeight;
}

// Snake Functions

Snake::Snake()
{
	xDir = 1;
	yDir = 0;
}

void Snake::insertNode(SDL_Point point)
{
	BodyPart* newPart = new BodyPart(point.x, point.y);

	// assign to head
	if (head == NULL)
	{
		head = newPart;
		return;
	}
	// traverse till end
	BodyPart* temp = head;
	while (temp->nextBody != NULL)
	{
		temp = temp->nextBody;
	}

	temp->nextBody = newPart;
}

void Snake::handleInput(SDL_Event e, bool* canInput)
{
	if (e.type == SDL_KEYDOWN && *canInput)
	{
		*canInput = false;
		switch (e.key.keysym.sym)
		{
		case SDLK_UP:
			if (yDir == 1)
			{
		
				break;
			}
			else
			{
				yDir = -1;
				xDir = 0;
			}
			break;
		case SDLK_DOWN:
			if (yDir == -1)
			{
				break;
			}
			else
			{
				yDir = 1;
				xDir = 0;
			}
			break;
		case SDLK_LEFT:
			if (xDir == 1)
			{
				break;
			}
			else
			{
				xDir = -1;
				yDir = 0;
			}
			break;
		case SDLK_RIGHT:
			if (xDir == -1)
			{
				break;
			}
			else
			{
				xDir = 1;
				yDir = 0;
			}
			break;
		}
	}
}

void Snake::updatePosition()
{
	BodyPart* pthis = head;
	SDL_Rect thisBodyRect = pthis->bodyRect;
	SDL_Rect nextBodyRect;
	while (pthis->nextBody != NULL)
	{
		nextBodyRect = pthis->nextBody->bodyRect;
		pthis->nextBody->bodyRect = thisBodyRect;
		thisBodyRect = nextBodyRect;
		pthis = pthis->nextBody;
	}
	head->bodyRect.x += xDir * SNAKE_WIDTH;
	head->bodyRect.y += yDir * SNAKE_WIDTH;
}

bool Snake::boundCollision()
{
	bool collided{ false };
	int winBoundLeft{ 0 };
	int winBoundRight{ SCREEN_WIDTH };
	int winBoundTop{ 0 };
	int winBoundBot{ SCREEN_HEIGHT };

	int headBoundLeft{ head->bodyRect.x };
	int headBoundRight{ head->bodyRect.x + head->bodyRect.w };
	int headBoundTop{ head->bodyRect.y };
	int headBoundBot{ head->bodyRect.y + head->bodyRect.h };

	if (headBoundLeft < winBoundLeft)
	{
		collided = true;
	}
	if (headBoundRight > winBoundRight)
	{
		collided = true;
	}
	if (headBoundTop < winBoundTop)
	{
		collided = true;
	}
	if (headBoundBot > winBoundBot)
	{
		collided = true;
	}
	return collided;
}

bool Snake::selfCollision()
{
	bool collided{ false };
	BodyPart* pthis = head->nextBody;
	while (pthis->nextBody != NULL)
	{
		if (head->bodyRect.x == pthis->bodyRect.x && head->bodyRect.y == pthis->bodyRect.y)
		{
			collided = true;
		}
		pthis = pthis->nextBody;
	}
	return collided;
}

void Snake::render()
{
	BodyPart* temp = head;

	// check for empty snake
	if (head == NULL)
	{
		std::cout << "No Snake Here!!\n";
		return;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	// traverse the list
	while (temp != NULL)
	{
		SDL_RenderDrawRect(gRenderer, &temp->bodyRect);
		temp = temp->nextBody;
	}
}

void Snake::clearBody()
{
	while (head)
	{
		BodyPart* ptemp = head;
		head = head->nextBody;
		delete ptemp;
	}
}

bool Snake::isPositionOfPart(int x, int y)
{
	bool isPart{ false };
	BodyPart* pthis = head;
	while (pthis->nextBody != NULL)
	{
		if (pthis->bodyRect.x == x && pthis->bodyRect.y == y)
		{
			isPart = true;
			break;
		}
		pthis = pthis->nextBody;
	}
	return isPart;
}

void Snake::reset()
{
	xDir = 1;
	yDir = 0;
}

void Snake::countBodyPart()
{
	int count = 0;
	BodyPart* pthis = head;
	while(pthis->nextBody != NULL)
	{
		count++;
		pthis = pthis->nextBody;
	}
	count++;
}

// food functions

Food::Food()
{
	foodRect = { 0, 0, FOOD_WIDTH, FOOD_WIDTH };
}

void Food::renderFood()
{
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x55, 0xFF);
	SDL_RenderFillRect(gRenderer, &foodRect);
}

void Food::updateFoodPos(int x, int y)
{
	foodRect.x = x;
	foodRect.y = y;
}

bool init()
{
	bool success{ true };

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not be initialzed! SDL Error: " << SDL_GetError() << '\n';
		success = false;
	}
	else
	{
		// created a window
		gWindow = SDL_CreateWindow("Slithering Saga", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << '\n';
			success = false;
		}
		else
		{
			// created renderer in the window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << '\n';
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				// png loading
				
				// initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					std::cout << "SDL_ttf could not be initialized! SDL_ttf ERROR: " << TTF_GetError() << '\n';
					success = false;
				}
			}

		}
	}

	return success;
}

bool loadmedia()
{
	bool success{ true };

	// opens the font
	gFont = TTF_OpenFont("fonts/lazy.ttf", 32);
	if (gFont == NULL)
	{
		std::cout << "Failed to load lazy font! SDL_ttf ERROR: " << TTF_GetError() << '\n';
		success = false;
	}
	else
	{
		// load text
		SDL_Color txtColor = { 255, 255, 255, 255 };
		if (!gGameOverTextPrompt.loadTextTextureFromFile("Collided! Game OVer!", txtColor))
		{
			std::cout << "Failed to render text texture!\n";
			success = false;
		}

		if (!gRestartGuidePrompt.loadTextTextureFromFile("Press Enter to Restart!", txtColor))
		{
			std::cout << "Failed to render restart guide text texture!\n";
			success = false;
		}
	}


	return success;
}

void close()
{
	gGameOverTextPrompt.free();
	gRestartGuidePrompt.free();

	// free global font
	TTF_CloseFont(gFont);
	gFont = NULL;

	SDL_DestroyWindow(gWindow);
	SDL_DestroyRenderer(gRenderer);
	gWindow = NULL;
	gRenderer = NULL;

	// close SDL
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	if (!init())
	{
		std::cout << "Failed to initialize SDL!\n";
	}
	else
	{
		if (!loadmedia())
		{
			std::cout << "Failed to load media!\n";
		}
		else
		{
			bool quit{ false };
			SDL_Event e;

			// game time
			Uint32 timeInterval = 100; // ms
			Uint32 currTime = 0;

			// game objects
			Snake snake;
			// inserting head
			SDL_Point point = { 0, 0 };
			snake.insertNode(point);
			int initLength = 3;
			for (int i = 1; i < initLength; ++i)
			{
				snake.insertNode(point);
			}

			Food food;
			int foodSpawnX{ std::rand() % (SCREEN_WIDTH + 1) / FOOD_WIDTH * FOOD_WIDTH };
			int foodSpawnY{ std::rand() % (SCREEN_HEIGHT + 1) / FOOD_WIDTH * FOOD_WIDTH };
			food.updateFoodPos(foodSpawnX, foodSpawnY);
			bool foodconsumed{ false };

			// scoring
			int score{ 0 };
			std::stringstream scoreText;
			scoreText.str("");
			scoreText << "Score: " << score;
			SDL_Color textColor = { 255, 255, 255, 255 };

			// inputs
			bool goodToTakeInput{ true };
			bool collided{ false };

			while (!quit) // main loop / game loop
			{
				while (SDL_PollEvent(&e) != 0) // EVENT LOOP
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}

					if (!collided)
					{
						snake.handleInput(e, &goodToTakeInput);
					}
					if (e.type == SDL_KEYDOWN && collided)
					{
						if (e.key.keysym.sym == SDLK_RETURN)
						{
							collided = false;
							score = 0;
							initLength = 3;
							for (int i = 0; i < initLength; ++i)
							{
								snake.insertNode(point);
							}
							snake.reset();
							goodToTakeInput = true;
						}
					}
				}
				// update
				if (!collided)
				{
					if (snake.head->bodyRect.x == food.foodRect.x && snake.head->bodyRect.y == food.foodRect.y)
					{
						foodconsumed = true;
						SDL_Point newpoint = { snake.head->nextBody->bodyRect.x, snake.head->nextBody->bodyRect.y };
						snake.insertNode(newpoint);
						foodSpawnX = std::rand() % (SCREEN_WIDTH + 1) / FOOD_WIDTH * FOOD_WIDTH;
						foodSpawnY = std::rand() % (SCREEN_HEIGHT + 1) / FOOD_WIDTH * FOOD_WIDTH;
						while (snake.isPositionOfPart(foodSpawnX, foodSpawnY))
						{
							foodSpawnX = std::rand() % (SCREEN_WIDTH + 1) / FOOD_WIDTH * FOOD_WIDTH;
							foodSpawnY = std::rand() % (SCREEN_HEIGHT + 1) / FOOD_WIDTH * FOOD_WIDTH;
						}
						food.updateFoodPos(foodSpawnX, foodSpawnY);
						score++;
						std::cout << "Score: " << score << '\n';
					}
				}

				if (SDL_GetTicks() - currTime >= timeInterval && !collided)
				{
					snake.countBodyPart();
					currTime = SDL_GetTicks();

					snake.updatePosition();

					if (snake.boundCollision() || snake.selfCollision())
					{
						collided = true;
						std::cout << "Game Over!\n";
					}

					// now can take input
					goodToTakeInput = true;
				}

				// clear renderer
				SDL_SetRenderDrawColor(gRenderer, 0x56, 0x80, 0x52, 0xFF);
				SDL_RenderClear(gRenderer);

				if (!collided)
				{
					snake.render();
					food.renderFood();
				}
				else
				{
					snake.clearBody();
					scoreText.str("");
					scoreText << "Score: " << score;

					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderClear(gRenderer);
					// render score display
					if (!gScoreTexture.loadTextTextureFromFile(scoreText.str().c_str(), textColor))
					{
						std::cout << "Unable to load score texture\n";
					}
					gScoreTexture.render((SCREEN_WIDTH - gScoreTexture.getWidth()) / 2, (SCREEN_HEIGHT - gScoreTexture.getHeight()) / 2 - gScoreTexture.getHeight() * 3);
					gGameOverTextPrompt.render((SCREEN_WIDTH - gGameOverTextPrompt.getWidth()) / 2, (SCREEN_HEIGHT - gGameOverTextPrompt.getHeight()) / 2 - gGameOverTextPrompt.getHeight());
					gRestartGuidePrompt.render((SCREEN_WIDTH - gRestartGuidePrompt.getWidth()) / 2, (SCREEN_HEIGHT - gRestartGuidePrompt.getHeight()) / 2 + gRestartGuidePrompt.getHeight());
					// mind this
					goodToTakeInput = false;
				}

				// show rendered objects
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	close();
	return 0;
}