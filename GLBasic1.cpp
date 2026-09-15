#include <gl/glew.h>
#include <gl/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

void InputProcess(GLFWwindow* window);
void DrawScene();

//--- 배경색
float rColor = 1.0f;
float gColor = 1.0f;
float bColor = 1.0f;

//--- 타이머
bool timerOn = false;
double lastTime = 0.0;

int main()
{
	srand((unsigned int)time(NULL));

	//--- GLFW 초기화
	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	//--- OpenGL 버전 설정 (예: 3.3 Core Profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
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

	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		// 키보드 입력 처리
		InputProcess(window);

		// 화면 지우기 (흰색)
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

//--- 키보드 입력 처리 함수
void InputProcess(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		rColor = 0.0f;
		gColor = 1.0f;
		bColor = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		rColor = 1.0f;
		gColor = 0.0f;
		bColor = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		rColor = 1.0f;
		gColor = 1.0f;
		bColor = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		rColor = (float)rand() / RAND_MAX;
		gColor = (float)rand() / RAND_MAX;
		bColor = (float)rand() / RAND_MAX;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		rColor = 0.4f;
		gColor = 0.4f;
		bColor = 0.4f;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		rColor = 0.0f;
		gColor = 0.0f;
		bColor = 0.0f;
	}

	//--- 타이머 시작
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		timerOn = true;
		lastTime = glfwGetTime();
	}

	//--- 타이머 종료
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		timerOn = false;
	}

	//--- 타이머가 실행 중이면 1초마다 랜덤 색상으로 변경
	if (timerOn) {
		double currentTime = glfwGetTime();

		if (currentTime - lastTime >= 1.0) {
			rColor = (float)rand() / RAND_MAX;
			gColor = (float)rand() / RAND_MAX;
			bColor = (float)rand() / RAND_MAX;

			lastTime = currentTime;
		}
	}
}

//--- 렌더링 함수
void DrawScene()
{
	glClearColor(rColor, gColor, bColor, 1.0f); // RGBA (흰색)
	glClear(GL_COLOR_BUFFER_BIT);
}