#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

typedef struct {
	int x ,y, z;
}vertex;

#define MAX 10

static vertex list[10];
static int front = 0, rear = -1, count = 0;
static vertex backup[MAX];
static vertex backup[MAX];
static int backup_front, backup_rear, backup_count;
static int sorted_mode = 0; // 0: 원본, 1: 정렬상태

void print_list(vertex list[], int front,int rear) {
	printf("\n"); 
	for (int i = 0; i < MAX; ++i) { 
		printf("%d | %d %d %d", i, list[i].x, list[i].y, list[i].z);
		if (front == i && count != 0) printf(" <--- front"); // front 표시 
		if (rear == i && count != 0) printf(" <--- rear"); // rear 표시 
		printf("\n");
	}
	printf("\n"); 
}

void list_clear(int n) {
	list[n].x = -1;
	list[n].y = -1;
	list[n].z = -1;
}

void push_top(vertex v) {
	if (count == MAX) { // full 일때 반환
		printf("List Full\n"); 
		return;
	}

	rear = (rear + 1) % MAX;
	list[rear] = v; count++;
} 

void pop_top() {
	if (count == 0) { 
		printf("List Empty\n"); 
		return; 
	} 

	list_clear(rear); 
	rear = (rear - 1 + MAX) % MAX; 
	count--; 
} 

void push_bottom(vertex v) { 
	if (count == MAX) { 
		printf("List Full\n");
		return; 
	}

	front = (front - 1 + MAX) % MAX; 
	list[front] = v; count++;
} 

void pop_bottom() { 
	if (count == 0) { 
		printf("List Empty\n"); 
		return; 
	}

	list_clear(front);
	front = (front + 1) % MAX;
	count--; 
} 

void shift_down() { 
	if (count == 0) return; 
	vertex temp = list[0]; 
	for (int i = 0; i < MAX - 1; ++i) { 
		list[i] = list[i + 1]; 
	}

	list[MAX - 1] = temp;
	front = (front - 1 + MAX) % MAX;
	rear = (rear - 1 + MAX) % MAX; 
} 

void shift_up() { 
	if (count == 0) return; 
	vertex tmp = list[MAX - 1]; 
	for (int i = MAX - 1; i > 0; --i) { 
		list[i] = list[i - 1]; 
	} 
	list[0] = tmp; 
	front = (front + 1) % MAX; 
	rear = (rear + 1) % MAX; 
}

int distanceXdiatance(vertex v) { 
	return v.x * v.x + v.y * v.y + v.z * v.z; 
}

int collect_to_buffer(vertex buf[]) {
	for (int i = 0; i < count; ++i) {
		int idx = (front + i) % MAX;
		buf[i] = list[idx];
	}
	return count;
}

void sort_by_distance(vertex a[], int n) {
	for (int i = 0; i < n - 1; ++i) {
		for (int j = 0; j < n - 1 - i; ++j) {
			if (distanceXdiatance(a[j]) > distanceXdiatance(a[j + 1])) {
				vertex t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
			}
		}
	}
}

void toggle_sort(void) {
	if (!sorted_mode) {
		// 백업
		for (int i = 0; i < MAX; ++i) backup[i] = list[i];
		backup_front = front; backup_rear = rear; backup_count = count;

		// 수집 → 정렬
		vertex buf[MAX];
		int n = collect_to_buffer(buf);
		sort_by_distance(buf, n);

		// 0부터 연속 저장
		for (int i = 0; i < n; ++i) list[i] = buf[i];
		for (int i = n; i < MAX; ++i) list_clear(i);

		front = 0;
		rear = (n > 0) ? n - 1 : -1;
		count = n;

		sorted_mode = 1;
	}
	else {
		// 복원
		for (int i = 0; i < MAX; ++i) list[i] = backup[i];
		front = backup_front; rear = backup_rear; count = backup_count;
		sorted_mode = 0;
	}
}

int main()
{
	vertex v;
	char command;
	int n1, n2, n3;
	for (int i = 0; i < MAX; ++i) {
		list_clear(i);
	}

	while (1) {
		printf("input command : ");
		scanf(" %c", &command);

		switch (command) {
		case '+': {
			printf("input number : ");
			scanf("%1d%1d%1d", &n1, &n2, &n3);
			v.x = n1; v.y = n2; v.z = n3;
			push_top(v);
			print_list(list, front, rear);
			break;
		}
		case '-': {
			pop_top();
			print_list(list, front, rear);
			break;
		}
		case 'e': {
			printf("input number : ");
			scanf("%1d%1d%1d", &n1, &n2, &n3);
			v.x = n1; v.y = n2; v.z = n3;
			shift_up();
			push_bottom(v);
			print_list(list, front, rear);
			break;
		}
		case 'd': {
			pop_bottom();
			print_list(list, front, rear);
			break;
		}
		case 'a': {
			printf("\nnumber of vertex : %d\n", count);
			print_list(list, front, rear);
			break;
		}
		case 'b': {
			shift_down();
			print_list(list, front, rear);
			break;
		}
		case 'c': {
		front = 0; rear = -1; count = 0;
			for (int i = 0; i < MAX; ++i) { 
				list_clear(i);
			}
			print_list(list, front, rear);
			break;
		}
		case 'f': {
			toggle_sort();
			print_list(list, front, rear);
			break;
		}
		case 'q': {
			return 0;
		}
		}
	} return 0;
}