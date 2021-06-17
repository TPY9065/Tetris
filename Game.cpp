#include "Game.h"

Color::Color()
{
	r = 0;
	g = 0;
	b = 0;
	a = 255;
}

Color::Color(int red, int green, int blue, int alpha)
{
	r = red;
	g = green;
	b = blue;
	a = alpha;
}

Piece::Piece()
{
	shape = "";
	color = Color();
}

Piece::Piece(string s, Color c)
{
	shape = s;
	color = c;
}

bool Piece::IsEmpty()
{
	return shape.empty();
}

// static member initialization
const string Game::PIECE[7] = {
	"..x...x...x...x.", // I shape
	".....x...xxx....", // J shape
	"......x.xxx.....", // L shape
	".....xx..xx.....", // O shape
	"......xx.xx.....", // S shape
	"......x..xxx....", // T shape
	".....xx...xx...."  // Z shape
};

const Color Game::COLOR[7] = {
	Color(255, 0, 0, 255),	   // red
	Color(255, 0, 255, 255),   // magenta
	Color(255, 255, 0, 255),   // yellow
	Color(0, 255, 255, 255),   // cyan
	Color(0, 0, 255, 255),	   // blue
	Color(211, 211, 211, 255), // light gray
	Color(0, 255, 0, 255),	   // lime
};

// Constructor
Game::Game()
{
	// inititialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		cout << "Failed to initialize SDL. SDL Errors: " << SDL_GetError() << endl;
	else
	{
		// create window
		window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (window == NULL)
			cout << "Failed to initialize window. SDL Errors: " << SDL_GetError() << endl;
		else
		{
			// create renderer
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
				cout << "Failed to initialize renderer. SDL Errors: " << SDL_GetError() << endl;
		}

		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
			cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
		else
		{
			bgm = Mix_LoadMUS("./bgm.mp3");
			if (bgm == NULL)
				cout << "Failed to load bgm! SDL_mixer Error: " << Mix_GetError() << endl;
			else
				Mix_VolumeMusic(20);
		}
	}

	if (TTF_Init() < 0)
		cout << "Fail to initialize TTF. TTF Errors: " << TTF_GetError() << endl;
	else
	{
		font = TTF_OpenFont("./score.ttf", 30);
		if (font == NULL)
			cout << "Fail to load score font. TTF Errors: " << TTF_GetError() << endl;
	}

	InitGameData();

	srand(time(NULL));
}

// Destructor
Game::~Game()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	Mix_FreeMusic(bgm);
	if (screen)
	{
		SDL_FreeSurface(screen);
		screen = nullptr;
	}
	bgm = nullptr;
	window = nullptr;
	renderer = nullptr;
	font = nullptr;
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}

// check if the constructor init everything successfully
bool Game::InitSuccess()
{
	if (renderer == NULL || window == NULL || font == NULL || bgm == NULL)
		return false;
	return true;
}

void Game::InitGameData()
{
	scores = 0;
	level = 0;
	clearLinesNum = 0;
	framePerGridCell = MAX_DROP_RATE - level * DECREASE_RATE;

	InitGameBoard();

	PrintMap();
}

void Game::InitGameBoard()
{
	for (int y = 0; y < GAMEBOARD_HEIGHT; y++)
		for (int x = 0; x < GAMEBOARD_WIDTH; x++)
		{
			gameboard[y * GAMEBOARD_WIDTH + x] = (x == 0 || x == GAMEBOARD_WIDTH - 1 || y == GAMEBOARD_HEIGHT - 1) ? 'o' : (y == 4)? '-': '.';
			colorBoard[y * GAMEBOARD_WIDTH + x] = (x == 0 || x == GAMEBOARD_WIDTH - 1 || y == GAMEBOARD_HEIGHT - 1) ? Color(128, 128, 128, 255) : Color(0, 0, 0, 255);
		}
}

void Game::GetNextOrder()
{
	index.push_back(0);
	index.push_back(1);
	index.push_back(2);
	index.push_back(3);
	index.push_back(4);
	index.push_back(5);
	index.push_back(6);
	random_shuffle(index.begin(), index.end());
}

// draw the generated piece
void Game::DrawPiece(int xPos, int yPos)
{
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if (currPiece.shape[y * 4 + x] == 'x')
				DrawPieceRect(currPiece.color, xPos + x, yPos + y);
}

void Game::DrawPieceRect(Color& color, int xPos, int yPos)
{
	SDL_SetRenderDrawColor(renderer, color.r / 2, color.g / 2, color.b / 2, 255);
	SDL_Rect border{ xPos * CELL_SIZE, (yPos - BUFFER_HEIGHT) * CELL_SIZE, CELL_SIZE, CELL_SIZE };
	SDL_RenderFillRect(renderer, &border);

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect pieceRect{ xPos * CELL_SIZE + 2, (yPos - BUFFER_HEIGHT) * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4 };
	SDL_RenderFillRect(renderer, &pieceRect);
}

// update screen
void Game::Update(int xPos, int yPos)
{
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if (currPiece.shape[y * 4 + x] == 'x')
			{
				gameboard[(y + yPos) * GAMEBOARD_WIDTH + (x + xPos)] = 'x';
				colorBoard[(y + yPos) * GAMEBOARD_WIDTH + (x + xPos)] = currPiece.color;
			}
				
	PrintMap();
}

void Game::DrawGameboard()
{
	for (int y = 0; y < GAMEBOARD_HEIGHT; y++)
		for (int x = 0; x < GAMEBOARD_WIDTH; x++)
			if (gameboard[y * GAMEBOARD_WIDTH + x] == 'x' || gameboard[y * GAMEBOARD_WIDTH + x] == 'o')
				DrawPieceRect(colorBoard[y * GAMEBOARD_WIDTH + x], x, y);
}

void Game::PrintMap()
{
	cout << "Map: " << endl;
	for (int y = 0; y < GAMEBOARD_HEIGHT; y++)
	{
		for (int x = 0; x < GAMEBOARD_WIDTH; x++)
			cout << gameboard[y * GAMEBOARD_WIDTH + x];
		cout << endl;
	}
}

// get new piece
void Game::GetNewPiece()
{
	if (index.empty())
	{
		GetNextOrder();
	}

	int n;
	if (currPiece.IsEmpty())
	{
		// get current piece
		n = index.back();
		index.pop_back();
		currPiece = { PIECE[n], COLOR[n] };
	}
	else
		currPiece = nextPiece;

	// get new piece
	n = index.back();
	index.pop_back();
	nextPiece = { PIECE[n], COLOR[n] };
}

// rotate piece
void Game::Rotate(int& xPos, int yPos)
{
	int tempXPos = xPos;
	string rotatedPiece(16, 'x');
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
		{
			rotatedPiece[y * 4 + x] = currPiece.shape[y + 12 - 4 * x];
			if (rotatedPiece[y * 4 + x] == 'x' && xPos + x <= 0)
				tempXPos += 1;
			else if (rotatedPiece[y * 4 + x] == 'x' && xPos + x >= GAMEBOARD_WIDTH - 1)
				tempXPos -= 1;
		}
	if (IsValidPosition(rotatedPiece, tempXPos, yPos))
	{
		currPiece.shape = rotatedPiece;
		xPos = tempXPos;
	}
}

void Game::CheckLine(int yLine)
{
	bool isLineFinished = true;
	int linesNum = 0;
	for (int y = 0; y < 4; y++)
	{
		// check line finished
		for (int x = 1; x < GAMEBOARD_WIDTH - 1; x++)
			if (yLine + y >= 0)
				if (gameboard[(yLine + y) * GAMEBOARD_WIDTH + x] != 'x')
				{
					isLineFinished = false;
					break;
				}
				else
					isLineFinished = true;
		// current line  = previous line
		if (isLineFinished)
		{
			ClearLine(yLine + y);
			linesNum += 1;
		}
	}
	AddScore(linesNum);
}

void Game::ClearLine(int yLine)
{
	for (int y = yLine; y > 0; y--)
		for (int x = 0; x < GAMEBOARD_WIDTH; x++)
		{
			gameboard[y * GAMEBOARD_WIDTH + x] = gameboard[(y - 1) * GAMEBOARD_WIDTH + x];
			colorBoard[y * GAMEBOARD_WIDTH + x] = colorBoard[(y - 1) * GAMEBOARD_WIDTH + x];
		}
}

void Game::HardDrop(int xPos, int& yPos)
{
	int lowestY = GAMEBOARD_HEIGHT - 1;
	// find the lowest possible row of each col
	for (int x = 0; x < 4; x++)
		for (int y = 3; y >= 0; y--)
			if (currPiece.shape[y * 4 + x] == 'x')
			{
				int lowestReachableY = FindLowestY(xPos + x, yPos + y);
				if (IsValidPosition(currPiece.shape, xPos, lowestReachableY - y) && lowestReachableY < lowestY)
					lowestY = lowestReachableY - y;
			}
	yPos = lowestY;
}

void Game::AddScore(int rowClear)
{
	clearLinesNum += rowClear;
	if (IsPerfectClear())
		scores = scores + 800 * (level + 1);
	else
		switch (rowClear)
		{
		case 1:
			scores = scores + 40 * (level + 1);
			break;
		case 2:
			scores = scores + 100 * (level + 1);
			break;
		case 3:
			scores = scores + 300 * (level + 1);
			break;
		case 4:
			scores = scores + 1200 * (level + 1);
			break;
		}
	// increase level
	if (clearLinesNum >= 10 * (level + 1))
	{
		cout << "level " << level << " -> level " << level + 1 << endl;
		level += 1;
		if (level < 9)
			framePerGridCell = MAX_DROP_RATE - level * DECREASE_RATE;
		else if (level == 9)
			framePerGridCell = 6;
		else if (level >= 10 && level <= 12)
			framePerGridCell = 5;
		else if (level >= 13 && level <= 15)
			framePerGridCell = 4;
		else if (level >= 16 && level <= 18)
			framePerGridCell = 3;
		else if (level >= 19 && level <= 28)
			framePerGridCell = 2;
		else
			framePerGridCell = 1;
	}

	cout << "Scores: " << scores << endl;
}

void Game::DrawLowestPos(int xPos, int yPos)
{
	int lowestY = GAMEBOARD_HEIGHT - 1;
	// find lowest rect in each col
	for (int x = 0; x < 4; x++)
		for (int y = 3; y >= 0; y--)
			if (currPiece.shape[y * 4 + x] == 'x')
			{
				int lowestReachableY = FindLowestY(xPos + x, yPos + y);
				if (IsValidPosition(currPiece.shape, xPos, lowestReachableY - y) && lowestReachableY < lowestY)
					lowestY = lowestReachableY - y;
			}

	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if (currPiece.shape[y * 4 + x] == 'x')
			{
				SDL_SetRenderDrawColor(renderer, currPiece.color.r, currPiece.color.g, currPiece.color.b, currPiece.color.a);
				SDL_Rect pieceRect{ (xPos + x) * CELL_SIZE, (lowestY - BUFFER_HEIGHT + y) * CELL_SIZE, CELL_SIZE, CELL_SIZE };
				SDL_RenderDrawRect(renderer, &pieceRect);
			}
}

void Game::DrawNextPiece()
{
	/*DrawPieceRect(nextPiece.color, 15 + x, 8 + y);*/
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if (nextPiece.shape[y * 4 + x] == 'x')
			{
				SDL_SetRenderDrawColor(renderer, nextPiece.color.r / 2, nextPiece.color.g / 2, nextPiece.color.b / 2, 255);
				SDL_Rect border{ WINDOW_WIDTH * 5 / 8 + x * CELL_SIZE, WINDOW_HEIGHT * 4 / 8 + y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
				SDL_RenderFillRect(renderer, &border);

				SDL_SetRenderDrawColor(renderer, nextPiece.color.r, nextPiece.color.g, nextPiece.color.b, nextPiece.color.a);
				SDL_Rect pieceRect{ WINDOW_WIDTH * 5 / 8 + x * CELL_SIZE + 2, WINDOW_HEIGHT * 4 / 8 + y * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4 };
				SDL_RenderFillRect(renderer, &pieceRect);
			}
				
}

void Game::DrawScore()
{
	string text = "Scores: " + to_string(scores);
	screen = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255 });
	SDL_Texture* scoreText = SDL_CreateTextureFromSurface(renderer, screen);
	SDL_Rect scoreRect{ WINDOW_WIDTH * 5 / 8, 0, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8 };
	SDL_RenderCopy(renderer, scoreText, NULL, &scoreRect);
	SDL_DestroyTexture(scoreText);
	SDL_FreeSurface(screen);
	screen = nullptr;
}

void Game::DrawLevel()
{
	string text = "Level: " + to_string(level);
	screen = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255 });
	SDL_Texture* levelText = SDL_CreateTextureFromSurface(renderer, screen);
	SDL_Rect levelRect{ WINDOW_WIDTH * 5 / 8, WINDOW_HEIGHT * 2 / 8, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8 };
	SDL_RenderCopy(renderer, levelText, NULL, &levelRect);
	SDL_DestroyTexture(levelText);
	SDL_FreeSurface(screen);
	screen = nullptr;
}

void Game::DrawTitle(SDL_Rect& newGameRect, SDL_Rect& quitRect)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Surface* titleSurf = TTF_RenderText_Solid(font, "TETRIS", { 255, 255, 255 });
	SDL_Texture* titleText = SDL_CreateTextureFromSurface(renderer, titleSurf);
	SDL_Rect titleRect{ WINDOW_WIDTH / 4, WINDOW_HEIGHT / 12, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 3 };
	SDL_RenderCopy(renderer, titleText, NULL, &titleRect);
	
	SDL_Surface* newGameSurf = TTF_RenderText_Solid(font, "New Game", { 255, 255, 255 });
	SDL_Texture* newGameText = SDL_CreateTextureFromSurface(renderer, newGameSurf);
	newGameRect = { WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3 + WINDOW_HEIGHT / 12, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 6 };
	SDL_RenderCopy(renderer, newGameText, NULL, &newGameRect);

	SDL_Surface* quitSurf = TTF_RenderText_Solid(font, "Quit", { 255, 255, 255 });
	SDL_Texture* quitText = SDL_CreateTextureFromSurface(renderer, quitSurf);
	quitRect = { WINDOW_WIDTH * 5 / 12, WINDOW_HEIGHT * 2 / 3 - WINDOW_HEIGHT / 12, WINDOW_WIDTH / 6, WINDOW_HEIGHT / 6 };
	SDL_RenderCopy(renderer, quitText, NULL, &quitRect);
	
	SDL_DestroyTexture(titleText);
	SDL_DestroyTexture(newGameText);
	SDL_DestroyTexture(quitText);
	SDL_FreeSurface(titleSurf);
	SDL_FreeSurface(newGameSurf);
	SDL_FreeSurface(quitSurf);
}

void Game::DrawGameOver(SDL_Rect& newGameRect, SDL_Rect& quitRect)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	// draw score
	string text = "Your score: " + to_string(scores);
	SDL_Surface* scoreSurf = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255 });
	SDL_Texture* scoreText = SDL_CreateTextureFromSurface(renderer, scoreSurf);
	SDL_Rect scroreRect = { WINDOW_WIDTH / 5, 0 + WINDOW_HEIGHT / 12, WINDOW_WIDTH * 3 / 5, WINDOW_HEIGHT / 3 };
	SDL_RenderCopy(renderer, scoreText, NULL, &scroreRect);

	// try again
	SDL_Surface* newGameSurf = TTF_RenderText_Solid(font, "Try again", { 255, 255, 255 });
	SDL_Texture* newGameText = SDL_CreateTextureFromSurface(renderer, newGameSurf);
	newGameRect = { WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3 + WINDOW_HEIGHT / 12, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 6 };
	SDL_RenderCopy(renderer, newGameText, NULL, &newGameRect);

	// quit
	SDL_Surface* quitSurf = TTF_RenderText_Solid(font, "Quit", { 255, 255, 255 });
	SDL_Texture* quitText = SDL_CreateTextureFromSurface(renderer, quitSurf);
	quitRect = { WINDOW_WIDTH * 5 / 12, WINDOW_HEIGHT * 2 / 3, WINDOW_WIDTH / 6, WINDOW_HEIGHT / 6 };
	SDL_RenderCopy(renderer, quitText, NULL, &quitRect);

	SDL_DestroyTexture(scoreText);
	SDL_DestroyTexture(newGameText);
	SDL_DestroyTexture(quitText);
	SDL_FreeSurface(scoreSurf);
	SDL_FreeSurface(newGameSurf);
	SDL_FreeSurface(quitSurf);

}

// return lowest y at each col which can place the rect
int Game::FindLowestY(int xPos, int yPos)
{
	int lowestY = yPos;
	for (int y = lowestY; y < GAMEBOARD_HEIGHT; y++)
		if (gameboard[y * GAMEBOARD_WIDTH + xPos] == 'x' && y > lowestY || y == GAMEBOARD_HEIGHT - 1)
			return y - 1;
	return lowestY;
}

// check if the xy-pos in gameboard is valid position
bool Game::IsValidPosition(string& piece, int xPos, int yPos)
{
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if ((gameboard[(y + yPos) * GAMEBOARD_WIDTH + (x + xPos)] == 'x' || gameboard[(y + yPos) * GAMEBOARD_WIDTH + (x + xPos)] == 'o') && piece[y * 4 + x] == 'x')
				return false;
	return true;
}

bool Game::IsPerfectClear()
{
	for (int y = 0; y < GAMEBOARD_HEIGHT; y++)
		for (int x = 1; x < GAMEBOARD_WIDTH; x++)
			if (gameboard[y * GAMEBOARD_WIDTH + x] == 'x')
				return false;
	return true;
}

bool Game::IsGameOver()
{
	for (int y = 0; y < BUFFER_HEIGHT; y++)
		for (int x = 1; x < GAMEBOARD_WIDTH - 1; x++)
			if (gameboard[y * GAMEBOARD_WIDTH + x] == 'x')
				return true;
	return false;
}

void Game::PlayBGM()
{
	const char* audioDriver = SDL_GetAudioDriver(0);
	if (SDL_AudioInit(audioDriver) < 0)
		cout << "Fail to initialize audio driver. SDL Error: " << SDL_GetError() << endl;
}

void Game::Start(bool& quit, bool& start, bool& run, SDL_Event& e, SDL_Rect& newGameRect, SDL_Rect& quitRect)
{
	int x, y;

	DrawTitle(newGameRect, quitRect);

	while (SDL_PollEvent(&e) != 0)
		if (e.type == SDL_QUIT)
			quit = true;
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			SDL_GetMouseState(&x, &y);
			if (x >= newGameRect.x && x <= newGameRect.x + newGameRect.w && y >= newGameRect.y && y <= newGameRect.y + newGameRect.h)
			{
				start = false;
				run = true;
			}
			else if (x >= quitRect.x && x <= quitRect.x + quitRect.w && y >= quitRect.y && y <= quitRect.y + quitRect.h)
				quit = true;
		}

	SDL_GetMouseState(&x, &y);
	if (x >= newGameRect.x && x <= newGameRect.x + newGameRect.w && y >= newGameRect.y && y <= newGameRect.y + newGameRect.h)
		SDL_RenderDrawRect(renderer, &newGameRect);
	else if (x >= quitRect.x && x <= quitRect.x + quitRect.w && y >= quitRect.y && y <= quitRect.y + quitRect.h)
		SDL_RenderDrawRect(renderer, &quitRect);

}

// main loop
void Game::Run(bool& quit, bool& run, bool& end, SDL_Event& e, bool& getNewPiece, int& xPos, int& yPos, int& frameCount, int& lockDelay, bool& lockDelayExpired)
{

	// game loop
	if (getNewPiece)
	{
		// get piece
		GetNewPiece();
		getNewPiece = false;
		xPos = 3;
		yPos = 0;
		frameCount = 0;
		lockDelay = 0;
		lockDelayExpired = false;
	}

	// user input
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
			quit = true;
		else if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
				Rotate(xPos, yPos);
				break;
			case SDLK_DOWN:
				if (IsValidPosition(currPiece.shape, xPos, yPos + 1))
					yPos += 1;
				break;
			case SDLK_LEFT:
				if (IsValidPosition(currPiece.shape, xPos - 1, yPos))
					xPos -= 1;
				break;
			case SDLK_RIGHT:
				if (IsValidPosition(currPiece.shape, xPos + 1, yPos))
					xPos += 1;
				break;
			case SDLK_SPACE:
				HardDrop(xPos, yPos);
				frameCount = framePerGridCell;
				lockDelayExpired = true;
				break;
			}
		}
	}

	// piece free fall
	if (IsValidPosition(currPiece.shape, xPos, yPos + 1) && frameCount == framePerGridCell)
	{
		yPos += 1;
		frameCount = 0;
	}
	else if (!IsValidPosition(currPiece.shape, xPos, yPos + 1) && frameCount >= framePerGridCell)
	{
		if (!lockDelayExpired)
		{
			lockDelay += 1;
			if (lockDelay == LOCKDELAYFRAME)
				lockDelayExpired = true;
		}
		else
		{
			getNewPiece = true;
			// check if gameover
			Update(xPos, yPos);
			CheckLine(yPos);
			if (IsGameOver())
			{
				run = false;
				end = true;
			}
		}
	}

	// draw scores
	DrawScore();
	// draw level
	DrawLevel();
	// draw current piece
	DrawPiece(xPos, yPos);
	// draw next piece
	DrawNextPiece();
	// draw the lowest possbile pos
	DrawLowestPos(xPos, yPos);
	// draw the entire map
	DrawGameboard();

	frameCount += 1;
}

void Game::End(bool& quit, bool& run, bool& end, SDL_Event& e, SDL_Rect& newGameRect, SDL_Rect& quitRect)
{
	int x, y;
	DrawGameOver(newGameRect, quitRect);

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
			quit = true;
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			SDL_GetMouseState(&x, &y);
			if (x >= newGameRect.x && x <= newGameRect.x + newGameRect.w && y >= newGameRect.y && y <= newGameRect.y + newGameRect.h)
			{
				InitGameData();
				run = true;
				end = false;
			}
			else if (x >= quitRect.x && x <= quitRect.x + quitRect.w && y >= quitRect.y && y <= quitRect.y + quitRect.h)
				quit = true;
		}
	}

	SDL_GetMouseState(&x, &y);
	if (x >= newGameRect.x && x <= newGameRect.x + newGameRect.w && y >= newGameRect.y && y <= newGameRect.y + newGameRect.h)
		SDL_RenderDrawRect(renderer, &newGameRect);
	else if (x >= quitRect.x && x <= quitRect.x + quitRect.w && y >= quitRect.y && y <= quitRect.y + quitRect.h)
		SDL_RenderDrawRect(renderer, &quitRect);

}

void Game::StartGame()
{
	bool quit = false;
	bool start = true;
	bool run = false;
	bool end = false;

	// variable used in Start, Run and End
	SDL_Event e;

	// variable used in Start
	SDL_Rect newGameRect, quitRect;

	// variable used in Run
	bool getNewPiece = true;
	int xPos = 3;
	int yPos = 0;
	int frameCount = 0;
	int lockDelay = 0;
	bool lockDelayExpired = false;

	Mix_PlayMusic(bgm, -1);

	while (!quit)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		if (start && !run && !end)
			Start(quit, start, run, e, newGameRect, quitRect);
		else if (!start && run && !end)
			Run(quit, run, end, e, getNewPiece, xPos, yPos, frameCount, lockDelay, lockDelayExpired);
		else if (!start && !run && end)
			End(quit, run, end, e, newGameRect, quitRect);

		SDL_RenderPresent(renderer);
	}
}