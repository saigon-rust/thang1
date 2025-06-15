// board.h
#ifndef BOARD_H
#define BOARD_H

#include "chess_types.h"

// Khởi tạo bàn cờ về trạng thái ban đầu
void initializeBoard(Board board);

// In bàn cờ ra console
void printBoard(const Board board);

// Lấy tên biểu tượng của quân cờ để in ra
const char* getPieceSymbol(PieceType type);

#endif // BOARD_H