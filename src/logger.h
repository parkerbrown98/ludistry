#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

namespace Ludistry
{
    class Logger
    {
    public:
        Logger(const std::string &path);
        ~Logger();
        void Info(const std::string &message);
        void Error(const std::string &message);

    private:
        std::ofstream file;
    };
}