// game.c
#include "game.h"
#include "board.h"
#include "move_validation.h"
#include "util.h"
#include "ai.h"
#include <stdio.h>
#include <string.h> // For memcpy
#include <stdlib.h> // For exit

void initializeGame(Game* game) {
    initializeBoard(game->board);
    game->currentPlayer = BLACK; // Đỏ đi trước
    game->state = GAME_RUNNING;
    printf("Game cờ tướng bắt đầu! Đỏ đi trước.\n");
}

// Hàm thực hiện nước đi trên bàn cờ
static void makeMove(Board board, int startRow, int startCol, int endRow, int endCol) {
    board[endRow][endCol] = board[startRow][startCol];
    board[startRow][startCol] = (Piece){EMPTY, NO_COLOR};
}

bool processPlayerTurn(Game* game) {
    int startRow, startCol, endRow, endCol;
    char startColChar, endColChar;

    printf("\nLượt của %s (Nhập nước đi, ví dụ: a0 a1 hoặc exit):\n",
           (game->currentPlayer == RED) ? "Đỏ" : "Đen");

    char input[10];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        game->state = GAME_EXIT;
        return false;
    }

    if (strcmp(input, "exit\n") == 0) {
        game->state = GAME_EXIT;
        return false;
    }

    // Phân tích input: e.g., "a0 a1" -> 'a' 0 'a' 1
    // Định dạng mong muốn: "cột_bắt_đầu hàng_bắt_đầu cột_kết_thúc hàng_kết_thúc"
    // Ví dụ: a0 a1 (hàng 0 cột a đến hàng 1 cột a)
    // Hoặc theo cách hiển thị: a9 a8 (hàng 9 cột a đến hàng 8 cột a)
    // Cần chuyển đổi tọa độ hiển thị (hàng 9-0) sang chỉ số mảng (hàng 0-9)
    if (sscanf(input, "%c%d %c%d", &startColChar, &startRow, &endColChar, &endRow) != 4) {
        printf("Đầu vào không hợp lệ. Vui lòng nhập theo định dạng 'cột_bắt_đầu hàng_bắt_đầu cột_kết_thúc hàng_kết_thúc' (ví dụ: a9 a8).\n");
        return false;
    }

    // Chuyển đổi tọa độ hiển thị sang chỉ số mảng
    // Hàng 9 -> index 0, Hàng 0 -> index 9
    startRow = 9 - startRow;
    endRow = 9 - endRow;

    startCol = colCharToIndex(startColChar);
    endCol = colCharToIndex(endColChar);

    if (!isValidPos(startRow, startCol) || !isValidPos(endRow, endCol)) {
        printf("Vị trí không hợp lệ trên bàn cờ.\n");
        return false;
    }

    if (isValidMove(game->board, startRow, startCol, endRow, endCol, game->currentPlayer)) {
        makeMove(game->board, startRow, startCol, endRow, endCol);

        // Chuyển lượt
        game->currentPlayer = (game->currentPlayer == RED) ? BLACK : RED;
        return true;
    } else {
        printf("Nước đi không hợp lệ. Vui lòng thử lại.\n");
        return false;
    }
}

// Hàm xử lý lượt đi của máy
void processAITurn(Game* game, int aiDepth) {
    printf("\nLượt của máy (%s). Máy đang tính toán...\n", (game->currentPlayer == RED) ? "Đỏ" : "Đen");
    Move bestMove = findBestMove(game->board, game->currentPlayer, aiDepth);

    if (bestMove.startRow == -1) {
        printf("Máy không tìm thấy nước đi hợp lệ. Trò chơi có thể kết thúc.\n");
        // Xử lý trường hợp bí nước hoặc hòa
        if (isKingInCheck(game->board, game->currentPlayer)) {
            game->state = (game->currentPlayer == RED) ? GAME_BLACK_WINS : GAME_RED_WINS; // Máy bị chiếu bí
        } else {
            game->state = GAME_DRAW; // Hòa (bí nước)
        }
        return;
    }

    // Gọi makeMove với 4 tham số riêng lẻ
    makeMove(game->board, bestMove.startRow, bestMove.startCol, bestMove.endRow, bestMove.endCol);
    printf("Máy đi: %c%d -> %c%d\n",
           (char)('a' + bestMove.startCol), 9 - bestMove.startRow,
           (char)('a' + bestMove.endCol), 9 - bestMove.endRow);

    // Chuyển lượt
    game->currentPlayer = (game->currentPlayer == RED) ? BLACK : RED;
}

void checkGameEndConditions(Game* game) {
    // Kiểm tra chiếu bí
    // Nếu tướng của người chơi hiện tại (người vừa được chuyển lượt) bị chiếu và không có nước đi nào hợp lệ để thoát khỏi chiếu
    PieceColor opponentColor = game->currentPlayer; // Người chơi vừa đi
    PieceColor kingColor = game->currentPlayer;     // Tướng của người chơi vừa đi

    // Kiểm tra xem tướng của người chơi tiếp theo (người vừa được chuyển lượt) có bị chiếu không
    if (isKingInCheck(game->board, kingColor)) {
        // Nếu bị chiếu, kiểm tra xem có bất kỳ nước đi nào hợp lệ để thoát khỏi chiếu không
        bool canEscapeCheck = false;
        // Duyệt qua tất cả các quân của người chơi hiện tại
        for (int r = 0; r < BOARD_ROWS; r++) {
            for (int c = 0; c < BOARD_COLS; c++) {
                if (game->board[r][c].color == game->currentPlayer) {
                    // Duyệt qua tất cả các ô trên bàn cờ
                    for (int tr = 0; tr < BOARD_ROWS; tr++) {
                        for (int tc = 0; tc < BOARD_COLS; tc++) {
                            // Tạo một bản sao tạm thời của bàn cờ
                            Board tempBoard;
                            memcpy(tempBoard, game->board, sizeof(Board));

                            if (isValidMove(tempBoard, r, c, tr, tc, game->currentPlayer)) {
                                // Nếu nước đi này là hợp lệ và không làm tướng bị chiếu
                                // (isValidMove đã kiểm tra điều này)
                                canEscapeCheck = true;
                                break;
                            }
                        }
                        if (canEscapeCheck) break;
                    }
                }
                if (canEscapeCheck) break;
            }
            if (canEscapeCheck) break;
        }

        if (!canEscapeCheck) {
            if (game->currentPlayer == RED) {
                game->state = GAME_BLACK_WINS;
            } else {
                game->state = GAME_RED_WINS;
            }
        }
    }
    // TODO: Thêm kiểm tra các điều kiện hòa (bí nước, lặp lại nước đi, v.v.)
}