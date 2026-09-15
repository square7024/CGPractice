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
};

Rect rects[5];
int rectCount = { 0 };

//--- 랜덤 엔진
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_real_distribution<float> sizeDist(0.1f, 0.3f);

void DrawScene();
void CreateRect(int area);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

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
}

void CreateRect(int area)
{

}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);

		else if (key == GLFW_KEY_Q)
			glfwSetWindowShouldClose(window, true);
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
	}
}