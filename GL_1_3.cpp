#include <iostream>
#include <random>
#include <gl/glew.h>											//--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

struct Square {
	float x1, y1, x2, y2;
	float r, g, b;
	float scale{ 0.1f };
};

constexpr int MAX_SQ = 30;
Square s[MAX_SQ];
int s_i = 0;

int draggingIndex = -1;   // 현재 드래그 중인 사각형 인덱스 (-1이면 없음)
float dragOffsetX = 0.0f; // 마우스 클릭 위치와 사각형 좌측 하단의 차이
float dragOffsetY = 0.0f;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> urd(0.0f, 1.0f);
std::uniform_real_distribution<float> urd_xy(-1.0f, 0.9f);

void randomize_color(Square& s) {
	s.r = urd(gen);
	s.g = urd(gen);
	s.b = urd(gen);
}

void randomize_xy(Square& s) {
	s.x1 = urd_xy(gen);
	s.y1 = urd_xy(gen);
	s.x2 = s.x1 + s.scale;
	s.y2 = s.y1 + s.scale;
}

bool isInside(const Square& sq, float x, float y) {
	return (x >= sq.x1 && x <= sq.x2 && y >= sq.y1 && y <= sq.y2);
}

bool isOverlap(const Square& a, const Square& b) {
	return !(a.x2 < b.x1 || a.x1 > b.x2 ||
		a.y2 < b.y1 || a.y1 > b.y2);
}

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid draw_square(Square& s);
GLvoid Motion(int x, int y);
Square mergeSquares(const Square& a, const Square& b);

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정 
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
	glutInitWindowPosition(100, 100);							// 윈도우의 위치 지정
	glutInitWindowSize(800, 800);								// 윈도우의 크기 지정
	glutCreateWindow("Practice_03");								// 윈도우 생성(윈도우 이름)

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
	glutMotionFunc(Motion);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene() {												//--- 콜백 함수: 출력 콜백 함수 
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);						// 바탕색을 ‘blue’로 지정
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	
	for (int i = 0; i < s_i; ++i) {
		glColor3f(s[i].r, s[i].g, s[i].b);
		draw_square(s[i]);
	}

	glutSwapBuffers();											// 화면에 출력하기
}

GLvoid Reshape(int w, int h) {									//--- 콜백 함수: 다시 그리기 콜백 함수 
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a' :
		if (s_i < 10) {
			randomize_color(s[s_i]);
			randomize_xy(s[s_i]);
			++s_i;
		}
		else {
			std::cout << "사각형은 10개 까지 생성 가능" << std::endl;
		}
		break;
	case 'q' :
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

	// 왼쪽 버튼 클릭 (드래그 시작)
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (int i = s_i - 1; i >= 0; --i) {
			if (isInside(s[i], nx, ny)) {
				draggingIndex = i;
				dragOffsetX = nx - s[i].x1;
				dragOffsetY = ny - s[i].y1;
				break;
			}
		}
	}

	// 왼쪽 버튼 놓기 (드래그 종료 & 합치기 검사)
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (draggingIndex != -1) {
			for (int i = 0; i < s_i; ++i) {
				if (i == draggingIndex) continue;
				if (isOverlap(s[draggingIndex], s[i])) {
					s[draggingIndex] = mergeSquares(s[draggingIndex], s[i]);
					for (int j = i; j < s_i - 1; ++j) {
						s[j] = s[j + 1];
					}
					--s_i;
					break;
				}
			}
		}
		draggingIndex = -1;
	}

	// 오른쪽 버튼 클릭 (분리)
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		for (int i = s_i - 1; i >= 0; --i) {
			if (isInside(s[i], nx, ny)) {
				if (s_i + 1 < MAX_SQ) {
					Square orig = s[i];

					float L = orig.x2 - orig.x1;   // 원래 변 길이
					float L2 = L * 0.5f;            // 새 변 길이
					float cx = (orig.x1 + orig.x2) * 0.5f;
					float cy = (orig.y1 + orig.y2) * 0.5f;

					// 왼쪽 사각형
					Square left{};
					left.scale = L2;
					left.x1 = cx - L2; left.x2 = cx;
					left.y1 = cy - (L2 * 0.5f); left.y2 = cy + (L2 * 0.5f);
					randomize_color(left);

					// 오른쪽 사각형
					Square right{};
					right.scale = L2;
					right.x1 = cx; right.x2 = cx + L2;
					right.y1 = left.y1; right.y2 = left.y2;
					randomize_color(right);

					// 기존 자리에 왼쪽, 배열 맨 뒤에 오른쪽 추가
					s[i] = left;
					s[s_i++] = right;
				}
				break;
			}
		}
	}

	glutPostRedisplay();
}


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

GLvoid draw_square(Square &s) {
	glRectf(s.x1, s.y1, s.x2, s.y2);
}

Square mergeSquares(const Square& a, const Square& b) {
	Square merged;
	merged.x1 = std::min(a.x1, b.x1);
	merged.y1 = std::min(a.y1, b.y1);
	merged.x2 = std::max(a.x2, b.x2);
	merged.y2 = std::max(a.y2, b.y2);
	randomize_color(merged);
	return merged;
}