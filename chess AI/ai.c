#include "ai.h"
#include "board.h"
#include "move_validation.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

#define MAX_MOVES 256
#define MAX_THREADS 10

// Giá trị quân cờ
#define KING_VALUE      10000
#define ADVISOR_VALUE   300
#define ELEPHANT_VALUE  300
#define ROOK_VALUE      900
#define CANNON_VALUE    500
#define HORSE_VALUE     400
#define PAWN_VALUE      120
#define CROSSED_PAWN_VALUE 220

typedef struct {
    Board board;
    Move move;
    PieceColor aiPlayer;
    int depth;
    int eval;
} ThreadData;

static int evaluateBoard(const Board board, PieceColor aiPlayer);
static void generateAllValidMoves(const Board board, PieceColor currentPlayer, Move moves[], int* numMoves);
static int minimax(Board board, int depth, int alpha, int beta, PieceColor currentPlayer, PieceColor aiPlayer);
static void makeMove(Board board, int sr, int sc, int er, int ec);
static void unmakeMove(Board board, int sr, int sc, int er, int ec, Piece captured);

void* minimaxThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    Board newBoard;
    memcpy(newBoard, data->board, sizeof(Board));
    Piece captured = newBoard[data->move.endRow][data->move.endCol];
    makeMove(newBoard, data->move.startRow, data->move.startCol, data->move.endRow, data->move.endCol);

    data->eval = minimax(newBoard, data->depth - 1, INT_MIN, INT_MAX, 
                         (data->aiPlayer == RED) ? BLACK : RED, data->aiPlayer);

    return NULL;
}

static void makeMove(Board board, int sr, int sc, int er, int ec) {
    board[er][ec] = board[sr][sc];
    board[sr][sc] = (Piece){EMPTY, NO_COLOR};
}

static void unmakeMove(Board board, int sr, int sc, int er, int ec, Piece captured) {
    board[sr][sc] = board[er][ec];
    board[er][ec] = captured;
}

static int piecePositionValue(PieceType type, int row, int col, PieceColor color) {
    int value = 0;

    // Đổi chiều bàn cờ nếu là quân đen
    int r = (color == RED) ? row : 9 - row;
    int c = (color == RED) ? col : 8 - col;

    switch (type) {
        case RED_PAWN:
        case BLACK_PAWN: {
            int crossed = (color == RED) ? (row >= 5) : (row <= 4);
            value = crossed ? CROSSED_PAWN_VALUE : PAWN_VALUE;

            // Bonus nếu chốt chiếm vị trí có thể cản mã hoặc uy hiếp tốt
            if (crossed && (col == 0 || col == 2 || col == 6 || col == 8))
                value += 10;
            break;
        }
        case RED_ROOK:
        case BLACK_ROOK: {
            value = ROOK_VALUE;

            // Giữ trục dọc thì ưu tiên (cột thẳng, dễ kiểm soát)
            if (c == 1 || c == 3 || c == 5 || c == 7 || r == 1 || r == 4 || r == 5 || r == 8)
                value += 20;

            // Ở hàng cuối (bảo vệ tướng)
            if (r == 9)
                value += 10;
            break;
        }
        case RED_HORSE:
        case BLACK_HORSE: {
            value = HORSE_VALUE;

            // Bonus nếu ở gần trung tâm (để phát triển)
            if (r >= 3 && r <= 6 && c >= 2 && c <= 6)
                value += 15;

            break;
        }
        case RED_CANNON:
        case BLACK_CANNON: {
            value = CANNON_VALUE;
            // uu tien phao dau
            if (r == 2 && c == 4)
                value += 25;
        }
        case RED_ELEPHANT:
        case BLACK_ELEPHANT: {
            value = ELEPHANT_VALUE;

            // Ưu tiên giữ thành
            if ((r == 0 || r == 4) && (c == 2 || c == 6))
                value += 10;
            break;
        }
        case RED_ADVISOR:
        case BLACK_ADVISOR: {
            value = ADVISOR_VALUE;

            // Ưu tiên giữ cung
            if ((r >= 0 && r <= 2) && (c >= 3 && c <= 5))
                value += 10;
            break;
        }
        case RED_KING:
        case BLACK_KING: {
            value = KING_VALUE;

            // Nếu bị ra khỏi cung => trừ điểm
            if (c < 3 || c > 5 || r < 0 || r > 2)
                value -= 100;

            break;
        }
    }

    return value;
}


static int evaluateBoard(const Board board, PieceColor aiPlayer) {
    int score = 0;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            Piece p = board[r][c];
            if (p.type == EMPTY) continue;

            int base = 0;
            switch (p.type) {
                case RED_KING: case BLACK_KING: base = KING_VALUE; break;
                case RED_ADVISOR: case BLACK_ADVISOR: base = ADVISOR_VALUE; break;
                case RED_ELEPHANT: case BLACK_ELEPHANT: base = ELEPHANT_VALUE; break;
                case RED_ROOK: case BLACK_ROOK: base = ROOK_VALUE; break;
                case RED_CANNON: case BLACK_CANNON: base = CANNON_VALUE; break;
                case RED_HORSE: case BLACK_HORSE: base = HORSE_VALUE; break;
                case RED_PAWN: case BLACK_PAWN:
                    base = piecePositionValue(p.type, r, c, p.color); break;
                default: continue;
            }

            score += (p.color == aiPlayer) ? base : -base;
        }
    }

    if (isKingInCheck(board, aiPlayer)) score -= 800;
    if (isKingInCheck(board, (aiPlayer == RED) ? BLACK : RED)) score += 800;

    return score;
}

static void generateAllValidMoves(const Board board, PieceColor player, Move moves[], int* numMoves) {
    *numMoves = 0;
    for (int sr = 0; sr < BOARD_ROWS; sr++) {
        for (int sc = 0; sc < BOARD_COLS; sc++) {
            if (board[sr][sc].color != player) continue;

            for (int er = 0; er < BOARD_ROWS; er++) {
                for (int ec = 0; ec < BOARD_COLS; ec++) {
                    if (isValidMove(board, sr, sc, er, ec, player)) {
                        moves[*numMoves] = (Move){sr, sc, er, ec};
                        (*numMoves)++;
                        if (*numMoves >= MAX_MOVES) return;
                    }
                }
            }
        }
    }
}

static int minimax(Board board, int depth, int alpha, int beta, PieceColor currentPlayer, PieceColor aiPlayer) {
    if (depth == 0)
        return evaluateBoard(board, aiPlayer);

    Move moves[MAX_MOVES];
    int numMoves = 0;
    generateAllValidMoves(board, currentPlayer, moves, &numMoves);

    if (numMoves == 0)
        return isKingInCheck(board, currentPlayer) ?
               (currentPlayer == aiPlayer ? INT_MIN : INT_MAX) : 0;

    int bestEval = (currentPlayer == aiPlayer) ? INT_MIN : INT_MAX;
    PieceColor nextPlayer = (currentPlayer == RED) ? BLACK : RED;

    for (int i = 0; i < numMoves; i++) {
        Piece captured = board[moves[i].endRow][moves[i].endCol];
        makeMove(board, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol);
        int eval = minimax(board, depth - 1, alpha, beta, nextPlayer, aiPlayer);
        unmakeMove(board, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, captured);

        if (currentPlayer == aiPlayer) {
            bestEval = (eval > bestEval) ? eval : bestEval;
            alpha = (eval > alpha) ? eval : alpha;
        } else {
            bestEval = (eval < bestEval) ? eval : bestEval;
            beta = (eval < beta) ? eval : beta;
        }

        if (beta <= alpha) break;
    }

    return bestEval;
}

Move findBestMove(Board board, PieceColor aiPlayer, int depth) {
    Move bestMove = {-1, -1, -1, -1};
    int bestEval = INT_MIN;

    Move moves[MAX_MOVES];
    int numMoves = 0;
    generateAllValidMoves(board, aiPlayer, moves, &numMoves);

    if (numMoves == 0) return bestMove;

    ThreadData data[MAX_THREADS];
    pthread_t threads[MAX_THREADS];
    int running = 0;

    for (int i = 0; i < numMoves; i++) {
        Board boardCopy;
        memcpy(boardCopy, board, sizeof(Board));

        data[running] = (ThreadData){
            .move = moves[i],
            .aiPlayer = aiPlayer,
            .depth = depth
        };
        memcpy(data[running].board, board, sizeof(Board));

        pthread_create(&threads[running], NULL, minimaxThread, &data[running]);
        running++;

        if (running == MAX_THREADS || i == numMoves - 1) {
            for (int j = 0; j < running; j++) {
                pthread_join(threads[j], NULL);
                if (data[j].eval > bestEval) {
                    bestEval = data[j].eval;
                    bestMove = data[j].move;
                }
            }
            running = 0;
        }
    }

    return bestMove;
}

