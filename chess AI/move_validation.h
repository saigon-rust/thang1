// move_validation.h
#ifndef MOVE_VALIDATION_H
#define MOVE_VALIDATION_H

#include "chess_types.h"
#include <stdbool.h>

// Hàm kiểm tra tổng quát cho một nước đi
bool isValidMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);

// Hàm kiểm tra xem tướng có đang bị chiếu không
bool isKingInCheck(const Board board, PieceColor kingColor);

// Hàm kiểm tra xem người chơi có bất kỳ nước đi hợp lệ nào không
bool canPlayerMakeAnyValidMove(const Board board, PieceColor playerColor);

#endif // MOVE_VALIDATION_H