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

void clear_board();                         // 보드 초기화
void draw_rect1();                          // 사각형 1 그리기
void draw_rect2();                          // 사각형 2 그리기
void print_board();                         // 보드 출력
void update_board();                        // 보드 업데이트
void move_x(Rect* rect, int direction);     // 사각형 x축 이동
void move_y(Rect* rect, int direction);     // 사각형 y축 이동

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

	char command = '\0';

    printf("input coord value1: ");
    scanf("%d %d %d %d",
        &rect1.x1, &rect1.y1,
        &rect1.x2, &rect1.y2);

    printf("input coord value2: ");
    scanf("%d %d %d %d",
        &rect2.x1, &rect2.y1,
        &rect2.x2, &rect2.y2);

	update_board();

    while (command != 'q') {
        printf("\n명령어를 입력하세요 : ");
        scanf(" %c", &command);

        switch (command) {

            // rect1 이동
        case 'a':
            move_x(&rect1, -1);
            break;

        case 'd':
            move_x(&rect1, 1);
            break;

        case 's':
            move_y(&rect1, 1);
            break;

        case 'w':
            move_y(&rect1, -1);
            break;


            // rect2 이동
        case 'j':
            move_x(&rect2, -1);
            break;

        case 'l':
            move_x(&rect2, 1);
            break;

        case 'k':
            move_y(&rect2, 1);
            break;

        case 'i':
            move_y(&rect2, -1);
            break;


        case 'q':
            return 0;

        default:
            printf("잘못된 명령어입니다.\n");
            continue;
        }

        update_board();
    }

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

void move_y(Rect* rect, int direction)
{
    if (direction == 1) {
        // 아래쪽
        rect->y1 = rect->y1 % board_size + 1;
        rect->y2 = rect->y2 % board_size + 1;
    }
    else {
        // 위쪽
        rect->y1 = (rect->y1 - 2 + board_size) % board_size + 1;
        rect->y2 = (rect->y2 - 2 + board_size) % board_size + 1;
    }
}