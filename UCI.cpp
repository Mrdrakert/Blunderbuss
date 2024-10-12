#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include <map>
#include "Board.h"


class UCI
{
public:
    UCI()
    {
        options["dp_penalty"] = "10";
        create_new_board();
    }

    ~UCI()
    { }

    void create_new_board()
    {
        board = new Board(stoi(options["dp_penalty"]));
    }

    void run()
    {
        print_welcome_message();
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
            else if (token == "print")
            {
                board->print_chessboard();
            }
            else if (token == "setoption") {
                handle_setoption_command(iss);
            }
        }
    }

private:
    Board* board;
    std::map<std::string, std::string> options;
    std::string log_file = "log_file.txt";

    void log(const std::string& message)
    {
        std::ofstream out_file(log_file, std::ios::app);
        if (out_file.is_open())
        {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            out_file << std::ctime(&now) << ": " << message << "\n";
            out_file.close();
        }
        else 
        {
            std::cerr << "Unable to open the file." << std::endl;
        }
    }

    void send_uci_response()
    {
        std::cout << "id name Blunderbuss\n";
        std::cout << "id author Kek\n";
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
        create_new_board();
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
                //log("Applied move: " + move_str);
            }

        }
    }

    void handle_go_command(std::istringstream& iss)
    {
        std::string token;
        int wtime = 0;
        int btime = 0;
        int depth = 0;
        bool normal_search = 0;
        while (iss >> token)
        {
            if (token == "depth")
            {
                iss >> depth;

                Move best_move = board->find_best_move_for_depth(depth);
                std::string best_move_str = format_move(best_move);
                std::cout << "bestmove " << best_move_str << "\n";
                log("Calculated best move: " + best_move_str + " at depth " + std::to_string(depth));
            }
            if (token == "timed")
            {
                iss >> wtime;

                Move best_move = board->find_best_move_with_time_limit(wtime);
                std::string best_move_str = format_move(best_move);
                std::cout << "bestmove " << best_move_str << "\n";
                log("Calculated best move: " + best_move_str + " with time " + std::to_string(wtime) + "ms");
            }
            if (token == "wtime")
            {
                iss >> wtime;
                normal_search = 1;
            }
            if (token == "btime")
            {
                iss >> btime;
                normal_search = 1;
            }
            if (token == "infinite")
            {
                btime = 0;
                wtime = 0;
                normal_search = 1;
            }
            if (token == "perft")
            {
                int depth;
                iss >> depth;
                uint64_t positions = board->count_legal_moves_at_depth(depth);
                std::cout << "Positions at depth " << depth << ": " << positions << "\n";
                log("Positions at depth " + std::to_string(depth) + ": " + std::to_string(positions));
            }
        }

        if (normal_search)
        {
            int time_left = (board->turn == 0) ? wtime : btime;
            int time_for_move = get_time_for_move(time_left);
            Move best_move = board->find_best_move_with_time_limit(time_for_move);
            std::string best_move_str = format_move(best_move);
            std::cout << "bestmove " << best_move_str << "\n";
            log("Calculated best move: " + best_move_str + " with time " + std::to_string(time_for_move) + "ms");
        }

    }

    int get_time_for_move(int time_left)
    {
        float y = 10000;
        if (time_left < 600000)
            y = 0.00000000000009 * time_left * time_left * time_left + -0.00000007 * time_left * time_left + 0.025 * time_left + 200.0;

        if (time_left < 10000)
            y = time_left / 22.0;

        if (time_left == 0)
            y = 4000;

        if (board->move_num < 4)
            y = (y * board->move_num) / (board->move_num + 1);

        int time_for_move = static_cast<int>(y);
        return time_for_move;
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

    void print_welcome_message()
    {
        std::cout << " ____  _                 _           _                   " << std::endl;
        std::cout << "| __ )| |_   _ _ __   __| | ___ _ __| |__  _   _ ___ ___ " << std::endl;
        std::cout << "|  _ \\| | | | | '_ \\ / _` |/ _ \\ '__| '_ \\| | | / __/ __|" << std::endl;
        std::cout << "| |_) | | |_| | | | | (_| |  __/ |  | |_) | |_| \\__ \\__ \\" << std::endl;
        std::cout << "|____/|_|\\__,_|_| |_|\\__,_|\\___|_|  |_.__/ \\__,_|___/___/" << std::endl << std::endl;
        std::cout << "_________________________________________________________" << std::endl << std::endl;
    }

    void handle_setoption_command(std::istringstream& iss) {
        std::string token;
        std::string option_name;
        std::string option_value;

        while (iss >> token) {
            if (token == "name") {
                iss >> option_name;
            }
            else if (token == "value") {
                iss >> option_value;
            }
        }

        options[option_name] = option_value;
        log("Set option " + option_name + " to " + option_value);
    }

};

int main()
{
    UCI uci;
    uci.run();
    return 0;
}
