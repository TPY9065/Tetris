#include <iostream>
#include "Game.h"

using namespace std;

int main(int argc, char* args[])
{
    cout << "Welcome to tetris" << endl;

    Game game;

    if (game.InitSuccess())
        game.StartGame();

    return 0;
}