#include <gl/glew.h>
#include <gl/glfw3.h>

#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_RECT_COUNT 10
#define MAX_ANIMATION_RECT_COUNT 100

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

struct AnimationRect
{
	float x;
	float y;

	float size;

	float r;
	float g;
	float b;

	float dx;
	float dy;

	bool brighten;
	bool active;
};

Rect rects[MAX_RECT_COUNT];
AnimationRect animationRects[MAX_ANIMATION_RECT_COUNT];

int rectCount{ 0 };

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_int_distribution<int> countDist(5, 10);
std::uniform_real_distribution<float> positionDist(-0.8f, 0.8f);
std::uniform_real_distribution<float> sizeDist(0.12f, 0.25f);
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

std::uniform_int_distribution<int> animationDist(1, 4);
std::uniform_int_distribution<int> directionDist(0, 3);
std::uniform_int_distribution<int> brightnessDist(0, 1);

const float MOVE_SPEED{ 0.35f };
const float SHRINK_SPEED{ 0.08f };
const float COLOR_SPEED{ 0.4f };
const float MIN_SIZE{ 0.01f };

void InitRects();

void DrawScene();
void DrawRect(float x, float y, float size, float r, float g, float b);

void SplitRect(int index);
void CreateAnimationRect(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b,
	float dx,
	float dy,
	bool brighten
);

void UpdateAnimation(float deltaTime);

void MouseButtonCallback(
	GLFWwindow* window,
	int button,
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
		"Practice 6",
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

	InitRects();

	double previousTime{ glfwGetTime() };

	while (!glfwWindowShouldClose(window))
	{
		double currentTime{ glfwGetTime() };
		float deltaTime{
			static_cast<float>(currentTime - previousTime)
		};

		previousTime = currentTime;

		UpdateAnimation(deltaTime);

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

// 처음 사각형 생성
void InitRects()
{
	rectCount = countDist(gen);

	for (int i = 0; i < rectCount; ++i)
	{
		rects[i].x = positionDist(gen);
		rects[i].y = positionDist(gen);

		rects[i].size = sizeDist(gen);

		rects[i].r = colorDist(gen);
		rects[i].g = colorDist(gen);
		rects[i].b = colorDist(gen);

		rects[i].visible = true;
	}

	for (int i = 0; i < MAX_ANIMATION_RECT_COUNT; ++i)
	{
		animationRects[i].active = false;
	}
}

// 화면 그리기
void DrawScene()
{
	// 처음 생성된 사각형
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

	// 사등분 후 움직이는 사각형
	for (int i = 0; i < MAX_ANIMATION_RECT_COUNT; ++i)
	{
		if (!animationRects[i].active)
		{
			continue;
		}

		DrawRect(
			animationRects[i].x,
			animationRects[i].y,
			animationRects[i].size,
			animationRects[i].r,
			animationRects[i].g,
			animationRects[i].b
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

// 사각형 클릭 후 분할
void SplitRect(int index)
{
	Rect& rect{ rects[index] };

	rect.visible = false;

	int animationType{ animationDist(gen) };
	//int animationType{ 1 };
	//int animationType{ 2 };
	//int animationType{ 3 };
	//int animationType{ 4 };

	bool brighten{
		brightnessDist(gen) == 1
	};

	float childSize{ rect.size / 2.0f };
	float offset{ rect.size / 4.0f };

	float diagonalSpeed{
		MOVE_SPEED / std::sqrt(2.0f)
	};

	// 1. 좌우상하 이동
	if (animationType == 1)
	{
		CreateAnimationRect(
			rect.x - offset,
			rect.y - offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			-MOVE_SPEED,
			0.0f,
			brighten
		);

		CreateAnimationRect(
			rect.x + offset,
			rect.y + offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			MOVE_SPEED,
			0.0f,
			brighten
		);

		CreateAnimationRect(
			rect.x - offset,
			rect.y + offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			0.0f,
			MOVE_SPEED,
			brighten
		);

		CreateAnimationRect(
			rect.x + offset,
			rect.y - offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			0.0f,
			-MOVE_SPEED,
			brighten
		);
	}

	// 2. 대각선 이동
	else if (animationType == 2)
	{
		CreateAnimationRect(
			rect.x - offset,
			rect.y + offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			-diagonalSpeed,
			diagonalSpeed,
			brighten
		);

		CreateAnimationRect(
			rect.x + offset,
			rect.y + offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			diagonalSpeed,
			diagonalSpeed,
			brighten
		);

		CreateAnimationRect(
			rect.x - offset,
			rect.y - offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			-diagonalSpeed,
			-diagonalSpeed,
			brighten
		);

		CreateAnimationRect(
			rect.x + offset,
			rect.y - offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			diagonalSpeed,
			-diagonalSpeed,
			brighten
		);
	}

	// 3. 네 사각형이 같은 방향으로 이동
	else if (animationType == 3)
	{
		int direction{ directionDist(gen) };

		float dx{ 0.0f };
		float dy{ 0.0f };

		if (direction == 0)
		{
			dx = -MOVE_SPEED;
		}
		else if (direction == 1)
		{
			dx = MOVE_SPEED;
		}
		else if (direction == 2)
		{
			dy = MOVE_SPEED;
		}
		else
		{
			dy = -MOVE_SPEED;
		}

		CreateAnimationRect(
			rect.x - offset,
			rect.y + offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			dx,
			dy,
			brighten
		);

		CreateAnimationRect(
			rect.x + offset,
			rect.y + offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			dx,
			dy,
			brighten
		);

		CreateAnimationRect(
			rect.x - offset,
			rect.y - offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			dx,
			dy,
			brighten
		);

		CreateAnimationRect(
			rect.x + offset,
			rect.y - offset,
			childSize,
			rect.r,
			rect.g,
			rect.b,
			dx,
			dy,
			brighten
		);
	}

	// 4. 8방향 이동
	else if (animationType == 4)
	{
		float smallSize{ rect.size * 0.4f };

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			-MOVE_SPEED,
			0.0f,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			MOVE_SPEED,
			0.0f,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			0.0f,
			MOVE_SPEED,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			0.0f,
			-MOVE_SPEED,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			-diagonalSpeed,
			diagonalSpeed,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			diagonalSpeed,
			diagonalSpeed,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			-diagonalSpeed,
			-diagonalSpeed,
			brighten
		);

		CreateAnimationRect(
			rect.x,
			rect.y,
			smallSize,
			rect.r,
			rect.g,
			rect.b,
			diagonalSpeed,
			-diagonalSpeed,
			brighten
		);
	}
}

// 움직이는 사각형 하나 생성
void CreateAnimationRect(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b,
	float dx,
	float dy,
	bool brighten
)
{
	for (int i = 0; i < MAX_ANIMATION_RECT_COUNT; ++i)
	{
		if (animationRects[i].active)
		{
			continue;
		}

		animationRects[i].x = x;
		animationRects[i].y = y;

		animationRects[i].size = size;

		animationRects[i].r = r;
		animationRects[i].g = g;
		animationRects[i].b = b;

		animationRects[i].dx = dx;
		animationRects[i].dy = dy;

		animationRects[i].brighten = brighten;
		animationRects[i].active = true;

		return;
	}
}

// 위치, 크기, 색상 애니메이션
void UpdateAnimation(float deltaTime)
{
	for (int i = 0; i < MAX_ANIMATION_RECT_COUNT; ++i)
	{
		if (!animationRects[i].active)
		{
			continue;
		}

		animationRects[i].x +=
			animationRects[i].dx * deltaTime;

		animationRects[i].y +=
			animationRects[i].dy * deltaTime;

		animationRects[i].size -=
			SHRINK_SPEED * deltaTime;

		if (animationRects[i].brighten)
		{
			animationRects[i].r +=
				COLOR_SPEED * deltaTime;

			animationRects[i].g +=
				COLOR_SPEED * deltaTime;

			animationRects[i].b +=
				COLOR_SPEED * deltaTime;

			animationRects[i].r =
				std::min(animationRects[i].r, 1.0f);

			animationRects[i].g =
				std::min(animationRects[i].g, 1.0f);

			animationRects[i].b =
				std::min(animationRects[i].b, 1.0f);
		}
		else
		{
			animationRects[i].r -=
				COLOR_SPEED * deltaTime;

			animationRects[i].g -=
				COLOR_SPEED * deltaTime;

			animationRects[i].b -=
				COLOR_SPEED * deltaTime;

			animationRects[i].r =
				std::max(animationRects[i].r, 0.0f);

			animationRects[i].g =
				std::max(animationRects[i].g, 0.0f);

			animationRects[i].b =
				std::max(animationRects[i].b, 0.0f);
		}

		// 일정 크기보다 작아지면 완전히 사라진다.
		if (animationRects[i].size <= MIN_SIZE)
		{
			animationRects[i].active = false;
		}
	}
}

// 마우스 클릭
void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT ||
		action != GLFW_PRESS)
	{
		return;
	}

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

	for (int i = 0; i < rectCount; ++i)
	{
		if (!rects[i].visible)
		{
			continue;
		}

		float half{
			rects[i].size / 2.0f
		};

		bool inside{
			x >= rects[i].x - half &&
			x <= rects[i].x + half &&
			y >= rects[i].y - half &&
			y <= rects[i].y + half
		};

		if (inside)
		{
			SplitRect(i);
			break;
		}
	}
}