// util.h
#ifndef UTIL_H
#define UTIL_H

#include "chess_types.h"
#include <stdbool.h>

// Chuyển đổi từ ký tự cột (a-i) sang chỉ số (0-8)
int colCharToIndex(char colChar);

// Kiểm tra xem vị trí (row, col) có hợp lệ trên bàn cờ không
bool isValidPos(int row, int col);

// Kiểm tra xem nước đi có phải là tự ăn quân mình không
bool isFriendlyPiece(const Board board, int row, int col, PieceColor currentPlayerColor);

// Hàm hỗ trợ để tìm vị trí của tướng trên bàn cờ
void findKingPosition(const Board board, PieceColor color, int* kingRow, int* kingCol);

#endif // UTIL_H