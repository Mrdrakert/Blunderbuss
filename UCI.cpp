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
    Board board;
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
        board = Board();
        log("Started new game.");
    }

    void handle_position_command(std::istringstream& iss) 
    {
        std::string sub_command;
        iss >> sub_command;
        if (sub_command == "startpos") 
        {
            board.setup_board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            log("Set up board from startpos.");
        }
        else if (sub_command == "fen") 
        {
            std::string fen;
            std::getline(iss, fen);
            board.setup_board_from_fen(fen);
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
                board.make_move_opponent(move);
                log("Applied move: " + move_str);
            }
            
        }
    }

    Move parse_move(const std::string& move_str) 
    {
        int from = (move_str[1] - '1') * 8 + (move_str[0] - 'a');
        int to = (move_str[3] - '1') * 8 + (move_str[2] - 'a');
        int piece_type = board.get_piece_bitboard(from).type;
        return Move(from, to, piece_type, 0);
    }

    void handle_go_command(std::istringstream& iss) 
    {
        std::string token;
        int depth = search_depth;
        while (iss >> token) 
        {
            if (token == "depth") 
            {
                iss >> depth;
                log("Received go command with depth: " + std::to_string(depth));
            }
        }

        Move best_move = search_best_move();
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
        return move_str;
    }

    Move search_best_move() 
    {
        Move move = board.find_best_move_with_time_limit(2000);
        return move;
    }
};

int main() 
{
    UCI uci;
    uci.run();
    return 0;
}
