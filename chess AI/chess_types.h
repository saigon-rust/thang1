// chess_types.h
#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

typedef enum {
    EMPTY = 0,
    RED_KING, RED_ADVISOR, RED_ELEPHANT, RED_ROOK, RED_CANNON, RED_HORSE, RED_PAWN,
    BLACK_KING, BLACK_ADVISOR, BLACK_ELEPHANT, BLACK_ROOK, BLACK_CANNON, BLACK_HORSE, BLACK_PAWN
} PieceType;

typedef enum {
    NO_COLOR = 0,
    RED,
    BLACK
} PieceColor;

// Cấu trúc biểu diễn một quân cờ
typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

// Định nghĩa kích thước bàn cờ
#define BOARD_ROWS 10
#define BOARD_COLS 9

// Bàn cờ là một mảng 2D các quân cờ
typedef Piece Board[BOARD_ROWS][BOARD_COLS];

// Cấu trúc để biểu diễn một nước đi
typedef struct {
    int startRow, startCol;
    int endRow, endCol;
} Move;

#endif // CHESS_TYPES_H