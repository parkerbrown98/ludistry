#include "net.h"
#include "core/game.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Ludistry
{
    SocketServer::SocketServer(int port) : port_(port), running_(false)
    {
        server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock_ < 0)
        {
            Game::Instance().GetLogger().Error("Failed to create socket");
            exit(1);
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);

        if (bind(server_sock_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            Game::Instance().GetLogger().Error("Failed to bind socket");
            exit(1);
        }

        if (listen(server_sock_, 5) < 0)
        {
            Game::Instance().GetLogger().Error("Failed to listen on socket");
            exit(1);
        }
    }

    SocketServer::~SocketServer()
    {
        Stop();
        close(server_sock_);
    }

    void SocketServer::Start()
    {
        running_ = true;
        accept_thread_ = std::thread(&SocketServer::Accept, this);
        Game::Instance().GetLogger().Info("Server started on port " + std::to_string(port_));
    }

    void SocketServer::Stop()
    {
        running_ = false;
        if (accept_thread_.joinable())
        {
            accept_thread_.join();
        }

        lua_State *L = Game::Instance().GetLuaState();
        for (auto &pair : message_handlers_)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, pair.second);
        }
        message_handlers_.clear();
    }

    void SocketServer::Accept()
    {
        while (running_)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_sock = accept(server_sock_, (struct sockaddr *)&client_addr, &client_len);
            if (client_sock < 0)
            {
                Game::Instance().GetLogger().Error("Failed to accept client");
                continue;
            }

            std::shared_ptr<Player> player = std::make_shared<Player>(client_sock, "Player");
            players_.emplace(client_sock, player);
            std::thread(&SocketServer::HandleClient, this, client_sock).detach();

            Game::Instance().GetLogger().Info("Client connected: " + player->GetName());
        }
    }

    void SocketServer::HandleClient(int client_sock)
    {
        char buffer[1024] = {0};
        while (running_)
        {
            ssize_t bytes = recv(client_sock, buffer, sizeof(buffer), 0);
            if (bytes < 0)
            {
                Game::Instance().GetLogger().Error("Error receiving data from client");
                break;
            }
            else if (bytes == 0)
            {
                Game::Instance().GetLogger().Info("Client disconnected");
                break;
            }

            std::string message(buffer, bytes);
            ProcessMessage(client_sock, message);
        }

        players_.erase(client_sock);
        close(client_sock);
    }

    void SocketServer::ProcessMessage(int client_sock, const std::string &message)
    {
        // Get associated player
        auto player = players_.find(client_sock);
        if (player == players_.end())
        {
            return;
        }

        try
        {
            // Parse message
            std::string action = message.substr(0, message.find(' '));
            std::string data = message.substr(message.find(' ') + 1);
            json parsed_data = json::parse(data);

            // Call Lua handler
            auto handler = message_handlers_.find(action);
            if (handler != message_handlers_.end())
            {
                lua_State *L = Game::Instance().GetLuaState();
                lua_rawgeti(L, LUA_REGISTRYINDEX, handler->second);

                // Push player userdata
                Player** pUserdata = static_cast<Player**>(lua_newuserdata(L, sizeof(Player*)));
                *pUserdata = player->second.get();

                luaL_getmetatable(L, "Player");
                lua_setmetatable(L, -2);

                // Push message payload as table
                lua_newtable(L);
                for (auto it = parsed_data.begin(); it != parsed_data.end(); ++it)
                {
                    lua_pushstring(L, it.key().c_str());
                    if (it.value().is_number())
                    {
                        lua_pushnumber(L, it.value());
                    }
                    else if (it.value().is_string())
                    {
                        lua_pushstring(L, it.value().get<std::string>().c_str());
                    }
                    else if (it.value().is_boolean())
                    {
                        lua_pushboolean(L, it.value());
                    }
                    else if (it.value().is_null())
                    {
                        lua_pushnil(L);
                    }
                    lua_settable(L, -3);
                }

                // Call handler function
                if (lua_pcall(L, 2, 0, 0) != LUA_OK)
                {
                    Game::Instance().GetLogger().Error(lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
        }
        catch (const json::parse_error &e)
        {
            Game::Instance().GetLogger().Error("Failed to parse JSON: " + std::string(e.what()));
        }
        catch (const std::exception &e)
        {
            Game::Instance().GetLogger().Error("Failed to process message: " + std::string(e.what()));
        }
    }

    void SocketServer::LuaSetup(lua_State *L)
    {
        lua_newtable(L);
        lua_pushcfunction(L, LuaReceive);
        lua_setfield(L, -2, "receive");
        lua_setglobal(L, "net");
    }

    int SocketServer::LuaReceive(lua_State *L)
    {
        const char *action = luaL_checkstring(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        // Store callback function and get reference
        int callback = luaL_ref(L, LUA_REGISTRYINDEX);

        // Get server instance
        SocketServer *server = &Game::Instance().GetServer();
        if (server != nullptr)
        {
            server->message_handlers_.emplace(action, callback);
        }

        return 0;
    }

    Player::Player(int sock, const std::string &name) : sock_(sock), name_(name) {}

    Player::~Player()
    {
        close(sock_);
    }

    void Player::Send(const std::string &message)
    {
        send(sock_, message.c_str(), message.size(), 0);
    }

    int Player::LuaGetName(lua_State *L)
    {
        Player *player = *static_cast<Player**>(luaL_checkudata(L, 1, "Player"));
        lua_pushstring(L, player->GetName().c_str());
        return 1;
    }

    void Player::LuaSetup(lua_State *L)
    {
        luaL_newmetatable(L, "Player");
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        
        lua_pushcfunction(L, LuaGetName);
        lua_setfield(L, -2, "GetName");

        lua_pop(L, 1);
    }
}