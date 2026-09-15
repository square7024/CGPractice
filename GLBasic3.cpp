#include <gl/glew.h>
#include <gl/glfw3.h>
#include <iostream>
#include <random>
#include <algorithm>

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
};

Rect rects[20];
int rectCount = 0;
int selectedRect = -1;
bool dragging = false;

float dragOffsetX = 0.0f;
float dragOffsetY = 0.0f;

//--- 랜덤 엔진
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_real_distribution<float> sizeDist(0.2f, 0.5f);

void DrawScene();
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
bool IsOverlap(const Rect& a, const Rect& b);
void MergeRect();
void CreateRandomRect();
void CreateRect();
void SplitRect(int index);

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
	//--- 마우스 이동 콜백 함수 등록
	glfwSetCursorPosCallback(window, CursorPosCallback);

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

void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < rectCount; i++) {

		Rect& rect = rects[i];

		glColor3f(rect.r, rect.g, rect.b);
		glRectf(rect.x1, rect.y1, rect.x2, rect.y2);
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);

		else if (key == GLFW_KEY_A)
			CreateRect();
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

		double xpos;
		double ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		float mouseX = (float)xpos / 400.0f - 1.0f;
		float mouseY = 1.0f - (float)ypos / 300.0f;

		//--- 가장 위에 있는 사각형부터 검사
		for (int i = rectCount - 1; i >= 0; i--) {

			Rect& rect = rects[i];

			if (mouseX >= rect.x1 && mouseX <= rect.x2 &&
				mouseY >= rect.y1 && mouseY <= rect.y2) {

				selectedRect = i;
				dragging = true;

				dragOffsetX = mouseX - rect.x1;
				dragOffsetY = mouseY - rect.y1;

				return;
			}
		}
	}

	//--- 왼쪽 마우스를 놓았을 때
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {

		if (dragging && selectedRect != -1) {
			//--- 다른 사각형과 겹쳤다면 합치기
			MergeRect();
		}

		dragging = false;
		selectedRect = -1;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

		double xpos;
		double ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		float mouseX = (float)xpos / 400.0f - 1.0f;
		float mouseY = 1.0f - (float)ypos / 300.0f;

		//--- 가장 위에 있는 사각형부터 검사
		for (int i = rectCount - 1; i >= 0; i--) {

			Rect& rect = rects[i];

			if (mouseX >= rect.x1 && mouseX <= rect.x2 &&
				mouseY >= rect.y1 && mouseY <= rect.y2) {

				SplitRect(i);
				return;
			}
		}
	}
}

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (!dragging || selectedRect == -1)
		return;

	float mouseX = (float)xpos / 400.0f - 1.0f;
	float mouseY = 1.0f - (float)ypos / 300.0f;

	Rect& rect = rects[selectedRect];

	float width = rect.x2 - rect.x1;
	float height = rect.y2 - rect.y1;

	rect.x1 = mouseX - dragOffsetX;
	rect.y1 = mouseY - dragOffsetY;

	rect.x2 = rect.x1 + width;
	rect.y2 = rect.y1 + height;
}

void CreateRect()
{
	if (rectCount >= 10)
		return;

	CreateRandomRect();
}

bool IsOverlap(const Rect& a, const Rect& b)
{
	if (a.x2 < b.x1 || a.x1 > b.x2)
		return false;

	if (a.y2 < b.y1 || a.y1 > b.y2)
		return false;

	return true;
}

void MergeRect()
{
	if (selectedRect == -1)
		return;

	for (int i = 0; i < rectCount; i++) {

		if (i == selectedRect)
			continue;

		if (IsOverlap(rects[selectedRect], rects[i])) {

			Rect& selected = rects[selectedRect];
			Rect& other = rects[i];

			//--- 두 사각형을 모두 포함하는 새로운 영역
			selected.x1 = std::min(selected.x1, other.x1);
			selected.y1 = std::min(selected.y1, other.y1);
			selected.x2 = std::max(selected.x2, other.x2);
			selected.y2 = std::max(selected.y2, other.y2);

			//--- 합쳐진 사각형의 색상을 랜덤하게 변경
			selected.r = colorDist(gen);
			selected.g = colorDist(gen);
			selected.b = colorDist(gen);

			//--- 겹친 다른 사각형 제거
			for (int j = i; j < rectCount - 1; j++) {
				rects[j] = rects[j + 1];
			}

			rectCount--;

			//--- 배열이 앞으로 당겨지면서 선택된 인덱스도 변하는 경우
			if (i < selectedRect)
				selectedRect--;

			return;
		}
	}
}

void CreateRandomRect()
{
	if (rectCount >= 20)
		return;

	float width = sizeDist(gen);
	float height = sizeDist(gen);

	std::uniform_real_distribution<float> xDist(
		-1.0f + width / 2.0f,
		1.0f - width / 2.0f
	);

	std::uniform_real_distribution<float> yDist(
		-1.0f + height / 2.0f,
		1.0f - height / 2.0f
	);

	float centerX = xDist(gen);
	float centerY = yDist(gen);

	Rect& rect = rects[rectCount];

	rect.x1 = centerX - width / 2.0f;
	rect.y1 = centerY - height / 2.0f;
	rect.x2 = centerX + width / 2.0f;
	rect.y2 = centerY + height / 2.0f;

	rect.r = colorDist(gen);
	rect.g = colorDist(gen);
	rect.b = colorDist(gen);

	rectCount++;
}

void SplitRect(int index)
{
	//--- 하나를 제거하고 두 개를 생성하므로
	//--- 최종 개수는 1개 증가
	if (rectCount >= 20)
		return;

	//--- 우클릭한 사각형 제거
	for (int i = index; i < rectCount - 1; i++) {
		rects[i] = rects[i + 1];
	}

	rectCount--;

	//--- 랜덤 사각형 2개 생성
	CreateRandomRect();
	CreateRandomRect();
}