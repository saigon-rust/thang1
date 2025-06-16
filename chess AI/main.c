#include <stdio.h>
#include <stdlib.h> // For exit()
#include "game.h"
#include "board.h"
#include "ai.h" // Thêm include cho AI

int main() {
    Game game;
    int choice;
    int aiDepth = 3; // Độ sâu của thuật toán Minimax (có thể thay đổi)

    printf("Chào mừng đến với Cờ tướng C!\n");
    printf("Bạn muốn chơi với máy hay người khác?\n");
    printf("1. Chơi với người (Người vs Người)\n");
    printf("2. Chơi với máy (Người vs Máy)\n");
    printf("Chọn lựa của bạn: ");
    scanf("%d", &choice);
    // Xóa bộ đệm bàn phím sau scanf
    while (getchar() != '\n');

    initializeGame(&game);

    if (choice == 1) { // Người vs Người
        while (game.state == GAME_RUNNING) {
            printBoard(game.board);
            if (!processPlayerTurn(&game)) {
                if (game.state == GAME_EXIT) {
                    printf("Game kết thúc. Hẹn gặp lại!\n");
                    break;
                }
                continue;
            }
            checkGameEndConditions(&game);
        }
    } else if (choice == 2) { // Người vs Máy
        // Giả sử người chơi là Đỏ, máy là Đen
        printf("Bạn sẽ chơi Đỏ, máy sẽ chơi Đen. Độ sâu AI: %d\n", aiDepth);
        while (game.state == GAME_RUNNING) {
            printBoard(game.board);
            if (game.currentPlayer == RED) { // Lượt người chơi
                if (!processPlayerTurn(&game)) {
                    if (game.state == GAME_EXIT) {
                        printf("Game kết thúc. Hẹn gặp lại!\n");
                        break;
                    }
                    continue;
                }
            } else { // Lượt máy
                processAITurn(&game, aiDepth);
            }
            checkGameEndConditions(&game);
        }
    } else {
        printf("Lựa chọn không hợp lệ. Thoát game.\n");
        return 1;
    }


    // In kết quả cuối cùng
    if (game.state == GAME_RED_WINS) {
        printBoard(game.board); // In bàn cờ cuối cùng
        printf("\n=====================================\n");
        printf(" ĐỎ THẮNG CUỘC!!!\n");
        printf("=====================================\n");
    } else if (game.state == GAME_BLACK_WINS) {
        printBoard(game.board); // In bàn cờ cuối cùng
        printf("\n=====================================\n");
        printf(" ĐEN THẮNG CUỘC!!!\n");
        printf("=====================================\n");
    } else if (game.state == GAME_DRAW) {
        printf("\n=====================================\n");
        printf(" HÒA!!!\n");
        printf("=====================================\n");
    }
    return 0;
}

//gcc -o chess main.c game.c board.c util.c move_validation.c ai.c -lm -pthread
