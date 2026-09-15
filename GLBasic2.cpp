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

	bool selected;
};

//--- 4개 영역의 배경색
float areaColor[4][3];

Rect rects[4][5];
int rectCount[4] = { 0, 0, 0, 0 };

//--- 랜덤 엔진
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_real_distribution<float> sizeDist(0.1f, 0.3f);

void SetRandomAreaColor();
void DrawScene();
void CreateRect(int area);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ResizeSelectedRect(float amount);
void ChangeSelectedRectColor();
void Reset();

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

	//--- 랜덤 색상 설정
	SetRandomAreaColor();

	//--- 뷰포트 설정
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
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

void SetRandomAreaColor()
{
	for (int i = 0; i < 4; i++) {
		areaColor[i][0] = colorDist(gen);
		areaColor[i][1] = colorDist(gen);
		areaColor[i][2] = colorDist(gen);
	}
}

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//--- 왼쪽 위
	glColor3f(areaColor[0][0], areaColor[0][1], areaColor[0][2]);
	glRectf(-1.0f, 0.0f, 0.0f, 1.0f);

	//--- 오른쪽 위
	glColor3f(areaColor[1][0], areaColor[1][1], areaColor[1][2]);
	glRectf(0.0f, 0.0f, 1.0f, 1.0f);

	//--- 왼쪽 아래
	glColor3f(areaColor[2][0], areaColor[2][1], areaColor[2][2]);
	glRectf(-1.0f, -1.0f, 0.0f, 0.0f);

	//--- 오른쪽 아래
	glColor3f(areaColor[3][0], areaColor[3][1], areaColor[3][2]);
	glRectf(0.0f, -1.0f, 1.0f, 0.0f);

	//--- 생성된 정사각형 그리기
	for (int area = 0; area < 4; area++) {
		for (int i = 0; i < rectCount[area]; i++) {

			Rect& rect = rects[area][i];

			//--- 정사각형 내부
			glColor3f(rect.r, rect.g, rect.b);
			glRectf(rect.x1, rect.y1, rect.x2, rect.y2);

			//--- 선택된 정사각형이면 테두리 표시
			if (rect.selected) {
				glColor3f(1.0f, 1.0f, 1.0f);

				glBegin(GL_LINE_LOOP);
				glLineWidth(3.0f);
				glVertex2f(rect.x1, rect.y1);
				glVertex2f(rect.x2, rect.y1);
				glVertex2f(rect.x2, rect.y2);
				glVertex2f(rect.x1, rect.y2);
				glEnd();
			}
		}
	}
}

void CreateRect(int area)
{
	if (rectCount[area] >= 5)
		return;

	float size = sizeDist(gen);

	float minX, maxX;
	float minY, maxY;

	if (area == 0) {
		minX = -1.0f;
		maxX = 0.0f;
		minY = 0.0f;
		maxY = 1.0f;
	}
	else if (area == 1) {
		minX = 0.0f;
		maxX = 1.0f;
		minY = 0.0f;
		maxY = 1.0f;
	}
	else if (area == 2) {
		minX = -1.0f;
		maxX = 0.0f;
		minY = -1.0f;
		maxY = 0.0f;
	}
	else {
		minX = 0.0f;
		maxX = 1.0f;
		minY = -1.0f;
		maxY = 0.0f;
	}

	//--- 정사각형이 사분면 밖으로 나가지 않도록 중심점 범위 제한
	std::uniform_real_distribution<float> xDist(
		minX + size / 2.0f,
		maxX - size / 2.0f
	);

	std::uniform_real_distribution<float> yDist(
		minY + size / 2.0f,
		maxY - size / 2.0f
	);

	float centerX = xDist(gen);
	float centerY = yDist(gen);

	Rect& rect = rects[area][rectCount[area]];

	rect.x1 = centerX - size / 2.0f;
	rect.y1 = centerY - size / 2.0f;
	rect.x2 = centerX + size / 2.0f;
	rect.y2 = centerY + size / 2.0f;

	rect.r = colorDist(gen);
	rect.g = colorDist(gen);
	rect.b = colorDist(gen);

	rect.selected = false;

	rectCount[area]++;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);

		else if (key == GLFW_KEY_Q)
			glfwSetWindowShouldClose(window, true);

		else if (key == GLFW_KEY_1)
			CreateRect(0);

		else if (key == GLFW_KEY_2)
			CreateRect(1);

		else if (key == GLFW_KEY_3)
			CreateRect(2);

		else if (key == GLFW_KEY_4)
			CreateRect(3);

		//--- 선택된 정사각형 크기 증가
		else if (key == GLFW_KEY_EQUAL)
			ResizeSelectedRect(0.01f);

		//--- 선택된 정사각형 크기 감소
		else if (key == GLFW_KEY_MINUS)
			ResizeSelectedRect(-0.01f);

		//--- 선택된 정사각형 색상 변경
		else if (key == GLFW_KEY_C)
			ChangeSelectedRectColor();

		else if (key == GLFW_KEY_R)
			Reset();
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

		double xpos;
		double ypos;

		glfwGetCursorPos(window, &xpos, &ypos);

		//--- 마우스 좌표를 OpenGL 좌표로 변환
		float mouseX = (float)xpos / 400.0f - 1.0f;
		float mouseY = 1.0f - (float)ypos / 300.0f;

		//--- 기존 선택 해제
		for (int area = 0; area < 4; area++) {
			for (int i = 0; i < rectCount[area]; i++) {
				rects[area][i].selected = false;
			}
		}

		//--- 가장 최근에 생성된 사각형부터 검사
		for (int area = 0; area < 4; area++) {
			for (int i = rectCount[area] - 1; i >= 0; i--) {

				Rect& rect = rects[area][i];

				if (mouseX >= rect.x1 && mouseX <= rect.x2 &&
					mouseY >= rect.y1 && mouseY <= rect.y2) {

					rect.selected = true;
					return;
				}
			}
		}
	}
}

void ResizeSelectedRect(float amount)
{
	for (int area = 0; area < 4; area++) {
		for (int i = 0; i < rectCount[area]; i++) {

			Rect& rect = rects[area][i];

			if (rect.selected) {

				float newX1 = rect.x1 - amount;
				float newY1 = rect.y1 - amount;
				float newX2 = rect.x2 + amount;
				float newY2 = rect.y2 + amount;

				//--- 최소 크기 제한
				if (newX2 - newX1 < 0.02f)
					return;

				//--- 사분면 경계 설정
				float minX, maxX;
				float minY, maxY;

				if (area == 0) {
					minX = -1.0f;
					maxX = 0.0f;
					minY = 0.0f;
					maxY = 1.0f;
				}
				else if (area == 1) {
					minX = 0.0f;
					maxX = 1.0f;
					minY = 0.0f;
					maxY = 1.0f;
				}
				else if (area == 2) {
					minX = -1.0f;
					maxX = 0.0f;
					minY = -1.0f;
					maxY = 0.0f;
				}
				else {
					minX = 0.0f;
					maxX = 1.0f;
					minY = -1.0f;
					maxY = 0.0f;
				}

				//--- 사분면을 벗어나면 크기 변경하지 않음
				if (newX1 < minX || newX2 > maxX ||
					newY1 < minY || newY2 > maxY)
					return;

				//--- 새로운 크기 적용
				rect.x1 = newX1;
				rect.y1 = newY1;
				rect.x2 = newX2;
				rect.y2 = newY2;

				return;
			}
		}
	}
}

void ChangeSelectedRectColor()
{
	for (int area = 0; area < 4; area++) {
		for (int i = 0; i < rectCount[area]; i++) {

			Rect& rect = rects[area][i];

			if (rect.selected) {
				rect.r = colorDist(gen);
				rect.g = colorDist(gen);
				rect.b = colorDist(gen);

				return;
			}
		}
	}
}

void Reset()
{
	//--- 모든 정사각형 제거
	for (int area = 0; area < 4; area++) {
		rectCount[area] = 0;
	}

	//--- 4개 영역 배경색 다시 설정
	SetRandomAreaColor();
}