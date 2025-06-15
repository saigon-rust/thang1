// game.h
#ifndef GAME_H
#define GAME_H

#include "chess_types.h"
#include <stdbool.h>

typedef enum {
    GAME_RUNNING,
    GAME_RED_WINS,
    GAME_BLACK_WINS,
    GAME_DRAW,
    GAME_EXIT
} GameState;

typedef struct {
    Board board;
    PieceColor currentPlayer;
    GameState state;
    // Có thể thêm lịch sử nước đi, đếm nước đi, v.v.
} Game;

// Khởi tạo game mới
void initializeGame(Game* game);

// Xử lý một lượt đi của người chơi
bool processPlayerTurn(Game* game);

// Kiểm tra trạng thái kết thúc game
void checkGameEndConditions(Game* game);

#endif // GAME_H