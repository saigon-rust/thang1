#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>

// Định nghĩa cấu trúc hình chữ nhật
typedef struct {
    double width;
    double height;
} Rectangle;

// Các hằng số hiển thị
const int SCALE = 100; // Tỉ lệ phóng to khi hiển thị
const int WINDOW_WIDTH = (int)(1.22 * SCALE);
const int WINDOW_HEIGHT = (int)(2.44 * SCALE);
const int GRID_WIDTH = (int)(1.22 * 10);
const int GRID_HEIGHT = (int)(2.44 * 10);

// Màu sắc
const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
const SDL_Color COLOR_BLACK = {0, 0, 0, 255};
const SDL_Color COLOR_SMALL1 = {0, 128, 255, 255}; // Xanh dương
const SDL_Color COLOR_SMALL2_H = {255, 165, 0, 255}; // Cam (nằm ngang)
const SDL_Color COLOR_SMALL2_V = {255, 215, 0, 255}; // Vàng (thẳng đứng)

// Hàm kiểm tra xem có thể đặt hình chữ nhật nhỏ vào vị trí (x, y) trong hình chữ nhật lớn hay không
int canPlace(Rectangle large, Rectangle small, double x, double y, int **grid) {
    int w_large = (int)(large.width * 10);   // Nhân 10 để làm việc với số nguyên (độ chính xác 0.1)
    int h_large = (int)(large.height * 10);
    int w_small = (int)(small.width * 10);
    int h_small = (int)(small.height * 10);
    int start_x = (int)(x * 10);
    int start_y = (int)(y * 10);

    if (start_x + w_small > w_large || start_y + h_small > h_large) {
        return 0; // Vượt quá kích thước hình chữ nhật lớn
    }

    for (int i = start_y; i < start_y + h_small; i++) {
        for (int j = start_x; j < start_x + w_small; j++) {
            if (grid[i][j] == 1) {
                return 0; // Vị trí đã bị chiếm
            }
        }
    }
    return 1; // Có thể đặt
}

// Hàm đặt hình chữ nhật nhỏ vào vị trí (x, y) trong lưới
void placeRectangle(Rectangle small, double x, double y, int **grid) {
    int w_small = (int)(small.width * 10);
    int h_small = (int)(small.height * 10);
    int start_x = (int)(x * 10);
    int start_y = (int)(y * 10);

    for (int i = start_y; i < start_y + h_small; i++) {
        for (int j = start_x; j < start_x + w_small; j++) {
            grid[i][j] = 1;
        }
    }
}

// Hàm xóa hình chữ nhật nhỏ khỏi vị trí (x, y) trong lưới
void removeRectangle(Rectangle small, double x, double y, int **grid) {
    int w_small = (int)(small.width * 10);
    int h_small = (int)(small.height * 10);
    int start_x = (int)(x * 10);
    int start_y = (int)(y * 10);

    for (int i = start_y; i < start_y + h_small; i++) {
        for (int j = start_x; j < start_x + w_small; j++) {
            grid[i][j] = 0;
        }
    }
}

// Hàm vẽ một hình chữ nhật trên renderer
void drawSDLRectangle(SDL_Renderer *renderer, double x, double y, double width, double height, const SDL_Color *color) {
    SDL_Rect rect;
    rect.x = (int)(x * SCALE);
    rect.y = (int)(y * SCALE);
    rect.w = (int)(width * SCALE);
    rect.h = (int)(height * SCALE);
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, COLOR_BLACK.r, COLOR_BLACK.g, COLOR_BLACK.b, COLOR_BLACK.a);
    SDL_RenderDrawRect(renderer, &rect); // Vẽ viền đen
}

// Hàm vẽ trạng thái hiện tại của lưới lên cửa sổ SDL
void drawArrangement(SDL_Renderer *renderer, Rectangle large, int **grid) {
    SDL_SetRenderDrawColor(renderer, COLOR_WHITE.r, COLOR_WHITE.g, COLOR_WHITE.b, COLOR_WHITE.a);
    SDL_RenderClear(renderer);

    // Vẽ hình chữ nhật lớn (khung)
    drawSDLRectangle(renderer, 0, 0, large.width, large.height, &COLOR_WHITE);

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (grid[i][j] == 1) {
                // Xác định loại hình chữ nhật nhỏ dựa trên cách đặt
                // (Cần theo dõi thông tin này khi đặt)
                // Đây là một cách đơn giản hóa, cần cải tiến để xác định chính xác
                if (j > 0 && grid[i][j - 1] == 1) {
                    // Có thể là phần bên phải của hình 0.5x1 nằm ngang
                    if (i > 0 && grid[i - 1][j] == 0 && grid[i - 1][j - 1] == 0) {
                        // Xác định là hình 0.5x1 ngang
                        drawSDLRectangle(renderer, (double)j / 10.0 - 0.5, (double)i / 10.0, 0.5, 1.0, &COLOR_SMALL2_H);
                    }
                } else if (i > 0 && grid[i - 1][j] == 1) {
                    // Có thể là phần dưới của hình 1x0.5 nằm dọc
                    if (j > 0 && grid[i][j - 1] == 0 && grid[i - 1][j - 1] == 0) {
                        // Xác định là hình 1x0.5 dọc
                        drawSDLRectangle(renderer, (double)j / 10.0, (double)i / 10.0 - 0.5, 1.0, 0.5, &COLOR_SMALL2_V);
                    }
                } else {
                    // Có thể là hình 1x1 hoặc phần đầu của hình 0.5x1 hoặc 1x0.5
                    // Cần logic thông minh hơn để phân biệt
                    // Giả định nếu không có láng giềng đã vẽ, thì vẽ nó
                    int is_part_of_other = 0;
                    if (j > 0 && grid[i][j - 1] == 1) is_part_of_other = 1;
                    if (i > 0 && grid[i - 1][j] == 1) is_part_of_other = 1;

                    if (!is_part_of_other) {
                        // Ưu tiên vẽ 1x1 nếu có đủ không gian xung quanh
                        int can_be_1x1 = (j + 10 <= GRID_WIDTH && i + 10 <= GRID_HEIGHT);
                        for (int row = i; row < i + 10 && can_be_1x1; ++row) {
                            for (int col = j; col < j + 10; ++col) {
                                if (row >= GRID_HEIGHT || col >= GRID_WIDTH || grid[row][col] == 0) {
                                    can_be_1x1 = 0;
                                    break;
                                }
                            }
                            if (!can_be_1x1) break;
                        }
                        if (can_be_1x1) {
                            drawSDLRectangle(renderer, (double)j / 10.0, (double)i / 10.0, 1.0, 1.0, &COLOR_SMALL1);
                            // Đánh dấu các ô đã vẽ của hình 1x1 (tránh vẽ trùng lặp)
                            for (int row = i; row < i + 10; ++row) {
                                for (int col = j; col < j + 10; ++col) {
                                    grid[row][col] = 2; // Đánh dấu là đã vẽ
                                }
                            }
                        } else if (j + 5 <= GRID_WIDTH && grid[i][j + 4] == 1) {
                            drawSDLRectangle(renderer, (double)j / 10.0, (double)i / 10.0, 0.5, 1.0, &COLOR_SMALL2_H);
                            for (int row = i; row < i + 10; ++row) grid[row][j+1] = 2;
                        } else if (i + 5 <= GRID_HEIGHT && grid[i + 4][j] == 1) {
                            drawSDLRectangle(renderer, (double)j / 10.0, (double)i / 10.0, 1.0, 0.5, &COLOR_SMALL2_V);
                            for (int col = j; col < j + 10; ++col) grid[i+1][col] = 2;
                        } else if (grid[i][j] == 1) {
                            // Trường hợp còn lại (có thể là phần lẻ hoặc lỗi logic)
                            drawSDLRectangle(renderer, (double)j / 10.0, (double)i / 10.0, 0.1, 0.1, &COLOR_BLACK);
                        }
                    }
                }
            }
        }
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(2000); // Hiển thị mỗi phương án trong 2 giây
}

// Hàm đệ quy để tìm các phương án sắp xếp và hiển thị
void findArrangementsAndDraw(Rectangle large, Rectangle small1, Rectangle small2, int count1, int count2, int **grid, int *arrangement_count, SDL_Renderer *renderer) {
    int w_large = (int)(large.width * 10);
    int h_large = (int)(large.height * 10);

    // Trường hợp cơ sở: đã đặt hết các hình chữ nhật nhỏ
    if (count1 == 0 && count2 == 0) {
        (*arrangement_count)++;
        printf("Tìm thấy phương án sắp xếp thứ %d\n", *arrangement_count);
        drawArrangement(renderer, large, grid);
        return;
    }

    // Thử đặt hình chữ nhật 1x1
    if (count1 > 0) {
        for (double y = 0; y < large.height; y += 0.1) {
            for (double x = 0; x < large.width; x += 0.1) {
                Rectangle current_small = {1.0, 1.0};
                if (canPlace(large, current_small, x, y, grid)) {
                    placeRectangle(current_small, x, y, grid);
                    findArrangementsAndDraw(large, small1, small2, count1 - 1, count2, grid, arrangement_count, renderer);
                    removeRectangle(current_small, x, y, grid); // Backtrack
                }
            }
        }
    }

    // Thử đặt hình chữ nhật 0.5x1
    if (count2 > 0) {
        for (double y = 0; y < large.height; y += 0.1) {
            for (double x = 0; x < large.width; x += 0.1) {
                // Thử đặt theo chiều rộng 0.5, chiều cao 1
                Rectangle current_small_wh = {0.5, 1.0};
                if (canPlace(large, current_small_wh, x, y, grid)) {
                    placeRectangle(current_small_wh, x, y, grid);
                    findArrangementsAndDraw(large, small1, small2, count1, count2 - 1, grid, arrangement_count, renderer);
                    removeRectangle(current_small_wh, x, y, grid); // Backtrack
                }
                // Thử đặt theo chiều rộng 1, chiều cao 0.5
                Rectangle current_small_hw = {1.0, 0.5};
                if (canPlace(large, current_small_hw, x, y, grid)) {
                    placeRectangle(current_small_hw, x, y, grid);
                    findArrangementsAndDraw(large, small1, small2, count1, count2 - 1, grid, arrangement_count, renderer);
                    removeRectangle(current_small_hw, x, y, grid); // Backtrack
                }
            }
        }
    }
}

int main() {
    Rectangle large_rect = {1.22, 2.44};
    Rectangle small_rect1 = {1.0, 1.0};
    Rectangle small_rect2 = {0.5, 1.0};
    int num_small1 = 1;
    int num_small2 = 2;
    int arrangement_count = 0;

    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Không thể khởi tạo SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Sắp xếp hình chữ nhật", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Không thể tạo cửa sổ SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Không thể tạo renderer SDL: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Khởi tạo lưới đại diện cho hình chữ nhật lớn
    int **grid = (int **)malloc(GRID_HEIGHT * sizeof(int *));
    for (int i = 0; i < GRID_HEIGHT; i++) {
        grid[i] = (int *)malloc(GRID_WIDTH * sizeof(int));
        for (int j = 0; j < GRID_WIDTH; j++) {
            grid[i][j] = 0; // 0 biểu thị ô trống
        }
    }

    printf("Tìm và hiển thị các phương án sắp xếp:\n");
    findArrangementsAndDraw(large_rect, small_rect1, small_rect2, num_small1, num_small2, grid, &arrangement_count, renderer);

    if (arrangement_count == 0) {
        printf("Không tìm thấy phương án sắp xếp nào.\n");
    }

    // Giải phóng bộ nhớ
    for (int i = 0; i < GRID_HEIGHT; i++) {
        free(grid[i]);
    }
    free(grid);

    // Giải phóng SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}