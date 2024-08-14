#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace Ludistry
{
    class SocketServer
    {
    public:
        SocketServer(int port);
        ~SocketServer();

        void Start();
        void Stop();
        void Broadcast(const std::string &message);

        static void LuaSetup(lua_State *L);

    private:
        void Accept();
        void HandleClient(int client_sock);
        void ProcessMessage(int client_sock, const std::string &message);

        static int LuaReceive(lua_State *L);

        int server_sock_;
        int port_;
        bool running_;
        std::thread accept_thread_;
        std::unordered_map<int, std::shared_ptr<class Player>> players_;
        std::unordered_map<std::string, int> message_handlers_;
    };

    class Player
    {
    public:
        Player(int sock, const std::string &name);
        ~Player();

        void Send(const std::string &message);
        static void LuaSetup(lua_State *L);
        static int LuaGetName(lua_State *L);

        inline std::string GetName() { return name_; }

    private:
        int sock_;
        std::string name_;
    };
}