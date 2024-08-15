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

        LoadLua("lua/init.lua");
        CallLuaFunction("Initialize");
    }

    Game::~Game()
    {
        Destroy();
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

    int Game::CallLuaFunction(const char *name, const std::vector<LuaValue> &args = {})
    {
        lua_getglobal(L, "GAME"); // GAME
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, name); // GAME[name]
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, -2); // GAME table (self)

                // Push arguments
                for (const auto &arg : args)
                {
                    std::visit([&](auto &&arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::string>)
                        {
                            lua_pushstring(L, arg.c_str());
                        }
                        else if constexpr (std::is_same_v<T, int>)
                        {
                            lua_pushinteger(L, arg);
                        }
                        else if constexpr (std::is_same_v<T, double>)
                        {
                            lua_pushnumber(L, arg);
                        }
                        else if constexpr (std::is_same_v<T, bool>)
                        {
                            lua_pushboolean(L, arg);
                        }
                    },
                    arg);
                }

                if (lua_pcall(L, args.size() + 1, 0, 0) != LUA_OK)
                {
                    logger->Error(lua_tostring(L, -1));
                    lua_pop(L, 1); // Pop error message
                    return 0;
                }
            } else {
                logger->Error("Function not found: " + std::string(name));
                lua_pop(L, 2); // Pop GAME table and GAME[name]
                return 0;
            }
        } else {
            logger->Error("GAME table not found");
            lua_pop(L, 1); // Pop GAME table
            return 0;
        }

        // Pop GAME table
        lua_pop(L, 1);

        return 1;
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