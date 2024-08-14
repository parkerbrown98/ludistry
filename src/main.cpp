#include "core/game.h"

using namespace Ludistry;

int main()
{
    Game::Instance().Initialize();

    while (true)
    {
        Game::Instance().Update();
    }

    return 0;
}