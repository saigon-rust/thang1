#include "ai.h"
#include "board.h" // For printBoard (debugging)
#include "move_validation.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // For srand, rand

// Giá trị quân cờ để đánh giá bàn cờ
// Các giá trị này có thể tinh chỉnh để AI chơi tốt hơn
#define KING_VALUE      10000
#define ADVISOR_VALUE   200
#define ELEPHANT_VALUE  200
#define ROOK_VALUE      900
#define CANNON_VALUE    450
#define HORSE_VALUE     400
#define PAWN_VALUE      100 // Tốt chưa qua sông
#define CROSSED_PAWN_VALUE 200 // Tốt đã qua sông

// Forward declarations
static int evaluateBoard(const Board board, PieceColor aiPlayer);
static void generateAllValidMoves(const Board board, PieceColor currentPlayer, Move moves[], int* numMoves);
static int minimax(Board board, int depth, int alpha, int beta, PieceColor currentPlayer, PieceColor aiPlayer);
// *** KHÔNG ĐỔI makeMove này, nó là bản local của AI ***
static void makeMove(Board board, int startRow, int startCol, int endRow, int endCol);
static void unmakeMove(Board board, int startRow, int startCol, int endRow, int endCol, Piece capturedPiece);


// Hàm hỗ trợ để thực hiện nước đi trên một bản sao của bàn cờ
// Giữ nguyên định dạng này như bạn yêu cầu cho ai.c
static void makeMove(Board board, int startRow, int startCol, int endRow, int endCol) {
    board[endRow][endCol] = board[startRow][startCol];
    board[startRow][startCol] = (Piece){EMPTY, NO_COLOR};
}

// Hàm hỗ trợ để hoàn tác nước đi (quan trọng cho Minimax)
// Giữ nguyên định dạng này như bạn yêu cầu cho ai.c
static void unmakeMove(Board board, int startRow, int startCol, int endRow, int endCol, Piece capturedPiece) {
    board[startRow][startCol] = board[endRow][endCol];
    board[endRow][endCol] = capturedPiece;
}

// Hàm đánh giá trạng thái bàn cờ
// Trả về một giá trị số, càng cao càng tốt cho aiPlayer
static int evaluateBoard(const Board board, PieceColor aiPlayer) {
    int score = 0;
    PieceColor opponentPlayer = (aiPlayer == RED) ? BLACK : RED;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            Piece p = board[r][c];
            int pieceValue = 0;

            switch (p.type) {
                case RED_KING:
                case BLACK_KING:
                    pieceValue = KING_VALUE;
                    break;
                case RED_ADVISOR:
                case BLACK_ADVISOR:
                    pieceValue = ADVISOR_VALUE;
                    break;
                case RED_ELEPHANT:
                case BLACK_ELEPHANT:
                    pieceValue = ELEPHANT_VALUE;
                    break;
                case RED_ROOK:
                case BLACK_ROOK:
                    pieceValue = ROOK_VALUE;
                    break;
                case RED_CANNON:
                case BLACK_CANNON:
                    pieceValue = CANNON_VALUE;
                    break;
                case RED_HORSE:
                case BLACK_HORSE:
                    pieceValue = HORSE_VALUE;
                    break;
                case RED_PAWN:
                    pieceValue = PAWN_VALUE;
                    if (r >= 5) { // Đã qua sông
                        pieceValue = CROSSED_PAWN_VALUE;
                    }
                    break;
                case BLACK_PAWN:
                    pieceValue = PAWN_VALUE;
                    if (r <= 4) { // Đã qua sông
                        pieceValue = CROSSED_PAWN_VALUE;
                    }
                    break;
                case EMPTY:
                    continue;
            }

            if (p.color == aiPlayer) {
                score += pieceValue;
            } else {
                score -= pieceValue;
            }
        }
    }

    // Thêm điểm cho trạng thái chiếu tướng (nếu tướng đối thủ bị chiếu, là điểm cộng lớn)
    // Hoặc trừ điểm nếu tướng của mình bị chiếu
    if (isKingInCheck(board, opponentPlayer)) {
        score += 500; // Tăng điểm nếu chiếu tướng đối thủ
    }
    if (isKingInCheck(board, aiPlayer)) {
        score -= 500; // Giảm điểm nếu tướng của mình bị chiếu
    }

    // TODO: Thêm các yếu tố đánh giá phức tạp hơn:
    // - Kiểm soát trung tâm
    // - Tính linh hoạt của quân cờ
    // - Vị trí của từng quân cờ (bảng điểm vị trí)
    // - Bảo vệ tướng

    return score;
}

// Hàm tạo tất cả các nước đi hợp lệ cho người chơi hiện tại
static void generateAllValidMoves(const Board board, PieceColor currentPlayer, Move moves[], int* numMoves) {
    *numMoves = 0;
    for (int startRow = 0; startRow < BOARD_ROWS; startRow++) {
        for (int startCol = 0; startCol < BOARD_COLS; startCol++) {
            if (board[startRow][startCol].color == currentPlayer) {
                for (int endRow = 0; endRow < BOARD_ROWS; endRow++) {
                    for (int endCol = 0; endCol < BOARD_COLS; endCol++) {
                        // Tạo một bản sao tạm thời của bàn cờ để kiểm tra nước đi
                        Board tempBoard;
                        memcpy(tempBoard, board, sizeof(Board));

                        if (isValidMove(tempBoard, startRow, startCol, endRow, endCol, currentPlayer)) {
                            moves[*numMoves].startRow = startRow;
                            moves[*numMoves].startCol = startCol;
                            moves[*numMoves].endRow = endRow;
                            moves[*numMoves].endCol = endCol;
                            (*numMoves)++;
                            // Tăng kích thước mảng moves nếu cần, hoặc đặt giới hạn
                            if (*numMoves >= 2000) { // Giới hạn số nước đi để tránh tràn bộ nhớ
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Thuật toán Minimax với Alpha-Beta Pruning
static int minimax(Board board, int depth, int alpha, int beta, PieceColor currentPlayer, PieceColor aiPlayer) {
    // Trường hợp cơ sở: đạt đến độ sâu mong muốn hoặc game kết thúc
    if (depth == 0) {
        return evaluateBoard(board, aiPlayer);
    }

    // Kiểm tra chiếu bí
    // Tạm thời bỏ qua kiểm tra hòa/chiếu bí sâu trong Minimax để đơn giản
    // vì việc này sẽ được xử lý ở hàm checkGameEndConditions chính.
    // Nếu tướng bị chiếu bí, giá trị điểm sẽ rất lớn (cho người chiến thắng)
    // Hoặc rất nhỏ (cho người bị chiếu bí)
    PieceColor opponentColor = (currentPlayer == RED) ? BLACK : RED;
    if (isKingInCheck(board, currentPlayer) && !canPlayerMakeAnyValidMove(board, currentPlayer)) {
        return (currentPlayer == aiPlayer) ? INT_MIN : INT_MAX; // AI bị chiếu bí -> rất tệ, AI chiếu bí đối thủ -> rất tốt
    }
    if (isKingInCheck(board, opponentColor) && !canPlayerMakeAnyValidMove(board, opponentColor)) {
        return (currentPlayer == aiPlayer) ? INT_MAX : INT_MIN; // AI chiếu bí đối thủ -> rất tốt, AI bị chiếu bí -> rất tệ
    }


    Move moves[2000]; // Mảng để lưu trữ các nước đi
    int numMoves = 0;
    generateAllValidMoves(board, currentPlayer, moves, &numMoves);

    if (numMoves == 0) {
        // Trường hợp hết nước đi (bị bí hoặc hòa)
        if (isKingInCheck(board, currentPlayer)) {
             return (currentPlayer == aiPlayer) ? INT_MIN : INT_MAX; // Bị chiếu bí
        }
        return 0; // Hòa (hết nước đi nhưng không bị chiếu)
    }

    if (currentPlayer == aiPlayer) { // Lượt của AI (maximizing player)
        int maxEval = INT_MIN;
        for (int i = 0; i < numMoves; i++) {
            Board newBoard;
            memcpy(newBoard, board, sizeof(Board)); // Tạo bản sao của bàn cờ
            Piece capturedPiece = newBoard[moves[i].endRow][moves[i].endCol]; // Lưu quân bị ăn

            // Gọi makeMove với 4 tham số riêng lẻ
            makeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol);
            int eval = minimax(newBoard, depth - 1, alpha, beta, opponentColor, aiPlayer);
            // Gọi unmakeMove với 4 tham số riêng lẻ và quân bị ăn
            unmakeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, capturedPiece);

            maxEval = (eval > maxEval) ? eval : maxEval;
            alpha = (alpha > eval) ? alpha : eval;
            if (beta <= alpha) {
                break; // Alpha-beta pruning
            }
        }
        return maxEval;
    } else { // Lượt của người chơi (minimizing player)
        int minEval = INT_MAX;
        for (int i = 0; i < numMoves; i++) {
            Board newBoard;
            memcpy(newBoard, board, sizeof(Board)); // Tạo bản sao của bàn cờ
            Piece capturedPiece = newBoard[moves[i].endRow][moves[i].endCol]; // Lưu quân bị ăn

            // Gọi makeMove với 4 tham số riêng lẻ
            makeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol);
            int eval = minimax(newBoard, depth - 1, alpha, beta, opponentColor, aiPlayer);
            // Gọi unmakeMove với 4 tham số riêng lẻ và quân bị ăn
            unmakeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, capturedPiece);

            minEval = (eval < minEval) ? eval : minEval;
            beta = (beta < eval) ? beta : eval;
            if (beta <= alpha) {
                break; // Alpha-beta pruning
            }
        }
        return minEval;
    }
}

// Hàm kiểm tra xem người chơi có bất kỳ nước đi hợp lệ nào không
// (để kiểm tra chiếu bí/bí nước)


// Hàm chính cho AI để tìm nước đi tốt nhất
Move findBestMove(Board board, PieceColor aiPlayer, int depth) {
    srand(time(NULL)); // Khởi tạo seed cho số ngẫu nhiên

    Move bestMove = {-1, -1, -1, -1};
    int bestEval = INT_MIN; // Cho AI (maximizing player)
    Move moves[2000];
    int numMoves = 0;

    generateAllValidMoves(board, aiPlayer, moves, &numMoves);

    // Xáo trộn các nước đi để AI không chơi quá "định hướng" và có vẻ ngẫu nhiên hơn một chút
    // Điều này giúp tránh việc AI luôn chọn cùng cùng một nước đi trong các tình huống tương tự
    for (int i = 0; i < numMoves; i++) {
        int j = rand() % numMoves;
        Move temp = moves[i];
        moves[i] = moves[j];
        moves[j] = temp;
    }

    if (numMoves == 0) {
        // Không có nước đi nào hợp lệ (có thể là chiếu bí hoặc hòa)
        return bestMove; // Trả về nước đi không hợp lệ
    }

    for (int i = 0; i < numMoves; i++) {
        Board newBoard;
        memcpy(newBoard, board, sizeof(Board));
        Piece capturedPiece = newBoard[moves[i].endRow][moves[i].endCol];

        // Gọi makeMove với 4 tham số riêng lẻ
        makeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol);
        int eval = minimax(newBoard, depth - 1, INT_MIN, INT_MAX, (aiPlayer == RED) ? BLACK : RED, aiPlayer);
        // Gọi unmakeMove với 4 tham số riêng lẻ và quân bị ăn
        unmakeMove(newBoard, moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, capturedPiece);

        // Debugging:
        // printf("Move: (%d,%d)->(%d,%d) Eval: %d\n", moves[i].startRow, moves[i].startCol, moves[i].endRow, moves[i].endCol, eval);


        if (eval > bestEval) {
            bestEval = eval;
            bestMove = moves[i];
        }
    }
    return bestMove;
}