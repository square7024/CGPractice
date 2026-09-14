#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define BOARD_SIZE 30
#define MAX_BOARD_SIZE 40

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} Rect;

char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
int board_size = BOARD_SIZE;
Rect rect1;
Rect rect2;

void clear_board();
void draw_rect1();
void draw_rect2();
void print_board();
void update_board();
void move_x(Rect* rect, int direction);

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    printf("input coord value1: ");
    scanf("%d %d %d %d",
        &rect1.x1, &rect1.y1,
        &rect1.x2, &rect1.y2);

    printf("input coord value2: ");
    scanf("%d %d %d %d",
        &rect2.x1, &rect2.y1,
        &rect2.x2, &rect2.y2);

	update_board();

    return 0;
}

void clear_board()
{
    for (int y = 0; y < board_size; ++y) {
        for (int x = 0; x < board_size; ++x) {
            board[y][x] = '.';
        }
    }
}

void draw_rect1()
{
    int width = (rect1.x2 - rect1.x1 + board_size) % board_size + 1;
    int height = (rect1.y2 - rect1.y1 + board_size) % board_size + 1;

    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {

            int x = (rect1.x1 - 1 + dx) % board_size;
            int y = (rect1.y1 - 1 + dy) % board_size;

            if (board[y][x] == 'X') {
                board[y][x] = '#';
            }
            else {
                board[y][x] = '0';
            }
        }
    }
}

void draw_rect2()
{
    int width = (rect2.x2 - rect2.x1 + board_size) % board_size + 1;
    int height = (rect2.y2 - rect2.y1 + board_size) % board_size + 1;

    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {

            int x = (rect2.x1 - 1 + dx) % board_size;
            int y = (rect2.y1 - 1 + dy) % board_size;

            if (board[y][x] == '0') {
                board[y][x] = '#';
            }
            else {
                board[y][x] = 'X';
            }
        }
    }
}

void print_board()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    for (int y = 0; y < board_size; ++y) {
        for (int x = 0; x < board_size; ++x) {

            if (board[y][x] == '#') {
                // 빨간색
                SetConsoleTextAttribute(hConsole, 12);

                printf("# ");

                // 기본색으로 복구
                SetConsoleTextAttribute(hConsole, 7);
            }
            else {
                printf("%c ", board[y][x]);
            }
        }

        printf("\n");
    }
}

void update_board()
{
    clear_board();

    draw_rect1();
    draw_rect2();

    print_board();
}

void move_x(Rect* rect, int direction)
{
    if (direction == 1) {
        // 오른쪽
        rect->x1 = rect->x1 % board_size + 1;
        rect->x2 = rect->x2 % board_size + 1;
    }
    else {
        // 왼쪽
        rect->x1 = (rect->x1 - 2 + board_size) % board_size + 1;
        rect->x2 = (rect->x2 - 2 + board_size) % board_size + 1;
    }
}