#pragma once

#include <string>
#include <map>
#include <sstream>
#include "Board.h"

class UCI
{
public:
    UCI();
    ~UCI();
    void Run();

private:
    Board* board;
    std::map<std::string, std::string> options;
    std::string logFile = "log_file.txt";

    void Log(const std::string& message);
    void SendUciResponse();
    void SendReadyOk();
    void StartNewGame();
    void PrintWelcomeMessage();
    void HandleSetOptionCommand(std::istringstream& iss);
    void HandleGoCommand(std::istringstream& iss);
    void HandlePositionCommand(std::istringstream& iss);
};
