#include <iostream>
#include <random>
#include <gl/glew.h>											//--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

constexpr float SQUARE_SIZE = 0.05f;
constexpr int MAX_SQ = 20;

struct Square {
	float cx, cy;
	float r, g, b;
	float size{ SQUARE_SIZE };
};

Square s[MAX_SQ];
int sq_index = 0;				// 현재 사각형 개수

Square eraser = { 0, 0,    0.0f, 0.0f, 0.0f,    SQUARE_SIZE * 2.0f };

bool erasing = false;   // 지우개 플래그

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> color_urd(0.0f, 1.0f);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
void draw_square(Square& s);

void randomize_color(Square& s) {
	s.r = color_urd(gen);
	s.g = color_urd(gen);
	s.b = color_urd(gen);
}

void randomize_xy(Square& s) {
	std::uniform_real_distribution<float> sq_urd(-1.0f + SQUARE_SIZE, 1.0f - SQUARE_SIZE);
	s.cx = sq_urd(gen);
	s.cy = sq_urd(gen);
}

void init_squares();

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정 
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
	glutInitWindowPosition(100, 100);							// 윈도우의 위치 지정
	glutInitWindowSize(800, 800);								// 윈도우의 크기 지정
	glutCreateWindow("Practice_05");								// 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {								// glew 초기화 
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene);									// 출력 함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	//glutMotionFunc(Motion);
	init_squares();
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene() {												//--- 콜백 함수: 출력 콜백 함수 
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);						// 바탕색을 ‘blue’로 지정
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.

	for (int i = 0; i < sq_index; ++i) {
		glColor3f(s[i].r, s[i].g, s[i].b);
		draw_square(s[i]);
	}

	if (erasing) {
		glColor3f(eraser.r, eraser.g, eraser.b);
		draw_square(eraser);
	}

	glutSwapBuffers();											// 화면에 출력하기
}

GLvoid Reshape(int w, int h) {									//--- 콜백 함수: 다시 그리기 콜백 함수 
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		init_squares();
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y) {
	int win_h = glutGet(GLUT_WINDOW_HEIGHT);
	int win_w = glutGet(GLUT_WINDOW_WIDTH);
	float nx = (2.0f * x / win_w) - 1.0f;
	float ny = 1.0f - (2.0f * y / win_h);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		eraser.cx = nx;
		eraser.cy = ny;
		eraser.size = SQUARE_SIZE * 2.0f;
		erasing = true;
		eraser.r = eraser.g = eraser.b = 0.f;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		erasing = false;
	}

	glutPostRedisplay();
}
/*
GLvoid Motion(int x, int y) {
	if (draggingIndex == -1) return;

	int win_w = glutGet(GLUT_WINDOW_WIDTH);
	int win_h = glutGet(GLUT_WINDOW_HEIGHT);
	float nx = (2.0f * x / win_w) - 1.0f;
	float ny = 1.0f - (2.0f * y / win_h);

	Square& sq = s[draggingIndex];
	float w = sq.x2 - sq.x1;
	float h = sq.y2 - sq.y1;

	// 마우스 좌표 - offset = 새 좌측 하단
	sq.x1 = nx - dragOffsetX;
	sq.y1 = ny - dragOffsetY;
	sq.x2 = sq.x1 + w;
	sq.y2 = sq.y1 + h;

	glutPostRedisplay();
}
*/

void init_squares() {
	sq_index = 0;
	for (int i = 0; i < MAX_SQ && sq_index < MAX_SQ; ++i) {
		randomize_color(s[sq_index]);
		randomize_xy(s[sq_index]);
		++sq_index;
	}
}

void draw_square(Square& s) {
	glRectf(s.cx - (s.size / 2), s.cy - (s.size / 2), s.cx + (s.size / 2), s.cy + (s.size / 2));
}
