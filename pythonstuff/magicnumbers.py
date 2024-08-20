
def print_board(board):
    for i in range(8):
        for j in range(8):
            print(board[i * 8 + j], end=' ')
        print()

def reset_board(board):
    for i in range(64):
        board[i] = 0

board = [0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0]



def get_knight_moves():
    bitboards = [0] * 64
    knight_moves = [10, 17, 15, 6, -10, -17, -15, -6]

    for i in range(64):
        reset_board(board)
        
        for move in knight_moves:
            if i + move >= 0 and i + move < 64:
                if abs((i % 8) - ((i + move) % 8)) <= 2:
                    board[i + move] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('knight_moves.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)


def get_rook_moves():
    bitboards = [0] * 64
    rook_moves = [1, -1, 8, -8]

    for i in range(64):
        reset_board(board)
        for move in rook_moves:
            for j in range(1, 8):
                if i + (move * j) >= 0 and i + (move * j) < 64:
                    if move == 1 and (i % 8) + j > 7:
                        break
                    if move == -1 and (i % 8) - j < 0:
                        break
                    board[i + (move * j)] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('rook_moves.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)


def get_bishop_moves():
    bitboards = [0] * 64
    bishop_moves = [9, 7, -9, -7]

    for i in range(64):
        reset_board(board)
        for move in bishop_moves:
            for j in range(1, 8):
                if i + (move * j) >= 0 and i + (move * j) < 64:
                    if move == 9 and (i % 8) + j > 7:
                        break
                    if move == 7 and (i % 8) - j < 0:
                        break
                    if move == -9 and (i % 8) - j < 0:
                        break
                    if move == -7 and (i % 8) + j > 7:
                        break
                    board[i + (move * j)] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('bishop_moves.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)

def get_queen_moves():
    #read rook and bishop moves and combine them

    with open('rook_moves.txt', 'r') as f:
        rook_moves = f.readlines()
    with open('bishop_moves.txt', 'r') as f:
        bishop_moves = f.readlines()

    bitboards = [0] * 64
    for i in range(64):
        rook = int(rook_moves[i], 16)
        bishop = int(bishop_moves[i], 16)
        queen = rook | bishop
        formatted_hex = f"0x{queen:016X}"
        bitboards[i] = formatted_hex
    
    with open('queen_moves.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)

def get_king_moves():
    bitboards = [0] * 64
    king_moves = [1, -1, 8, -8, 9, 7, -9, -7]

    for i in range(64):
        reset_board(board)
        for move in king_moves:
            if i + move >= 0 and i + move < 64:
                if abs((i % 8) - ((i + move) % 8)) <= 1:
                    board[i + move] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('king_moves.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)

def get_white_pawn_capture_moves():
    bitboards = [0] * 64
    pawn_capture_moves = [7, 9]

    for i in range(64):
        reset_board(board)
        for move in pawn_capture_moves:
            if i + move >= 0 and i + move < 64:
                if abs((i % 8) - ((i + move) % 8)) <= 1:
                    if i // 8 < 7:
                        board[i + move] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('pawn_capture_moves_white.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)

def get_black_pawn_capture_moves():
    bitboards = [0] * 64
    pawn_capture_moves = [-7, -9]

    for i in range(64):
        reset_board(board)
        for move in pawn_capture_moves:
            if i + move >= 0 and i + move < 64:
                if abs((i % 8) - ((i + move) % 8)) <= 1:
                    if i // 8 > 0:
                        board[i + move] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('pawn_capture_moves_black.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)

def get_white_pawn_moves():
    bitboards = [0] * 64
    pawn_moves = [8]

    for i in range(64):
        reset_board(board)
        for move in pawn_moves:
            if i + move >= 0 and i + move < 64:
                if i // 8 == 1:
                    board[i + 2*move] = 1
                if i // 8 < 7:
                    board[i + move] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('pawn_moves_white.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)

def get_black_pawn_moves():
    bitboards = [0] * 64
    pawn_moves = [-8]

    for i in range(64):
        reset_board(board)
        for move in pawn_moves:
            if i + move >= 0 and i + move < 64:
                if i // 8 == 6:
                    board[i + 2*move] = 1
                if i // 8 > 0:
                    board[i + move] = 1

        binary_str = ''.join(map(str, board))
        bitboard = int(binary_str, 2)
        formatted_hex = f"0x{bitboard:016X}"
        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboards[index] = formatted_hex

    with open('pawn_moves_black.txt', 'w') as f:
        for item in bitboards:
            f.write("%s\n" % item)


def get_rook_moves_dirs():
    bitboardup = [0] * 64
    bitboarddown = [0] * 64
    bitboardleft = [0] * 64
    bitboardright = [0] * 64

    rook_moves = [1, -1, 8, -8]

    for i in range(64):
        boardup = [0] * 64
        boarddown = [0] * 64
        boardleft = [0] * 64
        boardright = [0] * 64
        for move in rook_moves:
            for j in range(1, 8):
                if i + (move * j) >= 0 and i + (move * j) < 64:
                    if move == 1 and (i % 8) + j > 7:
                        break
                    if move == -1 and (i % 8) - j < 0:
                        break
                    if move == 1:
                        boardright[i + (move * j)] = 1
                    elif move == -1:
                        boardleft[i + (move * j)] = 1
                    elif move == 8:
                        boardup[i + (move * j)] = 1
                    elif move == -8:
                        boarddown[i + (move * j)] = 1

        binary_str1 = ''.join(map(str, boardup))
        binary_str2 = ''.join(map(str, boarddown))
        binary_str3 = ''.join(map(str, boardleft))
        binary_str4 = ''.join(map(str, boardright))
                              
        bitboard1 = int(binary_str1, 2)
        bitboard2 = int(binary_str2, 2)
        bitboard3 = int(binary_str3, 2)
        bitboard4 = int(binary_str4, 2)

        formatted_hex1 = f"0x{bitboard1:016X}"
        formatted_hex2 = f"0x{bitboard2:016X}"
        formatted_hex3 = f"0x{bitboard3:016X}"
        formatted_hex4 = f"0x{bitboard4:016X}"

        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboardup[index] = formatted_hex1
        bitboarddown[index] = formatted_hex2
        bitboardleft[index] = formatted_hex3
        bitboardright[index] = formatted_hex4

    with open('rook_moves_up.txt', 'w') as f:
        for item in bitboardup:
            f.write("%s\n" % item)
    with open('rook_moves_down.txt', 'w') as f:
        for item in bitboarddown:
            f.write("%s\n" % item)
    with open('rook_moves_left.txt', 'w') as f:
        for item in bitboardleft:
            f.write("%s\n" % item)
    with open('rook_moves_right.txt', 'w') as f:
        for item in bitboardright:
            f.write("%s\n" % item)


def get_bishop_moves_dirs():
    
    bitboardup = [0] * 64
    bitboarddown = [0] * 64
    bitboardleft = [0] * 64
    bitboardright = [0] * 64

    bishop_moves = [9, 7, -9, -7]

    for i in range(64):
        boardup = [0] * 64
        boarddown = [0] * 64
        boardleft = [0] * 64
        boardright = [0] * 64
        for move in bishop_moves:
            for j in range(1, 8):
                if i + (move * j) >= 0 and i + (move * j) < 64:
                    if move == 9 and (i % 8) + j > 7:
                        break
                    if move == 7 and (i % 8) - j < 0:
                        break
                    if move == -9 and (i % 8) - j < 0:
                        break
                    if move == -7 and (i % 8) + j > 7:
                        break
                    if move == 9:
                        boardright[i + (move * j)] = 1
                    elif move == -9:
                        boardleft[i + (move * j)] = 1
                    elif move == 7:
                        boardup[i + (move * j)] = 1
                    elif move == -7:
                        boarddown[i + (move * j)] = 1

        binary_str1 = ''.join(map(str, boardup))
        binary_str2 = ''.join(map(str, boarddown))
        binary_str3 = ''.join(map(str, boardleft))
        binary_str4 = ''.join(map(str, boardright))
                              
        bitboard1 = int(binary_str1, 2)
        bitboard2 = int(binary_str2, 2)
        bitboard3 = int(binary_str3, 2)
        bitboard4 = int(binary_str4, 2)

        formatted_hex1 = f"0x{bitboard1:016X}"
        formatted_hex2 = f"0x{bitboard2:016X}"
        formatted_hex3 = f"0x{bitboard3:016X}"
        formatted_hex4 = f"0x{bitboard4:016X}"

        row = i // 8
        col = i % 8
        row = 7 - row
        col = 7 - col
        index = row * 8 + col
        bitboardup[index] = formatted_hex1
        bitboarddown[index] = formatted_hex2
        bitboardleft[index] = formatted_hex3
        bitboardright[index] = formatted_hex4

    with open('bishop_moves_up.txt', 'w') as f:
        for item in bitboardup:
            f.write("%s\n" % item)
    with open('bishop_moves_down.txt', 'w') as f:
        for item in bitboarddown:
            f.write("%s\n" % item)
    with open('bishop_moves_left.txt', 'w') as f:
        for item in bitboardleft:
            f.write("%s\n" % item)
    with open('bishop_moves_right.txt', 'w') as f:
        for item in bitboardright:
            f.write("%s\n" % item)

get_bishop_moves_dirs()