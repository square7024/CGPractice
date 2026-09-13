#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define LINE_COUNT 10												// 최대 10줄까지 읽기
#define MAX_LEN 500													// 한 줄 최대 500자까지 읽기

int open_file(char filename[], char str[][MAX_LEN]);				// 파일 열기 함수
void print_file(char str[][MAX_LEN]);								// 파일 내용 출력 함수
void change_case(char str[][MAX_LEN]);								// 대소문자 변환 함수
void count_word(char str[][MAX_LEN]);								// 단어 개수 세기 함수
void capital_word(char str[][MAX_LEN]);								// 단어 첫 글자 대문자 변환 함수
void reverse_sentence(char str[][MAX_LEN]);							// 문장 뒤집기 함수
void change_space(char str[][MAX_LEN]);								// 공백과 * 변환 함수
void reverse_word(char str[][MAX_LEN]);								// 단어 뒤집기 함수
void replace_char(char str[][MAX_LEN], char before, char after);	// 특정 문자 변환 함수
void print_number_break(char str[][MAX_LEN]);						// 숫자 뒤의 내용을 다음 줄에 출력하는 함수
void find_word(char str[][MAX_LEN], char word[]);					// 입력한 단어를 찾아 색상 변경 및 개수 출력하는 함수
void rotate_sentence(char str[][MAX_LEN]);							// 문장 순서 변경 함수

int main()	
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	
	char filename[100];
	char str[LINE_COUNT][MAX_LEN];
	char command = '\0';
	int c_state = 0;
	int g_state = 0;
	char before;
	char after;
	int h_state = 0;


	// 파일 열기
	while (1) {
		printf("파일 이름을 입력하세요 : ");
		scanf("%s", filename);

		if (open_file(filename, str) == 1)
			print_file(str);
			break;
	}

	// 명령어 입력
	while (command != 'q') {
		printf("\n명령어를 입력하세요 : ");
		scanf(" %c", &command);

		switch (command) {
			case 'a':
				change_case(str);
				print_file(str);
				break;
			case 'b':
				count_word(str);
				break;
			case 'c':
				if (c_state == 0) {
					capital_word(str);
					c_state = 1;
				}
				else {
					print_file(str);
					c_state = 0;
				}
				break;
			case 'd':
				reverse_sentence(str);
				print_file(str);
				break;
			case 'e':
				change_space(str);
				print_file(str);
				break;
			case 'f':
				reverse_word(str);
				print_file(str);
				break;
			case 'g':
				if (g_state == 0) {
					printf("변경할 문자와 바꿀 문자를 입력하세요 : ");
					scanf(" %c %c", &before, &after);

					replace_char(str, before, after);
					g_state = 1;
				}
				else {
					replace_char(str, after, before);
					g_state = 0;
				}

				print_file(str);
				break;
			case 'h':
				if (h_state == 0) {
					print_number_break(str);
					h_state = 1;
				}
				else {
					print_file(str);
					h_state = 0;
				}
				break;
			case 'i':
			{
				char word[100];

				printf("찾을 단어를 입력하세요 : ");
				scanf("%s", word);

				find_word(str, word);
				break;
			}
			case 'j':
				rotate_sentence(str);
				print_file(str);
				break;
		}
	}

	return 0;
}

// 파일 내용 출력 함수
void print_file(char str[][MAX_LEN])
{
	printf("\n");
	for (int i = 0; i < LINE_COUNT; ++i) {
		printf("%s", str[i]);
	}
	printf("\n");
}

// 파일 열기 함수
int open_file(char filename[], char str[][MAX_LEN])
{
	FILE* file;

	file = fopen(filename, "r");

	if (file == NULL) {
		printf("파일을 열 수 없습니다.\n\n");
		return 0;
	}

	int i = 0;

	while (i < LINE_COUNT && fgets(str[i], MAX_LEN, file) != NULL) {
		++i;
	}

	fclose(file);

	return 1;
}

// 대소문자 변환 함수
void change_case(char str[][MAX_LEN])
{
	for (int i = 0; i < LINE_COUNT; ++i) {
		for (int j = 0; str[i][j] != '\0'; ++j) {

			if (str[i][j] >= 'A' && str[i][j] <= 'Z') {
				str[i][j] += 'a' - 'A';
			}
			else if (str[i][j] >= 'a' && str[i][j] <= 'z') {
				str[i][j] -= 'a' - 'A';
			}
		}
	}
}

// 단어 개수 세기 함수
void count_word(char str[][MAX_LEN])
{
	for (int i = 0; i < LINE_COUNT; ++i) {
		int count = 0;
		int in_word = 0;

		for (int j = 0; str[i][j] != '\0'; ++j) {
			if (str[i][j] != ' ' && str[i][j] != '\n') {
				if (in_word == 0) {
					++count;
					in_word = 1;
				}
			}
			else {
				in_word = 0;
			}
		}

		printf("\n");
		for (int j = 0; str[i][j] != '\0'; ++j) {
			if (str[i][j] != '\n') {
				printf("%c", str[i][j]);
			}
		}

		printf(" / 단어 개수 : %d", count);
	}

	printf("\n");
}

// 단어 첫 글자 대문자 변환 함수
void capital_word(char str[][MAX_LEN])
{
	WORD normal_color = 7;
	WORD capital_color = 12;

	int count = 0;
	int in_word = 0;
	int is_capital = 0;

	printf("\n");

	for (int i = 0; i < LINE_COUNT; ++i) {
		in_word = 0;
		is_capital = 0;

		for (int j = 0; str[i][j] != '\0'; ++j) {

			// 공백 또는 줄바꿈을 만난 경우
			if (str[i][j] == ' ' || str[i][j] == '\n') {

				// 색칠 중이었다면 원래 색으로
				if (is_capital == 1) {
					SetConsoleTextAttribute(
						GetStdHandle(STD_OUTPUT_HANDLE),
						normal_color
					);

					is_capital = 0;
				}

				in_word = 0;
				printf("%c", str[i][j]);
			}
			else {

				// 새로운 단어의 시작
				if (in_word == 0) {

					// 대문자로 시작하는지 확인
					if (str[i][j] >= 'A' && str[i][j] <= 'Z') {
						SetConsoleTextAttribute(
							GetStdHandle(STD_OUTPUT_HANDLE),
							capital_color
						);

						is_capital = 1;
						++count;
					}

					in_word = 1;
				}

				printf("%c", str[i][j]);
			}
		}

		// 마지막 단어 뒤에 '\n'이 없는 경우를 대비
		if (is_capital == 1) {
			SetConsoleTextAttribute(
				GetStdHandle(STD_OUTPUT_HANDLE),
				normal_color
			);
		}
	}

	printf("\n대문자로 시작하는 단어 개수 : %d\n", count);
}

// 문장 뒤집기 함수
void reverse_sentence(char str[][MAX_LEN])
{
	for (int i = 0; i < LINE_COUNT; ++i) {
		int len = 0;

		// 문장의 길이 구하기
		while (str[i][len] != '\0' && str[i][len] != '\n') {
			++len;
		}

		// 문장 뒤집기
		for (int j = 0; j < len / 2; ++j) {
			char temp = str[i][j];
			str[i][j] = str[i][len - 1 - j];
			str[i][len - 1 - j] = temp;
		}
	}
}

// 공백과 * 변환 함수
void change_space(char str[][MAX_LEN])
{
	for (int i = 0; i < LINE_COUNT; ++i) {
		for (int j = 0; str[i][j] != '\0'; ++j) {

			if (str[i][j] == ' ') {
				str[i][j] = '*';
			}
			else if (str[i][j] == '*') {
				str[i][j] = ' ';
			}
		}
	}
}

// 단어 뒤집기 함수
void reverse_word(char str[][MAX_LEN])
{
	for (int i = 0; i < LINE_COUNT; ++i) {
		int start = 0;

		for (int j = 0; ; ++j) {

			// 공백, *, 줄바꿈, 문자열 끝을 만나면 단어의 끝
			if (str[i][j] == ' ' ||
				str[i][j] == '*' ||
				str[i][j] == '\n' ||
				str[i][j] == '\0') {

				int end = j - 1;

				// start ~ end 범위의 단어 뒤집기
				while (start < end) {
					char temp = str[i][start];
					str[i][start] = str[i][end];
					str[i][end] = temp;

					++start;
					--end;
				}

				// 다음 단어의 시작 위치
				start = j + 1;
			}

			// 문자열 끝이면 반복 종료
			if (str[i][j] == '\0') {
				break;
			}
		}
	}
}

// 문자 바꾸기 함수
void replace_char(char str[][MAX_LEN], char before, char after)
{
	for (int i = 0; i < LINE_COUNT; ++i) {
		for (int j = 0; str[i][j] != '\0'; ++j) {
			if (str[i][j] == before) {
				str[i][j] = after;
			}
		}
	}
}

// 숫자 뒤의 내용을 다음 줄에 출력하는 함수
void print_number_break(char str[][MAX_LEN])
{
	printf("\n");

	for (int i = 0; i < LINE_COUNT; ++i) {
		for (int j = 0; str[i][j] != '\0'; ++j) {

			printf("%c", str[i][j]);

			if (str[i][j] >= '0' && str[i][j] <= '9') {

				// 숫자 뒤가 공백이면
				if (str[i][j + 1] == ' ') {
					printf("\n");
					++j;    // 기존 공백은 출력하지 않음
				}

				// 숫자 뒤에 문자나 숫자가 붙어 있으면
				else if (str[i][j + 1] != '\0' &&
					str[i][j + 1] != '\n') {
					printf("\n");
				}
			}
		}
	}

	printf("\n");
}

// 입력한 단어를 찾아 색상 변경 및 개수 출력하는 함수
void find_word(char str[][MAX_LEN], char word[])
{
	WORD normal_color = 7;
	WORD find_color = 12;

	int count = 0;

	printf("\n");

	for (int i = 0; i < LINE_COUNT; ++i) {

		for (int j = 0; str[i][j] != '\0'; ) {

			// 단어의 시작 위치
			if (str[i][j] != ' ' &&
				str[i][j] != '*' &&
				str[i][j] != '\n') {

				int start = j;

				// 단어의 끝까지 이동
				while (str[i][j] != '\0' &&
					str[i][j] != ' ' &&
					str[i][j] != '*' &&
					str[i][j] != '\n') {
					++j;
				}

				int end = j;
				int same = 1;
				int k = 0;

				// 입력한 단어와 길이 및 문자 비교
				while (start + k < end && word[k] != '\0') {

					char c1 = str[i][start + k];
					char c2 = word[k];

					// 대문자 -> 소문자
					if (c1 >= 'A' && c1 <= 'Z') {
						c1 += 'a' - 'A';
					}

					if (c2 >= 'A' && c2 <= 'Z') {
						c2 += 'a' - 'A';
					}

					if (c1 != c2) {
						same = 0;
						break;
					}

					++k;
				}

				// 두 단어의 길이가 다른 경우
				if (start + k != end || word[k] != '\0') {
					same = 0;
				}

				// 일치하는 단어
				if (same == 1) {
					SetConsoleTextAttribute(
						GetStdHandle(STD_OUTPUT_HANDLE),
						find_color
					);

					++count;
				}

				// 단어 출력
				for (int p = start; p < end; ++p) {
					printf("%c", str[i][p]);
				}

				// 원래 색상으로 복구
				if (same == 1) {
					SetConsoleTextAttribute(
						GetStdHandle(STD_OUTPUT_HANDLE),
						normal_color
					);
				}
			}
			else {
				printf("%c", str[i][j]);
				++j;
			}
		}
	}

	printf("\n찾은 단어 개수 : %d\n", count);
}

// 문장 순서 변경 함수
void rotate_sentence(char str[][MAX_LEN])
{
	char temp[MAX_LEN];

	// 마지막 문장을 임시 저장
	strcpy(temp, str[LINE_COUNT - 1]);

	// 문장을 한 칸씩 뒤로 이동
	for (int i = LINE_COUNT - 1; i > 0; --i) {
		strcpy(str[i], str[i - 1]);
	}

	// 마지막 문장을 첫 번째 위치로 이동
	strcpy(str[0], temp);
}