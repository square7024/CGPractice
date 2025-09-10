#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>

#define MAX 5
#define MAX_COUNT 50

typedef struct {
	int r, c;
}pos;

static char board[MAX][MAX];
static char back[MAX][MAX];
static char pool[MAX * MAX];

static pos joker;
static int count;
static int score;

int already_open(int r, int c);
void find_another(char ch, int skip_r, int skip_c);

void init_board() {
	for (int i = 0; i < MAX; ++i) {
		for (int j = 0; j < MAX; ++j) {
			board[i][j] = '*';
		}
	}
}

void build_pool(void) { // 전역 pool 채우기
	int idx = 0;
	for (char ch = 'a'; ch <= 'l'; ++ch) {
		pool[idx++] = ch;
		pool[idx++] = ch;
	}
	pool[idx++] = 'x'; // 25개 완료
}

void shuffle_board() {
	for (int i = (MAX * MAX) - 1; i > 0; --i) {
		int j = rand() % (i + 1);
		char tmp = pool[i];
		pool[i] = pool[j];
		pool[j] = tmp;
	}

	int k = 0;
	for (int r = 0; r < MAX; ++r) {
		for (int c = 0; c < MAX; ++c) {
			back[r][c] = pool[k++];
		}
	}
}

void print_board(char board[][MAX]) {
	int i;
	int j;
	unsigned short text;

	printf(" ");
	for (i = 0; i < MAX; ++i) printf("%3c", 'a' + i);
	printf("\n");
	for (i = 0; i < MAX; ++i) {
		printf("%d", i + 1);
		for (j = 0; j < MAX; ++j) {
			if (board[i][j] >= 'a' && board[i][j] <= 'l') {
				text = board[i][j] - ('a' - 1);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), text);
			}
			else if (board[i][j] >= 'A' && board[i][j] <= 'L') {
				text = board[i][j] - ('A' - 1);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), text);
			}
			printf("%3c", board[i][j]);
			text = 15;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), text);
		}
		printf("\n");
	}
	printf("\n");
}

void reset_game(void) {
	init_board();
	build_pool();
	shuffle_board();
	print_board(board);
}

void show_hint_ms(int ms) {
	print_board(back);
	Sleep(ms);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	print_board(board);
}

int already_open(int r, int c) {
	char ch = board[r][c];
	if (ch >= 'A' && ch <= 'X') return 1;
	return 0;
}

void find_another(char ch, int skip_r, int skip_c) {
	joker.r = -1; joker.c = -1;
	for (int i = 0; i < MAX; ++i) {
		for (int j = 0; j < MAX; ++j) {
			if (i == skip_r && j == skip_c) continue;  // 같은 칸만 제외
			if (back[i][j] != ch) continue;            // 같은 글자만
			// 이미 고정된 칸(A~Z or X)은 건너뛰기
			char cur = board[i][j];
			if (cur == 'X' || (cur >= 'A' && cur <= 'Z')) continue;
			joker.r = i; joker.c = j;
			return;
		}
	}
}

int main()
{
	int row;
	char col;
	int r, c;
	int turn = 0;
	count = 0;
	score = 0;
	pos pick;
	joker.r = -1; joker.c = -1;
	init_board();
	print_board(board);
	build_pool();

	srand(time(NULL));
	shuffle_board();

	while (1) {
		if (score == 12) {
			printf("game clear!\n");
			return 0;
		}
		if (count >= MAX_COUNT) {
			printf("game over! try again\n");
			return 0;
		}
		printf("input command or column : ");
		scanf(" %c", &col);

		if (col >= 'a' && col <= 'e') {
			printf("input row : ");
			scanf(" %d", &row);
			if (row < 1 || row > 5) {
				printf("row must be 1~5\n");
				continue;
			}
			r = row - 1;
			c = col - 'a';

			if (already_open(r, c)) {
				printf("it's already opened!\n");
				continue;
			}

			if (turn == 0) {
				pick.r = r;
				pick.c = c;
				board[r][c] = back[r][c];
				print_board(board);
				turn = 1;
			}
			else if (turn == 1) {
				// 같은 칸 두 번 선택 방지
				if (r == pick.r && c == pick.c) {
					printf("same cell twice!\n");
					continue;
				}
				// 이미 고정된 칸 가드
				if (already_open(r, c)) {
					printf("it's already opened!\n");
					continue;
				}

				// 두 번째 카드 임시 공개
				board[r][c] = back[r][c];
				print_board(board);
				Sleep(1200);

				char a = back[pick.r][pick.c];
				char b = back[r][c];

				if (a == 'x' || b == 'x') {
					// 조커 위치(xr,xc)와 글자 위치(lr,lc) 구분
					int xr = (a == 'x') ? pick.r : r;
					int xc = (a == 'x') ? pick.c : c;
					int lr = (a == 'x') ? r : pick.r;
					int lc = (a == 'x') ? c : pick.c;

					char letter = back[lr][lc]; // 'a'~'l' (선택된 글자)

					// 선택된 글자 고정(대문자)
					board[lr][lc] = (char)('A' + (letter - 'a'));

					// 보드 어딘가의 같은 글자 한 장도 자동 고정
					find_another(letter, lr, lc);   // 전역 joker.r/joker.c에 결과 저장
					if (joker.r != -1) {
						board[joker.r][joker.c] = (char)('A' + (letter - 'a'));
					}

					// 조커 고정 표시
					board[xr][xc] = 'X';

					turn = 0;
					score++;
				}
				else if (a == b) {
					// 일반 성공: 두 장 모두 대문자로 고정
					board[pick.r][pick.c] = (char)('A' + (a - 'a'));
					board[r][c] = (char)('A' + (b - 'a'));
					turn = 0;
					score++;
				}
				else {
					// 실패: 다시 가리기
					board[pick.r][pick.c] = '*';
					board[r][c] = '*';
					turn = 0;
				}
				count++;
				printf("count : %d, score : %d\n", count, score);
				print_board(board);
			}

		}
		else if (col == 'r') {
			reset_game();

		}
		else if (col == 'h') {
			show_hint_ms(1200);
		}
		else if (col == 'q') return 0;
	}

	return 0;
}