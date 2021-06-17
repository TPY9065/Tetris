#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct Color
{
	int r, g, b, a;
	Color();
	Color(int red, int green, int blue, int alpha);
};

struct Piece
{
	string shape = "";
	Color color = Color();
	Piece();
	Piece(string s, Color c);
	bool IsEmpty();
};

class Game
{
public:
	static const int WINDOW_WIDTH = 720;
	static const int WINDOW_HEIGHT = 630;
	static const int CELL_SIZE = 30;
	static const int GAMEBOARD_WIDTH = 12;
	static const int GAMEBOARD_HEIGHT = 26;
	static const int BUFFER_HEIGHT = 5;
	static const int FPS = 60;
	static const int SECOND_PER_FRAME = 1000 / 6;
	static const int MAX_DROP_RATE = 48;
	static const int DECREASE_RATE = 5;
	static const int LOCKDELAYFRAME = 15;
	static const string PIECE[7];
	static const Color COLOR[7];
private:
	SDL_Window* window = nullptr;
	SDL_Surface* screen = nullptr;
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font;
	Mix_Music* bgm;
	char gameboard[GAMEBOARD_WIDTH * GAMEBOARD_HEIGHT];
	Color colorBoard[GAMEBOARD_WIDTH * GAMEBOARD_HEIGHT];
	int level;
	int framePerGridCell;
	int scores;
	int clearLinesNum;
	Piece currPiece;
	Piece nextPiece;
	vector<int> index;
public:
	Game();
	~Game();
	bool InitSuccess();
	void InitGameData();
	void InitGameBoard();
	void GetNextOrder();
	void DrawPiece(int xPos, int yPos);
	void DrawPieceRect(Color& color, int xPos, int yPos);
	void Update(int xPos, int yPos);
	void DrawGameboard();
	void PrintMap();
	void GetNewPiece();
	void Rotate(int& xPos, int yPos);
	void CheckLine(int yLine);
	void ClearLine(int yLine);
	void HardDrop(int xPos, int& yPos);
	void AddScore(int rowClear);
	void DrawLowestPos(int xPos, int yPos);
	void DrawNextPiece();
	void DrawScore();
	void DrawLevel();
	void DrawTitle(SDL_Rect& newGameRect, SDL_Rect& quitRect);\
	void DrawGameOver(SDL_Rect& newGameRect, SDL_Rect& quitRect);
	int FindLowestY(int xPos, int yPos);
	bool IsValidPosition(string& piece, int xPos, int yPos);
	bool IsPerfectClear();
	bool IsGameOver();
	void PlayBGM();
	void Start(bool& quit, bool& start, bool& run, SDL_Event &e, SDL_Rect& newGameRect, SDL_Rect& quitRect);
	void Run(bool& quit, bool& run, bool& end, SDL_Event& e, bool& getNewPiece, int& xPos, int& yPos, int& frameCount, int& lockDelay, bool& lockDelayExpired);
	void End(bool& quit, bool& run, bool& end, SDL_Event& e, SDL_Rect& newGameRect, SDL_Rect& quitRect);
	void StartGame();
};