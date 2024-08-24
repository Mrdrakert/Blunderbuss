#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include "Blunderbuss.h"  // Replace with the correct include if necessary


class UCI 
{
public:
    UCI() : board(), search_depth(MAX_DEPTH)
    { }

    ~UCI() 
    { }

    void run() 
    {
        std::string input;
        while (std::getline(std::cin, input)) 
        {
            log("Received command: " + input);
            std::istringstream iss(input);
            std::string token;
            iss >> token;

            if (token == "uci") 
            {
                send_uci_response();
            }
            else if (token == "isready") 
            {
                send_ready_ok();
            }
            else if (token == "ucinewgame") 
            {
                start_new_game();
            }
            else if (token == "position") 
            {
                handle_position_command(iss);
            }
            else if (token == "go") 
            {
                handle_go_command(iss);
            }
            else if (token == "quit") 
            {
                log("Quitting UCI loop.");
                break;
            }
        }
    }

private:
    Board* board;
    int search_depth;
    std::ofstream log_file;

    void log(const std::string& message) 
    {
        std::ofstream out_file("log_file.txt", std::ios::app);
        if (out_file.is_open()) 
        {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            out_file << std::ctime(&now) << ": " << message << "\n";
            out_file.close();
        }
        else {
            std::cerr << "Unable to open the file." << std::endl;
        }
    }

    void send_uci_response() 
    {
        std::cout << "id name Blunderbuss\n";
        std::cout << "id author YourName\n";
        std::cout << "uciok\n";
        log("Sent UCI response.");
    }

    void send_ready_ok() 
    {
        std::cout << "readyok\n";
        log("Sent readyok.");
    }

    void start_new_game() 
    {
        board = new Board();
        log("Started new game.");
    }

    void handle_position_command(std::istringstream& iss) 
    {
        std::string sub_command;
        iss >> sub_command;
        if (sub_command == "startpos") 
        {
            board->setup_board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            log("Set up board from startpos.");
        }
        else if (sub_command == "fen") 
        {
            std::string parsing;
            std::string fen;

            for (int i = 0; i < 6 && iss >> parsing; ++i) {
                if (i > 0) {
                    fen += " ";
                }
                fen += parsing;
            }

            board->setup_board_from_fen(fen);
            log("Set up board from FEN: " + fen);
        }
        handle_moves(iss);
    }

    void handle_moves(std::istringstream& iss) 
    {
        std::string move_str;
        while (iss >> move_str) 
        {
            if (move_str != "moves")
            {
                Move move = parse_move(move_str);
                board->make_move_opponent(move);
                log("Applied move: " + move_str);
            }
            
        }
    }

    Move parse_move(const std::string& move_str) 
    {
        int from = (move_str[1] - '1') * 8 + (move_str[0] - 'a');
        int to = (move_str[3] - '1') * 8 + (move_str[2] - 'a');
        int piece_type = board->get_piece_bitboard(from).type;
        int move_type = 0;

        if (move_str.length() == 5)
        {
            if (move_str[4] == 'q')
                move_type = 4;
            else if (move_str[4] == 'n')
                move_type = 6;
            else if (move_str[4] == 'b')
                move_type = 7;
            else if (move_str[4] == 'r')
                move_type = 8;
        }
        return Move(from, to, piece_type, move_type);
    }

    void handle_go_command(std::istringstream& iss) 
    {
        std::string token;
        int depth = search_depth;
        int wtime = 0;
        int btime = 0;
        while (iss >> token) 
        {
            if (token == "depth") 
            {
                iss >> depth;
                log("Received go command with depth: " + std::to_string(depth));
            }
            if (token == "wtime")
            {
                iss >> wtime;
            }
            if (token == "btime")
            {
                iss >> btime;
            }
            if (token == "infinite")
            {
                btime = 0;
                wtime = 0;
            }
        }
        int time_left = (board->turn == 0) ? wtime : btime;
        Move best_move = search_best_move(time_left);
        std::string best_move_str = format_move(best_move);
        std::cout << "bestmove " << best_move_str << "\n";
        log("Calculated best move: " + best_move_str);
    }

    std::string format_move(const Move& move) 
    {
        std::string move_str;
        move_str += (move.from % 8) + 'a';
        move_str += (move.from / 8) + '1';
        move_str += (move.to % 8) + 'a';
        move_str += (move.to / 8) + '1';

        if (move.type == 4)
            move_str += 'q';
        else if (move.type == 6)
            move_str += 'n';
        else if (move.type == 7)
            move_str += 'b';
        else if (move.type == 8)
            move_str += 'r';

        return move_str;
    }

    Move search_best_move(int time_left)
    {
        int time_for_move = get_time_for_move(time_left);

        log("Time left " + std::to_string(time_left) + ", searching move with " + std::to_string(time_for_move));
        Move move = board->find_best_move_with_time_limit(time_for_move);
        return move;
    }

    int get_time_for_move(int time_left)
    {
        float y = 10000;
        if (time_left < 600000)
            y = 0.000000000000124 * time_left * time_left * time_left + -0.0000001072 * time_left * time_left + 0.036053 * time_left + 203.05;

        if (time_left < 5000)
            y = time_left / 14;

        if (time_left == 0)
            y = 4000;

        int time_for_move = static_cast<int>(y);
        return time_for_move;
    }
};

int main() 
{
    UCI uci;
    uci.run();
    return 0;
}
