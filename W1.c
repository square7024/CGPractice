#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void reset_mat();
void print_base_mat();
void print_result_mat();
void print_matrix(int mat[4][4]);
int determinant3(int mat[3][3]);
int determinant4(int mat[4][4]);
void transpose(int mat[4][4], int result[4][4]);

int mat1[4][4];
int mat2[4][4];
int mat3[4][4];

int main()
{
	char command = 'a';
	int e_state = 0;
	int f_state = 0;

	srand((unsigned int)time(NULL));

	reset_mat();
	print_base_mat();

	while (command != 'q') {
		printf("Enter the command : ");
		scanf(" %c", &command);

		switch (command) {
		case 's':
			reset_mat();
			print_base_mat();
			break;
		case '+':
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat1[i][j] = (mat1[i][j] + 1) % 10;
				}
			}
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat2[i][j] = (mat2[i][j] + 1) % 10;
				}
			}
			print_base_mat();
			break;
		case '-':
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat1[i][j] = (mat1[i][j] + 10 - 1) % 10;
				}
			}
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat2[i][j] = (mat2[i][j] + 10 - 1) % 10;
				}
			}
			print_base_mat();
			break;
		case 'm':
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat3[i][j] = 0;
					for (int k = 0; k < 4; ++k) {
						mat3[i][j] += mat1[i][k] * mat2[k][j];
					}
				}
			}
			print_base_mat();
			print_result_mat();
			break;
		case 'a':
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat3[i][j] = mat1[i][j] + mat2[i][j];
				}
			}
			print_base_mat();
			print_result_mat();
			break;
		case 'd':
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mat3[i][j] = mat1[i][j] - mat2[i][j];
				}
			}
			print_base_mat();
			print_result_mat();
			break;
		case 'r':
			printf("\ndet(mat1) = %d\n", determinant4(mat1));
			printf("det(mat2) = %d\n\n", determinant4(mat2));
			break;
		case 't':
		{
			int trans1[4][4];
			int trans2[4][4];

			transpose(mat1, trans1);
			transpose(mat2, trans2);

			printf("\ntransposed mat 1 :\n");
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					printf("%d ", trans1[i][j]);
				}
				printf("\n");
			}

			printf("det(transposed mat1) = %d\n\n", determinant4(trans1));

			printf("transposed mat 2 :\n");
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					printf("%d ", trans2[i][j]);
				}
				printf("\n");
			}

			printf("det(transposed mat2) = %d\n\n", determinant4(trans2));

			break;
		}
		case 'e':
		{
			if (e_state == 0) {
				int result1[4][4];
				int result2[4][4];

				for (int i = 0; i < 4; ++i) {
					int min1 = mat1[i][0];
					int min2 = mat2[i][0];

					// 각 행의 최소값 찾기
					for (int j = 1; j < 4; ++j) {
						if (mat1[i][j] < min1)
							min1 = mat1[i][j];

						if (mat2[i][j] < min2)
							min2 = mat2[i][j];
					}

					// 해당 행에서 최소값 빼기
					for (int j = 0; j < 4; ++j) {
						result1[i][j] = mat1[i][j] - min1;
						result2[i][j] = mat2[i][j] - min2;
					}
				}

				printf("\nresult mat 1 :\n");
				print_matrix(result1);

				printf("\nresult mat 2 :\n");
				print_matrix(result2);
				printf("\n");

				e_state = 1;
			}
			else {
				print_base_mat();
				e_state = 0;
			}

			break;
		}
		case 'f':
		{
			if (f_state == 0) {
				int result1[4][4];
				int result2[4][4];

				for (int j = 0; j < 4; ++j) {
					int max1 = mat1[0][j];
					int max2 = mat2[0][j];

					// 각 열의 최대값 찾기
					for (int i = 1; i < 4; ++i) {
						if (mat1[i][j] > max1)
							max1 = mat1[i][j];

						if (mat2[i][j] > max2)
							max2 = mat2[i][j];
					}

					// 해당 열에 최대값 더하기
					for (int i = 0; i < 4; ++i) {
						result1[i][j] = mat1[i][j] + max1;
						result2[i][j] = mat2[i][j] + max2;
					}
				}

				printf("\nresult mat 1 :\n");
				print_matrix(result1);

				printf("\nresult mat 2 :\n");
				print_matrix(result2);
				printf("\n");

				f_state = 1;
			}
			else {
				print_base_mat();
				f_state = 0;
			}

			break;
		}
		}
	}

	return 0;
}

void print_base_mat()
{
	printf("\nmat 1 : \n");
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%d ", mat1[i][j]);
		}
		printf("\n");
	}

	printf("\nmat 2 : \n");
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%d ", mat2[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void print_result_mat()
{
	printf("\nresult : \n");
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%d ", mat3[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void print_matrix(int mat[4][4])
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%d ", mat[i][j]);
		}
		printf("\n");
	}
}

void reset_mat()
{
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat1[i][j] = rand() % 9 + 1;
			mat2[i][j] = rand() % 9 + 1;
			mat3[i][j] = rand() % 9 + 1;
		}
	}
}

int determinant3(int mat[3][3])
{
	return
		mat[0][0] * (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1])
		- mat[0][1] * (mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0])
		+ mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
}

int determinant4(int mat[4][4])
{
	int det = 0;

	for (int col = 0; col < 4; ++col) {

		int temp[3][3];
		int temp_i = 0;

		for (int i = 1; i < 4; ++i) {
			int temp_j = 0;

			for (int j = 0; j < 4; ++j) {

				if (j == col)
					continue;

				temp[temp_i][temp_j] = mat[i][j];
				temp_j++;
			}

			temp_i++;
		}

		if (col % 2 == 0)
			det += mat[0][col] * determinant3(temp);
		else
			det -= mat[0][col] * determinant3(temp);
	}

	return det;
}

void transpose(int mat[4][4], int result[4][4])
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = mat[j][i];
		}
	}
}