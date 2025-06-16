#ifndef AI_H
#define AI_H

#include "chess_types.h"
#include <limits.h> // For INT_MIN, INT_MAX

// Hàm chính cho AI để tìm nước đi tốt nhất
Move findBestMove(Board board, PieceColor aiPlayer, int depth);

#endif // AI_H