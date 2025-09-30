#include <iostream>
#include <random>
#include <gl/glew.h>											//--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

struct Square {
	float cx, cy;   // 현재 중심 좌표
	float size;     // 현재 크기
	float r, g, b;  // 색상

	// 초기 상태
	float base_cx, base_cy;
	float base_size;

	// 애니메이션
	float dx = 0.01f, dy = 0.01f; // 이동 속도
	int scaleDir = 1;             // 크기 변화 방향

	// 플래그
	bool moveDiag = false;
	bool moveZig = false;
	bool sizeAnim = false;
	bool colorAnim = false;
};

constexpr int MAX_SQ = 5;   // 최대 5개
Square s[MAX_SQ];
int s_i = 0;				// 현재 사각형 개수
int followIndex = -1;  // -1이면 따라하기 OFF, 0 이상이면 해당 사각형을 따라감

constexpr int TRAIL_LEN = 100; // 기록할 최대 프레임 수
float trailX[MAX_SQ][TRAIL_LEN];
float trailY[MAX_SQ][TRAIL_LEN];
int trailHead[MAX_SQ] = { 0 };   // 각 사각형 trail의 현재 위치

// 랜덤 엔진
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> urd01(0.0f, 1.0f);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
GLvoid draw_square(const Square& s);

void randomize_color(Square& s) {
	s.r = urd01(gen);
	s.g = urd01(gen);
	s.b = urd01(gen);
}

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정 
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
	glutInitWindowPosition(100, 100);							// 윈도우의 위치 지정
	glutInitWindowSize(800, 800);								// 윈도우의 크기 지정
	glutCreateWindow("Practice_00");								// 윈도우 생성(윈도우 이름)

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
	glutTimerFunc(30, Timer, 1);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene() {												//--- 콜백 함수: 출력 콜백 함수 
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);						// 바탕색을 ‘blue’로 지정
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.

	for (int i = 0; i < s_i; i++) {
		glColor3f(s[i].r, s[i].g, s[i].b);
		draw_square(s[i]);
	}

	glutSwapBuffers();											// 화면에 출력하기
}

GLvoid Reshape(int w, int h) {									//--- 콜백 함수: 다시 그리기 콜백 함수 
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1': // 대각선 이동 토글
		for (int i = 0; i < s_i; ++i) s[i].moveDiag = !s[i].moveDiag;
		break;
	case '2':
		for (int i = 0; i < s_i; i++) {
			s[i].moveZig = !s[i].moveZig;   // 지그재그 토글
			if (s[i].moveZig) s[i].dx = 0.05f; // 기본 속도 설정
		}
		break;
	case '3': // 크기 변화 토글
		for (int i = 0; i < s_i; ++i) s[i].sizeAnim = !s[i].sizeAnim;
		break;
	case '4': // 색상 변화 토글
		for (int i = 0; i < s_i; ++i) s[i].colorAnim = !s[i].colorAnim;
		break;
	case '5':
		if (followIndex != -1) {
			followIndex = -1; // 이미 따라중이면 멈춤
		}
		else {
			if (s_i > 1) {
				std::uniform_int_distribution<int> urdIdx(0, s_i - 1);
				followIndex = urdIdx(gen);
				std::cout << followIndex << "번 사각형을 따라하기 시작!\n";
			}
		}
		break;

	case 's':
		// TODO: 모든 애니메이션 멈춤
		break;
	case 'm':
		// TODO: 원래 위치로 복귀
		break;
	case 'r':
		s_i = 0; // 전부 삭제
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (s_i < MAX_SQ) {
			int win_w = glutGet(GLUT_WINDOW_WIDTH);
			int win_h = glutGet(GLUT_WINDOW_HEIGHT);

			float nx = (2.0f * x / win_w) - 1.0f;
			float ny = 1.0f - (2.0f * y / win_h);

			s[s_i].cx = nx;
			s[s_i].cy = ny;
			s[s_i].size = 0.2f; // 기본 크기
			randomize_color(s[s_i]);
			++s_i;
		}
	}
	glutPostRedisplay();
}

GLvoid Timer(int value)
{
	for (int i = 0; i < s_i; ++i) {
		Square& sq = s[i];

		// 1. 대각선 이동
		if (sq.moveDiag) {
			sq.cx += sq.dx;
			sq.cy += sq.dy;
			if (sq.cx + sq.size / 2 > 1.0f || sq.cx - sq.size / 2 < -1.0f) sq.dx *= -1;
			if (sq.cy + sq.size / 2 > 1.0f || sq.cy - sq.size / 2 < -1.0f) sq.dy *= -1;
		}

		// 2. 지그재그 이동
		if (sq.moveZig) {
			sq.cx += sq.dx; // 좌우 이동

			// 좌/우 벽 충돌 처리
			if (sq.cx + sq.size / 2 > 1.0f || sq.cx - sq.size / 2 < -1.0f) {
				sq.dx *= -1;           // 방향 반전
				sq.cy -= 0.05f;        // 살짝 아래로
			}

			// 바닥을 넘어가면 화면 위로 이동
			if (sq.cy - sq.size / 2 < -1.0f) {
				sq.cy = 1.0f + sq.size / 2;   // 맨 위로 위치 재설정
			}
		}

		// 3. 크기 변화
		if (sq.sizeAnim) {
			sq.size += 0.01f * sq.scaleDir;
			if (sq.size > 0.4f || sq.size < 0.1f) sq.scaleDir *= -1;
		}

		// 4. 색상 변화
		if (sq.colorAnim) {
			randomize_color(sq);
		}
		
		// 5. 따라하기
		if (followIndex != -1) {
			for (int i = followIndex + 1; i < s_i; i++) {
				int delay = (i - followIndex) * 10; // 사각형 번호 차이 * 지연
				int idx = (trailHead[i - 1] - delay + TRAIL_LEN) % TRAIL_LEN;
				s[i].cx = trailX[i - 1][idx];
				s[i].cy = trailY[i - 1][idx];
			}
		}

		for (int i = 0; i < s_i; i++) {
			trailX[i][trailHead[i]] = s[i].cx;
			trailY[i][trailHead[i]] = s[i].cy;
			trailHead[i] = (trailHead[i] + 1) % TRAIL_LEN; // 원형 버퍼 회전
		}

	}

	glutPostRedisplay();
	glutTimerFunc(30, Timer, 1);
}

GLvoid draw_square(const Square& s)
{
	float half = s.size * 0.5f;
	glRectf(s.cx - half, s.cy - half, s.cx + half, s.cy + half);
}