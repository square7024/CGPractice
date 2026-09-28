#include <gl/glew.h>
#include <gl/glfw3.h>
#include <iostream>
#include <random>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct Rect
{
	float x1;
	float y1;
	float x2;
	float y2;

	float r;
	float g;
	float b;

	//--- 이동 방향
	float dx;
	float dy;

	// 0 : 위쪽, 1 : 오른쪽, 2 : 아래쪽, 3 : 왼쪽
	int edge;

	//--- 크기 변화 방향
	float sizeDir;

	//--- 처음 생성한 위치
	float originX;
	float originY;
};

Rect rects[5];
int rectCount = { 0 };

bool move1 = { false };
bool move2 = { false };
bool move3 = { false };
bool sizeAnimation = { false };
bool colorAnimation = { false };
bool moveOriginal = { false };

double lastColorTime = 0.0;

//--- 랜덤 엔진
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_int_distribution<int> directionDist(0, 1);

void DrawScene();
void CreateRect(float x, float y);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MoveRect1();																		// 위치 변화 1
void MoveRect2();																		// 위치 변화 2
void MoveRect3();																		// 위치 변화 3
void ChangeSize();																		// 크기 변화
void ChangeColor();																		// 색상 변화
void MoveOriginal();																	// 원래 위치로 이동

int main()
{
	//--- GLFW 초기화
	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	//--- OpenGL 버전 설정 (예: 3.3 Core Profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Window", nullptr, nullptr);
	if (!window) {
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	//--- 컨텍스트 설정
	glfwMakeContextCurrent(window);

	//--- GLEW 초기화
	glewExperimental = GL_TRUE; // 최신 기능 사용
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		return -1;
	}

	//--- 키보드 콜백 함수 등록
	glfwSetKeyCallback(window, KeyCallback);
	//--- 마우스 버튼 콜백 함수 등록
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	//--- 뷰포트 설정
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {

		//--- 위치 변화 1
		if (move1) {
			MoveRect1();
		}

		//--- 위치 변화 2
		if (move2) {
			MoveRect2();
		}

		//--- 위치 변화 3
		if (move3) {
			MoveRect3();
		}

		//--- 원래 위치로 이동
		if (moveOriginal) {
			MoveOriginal();
		}

		//--- 크기 변화
		if (sizeAnimation) {
			ChangeSize();
		}

		//--- 색상 변화
		if (colorAnimation) {
			ChangeColor();
		}

		// 화면 그리기
		DrawScene();

		// 버퍼 교체
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//--- 종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void DrawScene()
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < rectCount; ++i) {

		glColor3f(rects[i].r, rects[i].g, rects[i].b);

		glBegin(GL_QUADS);

		glVertex2f(rects[i].x1, rects[i].y1);
		glVertex2f(rects[i].x2, rects[i].y1);
		glVertex2f(rects[i].x2, rects[i].y2);
		glVertex2f(rects[i].x1, rects[i].y2);

		glEnd();
	}
}

void CreateRect(float x, float y)
{
	//--- 최대 5개까지만 생성
	if (rectCount >= 5)
		return;

	//--- 모든 정사각형의 크기를 동일하게 설정
	float size = 0.15f;

	Rect& rect = rects[rectCount];

	//--- 마우스 위치를 중심으로 정사각형 생성
	rect.x1 = x - size / 2.0f;
	rect.y1 = y - size / 2.0f;
	rect.x2 = x + size / 2.0f;
	rect.y2 = y + size / 2.0f;

	//--- 랜덤 색상
	rect.r = colorDist(gen);
	rect.g = colorDist(gen);
	rect.b = colorDist(gen);

	//--- 랜덤 대각선 이동 방향
	float speed = 0.005f;

	if (directionDist(gen) == 0)
		rect.dx = speed;
	else
		rect.dx = -speed;

	if (directionDist(gen) == 0)
		rect.dy = speed;
	else
		rect.dy = -speed;

	rect.edge = 0;

	//--- 처음에는 커지는 방향
	rect.sizeDir = 0.001f;

	//--- 처음 생성한 위치 저장
	rect.originX = x;
	rect.originY = y;

	rectCount++;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);

		else if (key == GLFW_KEY_Q)
			glfwSetWindowShouldClose(window, true);

		//--- 위치 변화 1 시작 / 정지
		else if (key == GLFW_KEY_1) {
			move1 = !move1;
			move2 = false;
			move3 = false;
			moveOriginal = false;
		}

		//--- 위치 변화 2 시작 / 정지
		else if (key == GLFW_KEY_2) {
			move2 = !move2;
			move1 = false;
			move3 = false;
			moveOriginal = false;
		}

		//--- 위치 변화 3 시작 / 정지
		else if (key == GLFW_KEY_3) {

			move3 = !move3;

			move1 = false;
			move2 = false;
			moveOriginal = false;

			if (move3) {

				for (int i = 0; i < rectCount; ++i) {

					float width = rects[i].x2 - rects[i].x1;
					float height = rects[i].y2 - rects[i].y1;

					//--- 위쪽 가장자리에 순서대로 배치
					rects[i].x1 = -1.0f + i * 0.25f;
					rects[i].x2 = rects[i].x1 + width;

					rects[i].y2 = 1.0f;
					rects[i].y1 = rects[i].y2 - height;

					rects[i].edge = 0;
				}
			}
		}

		//--- 크기 변화 시작 / 정지
		else if (key == GLFW_KEY_4) {
			sizeAnimation = !sizeAnimation;
		}

		//--- 색상 변화 시작 / 정지
		else if (key == GLFW_KEY_5) {
			colorAnimation = !colorAnimation;
		}

		//--- 모든 애니메이션 정지
		else if (key == GLFW_KEY_S) {
			move1 = false;
			move2 = false;
			move3 = false;
			sizeAnimation = false;
			colorAnimation = false;
			moveOriginal = false;
		}

		//--- 원래 위치로 이동
		else if (key == GLFW_KEY_M) {

			if (rectCount > 0) {

				move1 = false;
				move2 = false;
				move3 = false;

				moveOriginal = true;
			}
		}

		//--- 사각형 삭제 및 초기화
		else if (key == GLFW_KEY_R) {

			//--- 모든 애니메이션 정지
			move1 = false;
			move2 = false;
			move3 = false;
			sizeAnimation = false;
			colorAnimation = false;
			moveOriginal = false;

			//--- 모든 사각형 삭제
			rectCount = 0;
		}
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

		double xpos;
		double ypos;

		glfwGetCursorPos(window, &xpos, &ypos);

		//--- 마우스 좌표를 OpenGL 좌표로 변환
		float mouseX = (float)xpos / float(WINDOW_WIDTH / 2) - 1.0f;
		float mouseY = 1.0f - (float)ypos / float(WINDOW_HEIGHT / 2);

		CreateRect(mouseX, mouseY);
	}
}

void MoveRect1()
{
	for (int i = 0; i < rectCount; ++i) {

		//--- 사각형 이동
		rects[i].x1 += rects[i].dx;
		rects[i].x2 += rects[i].dx;

		rects[i].y1 += rects[i].dy;
		rects[i].y2 += rects[i].dy;

		//--- 좌우 벽 충돌
		if (rects[i].x1 <= -1.0f ||
			rects[i].x2 >= 1.0f) {

			rects[i].dx = -rects[i].dx;
		}

		//--- 위아래 벽 충돌
		if (rects[i].y1 <= -1.0f ||
			rects[i].y2 >= 1.0f) {

			rects[i].dy = -rects[i].dy;
		}
	}
}

void MoveRect2()
{
	for (int i = 0; i < rectCount; ++i) {

		//--- 가로 방향으로 이동
		rects[i].x1 += rects[i].dx;
		rects[i].x2 += rects[i].dx;

		//--- 오른쪽 벽에 닿으면
		if (rects[i].x2 >= 1.0f) {

			float width = rects[i].x2 - rects[i].x1;

			rects[i].x2 = 1.0f;
			rects[i].x1 = rects[i].x2 - width;

			rects[i].dx = -0.005f;

			rects[i].y1 -= 0.1f;
			rects[i].y2 -= 0.1f;
		}

		//--- 왼쪽 벽에 닿으면
		else if (rects[i].x1 <= -1.0f) {

			float width = rects[i].x2 - rects[i].x1;

			rects[i].x1 = -1.0f;
			rects[i].x2 = rects[i].x1 + width;

			rects[i].dx = 0.005f;

			rects[i].y1 -= 0.1f;
			rects[i].y2 -= 0.1f;
		}

		//--- 아래쪽을 벗어나면 위쪽으로 이동
		if (rects[i].y1 <= -1.0f) {

			float height = rects[i].y2 - rects[i].y1;

			rects[i].y2 = 1.0f;
			rects[i].y1 = rects[i].y2 - height;
		}
	}
}

void MoveRect3()
{
	float speed = 0.005f;

	for (int i = 0; i < rectCount; ++i) {

		float width = rects[i].x2 - rects[i].x1;
		float height = rects[i].y2 - rects[i].y1;

		//--- 위쪽 : 오른쪽으로 이동
		if (rects[i].edge == 0) {

			rects[i].x1 += speed;
			rects[i].x2 += speed;

			//--- 오른쪽 위 모서리에 도착
			if (rects[i].x2 >= 1.0f) {

				rects[i].x2 = 1.0f;
				rects[i].x1 = rects[i].x2 - width;

				rects[i].edge = 1;
			}
		}

		//--- 오른쪽 : 아래쪽으로 이동
		else if (rects[i].edge == 1) {

			rects[i].y1 -= speed;
			rects[i].y2 -= speed;

			//--- 오른쪽 아래 모서리에 도착
			if (rects[i].y1 <= -1.0f) {

				rects[i].y1 = -1.0f;
				rects[i].y2 = rects[i].y1 + height;

				rects[i].edge = 2;
			}
		}

		//--- 아래쪽 : 왼쪽으로 이동
		else if (rects[i].edge == 2) {

			rects[i].x1 -= speed;
			rects[i].x2 -= speed;

			//--- 왼쪽 아래 모서리에 도착
			if (rects[i].x1 <= -1.0f) {

				rects[i].x1 = -1.0f;
				rects[i].x2 = rects[i].x1 + width;

				rects[i].edge = 3;
			}
		}

		//--- 왼쪽 : 위쪽으로 이동
		else if (rects[i].edge == 3) {

			rects[i].y1 += speed;
			rects[i].y2 += speed;

			//--- 왼쪽 위 모서리에 도착
			if (rects[i].y2 >= 1.0f) {

				rects[i].y2 = 1.0f;
				rects[i].y1 = rects[i].y2 - height;

				rects[i].edge = 0;
			}
		}
	}
}

void ChangeSize()
{
	for (int i = 0; i < rectCount; ++i) {

		//--- 사각형의 중심점
		float centerX = (rects[i].x1 + rects[i].x2) / 2.0f;
		float centerY = (rects[i].y1 + rects[i].y2) / 2.0f;

		//--- 현재 크기
		float width = rects[i].x2 - rects[i].x1;

		//--- 크기 변경
		width += rects[i].sizeDir;

		//--- 최대 크기에 도달하면 작아지기
		if (width >= 0.3f) {
			width = 0.3f;
			rects[i].sizeDir = -0.001f;
		}

		//--- 최소 크기에 도달하면 커지기
		else if (width <= 0.1f) {
			width = 0.1f;
			rects[i].sizeDir = 0.001f;
		}

		//--- 중심점을 기준으로 크기 변경
		rects[i].x1 = centerX - width / 2.0f;
		rects[i].x2 = centerX + width / 2.0f;
		rects[i].y1 = centerY - width / 2.0f;
		rects[i].y2 = centerY + width / 2.0f;

		//--- 위치 변화 3 실행 중이면 가장자리에 맞추기
		if (move3) {

			//--- 위쪽
			if (rects[i].edge == 0) {
				rects[i].y2 = 1.0f;
				rects[i].y1 = 1.0f - width;
			}

			//--- 오른쪽
			else if (rects[i].edge == 1) {
				rects[i].x2 = 1.0f;
				rects[i].x1 = 1.0f - width;
			}

			//--- 아래쪽
			else if (rects[i].edge == 2) {
				rects[i].y1 = -1.0f;
				rects[i].y2 = -1.0f + width;
			}

			//--- 왼쪽
			else if (rects[i].edge == 3) {
				rects[i].x1 = -1.0f;
				rects[i].x2 = -1.0f + width;
			}
		}
	}
}

void ChangeColor()
{
	double currentTime = glfwGetTime();

	//--- 0.2초마다 색상 변경
	if (currentTime - lastColorTime < 0.2)
		return;

	for (int i = 0; i < rectCount; ++i) {

		rects[i].r = colorDist(gen);
		rects[i].g = colorDist(gen);
		rects[i].b = colorDist(gen);
	}

	lastColorTime = currentTime;
}

void MoveOriginal()
{
	float speed = 0.005f;
	bool allArrived = true;

	for (int i = 0; i < rectCount; ++i) {

		//--- 현재 중심점
		float centerX = (rects[i].x1 + rects[i].x2) / 2.0f;
		float centerY = (rects[i].y1 + rects[i].y2) / 2.0f;

		float moveX = 0.0f;
		float moveY = 0.0f;

		//--- X축 이동
		if (centerX < rects[i].originX - speed) {
			moveX = speed;
			allArrived = false;
		}
		else if (centerX > rects[i].originX + speed) {
			moveX = -speed;
			allArrived = false;
		}
		else {
			moveX = rects[i].originX - centerX;
		}

		//--- Y축 이동
		if (centerY < rects[i].originY - speed) {
			moveY = speed;
			allArrived = false;
		}
		else if (centerY > rects[i].originY + speed) {
			moveY = -speed;
			allArrived = false;
		}
		else {
			moveY = rects[i].originY - centerY;
		}

		//--- 사각형 이동
		rects[i].x1 += moveX;
		rects[i].x2 += moveX;
		rects[i].y1 += moveY;
		rects[i].y2 += moveY;
	}

	//--- 모든 사각형이 원래 위치에 도착
	if (allArrived) {
		moveOriginal = false;
	}
}