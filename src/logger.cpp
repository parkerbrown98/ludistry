#include "logger.h"

namespace Ludistry
{
    Logger::Logger(const std::string &path)
    {
        file.open(path, std::ios::out | std::ios::app);
    }

    Logger::~Logger()
    {
        if (file.is_open())
        {
            file.close();
        }
    }

    void Logger::Info(const std::string &message)
    {
        if (!file.is_open())
        {
            return;
        }

        time_t now = time(0);
        tm *ltm = localtime(&now);
        std::string time = "[" + std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(1 + ltm->tm_mon) + "-" + std::to_string(ltm->tm_mday) + " " + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec) + "]";

        file << time << " INFO: " << message << std::endl;
        std::cout << time << " INFO: " << message << std::endl;
    }

    void Logger::Error(const std::string &message)
    {
        if (!file.is_open())
        {
            return;
        }

        time_t now = time(0);
        tm *ltm = localtime(&now);
        std::string time = "[" + std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(1 + ltm->tm_mon) + "-" + std::to_string(ltm->tm_mday) + " " + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec) + "]";

        file << time << " ERROR: " << message << std::endl;
        std::cout << time << " ERROR: " << message << std::endl;
    }
}