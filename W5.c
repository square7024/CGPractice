/*

rect1
W/A/S/D : 위/왼쪽/아래/오른쪽 이동
z/Z     : x,y 축소 / 확대
x/X     : x축 확대 / 축소
c/C     : y축 확대 / 축소
v/V     : x확대+y축소 / x축소+y확대
b       : 면적 출력

rect2
I/J/K/L : 위/왼쪽/아래/오른쪽 이동
1/!     : x,y 축소 / 확대
2/@     : x축 확대 / 축소
3/#     : y축 확대 / 축소
4/$     : x확대+y축소 / x축소+y확대
5       : 면적 출력

공통
+       : 보드 크기 +1, 최대 40×40
-       : 보드 크기 -1, 최소 10×10
r       : 30×30으로 리셋 후 좌표 재입력
q       : 종료

*/

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

void clear_board();                                 // 보드 초기화
void draw_rect1();                                  // 사각형 1 그리기
void draw_rect2();                                  // 사각형 2 그리기
void print_board();                                 // 보드 출력
void update_board();                                // 보드 업데이트
void move_x(Rect* rect, int direction);             // 사각형 x축 이동
void move_y(Rect* rect, int direction);             // 사각형 y축 이동
void scale_rect(Rect* rect, int direction);         // 사각형 확대/축소
void scale_x(Rect* rect, int direction);            // x축 확대/축소
void scale_y(Rect* rect, int direction);            // y축 확대/축소
void increase_board();                              // 보드 크기 증가
void decrease_board();                              // 보드 크기 감소
void fit_rect_to_board(Rect* rect, int new_size);   
void reset();                                       // 전체 리셋

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
            

            // rect1 확대/축소
        case 'z':
            // x, y 모두 한 칸 축소
            scale_x(&rect1, -1);
            scale_y(&rect1, -1);
            break;

        case 'Z':
            // x, y 모두 한 칸 확대
            scale_x(&rect1, 1);
            scale_y(&rect1, 1);
            break;

        case 'x':
            // x축 한 칸 확대
            scale_x(&rect1, 1);
            break;

        case 'X':
            // x축 한 칸 축소
            scale_x(&rect1, -1);
            break;

        case 'c':
            // y축 한 칸 확대
            scale_y(&rect1, 1);
            break;

        case 'C':
            // y축 한 칸 축소
            scale_y(&rect1, -1);
            break;

        case 'v':
            // x축 확대, y축 축소
            scale_x(&rect1, 1);
            scale_y(&rect1, -1);
            break;

        case 'V':
            // x축 축소, y축 확대
            scale_x(&rect1, -1);
            scale_y(&rect1, 1);
            break;

        case 'b':
        {
            int width = (rect1.x2 - rect1.x1 + board_size) % board_size + 1;
            int height = (rect1.y2 - rect1.y1 + board_size) % board_size + 1;

            printf("0 사각형의 면적 : %d\n", width * height);
            break;
        }


            // rect2 확대/축소
        case '1':
            // x, y 모두 한 칸 축소
            scale_x(&rect2, -1);
            scale_y(&rect2, -1);
            break;

        case '!':
            // x, y 모두 한 칸 확대
            scale_x(&rect2, 1);
            scale_y(&rect2, 1);
            break;

        case '2':
            // x축 한 칸 확대
            scale_x(&rect2, 1);
            break;

        case '@':
            // x축 한 칸 축소
            scale_x(&rect2, -1);
            break;

        case '3':
            // y축 한 칸 확대
            scale_y(&rect2, 1);
            break;

        case '#':
            // y축 한 칸 축소
            scale_y(&rect2, -1);
            break;

        case '4':
            // x축 확대, y축 축소
            scale_x(&rect2, 1);
            scale_y(&rect2, -1);
            break;

        case '$':
            // x축 축소, y축 확대
            scale_x(&rect2, -1);
            scale_y(&rect2, 1);
            break;

        case '5':
        {
            int width = (rect2.x2 - rect2.x1 + board_size) % board_size + 1;
            int height = (rect2.y2 - rect2.y1 + board_size) % board_size + 1;

            printf("X 사각형의 면적 : %d\n", width * height);
            break;
        }


        case '+':
            increase_board();
            break;

        case '-':
            decrease_board();
            break;

        case 'r':
            reset();
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

void scale_x(Rect* rect, int direction)
{
    if (direction == 1) {
        // x축 확대
        if (rect->x2 < board_size)
            rect->x2++;
    }
    else {
        // x축 축소
        if (rect->x2 > rect->x1)
            rect->x2--;
    }
}

void scale_y(Rect* rect, int direction)
{
    if (direction == 1) {
        // y축 확대
        if (rect->y2 < board_size)
            rect->y2++;
    }
    else {
        // y축 축소
        if (rect->y2 > rect->y1)
            rect->y2--;
    }
}

void increase_board()
{
    if (board_size < MAX_BOARD_SIZE) {
        board_size++;
    }
}

void decrease_board()
{
    //--- 최소 보드 크기는 10
    if (board_size <= 10) {
        printf("보드의 최소 크기는 10x10입니다.\n");
        return;
    }

    int new_size = board_size - 1;

    //--- rect1의 크기
    int width1 = (rect1.x2 - rect1.x1 + board_size) % board_size + 1;
    int height1 = (rect1.y2 - rect1.y1 + board_size) % board_size + 1;

    //--- rect2의 크기
    int width2 = (rect2.x2 - rect2.x1 + board_size) % board_size + 1;
    int height2 = (rect2.y2 - rect2.y1 + board_size) % board_size + 1;

    //--- 새 보드보다 사각형이 큰 경우
    if (width1 > new_size || height1 > new_size ||
        width2 > new_size || height2 > new_size) {

        printf("사각형의 크기가 보드보다 커서 보드를 줄일 수 없습니다.\n");
        return;
    }

    //--- 줄어든 보드에 맞게 사각형 이동
    fit_rect_to_board(&rect1, new_size);
    fit_rect_to_board(&rect2, new_size);

    //--- 보드 크기 감소
    board_size = new_size;
}

void fit_rect_to_board(Rect* rect, int new_size)
{
    int width = (rect->x2 - rect->x1 + board_size) % board_size + 1;
    int height = (rect->y2 - rect->y1 + board_size) % board_size + 1;

    //--- x축이 경계를 넘어간 상태
    if (rect->x1 > rect->x2) {
        rect->x1 = new_size - width + 1;
        rect->x2 = new_size;
    }
    //--- 오른쪽 경계를 넘어가는 경우
    else if (rect->x2 > new_size) {
        int move = rect->x2 - new_size;

        rect->x1 -= move;
        rect->x2 -= move;
    }

    //--- y축이 경계를 넘어간 상태
    if (rect->y1 > rect->y2) {
        rect->y1 = new_size - height + 1;
        rect->y2 = new_size;
    }
    //--- 아래쪽 경계를 넘어가는 경우
    else if (rect->y2 > new_size) {
        int move = rect->y2 - new_size;

        rect->y1 -= move;
        rect->y2 -= move;
    }
}

void reset()
{
    //--- 보드 크기 초기화
    board_size = BOARD_SIZE;

    //--- 사각형 1 좌표 다시 입력
    printf("input coord value1: ");
    scanf("%d %d %d %d",
        &rect1.x1, &rect1.y1,
        &rect1.x2, &rect1.y2);

    //--- 사각형 2 좌표 다시 입력
    printf("input coord value2: ");
    scanf("%d %d %d %d",
        &rect2.x1, &rect2.y1,
        &rect2.x2, &rect2.y2);
}