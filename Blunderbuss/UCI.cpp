#define _CRT_SECURE_NO_WARNINGS

#include "uci.h"
#include <iostream>
#include <fstream>
#include <chrono>

UCI::UCI()
{
    board = InitBoard();
}

UCI::~UCI()
{
    // Clean up if necessary
}

void UCI::Run()
{
    PrintWelcomeMessage();
    std::string input;
    while (std::getline(std::cin, input))
    {
        Log("Received command: " + input);
        std::istringstream iss(input);
        std::string token;
        iss >> token;

        if (token == "uci")
        {
            SendUciResponse();
        }
        else if (token == "isready")
        {
            SendReadyOk();
        }
        else if (token == "ucinewgame")
        {
            StartNewGame();
        }
        else if (token == "position")
        {
            HandlePositionCommand(iss);
        }
        else if (token == "go")
        {
            HandleGoCommand(iss);
        }
        else if (token == "quit")
        {
            Log("Quitting UCI loop.");
            break;
        }
        else if (token == "print")
        {
            PrintBoard(board);
        }
        else if (token == "setoption")
        {
            HandleSetOptionCommand(iss);
        }
    }
}

void UCI::Log(const std::string& message)
{
    std::ofstream out_file(logFile, std::ios::app);
    if (out_file.is_open())
    {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        out_file << std::ctime(&now) << ": " << message << "\n";
        out_file.close();
    }
    else
    {
        std::cerr << "Unable to open the log file." << std::endl;
    }
}

void UCI::SendUciResponse()
{
    std::cout << "id name Blunderbuss\n";
    std::cout << "id author Kek\n";
    std::cout << "uciok\n";
    Log("Sent UCI response.");
}

void UCI::SendReadyOk()
{
    std::cout << "readyok\n";
    Log("Sent readyok.");
}

void UCI::StartNewGame()
{
    Log("Started new game.");
}

void UCI::PrintWelcomeMessage()
{
    std::cout << " ____  _                 _           _                   \n";
    std::cout << "| __ )| |_   _ _ __   __| | ___ _ __| |__  _   _ ___ ___ \n";
    std::cout << "|  _ \\| | | | | '_ \\ / _` |/ _ \\ '__| '_ \\| | | / __/ __|\n";
    std::cout << "| |_) | | |_| | | | | (_| |  __/ |  | |_) | |_| \\__ \\__ \\\n";
    std::cout << "|____/|_|\\__,_|_| |_|\\__,_|\\___|_|  |_.__/ \\__,_|___/___/\n\n";
    std::cout << "_________________________________________________________\n\n";
}

void UCI::HandleSetOptionCommand(std::istringstream& iss)
{
    std::string token;
    std::string option_name;
    std::string option_value;

    while (iss >> token)
    {
        if (token == "name")
        {
            iss >> option_name;
        }
        else if (token == "value")
        {
            iss >> option_value;
        }
    }

    options[option_name] = option_value;
    Log("Set option " + option_name + " to " + option_value);
}

void UCI::HandleGoCommand(std::istringstream& iss)
{
    std::string token;
    std::string value;

    while (iss >> token)
    {
        if (token == "perft")
        {
            int depth;
            if (iss >> depth)
            {
				uint64_t nodes = Perft(board, depth, true);
				std::cout << "info string Perft " << depth << ": " << nodes << "\n";
				Log("Perft " + std::to_string(depth) + ": " + std::to_string(nodes));
            }
            else
            {
                Log("Invalid perft depth value.");
            }
        }
    }
}

void UCI::HandlePositionCommand(std::istringstream& iss)
{
    std::string token;

    while (iss >> token)
    {
        if (token == "fen")
        {
            //read the fen and call LoadFen
			std::string fen;
			std::getline(iss, fen);
			LoadFEN(board, fen);
			Log("Loaded FEN: " + fen);
		}
    }
}