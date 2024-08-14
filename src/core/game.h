#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "logger.h"
#include "core/net.h"
#include <memory>

namespace Ludistry
{
    class Game
    {
    public:
        static Game &Instance()
        {
            static Game instance;
            return instance;
        }

        Game(const Game &) = delete;
        Game &operator=(const Game &) = delete;
        void Initialize();
        void Update();
        void Destroy();
        void LoadLua(const char *path);

        static void LuaSetup(lua_State *L);
        static int LuaInclude(lua_State *L);

        inline lua_State *GetLuaState() { return L; }
        inline Logger &GetLogger() { return *logger; }
        inline SocketServer &GetServer() { return *server; }

    private:
        Game() = default;
        lua_State *L;
        std::unique_ptr<Logger> logger;
        std::unique_ptr<SocketServer> server;
    };
}