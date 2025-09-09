#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

typedef struct {
	int x ,y, z;
}vertex;

#define MAX 10

static vertex list[10];
static int front = -1, rear = -1, count = 0;

void print_list(vertex list[], int front,int rear) {
	for (int i = 0; i < MAX; ++i) {
		printf("%d | %d %d %d", i, list[i].x, list[i].y, list[i].z);
		if (front == i) printf(" <--- front");	// front 표시
		if (rear == i) printf(" <--- rear");	// rear 표시
		printf("\n");
	}
	printf("\n");
}

void push_top(vertex v) {
	if (count == MAX) {			// full 일때 반환
		printf("List Full\n");
		return;
	}

	rear = 
}

int main()
{
	vertex v;
	char command;
	int n1, n2, n3;

	while (1) {
		printf("input command : ");
		scanf(" %c", &command);

		switch (command) {
		case '+': {
			printf("input number : ");
			scanf("%1d%1d%1d", n1, n2, n3);
			v.x = n1; v.y = n2; v.z = n3;

			print_list(list, front, rear);

			break;
		}
		case 'q': {
			return 0;
		}
		}
	}
	return 0;
}