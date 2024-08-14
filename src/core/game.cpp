#include "core/game.h"
#include "core/net.h"
#include <cstring>
#include <filesystem>

namespace Ludistry
{
    void Game::Initialize()
    {
        // Initialize logger
        logger = std::make_unique<Logger>("game.log");

        // Initialize Lua
        logger->Info("Initializing Lua");
        L = luaL_newstate();
        luaL_openlibs(L);

        Game::LuaSetup(L);
        SocketServer::LuaSetup(L);
        Player::LuaSetup(L);

        logger->Info("Initializing network");
        server = std::make_unique<SocketServer>(12345);
        server->Start();

        LoadLua("init.lua");
    }

    void Game::Update()
    {
    }

    void Game::Destroy()
    {
        // Close Lua state
        lua_close(L);
    }

    void Game::LoadLua(const char *path)
    {
        // Check if the requested path is within the application's working directory or its subdirectories
        std::string fullPath = std::filesystem::absolute(path);
        std::string workingDirectory = std::filesystem::current_path();

        if (fullPath.find(workingDirectory) != 0)
        {
            logger->Error("Invalid include path: " + fullPath);
            return;
        }

        if (luaL_dofile(L, path) != LUA_OK)
        {
            logger->Error(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    void Game::LuaSetup(lua_State *L)
    {
        lua_register(L, "include", LuaInclude);

        // Add GAME table
        lua_newtable(L);
        lua_setglobal(L, "GAME");
    }

    int Game::LuaInclude(lua_State *L)
    {
        const char *path = lua_tostring(L, 1);
        Game::Instance().LoadLua(path);
        return 0;
    }
}