// move_validation.c
#include "move_validation.h"
#include "util.h"
#include <stdlib.h> // for abs
#include <stdio.h> 
#include <string.h>
#include <math.h>

// Forward declarations for specific piece move validations
static bool isValidKingMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);
static bool isValidRookMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);
static bool isValidCannonMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);
static bool isValidHorseMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);
static bool isValidAdvisorMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);
static bool isValidElephantMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);
static bool isValidPawnMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor);

// Helper function to check for obstruction in a straight line (for Rook/Cannon)
static bool isPathClear(const Board board, int r1, int c1, int r2, int c2, int* obstacles) {
    *obstacles = 0;
    if (r1 == r2) { // Horizontal
        for (int c = fmin(c1, c2) + 1; c < fmax(c1, c2); c++) {
            if (board[r1][c].type != EMPTY) {
                (*obstacles)++;
            }
        }
    } else if (c1 == c2) { // Vertical
        for (int r = fmin(r1, r2) + 1; r < fmax(r1, r2); r++) {
            if (board[r][c1].type != EMPTY) {
                (*obstacles)++;
            }
        }
    } else { // Not a straight line
        return false;
    }
    return true;
}

// =========================================================
// Quy tắc di chuyển của từng quân cờ
// =========================================================

static bool isValidKingMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    // 1. Kiểm tra giới hạn cung (Tướng Đỏ: hàng 0-2, cột 3-5; Tướng Đen: hàng 7-9, cột 3-5)
    if (currentPlayerColor == RED) {
        if (!((endRow >= 0 && endRow <= 2) && (endCol >= 3 && endCol <= 5))) return false;
    } else { // BLACK
        if (!((endRow >= 7 && endRow <= 9) && (endCol >= 3 && endCol <= 5))) return false;
    }

    // 2. Chỉ được đi từng ô một (ngang hoặc dọc)
    int dr = abs(endRow - startRow);
    int dc = abs(endCol - startCol);
    if (!((dr == 1 && dc == 0) || (dr == 0 && dc == 1))) return false;

    // 3. Kiểm tra chống tướng (tướng không được đối mặt trực tiếp)
    // Tạm thời di chuyển quân và kiểm tra
    Board tempBoard;
    memcpy(tempBoard, board, sizeof(Board));
    tempBoard[endRow][endCol] = tempBoard[startRow][startCol];
    tempBoard[startRow][startCol] = (Piece){EMPTY, NO_COLOR};

    int oppKingRow, oppKingCol;
    findKingPosition(tempBoard, (currentPlayerColor == RED) ? BLACK : RED, &oppKingRow, &oppKingCol);

    if (oppKingRow != -1 && oppKingCol != -1 && endCol == oppKingCol) { // Cùng cột
        int obstacles = 0;
        isPathClear(tempBoard, endRow, endCol, oppKingRow, oppKingCol, &obstacles);
        if (obstacles == 0) return false; // Tướng đối mặt
    }

    return true;
}

static bool isValidRookMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    if (startRow != endRow && startCol != endCol) return false; // Xe chỉ đi ngang hoặc dọc

    int obstacles = 0;
    if (!isPathClear(board, startRow, startCol, endRow, endCol, &obstacles)) return false;
    if (obstacles > 0) return false; // Có quân cản

    return true;
}

static bool isValidCannonMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    if (startRow != endRow && startCol != endCol) return false; // Pháo chỉ đi ngang hoặc dọc

    int obstacles = 0;
    if (!isPathClear(board, startRow, startCol, endRow, endCol, &obstacles)) return false;

    Piece targetPiece = board[endRow][endCol];

    if (targetPiece.type == EMPTY) { // Pháo đi không ăn quân
        return obstacles == 0;
    } else { // Pháo ăn quân
        return obstacles == 1; // Phải có đúng 1 quân cản (ngòi pháo)
    }
}

static bool isValidHorseMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    int dr = abs(endRow - startRow);
    int dc = abs(endCol - startCol);

    // Kiểm tra nước đi L (1 ngang 2 dọc hoặc 2 ngang 1 dọc)
    if (!((dr == 1 && dc == 2) || (dr == 2 && dc == 1))) return false;

    // Kiểm tra cản mã (chân mã)
    if (dr == 2) { // Đi 2 dọc
        if (board[startRow + (endRow > startRow ? 1 : -1)][startCol].type != EMPTY) return false;
    } else { // Đi 2 ngang
        if (board[startRow][startCol + (endCol > startCol ? 1 : -1)].type != EMPTY) return false;
    }
    return true;
}

static bool isValidAdvisorMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    // 1. Kiểm tra giới hạn cung (Tướng Đỏ: hàng 0-2, cột 3-5; Tướng Đen: hàng 7-9, cột 3-5)
    if (currentPlayerColor == RED) {
        if (!((endRow >= 0 && endRow <= 2) && (endCol >= 3 && endCol <= 5))) return false;
    } else { // BLACK
        if (!((endRow >= 7 && endRow <= 9) && (endCol >= 3 && endCol <= 5))) return false;
    }

    // 2. Chỉ được đi chéo 1 ô
    int dr = abs(endRow - startRow);
    int dc = abs(endCol - startCol);
    return dr == 1 && dc == 1;
}

static bool isValidElephantMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    // 1. Kiểm tra nước đi chéo 2 ô
    int dr = abs(endRow - startRow);
    int dc = abs(endCol - startCol);
    if (!(dr == 2 && dc == 2)) return false;

    // 2. Kiểm tra cản tượng (chân tượng)
    int midRow = (startRow + endRow) / 2;
    int midCol = (startCol + endCol) / 2;
    if (board[midRow][midCol].type != EMPTY) return false;

    // 3. Kiểm tra không được vượt sông (Hàng 5 là sông)
    if (currentPlayerColor == RED) {
        if (endRow > 4) return false; // Đỏ không được xuống quá hàng 4 (hàng 5 là sông)
    } else { // BLACK
        if (endRow < 5) return false; // Đen không được lên quá hàng 5 (hàng 4 là sông)
    }
    return true;
}

static bool isValidPawnMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    int dr = endRow - startRow;
    int dc = abs(endCol - startCol);

    if (currentPlayerColor == RED) {
        // Chỉ tiến lên hoặc ngang (khi đã qua sông)
        if (dr < 0) return false; // Không được lùi
        if (dr == 0 && dc == 0) return false; // Không đứng yên

        if (startRow < 5) { // Chưa qua sông
            return (dr == 1 && dc == 0); // Chỉ được tiến thẳng 1 ô
        } else { // Đã qua sông
            return ((dr == 1 && dc == 0) || (dr == 0 && dc == 1)); // Tiến thẳng hoặc ngang 1 ô
        }
    } else { // BLACK
        // Chỉ tiến xuống hoặc ngang (khi đã qua sông)
        if (dr > 0) return false; // Không được lùi
        if (dr == 0 && dc == 0) return false; // Không đứng yên

        if (startRow > 4) { // Chưa qua sông
            return (dr == -1 && dc == 0); // Chỉ được tiến thẳng 1 ô
        } else { // Đã qua sông
            return ((dr == -1 && dc == 0) || (dr == 0 && dc == 1)); // Tiến thẳng hoặc ngang 1 ô
        }
    }
}


// Hàm kiểm tra tổng quát cho một nước đi
bool isValidMove(const Board board, int startRow, int startCol, int endRow, int endCol, PieceColor currentPlayerColor) {
    if (!isValidPos(startRow, startCol) || !isValidPos(endRow, endCol)) {
        printf("DEBUG: Invalid start/end position.\n");
        return false;
    }

    Piece startPiece = board[startRow][startCol];
    Piece endPiece = board[endRow][endCol];

    // 1. Kiểm tra quân được chọn có phải của người chơi hiện tại không
    if (startPiece.color != currentPlayerColor) {
        printf("DEBUG: Not your piece to move.\n");
        return false;
    }

    // 2. Kiểm tra có tự ăn quân mình không
    if (endPiece.color == currentPlayerColor) {
        printf("DEBUG: Cannot capture your own piece.\n");
        return false;
    }

    // 3. Kiểm tra quy tắc di chuyển của từng quân cờ
    bool moveOK = false;
    switch (startPiece.type) {
        case RED_KING:
        case BLACK_KING:
            moveOK = isValidKingMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case RED_ROOK:
        case BLACK_ROOK:
            moveOK = isValidRookMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case RED_CANNON:
        case BLACK_CANNON:
            moveOK = isValidCannonMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case RED_HORSE:
        case BLACK_HORSE:
            moveOK = isValidHorseMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case RED_ADVISOR:
        case BLACK_ADVISOR:
            moveOK = isValidAdvisorMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case RED_ELEPHANT:
        case BLACK_ELEPHANT:
            moveOK = isValidElephantMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case RED_PAWN:
        case BLACK_PAWN:
            moveOK = isValidPawnMove(board, startRow, startCol, endRow, endCol, currentPlayerColor);
            break;
        case EMPTY:
            printf("DEBUG: No piece at start position.\n");
            return false; // Không có quân nào ở vị trí bắt đầu
    }

    if (!moveOK) {
        printf("DEBUG: Invalid move rule for the piece.\n");
        return false;
    }

    // 4. Kiểm tra xem nước đi có làm tướng bị chiếu không
    // Tạo một bản sao tạm thời của bàn cờ để kiểm tra
    Board tempBoard;
    memcpy(tempBoard, board, sizeof(Board));
    tempBoard[endRow][endCol] = tempBoard[startRow][startCol];
    tempBoard[startRow][startCol] = (Piece){EMPTY, NO_COLOR};

    if (isKingInCheck(tempBoard, currentPlayerColor)) {
        printf("DEBUG: Move puts your king in check.\n");
        return false;
    }

    return true;
}

// =========================================================
// Kiểm tra chiếu tướng
// =========================================================

bool isKingInCheck(const Board board, PieceColor kingColor) {
    int kingRow, kingCol;
    findKingPosition(board, kingColor, &kingRow, &kingCol);

    if (kingRow == -1) return false; // Should not happen

    PieceColor opponentColor = (kingColor == RED) ? BLACK : RED;

    // Kiểm tra tất cả các ô trên bàn cờ
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            Piece currentPiece = board[r][c];
            if (currentPiece.color == opponentColor) {
                // Tạm thời coi như quân đối phương đang ở vị trí (r, c)
                // và kiểm tra xem nó có thể ăn tướng không
                // Rất quan trọng: Không được gọi lại isValidMove ở đây
                // mà phải viết các hàm kiểm tra riêng
                // để tránh vòng lặp vô hạn và đơn giản hóa logic.

                // Ví dụ kiểm tra Xe/Pháo/Mã chiếu tướng:
                switch (currentPiece.type) {
                    case BLACK_ROOK:
                    case RED_ROOK:
                        // Kiểm tra đường ngang/dọc của xe
                        if ((r == kingRow || c == kingCol)) {
                            int obstacles = 0;
                            isPathClear(board, r, c, kingRow, kingCol, &obstacles);
                            if (obstacles == 0) return true;
                        }
                        break;
                    case BLACK_CANNON:
                    case RED_CANNON:
                        // Kiểm tra đường ngang/dọc của pháo với 1 ngòi
                        if ((r == kingRow || c == kingCol)) {
                            int obstacles = 0;
                            isPathClear(board, r, c, kingRow, kingCol, &obstacles);
                            if (obstacles == 1) return true;
                        }
                        break;
                    case BLACK_HORSE:
                    case RED_HORSE: {
                        // Kiểm tra nước đi L ngược của mã
                        int dr = abs(kingRow - r);
                        int dc = abs(kingCol - c);
                        if ((dr == 1 && dc == 2) || (dr == 2 && dc == 1)) {
                            // Kiểm tra chân mã
                            bool footBlocked = false;
                            if (dr == 2) { // Mã đi 2 dọc
                                if (board[r + (kingRow > r ? 1 : -1)][c].type != EMPTY) footBlocked = true;
                            } else { // Mã đi 2 ngang
                                if (board[r][c + (kingCol > c ? 1 : -1)].type != EMPTY) footBlocked = true;
                            }
                            if (!footBlocked) return true;
                        }
                        break;
                    }
                    case BLACK_PAWN:
                        if (kingColor == RED) {
                            if ((kingRow == r + 1 && kingCol == c) || // Tiến xuống
                                (kingRow == r && abs(kingCol - c) == 1 && r >= 5)) { // Ngang sau khi qua sông
                                return true;
                            }
                        }
                        break;
                    case RED_PAWN:
                         if (kingColor == BLACK) {
                            if ((kingRow == r - 1 && kingCol == c) || // Tiến lên
                                (kingRow == r && abs(kingCol - c) == 1 && r <= 4)) { // Ngang sau khi qua sông
                                return true;
                            }
                        }
                        break;
                    case BLACK_KING: // Kiểm tra tướng đối mặt
                    case RED_KING:
                         if (kingCol == c) { // Cùng cột
                            int obstacles = 0;
                            isPathClear(board, r, c, kingRow, kingCol, &obstacles);
                            if (obstacles == 0) return true; // Tướng đối mặt
                        }
                        break;
                    // TODO: Thêm kiểm tra cho Sĩ và Tượng nếu chúng có thể chiếu tướng (ít khi xảy ra nhưng cần chính xác)
                    case BLACK_ADVISOR:
                    case RED_ADVISOR:
                        // Sĩ không thể chiếu tướng trừ khi tướng ra khỏi cung, nhưng quy tắc này cấm điều đó.
                        // Cần kiểm tra nếu tướng bị ép vào đường đi của sĩ khi sĩ đã ở vị trí chiếu
                        break;
                    case BLACK_ELEPHANT:
                    case RED_ELEPHANT:
                        // Tượng không thể chiếu tướng
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return false;
}

// Hàm kiểm tra xem người chơi có bất kỳ nước đi hợp lệ nào không
bool canPlayerMakeAnyValidMove(const Board board, PieceColor playerColor) {
    for (int startRow = 0; startRow < BOARD_ROWS; startRow++) {
        for (int startCol = 0; startCol < BOARD_COLS; startCol++) {
            if (board[startRow][startCol].color == playerColor) {
                for (int endRow = 0; endRow < BOARD_ROWS; endRow++) {
                    for (int endCol = 0; endCol < BOARD_COLS; endCol++) {
                        // Không cần memcpy ở đây vì isValidMove đã làm rồi
                        if (isValidMove(board, startRow, startCol, endRow, endCol, playerColor)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}