// util.c
#include "util.h"
#include <ctype.h> // for toupper
#include <string.h>

int colCharToIndex(char colChar) {
    colChar = toupper(colChar);
    if (colChar >= 'A' && colChar <= 'I') {
        return colChar - 'A';
    }
    return -1; // Invalid column
}

bool isValidPos(int row, int col) {
    return row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS;
}

bool isFriendlyPiece(const Board board, int row, int col, PieceColor currentPlayerColor) {
    if (!isValidPos(row, col)) return false; // Not a valid position
    return board[row][col].color == currentPlayerColor;
}

void findKingPosition(const Board board, PieceColor color, int* kingRow, int* kingCol) {
    PieceType kingType = (color == RED) ? RED_KING : BLACK_KING;
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            if (board[r][c].type == kingType && board[r][c].color == color) {
                *kingRow = r;
                *kingCol = c;
                return;
            }
        }
    }
    *kingRow = -1; // Not found (should not happen in a valid game)
    *kingCol = -1;
}