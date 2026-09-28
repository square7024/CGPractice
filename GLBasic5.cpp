#include <gl/glew.h>
#include <gl/glfw3.h>

#include <iostream>
#include <random>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_RECT_COUNT 50

struct Rect
{
	float x;
	float y;

	float size;

	float r;
	float g;
	float b;

	bool visible;
};

Rect rects[MAX_RECT_COUNT];

int rectCount{ 0 };
int addedRectCount{ 0 };

// 기본 사각형 크기
const float RECT_SIZE{ 0.08f };

// 지우개
bool eraserActive{ false };

float eraserX{ 0.0f };
float eraserY{ 0.0f };

// 처음에는 일반 사각형의 2배
float eraserBaseSize{ RECT_SIZE * 2.0f };
float eraserSize{ RECT_SIZE * 2.0f };

float eraserR{ 0.0f };
float eraserG{ 0.0f };
float eraserB{ 0.0f };

// 랜덤
std::random_device rd;
std::mt19937 gen(rd());

std::uniform_int_distribution<int> countDist(20, 40);
std::uniform_real_distribution<float> positionXDist(
	-1.0f + RECT_SIZE,
	1.0f - RECT_SIZE
);
std::uniform_real_distribution<float> positionYDist(
	-1.0f + RECT_SIZE,
	1.0f - RECT_SIZE
);
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

// 함수 선언
void InitRects();
void AddRect(float x, float y);

void DrawScene();
void DrawRect(float x, float y, float size, float r, float g, float b);

void CheckCollision();

void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
);

void CursorPositionCallback(
	GLFWwindow* window,
	double xpos,
	double ypos
);

void KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods
);

int main()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		"Practice 5",
		NULL,
		NULL
	);

	if (!window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;

		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!" << std::endl;

		glfwDestroyWindow(window);
		glfwTerminate();

		return -1;
	}

	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetKeyCallback(window, KeyCallback);

	InitRects();

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(
			1.0f,
			1.0f,
			1.0f,
			1.0f
		);

		glClear(GL_COLOR_BUFFER_BIT);

		DrawScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// 사각형 초기 생성
void InitRects()
{
	rectCount = countDist(gen);
	addedRectCount = 0;

	for (int i = 0; i < rectCount; ++i)
	{
		rects[i].x = positionXDist(gen);
		rects[i].y = positionYDist(gen);

		rects[i].size = RECT_SIZE;

		rects[i].r = colorDist(gen);
		rects[i].g = colorDist(gen);
		rects[i].b = colorDist(gen);

		rects[i].visible = true;
	}

	// 지우개 초기화
	eraserActive = false;

	eraserBaseSize = RECT_SIZE * 2.0f;
	eraserSize = eraserBaseSize;

	eraserR = 0.0f;
	eraserG = 0.0f;
	eraserB = 0.0f;
}

// 새로운 사각형 추가
void AddRect(float x, float y)
{
	// 최대 10개까지만 새로 생성
	if (addedRectCount >= 10)
	{
		return;
	}

	if (rectCount >= MAX_RECT_COUNT)
	{
		return;
	}

	rects[rectCount].x = x;
	rects[rectCount].y = y;

	rects[rectCount].size = RECT_SIZE;

	rects[rectCount].r = colorDist(gen);
	rects[rectCount].g = colorDist(gen);
	rects[rectCount].b = colorDist(gen);

	rects[rectCount].visible = true;

	++rectCount;
	++addedRectCount;

	// 새 사각형이 생기면 지우개 기본 크기를 줄인다.
	eraserBaseSize -= RECT_SIZE * 0.1f;

	// 지나치게 작아지지 않도록 제한
	if (eraserBaseSize < RECT_SIZE)
	{
		eraserBaseSize = RECT_SIZE;
	}

	// 현재 지우개가 화면에 있다면 현재 크기도 같이 감소
	if (eraserActive)
	{
		eraserSize -= RECT_SIZE * 0.1f;

		if (eraserSize < RECT_SIZE)
		{
			eraserSize = RECT_SIZE;
		}
	}
}

// 화면 그리기
void DrawScene()
{
	// 일반 사각형
	for (int i = 0; i < rectCount; ++i)
	{
		if (!rects[i].visible)
		{
			continue;
		}

		DrawRect(
			rects[i].x,
			rects[i].y,
			rects[i].size,
			rects[i].r,
			rects[i].g,
			rects[i].b
		);
	}

	// 지우개
	if (eraserActive)
	{
		DrawRect(
			eraserX,
			eraserY,
			eraserSize,
			eraserR,
			eraserG,
			eraserB
		);
	}
}

// 사각형 하나 그리기
void DrawRect(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b
)
{
	float half{ size / 2.0f };

	glColor3f(r, g, b);

	glBegin(GL_QUADS);

	glVertex2f(
		x - half,
		y + half
	);

	glVertex2f(
		x + half,
		y + half
	);

	glVertex2f(
		x + half,
		y - half
	);

	glVertex2f(
		x - half,
		y - half
	);

	glEnd();
}

// 지우개와 사각형 충돌 검사
void CheckCollision()
{
	if (!eraserActive)
	{
		return;
	}

	float eraserHalf{ eraserSize / 2.0f };

	float eraserLeft{ eraserX - eraserHalf };
	float eraserRight{ eraserX + eraserHalf };
	float eraserTop{ eraserY + eraserHalf };
	float eraserBottom{ eraserY - eraserHalf };

	for (int i = 0; i < rectCount; ++i)
	{
		// 이미 사라진 사각형은 다시 검사하지 않는다.
		if (!rects[i].visible)
		{
			continue;
		}

		float rectHalf{ rects[i].size / 2.0f };

		float rectLeft{ rects[i].x - rectHalf };
		float rectRight{ rects[i].x + rectHalf };
		float rectTop{ rects[i].y + rectHalf };
		float rectBottom{ rects[i].y - rectHalf };

		// AABB 충돌 검사
		bool collision{
			eraserRight >= rectLeft &&
			eraserLeft <= rectRight &&
			eraserTop >= rectBottom &&
			eraserBottom <= rectTop
		};

		if (collision)
		{
			// 사각형 삭제
			rects[i].visible = false;

			// 지우개 색상을 부딪친 사각형 색으로 변경
			eraserR = rects[i].r;
			eraserG = rects[i].g;
			eraserB = rects[i].b;

			// 지우개 크기 증가
			eraserSize += RECT_SIZE * 0.25f;

			// 지우개 크기가 변했으므로 충돌 영역 다시 계산
			eraserHalf = eraserSize / 2.0f;

			eraserLeft = eraserX - eraserHalf;
			eraserRight = eraserX + eraserHalf;
			eraserTop = eraserY + eraserHalf;
			eraserBottom = eraserY - eraserHalf;
		}
	}
}

// 마우스 버튼
void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
)
{
	double mouseX;
	double mouseY;

	glfwGetCursorPos(
		window,
		&mouseX,
		&mouseY
	);

	// 마우스 좌표를 OpenGL 좌표로 변환
	float x{
		static_cast<float>(
			mouseX / WINDOW_WIDTH * 2.0 - 1.0
		)
	};

	float y{
		static_cast<float>(
			1.0 - mouseY / WINDOW_HEIGHT * 2.0
		)
	};

	// 왼쪽 마우스 버튼
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		// 누름
		if (action == GLFW_PRESS)
		{
			eraserActive = true;

			eraserX = x;
			eraserY = y;

			// 이전에 커졌던 크기는 사용하지 않는다.
			// 기본 지우개 크기로 다시 시작
			eraserSize = eraserBaseSize;

			// 다시 검정색으로 시작
			eraserR = 0.0f;
			eraserG = 0.0f;
			eraserB = 0.0f;

			CheckCollision();
		}

		// 뗌
		else if (action == GLFW_RELEASE)
		{
			eraserActive = false;

			// 사라졌던 모든 사각형 복구
			for (int i = 0; i < rectCount; ++i)
			{
				rects[i].visible = true;
			}

			// 지우개도 기본 상태로 초기화
			eraserSize = eraserBaseSize;

			eraserR = 0.0f;
			eraserG = 0.0f;
			eraserB = 0.0f;
		}
	}

	// 오른쪽 마우스 버튼
	if (button == GLFW_MOUSE_BUTTON_RIGHT &&
		action == GLFW_PRESS)
	{
		AddRect(x, y);
	}
}

// 마우스 이동
void CursorPositionCallback(
	GLFWwindow* window,
	double xpos,
	double ypos
)
{
	if (!eraserActive)
	{
		return;
	}

	// 마우스 좌표를 OpenGL 좌표로 변환
	eraserX = static_cast<float>(
		xpos / WINDOW_WIDTH * 2.0 - 1.0
		);

	eraserY = static_cast<float>(
		1.0 - ypos / WINDOW_HEIGHT * 2.0
		);

	// 이동하면서 충돌 검사
	CheckCollision();
}

// 키보드
void KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods
)
{
	if (action != GLFW_PRESS)
	{
		return;
	}

	// R : 전체 리셋
	if (key == GLFW_KEY_R)
	{
		InitRects();
	}
}