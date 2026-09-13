#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_SIZE 6
#define JOKER '@'

typedef struct {
    char value;     // 실제 카드 문자
    bool matched;   // 이미 맞춘 카드인지
} Card;

void print_board(Card board[][MAX_SIZE], int row, int col);             // 보드판 출력
void init_board(Card board[][MAX_SIZE], int row, int col);              // 보드판 초기화
void shuffle_board(Card board[][MAX_SIZE], int row, int col);           // 보드판 섞기
char select_card(Card board[][MAX_SIZE],
    int* row, int* col, int max_row, int max_col);                      // 카드 선택
void print_selected_board(Card board[][MAX_SIZE], int row, int col,
	int first_row, int first_col, int second_row, int second_col);      // 선택한 카드만 보여주는 보드판 출력
void print_all_board(Card board[][MAX_SIZE], int row, int col);         // 모든 카드 보여주는 보드판 출력
void set_card_color(char ch);                                           // 카드 색상 설정
bool check_clear(Card board[][MAX_SIZE], int row, int col);

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    char command = '\0';
    srand((unsigned int)time(NULL));
	
    Card board[MAX_SIZE][MAX_SIZE] = { 0 };
    int row;
    int col;

    char input[20];

    while (1) {
        printf("가로x세로 입력 (3~6, 종료 q) : ");

        scanf("%s", input);

        // q 입력 시 프로그램 종료
        if (strcmp(input, "q") == 0) {
            return 0;
        }

        // 첫 번째 값이 숫자가 아닌 경우
        if (sscanf(input, "%d", &col) != 1) {
            printf("숫자를 입력하세요.\n");
            while (getchar() != '\n');
            continue;
        }

        // 세로 입력
        if (scanf("%d", &row) != 1) {
            printf("숫자를 입력하세요.\n");
            while (getchar() != '\n');
            continue;
        }

        // 3~6 범위 검사
        if (col < 3 || col > 6 || row < 3 || row > 6) {
            printf("가로와 세로는 3~6 사이여야 합니다.\n");
            continue;
        }

        break;
    }

    init_board(board, row, col);
    shuffle_board(board, row, col);
    print_board(board, row, col);

    int score = 0;
    int max_turn = row * col;
    int remain_turn = max_turn;
   
    while (command != 'q') {
        int first_row, first_col;
        int second_row, second_col;

		// 첫 번째 카드 선택
        printf("\n첫 번째 카드 선택 : ");

        command = select_card(
            board,
            &first_row, &first_col,
            row, col
        );

        if (command == 'q') {
            break;
        }

        if (command == 'r') {
            init_board(board, row, col);
            shuffle_board(board, row, col);

            score = 0;
            remain_turn = max_turn;

            print_board(board, row, col);

            printf("점수 : %d점\n", score);
            printf("남은 횟수 : %d회\n", remain_turn);

            continue;
        }

        if (command == 'h') {
            print_all_board(board, row, col);

            Sleep(2000);       // 2초 동안 보여주기
            system("cls");     // 콘솔 화면 지우기

            print_board(board, row, col);
            continue;
        }

		// 두 번째 카드 선택
        printf("두 번째 카드 선택 : ");

        command = select_card(
            board,
            &second_row, &second_col,
            row, col
        );

        if (command == 'q') {
            break;
        }

        if (command == 'r') {
            init_board(board, row, col);
            shuffle_board(board, row, col);

            score = 0;
            remain_turn = max_turn;

            print_board(board, row, col);

            printf("점수 : %d점\n", score);
            printf("남은 횟수 : %d회\n", remain_turn);

            continue;
        }

        if (command == 'h') {
            print_all_board(board, row, col);

            Sleep(2000);       // 2초 동안 보여주기
            system("cls");     // 콘솔 화면 지우기

            print_board(board, row, col);
            continue;
        }

		// 같은 카드 선택 방지
        while (first_row == second_row && first_col == second_col) {
            printf("같은 카드는 선택할 수 없습니다. 다시 선택하세요 : ");

            command = select_card(
                board,
                &second_row, &second_col,
                row, col
            );

            if (command == 'q' || command == 'r') {
                break;
            }
        }

        if (command == 'q') {
            break;
        }

        if (command == 'r') {
            init_board(board, row, col);
            shuffle_board(board, row, col);

            score = 0;
            remain_turn = max_turn;

            print_board(board, row, col);

            printf("점수 : %d점\n", score);
            printf("남은 횟수 : %d회\n", remain_turn);

            continue;
        }

        if (command == 'h') {
            print_all_board(board, row, col);

            Sleep(2000);       // 2초 동안 보여주기
            system("cls");     // 콘솔 화면 지우기

            print_board(board, row, col);
            continue;
        }

        // 선택한 두 카드 공개
        print_selected_board(
            board, row, col,
            first_row, first_col,
            second_row, second_col
        );

        char first_value = board[first_row][first_col].value;
        char second_value = board[second_row][second_col].value;

        // 조커가 선택된 경우
        if (first_value == JOKER || second_value == JOKER) {

            char target;

            if (first_value == JOKER) {
                target = second_value;
            }
            else {
                target = first_value;
            }

            board[first_row][first_col].matched = true;
            board[second_row][second_col].matched = true;

            // 같은 알파벳의 다른 카드도 정답 처리
            for (int i = 0; i < row; ++i) {
                for (int j = 0; j < col; ++j) {
                    if (board[i][j].value == target) {
                        board[i][j].matched = true;
                    }
                }
            }

            score += 100;

            printf("조커가 사용되었습니다. %c 카드가 모두 맞았습니다.\n", target);
        }

        // 일반 카드끼리 일치
        else if (first_value == second_value) {

            board[first_row][first_col].matched = true;
            board[second_row][second_col].matched = true;

            score += 100;

            printf("카드가 일치합니다.\n");
        }

        // 실패
        else {
            printf("카드가 일치하지 않습니다.\n");
        }

        --remain_turn;

        print_board(board, row, col);

        printf("점수 : %d점\n", score);
        printf("남은 횟수 : %d회\n", remain_turn);

        if (check_clear(board, row, col)) {
            printf("\n게임을 클리어했습니다!\n");
            printf("최종 점수 : %d점\n", score);
            break;
        }

        if (remain_turn == 0) {
            printf("\n남은 횟수를 모두 사용했습니다.\n");
            printf("게임 클리어 실패\n");
            printf("최종 점수 : %d점\n", score);
            break;
        }
    }

    return 0;
}

// 보드판 출력
void print_board(Card board[][MAX_SIZE], int row, int col)
{
    printf("\n    ");

    // 위쪽 a b c d ...
    for (int j = 0; j < col; ++j) {
        printf("%c   ", 'a' + j);
    }

    printf("\n");

    // 왼쪽 1 2 3 ...
    for (int i = 0; i < row; ++i) {
        printf("%d   ", i + 1);

        for (int j = 0; j < col; ++j) {
            if (board[i][j].matched) {
                char ch = board[i][j].value - 'a' + 'A';

                set_card_color(ch);
                printf("%c", ch);

                SetConsoleTextAttribute(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    7
                );

                printf("   ");
            }
            else {
                printf("*   ");
            }
        }

        printf("\n");
    }

    printf("\n");
}

// 보드판 초기화
void init_board(Card board[][MAX_SIZE], int row, int col)
{
    // 기존 보드판 비우기
    for (int i = 0; i < MAX_SIZE; ++i) {
        for (int j = 0; j < MAX_SIZE; ++j) {
            board[i][j].value = '\0';
            board[i][j].matched = false;
        }
    }

    int total = row * col;
    int pair_count = total / 2;

    int index = 0;

    for (int i = 0; i < pair_count; ++i) {
        char ch = 'a' + i;

        int r = index / col;
        int c = index % col;

        board[r][c].value = ch;
        board[r][c].matched = false;
        ++index;

        r = index / col;
        c = index % col;

        board[r][c].value = ch;
        board[r][c].matched = false;
        ++index;
    }

    // 전체 칸이 홀수라면 마지막 한 칸은 조커
    if (total % 2 == 1) {
        int r = index / col;
        int c = index % col;

        board[r][c].value = JOKER;
        board[r][c].matched = false;
    }
}

// 보드판 섞기
void shuffle_board(Card board[][MAX_SIZE], int row, int col)
{
    int total = row * col;

    for (int i = total - 1; i > 0; --i) {
        int j = rand() % (i + 1);

        int row1 = i / col;
        int col1 = i % col;

        int row2 = j / col;
        int col2 = j % col;

        Card temp = board[row1][col1];
        board[row1][col1] = board[row2][col2];
        board[row2][col2] = temp;
    }
}

// 카드 선택
char select_card(Card board[][MAX_SIZE],
    int* row, int* col, int max_row, int max_col)
{
    char input;

    while (1) {
        scanf(" %c", &input);

        // 명령어 입력
        if (input == 'q' || input == 'r' || input == 'h') {
            return input;
        }

        // 가로 좌표 검사
        *col = input - 'a';

        if (*col < 0 || *col >= max_col) {
            printf("잘못된 입력입니다. 다시 입력하세요 : ");
            while (getchar() != '\n');
            continue;
        }

        // 세로 좌표 입력
        if (scanf("%d", row) != 1) {
            printf("잘못된 입력입니다. 다시 입력하세요 : ");
            while (getchar() != '\n');
            continue;
        }

        // 세로 범위 검사
        if (*row < 1 || *row > max_row) {
            printf("잘못된 위치입니다. 다시 입력하세요 : ");
            continue;
        }

        // 배열 인덱스로 변환
        --(*row);

        // 이미 맞춘 카드 검사
        if (board[*row][*col].matched) {
            printf("이미 맞춘 카드입니다. 다시 선택하세요 : ");
            continue;
        }

        return '\0';
    }
}

// 선택한 두 카드를 보여주는 보드판
void print_selected_board(Card board[][MAX_SIZE], int row, int col,
    int first_row, int first_col, int second_row, int second_col)
{
    printf("\n    ");

    for (int j = 0; j < col; ++j) {
        printf("%c   ", 'a' + j);
    }

    printf("\n");

    for (int i = 0; i < row; ++i) {
        printf("%d   ", i + 1);

        for (int j = 0; j < col; ++j) {

            // 이미 맞춘 카드
            if (board[i][j].matched) {
                char ch = board[i][j].value - 'a' + 'A';

                set_card_color(ch);
                printf("%c", ch);

                SetConsoleTextAttribute(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    7
                );

                printf("   ");
            }

            // 이번에 선택한 카드
            else if ((i == first_row && j == first_col) ||
                (i == second_row && j == second_col)) {

                char ch = board[i][j].value;

                set_card_color(ch);
                printf("%c", ch);

                SetConsoleTextAttribute(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    7
                );

                printf("   ");
            }

            // 나머지는 가림
            else {
                printf("*   ");
            }
        }

        printf("\n");
    }

    printf("\n");
}

// 모든 카드를 보여주는 보드판
void print_all_board(Card board[][MAX_SIZE], int row, int col)
{
    printf("\n    ");

    for (int j = 0; j < col; ++j) {
        printf("%c   ", 'a' + j);
    }

    printf("\n");

    for (int i = 0; i < row; ++i) {
        printf("%d   ", i + 1);

        for (int j = 0; j < col; ++j) {
            char ch = board[i][j].value;

            set_card_color(ch);
            printf("%c", ch);

            SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                7
            );

            printf("   ");
        }

        printf("\n");
    }

    printf("\n");
}

// 카드 색상 설정
void set_card_color(char ch)
{
    if (ch == JOKER) {
        SetConsoleTextAttribute(
            GetStdHandle(STD_OUTPUT_HANDLE),
            15
        );
        return;
    }

    if (ch >= 'A' && ch <= 'Z') {
        ch = ch - 'A' + 'a';
    }

    int colors[] = {
        1, 2, 3, 4, 5, 6,
        9, 10, 11, 12, 13, 14, 15, 8
    };

    int index = ch - 'a';

    SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE),
        colors[index % 14]
    );
}

// 모든 카드를 맞췄는지 확인
bool check_clear(Card board[][MAX_SIZE], int row, int col)
{
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {

            // 조커는 남아 있어도 클리어 가능
            if (board[i][j].value == JOKER) {
                continue;
            }

            // 일반 카드 중 하나라도 안 맞았으면 클리어 아님
            if (!board[i][j].matched) {
                return false;
            }
        }
    }

    return true;
}