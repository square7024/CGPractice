#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <math.h>

#define MAX_POINT 10

int count = 0;      // 현재 저장된 점의 개수
int bottom = -1;    // 맨 아래 점의 인덱스
int top = -1;       // 맨 위 점의 인덱스

typedef struct {
	int x;
	int y;
	int z;
	bool used;
} Point;

typedef struct {
    Point point;
    double distance;
} DistancePoint;

void print_list(Point list[]);                          // 리스트 출력
void push_top(Point list[], int x, int y, int z);       // 리스트의 맨 위에 점 추가
void pop_top(Point list[]);                             // 리스트의 맨 위 점 삭제
void push_bottom(Point list[], int x, int y, int z);    // 리스트의 맨 아래에 점 추가
void pop_bottom(Point list[]);                          // 리스트의 맨 아래 점 삭제
void move_down(Point list[]);                           // 리스트의 모든 점을 한 칸 아래로 이동
void clear_list(Point list[]);                          // 리스트 초기화
void print_distance_list(Point list[]);                 // 거리 오름차순으로 리스트 출력
void print_point_distance(Point list[]);

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
	Point list[MAX_POINT] = { 0 };
    char command = '\0';
    bool f_state = false;

    while (command != 'q') {
        printf("\n명령어를 입력하세요 : ");
        scanf(" %c", &command);

        switch (command) {
        case '+':
        {
            int x, y, z;

            scanf("%d %d %d", &x, &y, &z);

            push_top(list, x, y, z);
            print_list(list);
            break;
        }
        case '-':
            pop_top(list);
            print_list(list);
            break;
        case 'e':
        {
            int x, y, z;

            scanf("%d %d %d", &x, &y, &z);

            push_bottom(list, x, y, z);
            print_list(list);
            break;
        }
        case 'd':
            pop_bottom(list);
            print_list(list);
            break;
        case 'a':
            printf("\n저장된 점의 개수 : %d\n", count);
            break;
        case 'b':
            move_down(list);
            print_list(list);
            break;
        case 'c':
            clear_list(list);
            print_list(list);
            break;
        case 'f':
            f_state = !f_state;

            if (f_state) {
                print_distance_list(list);
            }
            else {
                print_list(list);
            }

            break;
        case 'g':
            print_point_distance(list);
            break;
        }
    }

    return 0;
}

// 리스트 출력
void print_list(Point list[])
{
    printf("\n");

    for (int i = MAX_POINT - 1; i >= 0; --i) {
        printf("%d\t", i);

        if (list[i].used) {
            printf("%d %d %d", list[i].x, list[i].y, list[i].z);
        }

        printf("\n");
    }
}

// 리스트의 맨 위에 점 추가
void push_top(Point list[], int x, int y, int z)
{
    if (count == MAX_POINT) {
        printf("\n리스트가 가득 찼습니다.\n");
        return;
    }

    int index;

    if (count == 0) {
        index = 0;
    }
    else {
        index = (top + 1) % MAX_POINT;

        while (list[index].used) {
            index = (index + 1) % MAX_POINT;
        }
    }

    list[index].x = x;
    list[index].y = y;
    list[index].z = z;
    list[index].used = true;

    ++count;
    top = index;

    if (count == 1) {
        bottom = index;
    }
}

// 리스트의 맨 위 점 삭제
void pop_top(Point list[])
{
    // 리스트가 비어 있는 경우
    if (count == 0) {
        printf("\n리스트가 비어 있습니다.\n");
        return;
    }

    // 현재 top 삭제
    list[top].used = false;

    --count;

    // 삭제 후 리스트가 비어 있으면
    if (count == 0) {
        top = -1;
        bottom = -1;
        return;
    }

    // 이전에 저장된 점을 찾아 새로운 top으로 설정
    do {
        top = (top - 1 + MAX_POINT) % MAX_POINT;
    } while (!list[top].used);
}

// 리스트의 맨 아래에 점 추가
void push_bottom(Point list[], int x, int y, int z)
{
    if (count == MAX_POINT) {
        printf("리스트가 가득 찼습니다.\n");
        return;
    }

    // 리스트가 비어 있는 경우
    if (count == 0) {
        bottom = 0;
        top = 0;
    }

    // bottom 아래에 빈 공간이 있는 경우
    else if (bottom > 0) {
        --bottom;
    }

    // bottom이 이미 0인 경우
    else {
        // 기존 데이터를 한 칸씩 위로 이동
        for (int i = top + 1; i > 0; --i) {
            list[i] = list[i - 1];
        }

        ++top;
        bottom = 0;
    }

    // 새로운 점 저장
    list[bottom].x = x;
    list[bottom].y = y;
    list[bottom].z = z;
    list[bottom].used = true;

    ++count;
}

// 리스트의 맨 아래 점 삭제
void pop_bottom(Point list[])
{
    // 리스트가 비어 있는 경우
    if (count == 0) {
        printf("리스트가 비어 있습니다.\n");
        return;
    }

    // 현재 bottom 삭제
    list[bottom].used = false;

    --count;

    // 삭제 후 리스트가 비어 있는 경우
    if (count == 0) {
        bottom = -1;
        top = -1;
        return;
    }

    // 다음에 저장된 점을 찾아 새로운 bottom으로 설정
    do {
        bottom = (bottom + 1) % MAX_POINT;
    } while (!list[bottom].used);
}

// 리스트의 모든 점을 한 칸 아래로 이동
void move_down(Point list[])
{
    Point temp = list[0];

    for (int i = 0; i < MAX_POINT - 1; ++i) {
        list[i] = list[i + 1];
    }

    list[MAX_POINT - 1] = temp;

    // bottom과 top도 같이 이동
    if (count > 0) {
        bottom = (bottom - 1 + MAX_POINT) % MAX_POINT;
        top = (top - 1 + MAX_POINT) % MAX_POINT;
    }
}

// 리스트 초기화
void clear_list(Point list[])
{
    for (int i = 0; i < MAX_POINT; ++i) {
        list[i].used = false;
    }

    count = 0;
    bottom = -1;
    top = -1;
}

// 거리 오름차순으로 리스트 출력
void print_distance_list(Point list[])
{
    DistancePoint temp[MAX_POINT];
    int n = 0;

    // 사용 중인 점만 temp에 복사
    for (int i = 0; i < MAX_POINT; ++i) {
        if (list[i].used) {
            temp[n].point = list[i];

            int x = list[i].x;
            int y = list[i].y;
            int z = list[i].z;

            temp[n].distance = sqrt(
                x * x +
                y * y +
                z * z
            );

            ++n;
        }
    }

    // 거리 오름차순 정렬
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - 1 - i; ++j) {
            if (temp[j].distance > temp[j + 1].distance) {
                DistancePoint swap = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = swap;
            }
        }
    }

    printf("\n");

    // 9부터 0까지 출력
    for (int i = MAX_POINT - 1; i >= 0; --i) {
        printf("%d\t", i);

        if (i < n) {
            printf("%d %d %d\t거리 : %.2f",
                temp[i].point.x,
                temp[i].point.y,
                temp[i].point.z,
                temp[i].distance);
        }

        printf("\n");
    }
}

void print_point_distance(Point list[])
{
    if (count < 2) {
        printf("점이 2개 이상 필요합니다.\n");
        return;
    }

    double min_distance = 0.0;
    double max_distance = 0.0;

    int min_i = -1;
    int min_j = -1;
    int max_i = -1;
    int max_j = -1;

    bool first = true;

    for (int i = 0; i < MAX_POINT; ++i) {

        if (!list[i].used) {
            continue;
        }

        for (int j = i + 1; j < MAX_POINT; ++j) {

            if (!list[j].used) {
                continue;
            }

            int dx = list[i].x - list[j].x;
            int dy = list[i].y - list[j].y;
            int dz = list[i].z - list[j].z;

            double distance = sqrt(
                dx * dx +
                dy * dy +
                dz * dz
            );

            // 첫 번째 조합
            if (first) {
                min_distance = distance;
                max_distance = distance;

                min_i = i;
                min_j = j;
                max_i = i;
                max_j = j;

                first = false;
            }

            // 가장 가까운 두 점
            if (distance < min_distance) {
                min_distance = distance;
                min_i = i;
                min_j = j;
            }

            // 가장 먼 두 점
            if (distance > max_distance) {
                max_distance = distance;
                max_i = i;
                max_j = j;
            }
        }
    }

    printf("\n가장 가까운 두 점\n");
    printf("(%d, %d, %d) - (%d, %d, %d)\n",
        list[min_i].x, list[min_i].y, list[min_i].z,
        list[min_j].x, list[min_j].y, list[min_j].z);
    printf("거리 : %.2f\n",
        min_distance);

    printf("\n가장 먼 두 점\n");
    printf("(%d, %d, %d) - (%d, %d, %d)\n",
        list[max_i].x, list[max_i].y, list[max_i].z,
        list[max_j].x, list[max_j].y, list[max_j].z);
    printf("거리 : %.2f\n",
        max_distance);
}