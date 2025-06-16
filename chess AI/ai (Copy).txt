#include "ai.h"
#include "board.h"
#include "move_validation.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define KING_VALUE      10000
#define ADVISOR_VALUE   200
#define ELEPHANT_VALUE  200
#define ROOK_VALUE      900
#define CANNON_VALUE    450
#define HORSE_VALUE     400
#define PAWN_VALUE      100
#define CROSSED_PAWN_VALUE 200

static int evaluateBoard(const Board board, PieceColor aiPlayer);
static void generateAllValidMoves(const Board board, PieceColor currentPlayer, Move moves[], int* numMoves);
static int minimax(Board board, int depth, int alpha, int beta, PieceColor currentPlayer, PieceColor aiPlayer);
static void makeMove(Board board, int startRow, int startCol, int endRow, int endCol);
static void unmakeMove(Board board, int startRow, int startCol, int endRow, int endCol, Piece capturedPiece);

typedef struct {
    Board board;
    Move move;
    PieceColor aiPlayer;
    int depth;
    int eval;
} ThreadData;

void* minimaxThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    Board newBoard;
    memcpy(newBoard, data->board, sizeof(Board));
    Piece capturedPiece = newBoard[data->move.endRow][data->move.endCol];

    makeMove(newBoard, data->move.startRow, data->move.startCol, data->move.endRow, data->move.endCol);
    int eval = minimax(newBoard, data->depth - 1, INT_MIN, INT_MAX, 
                      (data->aiPlayer == RED) ? BLACK : RED, data->aiPlayer);
    unmakeMove(newBoard, data->move.startRow, data->move.startCol, data->move.endRow, data->move.endCol, capturedPiece);

    data->eval = eval;
    return NULL;
}

static void makeMove(Board board, int startRow, int startCol, int endRow, int endCol) {
    board[endRow][endCol] = board[startRow][startCol];
    board[startRow][startCol] = (Piece){EMPTY, NO_COLOR};
}

static void unmakeMove(Board board, int startRow, int startCol, int endRow, int endCol, Piece capturedPiece) {
    board[startRow][startCol] = board[endRow][endCol];
    board[endRow][endCol] = capturedPiece;
}

static int evaluateBoard(const Board board, PieceColor aiPlayer) {
    int score = 0;
    PieceColor opponentPlayer = (aiPlayer == RED) ? BLACK : RED;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            Piece p = board[r][c];
            int pieceValue = 0;

            switch (p.type) {
                case RED_KING: case BLACK_KING:
                    pieceValue = KING_VALUE; break;
                case RED_ADVISOR: case BLACK_ADVISOR:
                    pieceValue = ADVISOR_VALUE; break;
                case RED_ELEPHANT: case BLACK_ELEPHANT:
                    pieceValue = ELEPHANT_VALUE; break;
                case RED_ROOK: case BLACK_ROOK:
                    pieceValue = ROOK_VALUE; break;
                case RED_CANNON: case BLACK_CANNON:
                    pieceValue = CANNON_VALUE; break;
                case RED_HORSE: case BLACK_HORSE:
                    pieceValue = HORSE_VALUE; break;
                case RED_PAWN:
                    pieceValue = (r >= 5) ? CROSSED_PAWN_VALUE : PAWN_VALUE; break;
                case BLACK_PAWN:
                    pieceValue = (r <= 4) ? CROSSED_PAWN_VALUE : PAWN_VALUE; break;
                case EMPTY: continue;
            }

            score += (p.color == aiPlayer) ? pieceValue : -pieceValue;
        }
    }

    if (isKingInCheck(board, opponentPlayer)) score += 500;
    if (isKingInCheck(board, aiPlayer)) score -= 500;

    return score;
}

static void generateAllValidMoves(const Board board, PieceColor currentPlayer, Move moves[], int* numMoves) {
    *numMoves = 0;
    for (int startRow = 0; startRow < BOARD_ROWS; startRow++) {
        for (int startCol = 0; startCol < BOARD_COLS; startCol++) {
            if (board[startRow][startCol].color == currentPlayer) {
                for (int endRow = 0; endRow < BOARD_ROWS; endRow++) {
                    for (int endCol = 0; endCol < BOARD_COLS; endCol++) {
                        Board tempBoard;
                        memcpy(tempBoard, board, sizeof(Board));
                        if (isValidMove(tempBoard, startRow, startCol, endRow, endCol, currentPlayer)) {
                            moves[*numMoves] = (Move){startRow, startCol, endRow, endCol};
                            (*numMoves)++;
                            if (*numMoves >= 2000) return;
                        }
                    }
                }
            }
        }
    }
}

static int minimax(Board board, int depth, int alpha, int beta, PieceColor currentPlayer, PieceColor aiPlayer) {
    if (depth == 0) return evaluateBoard(board, aiPlayer);

    PieceColor opponentColor = (currentPlayer == RED) ? BLACK : RED;
    if (isKingInCheck(board, currentPlayer) && !canPlayerMakeAnyValidMove(board, currentPlayer))
        return (currentPlayer == aiPlayer) ? INT_MIN : INT_MAX;

    if (isKingInCheck(board, opponentColor) && !canPlayerMakeAnyValidMove(board, opponentColor))
        return (currentPlayer == aiPlayer) ? INT_MAX : INT_MIN;

    Move moves[2000];
    int numMoves = 0;
    generateAllValidMoves(board, currentPlayer, moves, &numMoves);

    if (numMoves == 0) {
        if (isKingInCheck(board, currentPlayer))
            return (currentPlayer == aiPlayer) ? INT_MIN : INT_MAX;
        return 0;
    }

    if (currentPlayer == aiPlayer) {
        int maxEval = INT_MIN;
        for (int i = 0; i < numMoves; i++) {
            Board newBoard;
            memcpy(newBoard, board, sizeof(Board));
            Piece capturedPiece = newBoard[moves[i].endRow][moves[i].endCol];
            makeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol);
            int eval = minimax(newBoard, depth - 1, alpha, beta, opponentColor, aiPlayer);
            unmakeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, capturedPiece);
            maxEval = (eval > maxEval) ? eval : maxEval;
            alpha = (alpha > eval) ? alpha : eval;
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (int i = 0; i < numMoves; i++) {
            Board newBoard;
            memcpy(newBoard, board, sizeof(Board));
            Piece capturedPiece = newBoard[moves[i].endRow][moves[i].endCol];
            makeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol);
            int eval = minimax(newBoard, depth - 1, alpha, beta, opponentColor, aiPlayer);
            unmakeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, capturedPiece);
            minEval = (eval < minEval) ? eval : minEval;
            beta = (beta < eval) ? beta : eval;
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

Move findBestMove(Board board, PieceColor aiPlayer, int depth) {
    srand(time(NULL));

    Move bestMove = {-1, -1, -1, -1};
    int bestEval = INT_MIN;

    Move moves[2000];
    int numMoves = 0;
    generateAllValidMoves(board, aiPlayer, moves, &numMoves);

    for (int i = 0; i < numMoves; i++) {
        int j = rand() % numMoves;
        Move temp = moves[i];
        moves[i] = moves[j];
        moves[j] = temp;
    }

    if (numMoves == 0) return bestMove;

    pthread_t threads[2000];
    ThreadData threadData[2000];

    for (int i = 0; i < numMoves; i++) {
        memcpy(threadData[i].board, board, sizeof(Board));
        threadData[i].move = moves[i];
        threadData[i].aiPlayer = aiPlayer;
        threadData[i].depth = depth;
        pthread_create(&threads[i], NULL, minimaxThread, &threadData[i]);
    }

    for (int i = 0; i < numMoves; i++) {
        pthread_join(threads[i], NULL);
        if (threadData[i].eval > bestEval) {
            bestEval = threadData[i].eval;
            bestMove = threadData[i].move;
        }
    }

    return bestMove;
}
