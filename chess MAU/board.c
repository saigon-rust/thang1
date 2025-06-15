// board.c
#include "board.h"
#include <stdio.h>
#include <stdlib.h> // <-- Thêm thư viện này
#ifdef _WIN32
#include <windows.h>
#endif

// Các mã ANSI escape codes cho màu sắc
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_BLUE    "\x1b[34m" // Dùng màu xanh cho quân đen để dễ nhìn hơn trên terminal
#define ANSI_COLOR_RESET   "\x1b[0m"
void initializeBoard(Board board) {
    // Đặt tất cả về EMPTY trước
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            board[r][c] = (Piece){EMPTY, NO_COLOR};
        }
    }

    // Đặt quân Đỏ
    board[0][0] = (Piece){RED_ROOK, RED};
    board[0][1] = (Piece){RED_HORSE, RED};
    board[0][2] = (Piece){RED_ELEPHANT, RED};
    board[0][3] = (Piece){RED_ADVISOR, RED};
    board[0][4] = (Piece){RED_KING, RED};
    board[0][5] = (Piece){RED_ADVISOR, RED};
    board[0][6] = (Piece){RED_ELEPHANT, RED};
    board[0][7] = (Piece){RED_HORSE, RED};
    board[0][8] = (Piece){RED_ROOK, RED};

    board[2][1] = (Piece){RED_CANNON, RED};
    board[2][7] = (Piece){RED_CANNON, RED};

    board[3][0] = (Piece){RED_PAWN, RED};
    board[3][2] = (Piece){RED_PAWN, RED};
    board[3][4] = (Piece){RED_PAWN, RED};
    board[3][6] = (Piece){RED_PAWN, RED};
    board[3][8] = (Piece){RED_PAWN, RED};

    // Đặt quân Đen (tương tự, với hàng 9, 7, 6)
    board[9][0] = (Piece){BLACK_ROOK, BLACK};
    board[9][1] = (Piece){BLACK_HORSE, BLACK};
    board[9][2] = (Piece){BLACK_ELEPHANT, BLACK};
    board[9][3] = (Piece){BLACK_ADVISOR, BLACK};
    board[9][4] = (Piece){BLACK_KING, BLACK};
    board[9][5] = (Piece){BLACK_ADVISOR, BLACK};
    board[9][6] = (Piece){BLACK_ELEPHANT, BLACK};
    board[9][7] = (Piece){BLACK_HORSE, BLACK};
    board[9][8] = (Piece){BLACK_ROOK, BLACK};

    board[7][1] = (Piece){BLACK_CANNON, BLACK};
    board[7][7] = (Piece){BLACK_CANNON, BLACK};

    board[6][0] = (Piece){BLACK_PAWN, BLACK};
    board[6][2] = (Piece){BLACK_PAWN, BLACK};
    board[6][4] = (Piece){BLACK_PAWN, BLACK};
    board[6][6] = (Piece){BLACK_PAWN, BLACK};
    board[6][8] = (Piece){BLACK_PAWN, BLACK};
}

const char* getPieceSymbol(PieceType type) {
    switch (type) {
        case RED_KING:      return " K ";
        case RED_ADVISOR:   return " S ";
        case RED_ELEPHANT:  return " T ";
        case RED_ROOK:      return " X ";
        case RED_CANNON:    return " P ";
        case RED_HORSE:     return " M ";
        case RED_PAWN:      return " B ";
        case BLACK_KING:    return " k ";
        case BLACK_ADVISOR: return " s ";
        case BLACK_ELEPHANT:return " t ";
        case BLACK_ROOK:    return " x ";
        case BLACK_CANNON:  return " p ";
        case BLACK_HORSE:   return " m ";
        case BLACK_PAWN:    return " b ";
        case EMPTY:         return " . ";
        default:            return " ? ";
    }
}

void printBoard(const Board board) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    printf("\n   a  b  c  d  e  f  g  h  i\n");
    printf("  ---------------------------\n");
    for (int r = 0; r < BOARD_ROWS; r++) {
        printf("%2d|", 9 - r);
        for (int c = 0; c < BOARD_COLS; c++) {
            Piece currentPiece = board[r][c];
            if (currentPiece.color == RED) {
                printf(ANSI_COLOR_RED "%s" ANSI_COLOR_RESET, getPieceSymbol(currentPiece.type));
            } else if (currentPiece.color == BLACK) {
                printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET, getPieceSymbol(currentPiece.type));
            } else {
                printf("%s", getPieceSymbol(currentPiece.type)); // Quân trống không màu
            }
        }
        printf("|\n");
    }
    printf("  ---------------------------\n");
    printf("   a  b  c  d  e  f  g  h  i\n");
}