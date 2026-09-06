#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define LINE_COUNT 10
#define MAX_LEN 500

void open_file(char filename[], char str[][MAX_LEN]);

int main()
{
	FILE* file;
	char filename[100];
	char str[LINE_COUNT][MAX_LEN];

	printf("파일 이름을 입력하세요 : ");
	scanf("%s", filename);

	open_file(filename, str);

	return 0;
}

void open_file(char filename[], char str[][MAX_LEN])
{
	FILE* file;

	file = fopen(filename, "r");

	if (file == NULL) {
		printf("파일을 열 수 없습니다.\n");
		return;
	}

	int i = 0;

	while (i < 10 && fgets(str[i], MAX_LEN, file) != NULL) {
		printf("%s", str[i]);
		++i;
	}

	fclose(file);
}