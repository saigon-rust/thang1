// main.c
#include <stdio.h>
#include <stdlib.h> // For exit()
#include "game.h"
#include "board.h"

int main() {
    Game game;
    initializeGame(&game);

    while (game.state == GAME_RUNNING) {
        printBoard(game.board);
        if (!processPlayerTurn(&game)) {
            // Nếu processPlayerTurn trả về false (lỗi nhập hoặc exit)
            if (game.state == GAME_EXIT) {
                printf("Game kết thúc. Hẹn gặp lại!\n");
                break;
            }
            // Nếu không phải exit, thì là lỗi nhập/nước đi không hợp lệ, tiếp tục vòng lặp
            continue;
        }
        checkGameEndConditions(&game);
    }

    // In kết quả cuối cùng
    if (game.state == GAME_RED_WINS) {
        printBoard(game.board); // In bàn cờ cuối cùng
        printf("\n=====================================\n");
        printf("         ĐỎ THẮNG CUỘC!!!\n");
        printf("=====================================\n");
    } else if (game.state == GAME_BLACK_WINS) {
        printBoard(game.board); // In bàn cờ cuối cùng
        printf("\n=====================================\n");
        printf("         ĐEN THẮNG CUỘC!!!\n");
        printf("=====================================\n");
    } else if (game.state == GAME_DRAW) {
        printf("\n=====================================\n");
        printf("             HÒA!!!\n");
        printf("=====================================\n");
    }

    return 0;
}

//gcc -o chess_game main.c game.c board.c util.c move_validation.c -lm