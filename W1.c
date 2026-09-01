#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void reset_mat();
void print_base_mat();
void print_result_mat();

int mat1[4][4];
int mat2[4][4];
int mat3[4][4];

int main()
{
	char command = 'a';

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
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat1[i][j] = (mat1[i][j] + 1) % 10;
				}
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat2[i][j] = (mat2[i][j] + 1) % 10;
				}
			}
			print_base_mat();
			break;
		case '-':
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat1[i][j] = (mat1[i][j] + 10 - 1) % 10;
				}
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat2[i][j] = (mat2[i][j] + 10 - 1) % 10;
				}
			}
			print_base_mat();
			break;
		case 'm':
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat3[i][j] = 0;
					for (int k = 0; k < 4; k++) {
						mat3[i][j] += mat1[i][k] * mat2[k][j];
					}
				}
			}
			print_base_mat();
			print_result_mat();
			break;
		case 'a':
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat3[i][j] = mat1[i][j] + mat2[i][j];
				}
			}
			print_base_mat();
			print_result_mat();
			break;
		case 'd':
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat3[i][j] = mat1[i][j] - mat2[i][j];
				}
			}
			print_base_mat();
			print_result_mat();
			break;
		}
	}

	return 0;
}

void print_base_mat()
{
	printf("\nmat 1 : \n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%d ", mat1[i][j]);
		}
		printf("\n");
	}
	
	printf("\nmat 2 : \n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%d ", mat2[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void print_result_mat()
{
	printf("\nresult : \n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%d ", mat3[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void reset_mat()
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			mat1[i][j] = rand() % 9 + 1;
			mat2[i][j] = rand() % 9 + 1;
			mat3[i][j] = rand() % 9 + 1;
		}
	}
}