#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_matrix(int m[][4]) {
	for (int i = 0; i < 4; i++) { 
		for (int j = 0; j < 4; j++) { 
			printf("%3d ", m[i][j]);
		} printf("\n");
	} 
	printf("\n"); 
}

int min_matrix(int m[][4]) {
	int min = m[0][0];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (m[i][j] < min) min = m[i][j];
		}
	}

	return min;
}

int max_matrix(const int m[][4]) {
	int max = m[0][0];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (m[i][j] > max) max = m[i][j];
		}
	}

	return max;
}

void transpose(int m[][4]) {
	int t;

	for (int i = 0; i < 4; ++i)
		for (int j = i + 1; j < 4; ++j) {
			t = m[i][j];
			m[i][j] = m[j][i];
			m[j][i] = t;
		}
}

int value_3x3(int m[][3]) {
	int p1 = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
	int p2 = m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
	int p3 = m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	return p1 - p2 + p3;
}

int value_4x4(int m[][4]) {
	int i, j;
	int value;
	int matrix_1[3][3];
	int matrix_2[3][3];
	int matrix_3[3][3];
	int matrix_4[3][3];
	int a1 = m[0][0];
	int a2 = m[0][1];
	int a3 = m[0][2];
	int a4 = m[0][3];

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			matrix_1[i][j] = m[i + 1][j + 1];
			if (j == 0) matrix_2[i][j] = m[i + 1][j];
			else if (j == 1 || j == 2) matrix_2[i][j] = m[i + 1][j + 1];
			if (j == 0 || j == 1) matrix_3[i][j] = m[i + 1][j];
			else if (j == 2) matrix_3[i][j] = m[i + 1][j + 1];
			matrix_4[i][j] = m[i + 1][j];
		}
	}

	value = (a1 * value_3x3(matrix_1)) - (a2 * value_3x3(matrix_2)) + (a3 * value_3x3(matrix_3)) - (a4 * value_3x3(matrix_4));

	return value;
}


int main()
{
	int matrix1[4][4];
	int matrix2[4][4];
	int matrix_result[4][4];
	int i, j;
	char command;
	int min1, min2, max1, max2;
	int e_flag = 0;
	int f_flag = 0;

	srand(time(NULL));

	for (i = 0; i < 4; i++) { 
		for (j = 0; j < 4; j++) { 
			matrix1[i][j] = rand() % 10;
			matrix2[i][j] = rand() % 10;
		}
	}

	printf("matrix1 :\n");
	print_matrix(matrix1);
	printf("matrix2 :\n");
	print_matrix(matrix2);

	while (1) {
		printf("input the command : ");
		scanf(" %c", &command);
		if (e_flag == 1 && command != 'e') e_flag = 0;
		if (f_flag == 1 && command != 'f') f_flag = 0;
		printf("\n");

		switch (command) {
		case 'm': { // 행렬 곱셈
			for (i = 0; i < 4; i++) { 
				for (j = 0; j < 4; j++) { 
					matrix_result[i][j] = matrix1[i][0] * matrix2[0][j] + matrix1[i][1] * matrix2[1][j] + matrix1[i][2] * matrix2[2][j] + matrix1[i][3] * matrix2[3][j]; 
				}
			}
			printf("result :\n");
			print_matrix(matrix_result);
			break;
		}
		case 'a': { // 행렬 덧셈
			for (i = 0; i < 4; i++) { 
				for (j = 0; j < 4; j++) { 
					matrix_result[i][j] = matrix1[i][j] + matrix2[i][j];
				}
			}
			printf("result :\n");
			print_matrix(matrix_result);
			break;
		}
		case 'd': { // 행렬 뺄셈
			for (i = 0; i < 4; i++) { 
				for (j = 0; j < 4; j++) { 
					matrix_result[i][j] = matrix1[i][j] - matrix2[i][j];
				}
			}
			printf("result :\n");
			print_matrix(matrix_result);
			break;
		}
		case 'r': { // 행렬식의 값
			printf("matrix1 :\n");
			print_matrix(matrix1);
			printf("행렬식의 값 : %d\n\n", value_4x4(matrix1));
			printf("matrix2 :\n");
			print_matrix(matrix2);
			printf("행렬식의 값 : %d\n\n", value_4x4(matrix2));
			break;
		}
		case 't': { // 전치 행렬
			transpose(matrix1);
			transpose(matrix2);

			printf("matrix1 :\n");
			print_matrix(matrix1);
			printf("matrix2 :\n");
			print_matrix(matrix2);
			break;
		}
		case 'e': { // 행렬 최솟값 빼기
			if (e_flag == 0) {
				min1 = min_matrix(matrix1);
				min2 = min_matrix(matrix2);
				for (i = 0; i < 4; ++i) {
					for (j = 0; j < 4; ++j) {
						matrix1[i][j] = matrix1[i][j] - min1;
						matrix2[i][j] = matrix2[i][j] - min2;
					}
				}
				e_flag = 1;
			}
			else if (e_flag == 1) {
				for (i = 0; i < 4; ++i) {
					for (j = 0; j < 4; ++j) {
						matrix1[i][j] = matrix1[i][j] + min1;
						matrix2[i][j] = matrix2[i][j] + min2;
					}
				}
				e_flag = 0;
			}

			printf("matrix1 :\n");
			print_matrix(matrix1);
			printf("matrix2 :\n");
			print_matrix(matrix2);
			break;
		}
		case 'f': { // 행렬 최댓값 더하기
			if (f_flag == 0) {
				max1 = max_matrix(matrix1);
				max2 = max_matrix(matrix2);
				for (i = 0; i < 4; ++i) {
					for (j = 0; j < 4; ++j) {
						matrix1[i][j] = matrix1[i][j] + max1;
						matrix2[i][j] = matrix2[i][j] + max2;
					}
				}
				f_flag = 1;
			}
			else if (f_flag == 1) {
				for (i = 0; i < 4; ++i) {
					for (j = 0; j < 4; ++j) {
						matrix1[i][j] = matrix1[i][j] - max1;
						matrix2[i][j] = matrix2[i][j] - max2;
					}
				}
				f_flag = 0;
			}

			printf("matrix1 :\n");
			print_matrix(matrix1);
			printf("matrix2 :\n");
			print_matrix(matrix2);
			break;
			break;
		}
		case 's': { // 행렬 재생성
			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					matrix1[i][j] = rand() % 10;
					matrix2[i][j] = rand() % 10;
				}
			}
			printf("matrix1 :\n");
			print_matrix(matrix1);
			printf("matrix2 :\n");
			print_matrix(matrix2);
			break;
		}
		case 'q': { // 프로그램 종료
			return 0;
		}
		}
	}

	return 0;
}
