#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

constexpr int MAX_SHAPES = 10;

//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLvoid make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
static void RebuildBuffers();
static void rebuildFromShapes();
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);

//--- 필요한 변수 선언
GLint width, height;
GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

GLuint vao, vbo[2];
const GLfloat triShape[3][3] = { //--- 삼각형 위치 값
{ -0.5, -0.5, 0.0 }, { 0.5, -0.5, 0.0 }, { 0.0, 0.5, 0.0} };
const GLfloat colors[3][3] = { //--- 삼각형 꼭지점 색상
{ 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };

static std::vector<GLfloat> gPos;   // x,y,z 연속
static std::vector<GLfloat> gCol;   // r,g,b 연속
static GLsizei gVertexCount = 0;    // 현재 정점 수

enum DrawMode { M_POINT, M_LINE, M_TRI, M_RECT };
static DrawMode gMode = M_POINT;

struct Shape {
	// 어떤 형태의 도형인지 구분 (enum 내부 선언)
	enum Type { POINT_TRI, LINE_SEG, TRIANGLE, RECT } type;

	float cx, cy;       // 도형 중심좌표 (Normalized Device Coordinates)
	float size;         // 도형 크기 (한 변 또는 길이)
	float r, g, b;      // 색상값
};

static std::vector<Shape> gShapes;

// 선택된 도형의 인덱스 (-1이면 선택 없음)
static int gSelected = -1;

// 근접 선택 판정 임계치(화면 NDC 거리)
static const float PICK_RADIUS = 0.07f;

// 픽셀→NDC 변환 유틸은 이미 있음
static inline float d2(float x0, float y0, float x1, float y1) {
	float dx = x0 - x1, dy = y0 - y1;
	return dx * dx + dy * dy;
}

static inline float pix2ndcX(int x) { return  2.0f * x / width - 1.0f; }
static inline float pix2ndcY(int y) { return  1.0f - 2.0f * y / height; } // y 뒤집힘 주의

static void randColor(float& r, float& g, float& b)
{
	r = 0.3f + 0.7f * (float)rand() / RAND_MAX;
	g = 0.3f + 0.7f * (float)rand() / RAND_MAX;
	b = 0.3f + 0.7f * (float)rand() / RAND_MAX;
}

static inline void pushPos(float x, float y, float z = 0.0f) {
	gPos.push_back(x); gPos.push_back(y); gPos.push_back(z);
}
static inline void pushCol(float r, float g, float b) {
	gCol.push_back(r); gCol.push_back(g); gCol.push_back(b);
}

// 작은 정삼각형 (3정점)
static void emitPointTri(float cx, float cy, float size, float r, float g, float b)
{
	// 정삼각형 높이 = size * sqrt(3)/2 약0.866 * size
	const float h = size * 0.8660254f;
	const float half = size * 0.5f;

	// CCW 순서의 꼭짓점 3개
	const float tri[3][3] = {
		{ cx,        cy + (2.0f / 3.0f) * h, 0.0f }, // 위
		{ cx - half, cy - (1.0f / 3.0f) * h, 0.0f }, // 좌하
		{ cx + half, cy - (1.0f / 3.0f) * h, 0.0f }  // 우하
	};

	for (int i = 0;i < 3;i++) { pushPos(tri[i][0], tri[i][1], tri[i][2]); pushCol(r, g, b); }
	gVertexCount += 3;
}

static void emitLineBar(float cx, float cy, float size, float r, float g, float b, float thickness = 0.01f)
{
	float hx = size;          // half-width
	float hy = thickness * 0.5f; // half-height (세로 두께의 절반)

	// CCW 두 삼각형
	const float quad[6][3] = {
		{cx - hx, cy - hy, 0},{cx + hx, cy - hy, 0},{cx + hx, cy + hy, 0},
		{cx - hx, cy - hy, 0},{cx + hx, cy + hy, 0},{cx - hx, cy + hy, 0}
	};
	for (int i = 0;i < 6;i++) { pushPos(quad[i][0], quad[i][1], quad[i][2]); pushCol(r, g, b); }
	gVertexCount += 6;
}

static void emitTriangleAt(float cx, float cy, float size,
	float r, float g, float b)
{
	const float h = size * 0.8660254f; // √3/2
	const float half = size * 0.5f;

	const float tri[3][3] = {
		{ cx,        cy + (2.0f / 3.0f) * h, 0.0f }, // 위
		{ cx - half, cy - (1.0f / 3.0f) * h, 0.0f }, // 좌하
		{ cx + half, cy - (1.0f / 3.0f) * h, 0.0f }  // 우하
	};
	for (int i = 0;i < 3;i++) { pushPos(tri[i][0], tri[i][1], tri[i][2]); pushCol(r, g, b); }
	gVertexCount += 3;
}

static void emitRectAt(float cx, float cy, float size,
	float r, float g, float b)
{
	float hx = size * 0.6f;   // half-width
	float hy = size * 0.4f;   // half-height

	const float quad[6][3] = {
		{cx - hx, cy - hy, 0},{cx + hx, cy - hy, 0},{cx + hx, cy + hy, 0},
		{cx - hx, cy - hy, 0},{cx + hx, cy + hy, 0},{cx - hx, cy + hy, 0}
	};
	for (int i = 0;i < 6;i++) { pushPos(quad[i][0], quad[i][1], quad[i][2]); pushCol(r, g, b); }
	gVertexCount += 6;
}

static void addShapeAt(float nx, float ny)
{
	// 새 도형 기본 속성
	Shape s{};
	switch (gMode) {
	case M_POINT: s.type = Shape::POINT_TRI; break;
	case M_LINE:  s.type = Shape::LINE_SEG;  break;
	case M_TRI:   s.type = Shape::TRIANGLE;  break;
	case M_RECT:  s.type = Shape::RECT;      break;
	}
	s.cx = nx; s.cy = ny;
	s.size = 0.12f;                 // 기본 크기(모드별로 바꾸고 싶으면 분기 가능)
	if (s.type == Shape::POINT_TRI) s.size = 0.08f;
	if (s.type == Shape::RECT)      s.size = 0.12f;
	if (s.type == Shape::LINE_SEG)  s.size = 0.12f;

	randColor(s.r, s.g, s.b);

	// 최대 10개: 초과하면 가장 오래된 것 제거
	if ((int)gShapes.size() == MAX_SHAPES)
		gShapes.erase(gShapes.begin()); // 맨 앞 삭제(오래된 것)

	gShapes.push_back(s);
	gSelected = (int)gShapes.size() - 1; // 방금 추가한 걸 선택 상태로
}

static int pickNearestIndex(float nx, float ny)
{
	if (gShapes.empty()) return -1;

	int best = 0;
	float bestD = d2(nx, ny, gShapes[0].cx, gShapes[0].cy);
	for (int i = 1; i < (int)gShapes.size(); ++i) {
		float dd = d2(nx, ny, gShapes[i].cx, gShapes[i].cy);
		if (dd < bestD) { bestD = dd; best = i; }
	}
	// 너무 멀면 선택 안 함
	if (bestD > PICK_RADIUS * PICK_RADIUS) return -1;
	return best;
}

static void moveSelected(float dx, float dy)
{
	if (gSelected < 0 || gSelected >= (int)gShapes.size()) return;
	gShapes[gSelected].cx += dx;
	gShapes[gSelected].cy += dy;
	rebuildFromShapes();
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	width = 800;
	height = 800;
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("GLSL_2_8");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_shaderProgram();
	InitBuffer();

	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMainLoop();
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	vertexSource = filetobuf("vertex_8.glsl");

	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);

	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment_8.glsl");

	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);

	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLvoid make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기

	//-- shader Program
	shaderProgramID = glCreateProgram();

	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}

GLvoid InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(vao); //--- VAO를 바인드하기

	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기

	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	// 아직 데이터가 없으므로 크기 0의 빈 버퍼를 생성.
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	//--- 좌표값을 attribute 인덱스 0번에 명시한다: 버텍스 당 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//--- attribute 인덱스 0번을 사용가능하게 함
	glEnableVertexAttribArray(0);

	//--- 2번째 VBO를 활성화 하여 바인드 하고, 버텍스 속성 (색상)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	// 역시 빈 버퍼로 시작 (나중에 RebuildBuffers에서 실제 색상 데이터를 채움)
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	//--- 색상값을 attribute 인덱스 1번에 명시한다: 버텍스 당 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//--- attribute 인덱스 1번을 사용 가능하게 함.
	glEnableVertexAttribArray(1);

	gPos.reserve(3000); // 대략 정점 1000개 분량
	gCol.reserve(3000);
}

// CPU->GPU로 gPos/gCol을 밀어넣는 함수
static void RebuildBuffers()
{
	glBindVertexArray(vao);

	// Position VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, gPos.size() * sizeof(GLfloat), gPos.data(), GL_DYNAMIC_DRAW);

	// Color VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, gCol.size() * sizeof(GLfloat), gCol.data(), GL_DYNAMIC_DRAW);
}

static void rebuildFromShapes()
{
	gPos.clear(); gCol.clear();
	gVertexCount = 0;

	for (const auto& s : gShapes)
	{
		switch (s.type) {
		case Shape::POINT_TRI:
			emitPointTri(s.cx, s.cy, s.size, s.r, s.g, s.b);
			break;
		case Shape::LINE_SEG:
			emitLineBar(s.cx, s.cy, s.size, s.r, s.g, s.b);
			break;
		case Shape::TRIANGLE:
			emitTriangleAt(s.cx, s.cy, s.size, s.r, s.g, s.b);
			break;
		case Shape::RECT:
			emitRectAt(s.cx, s.cy, s.size, s.r, s.g, s.b);
			break;
		}
	}
	RebuildBuffers(); // CPU→GPU 전송 (STEP 3에서 만든 함수)
}

//--- 출력 콜백 함수
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	GLfloat rColor, gColor, bColor;

	rColor = bColor = gColor = 0.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(shaderProgramID);

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	//--- 삼각형 그리기
	glDrawArrays(GL_TRIANGLES, 0, gVertexCount);

	glutSwapBuffers(); //--- 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int, int)
{
	const float step = 0.03f;

	switch (key) {
		// 모드 전환
	case '1': gMode = M_POINT; break;
	case '2': gMode = M_LINE;  break;
	case '3': gMode = M_TRI;   break;
	case '4': gMode = M_RECT;  break;

		// 이동 (선택된 도형)
	case 'w': moveSelected(0.0f, +step); break;
	case 's': moveSelected(0.0f, -step); break;
	case 'a': moveSelected(-step, 0.0f); break;
	case 'd': moveSelected(+step, 0.0f); break;

	case 'j': moveSelected(-step, +step); break; // 좌상
	case 'i': moveSelected(+step, +step); break; // 우상
	case 'k': moveSelected(-step, -step); break; // 좌하
	case 'l': moveSelected(+step, -step); break; // 우하

		// 전체 삭제
	case 'c':
		gShapes.clear();
		gSelected = -1;
		gPos.clear(); gCol.clear(); gVertexCount = 0;
		RebuildBuffers(); // 빈 버퍼 업로드
		break;
	}

	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN) return;

	float nx = pix2ndcX(x);
	float ny = pix2ndcY(y);

	
	int idx = pickNearestIndex(nx, ny);
	
	if (idx != -1) {			// 1) 근접 도형이 있으면 선택 상태로 만들기
		gSelected = idx;
	}
	else {
		addShapeAt(nx, ny);		// 2) 없으면 새 도형 추가
	}

	// 3) 도형 목록 → 정점 버퍼 재생성
	rebuildFromShapes();
	glutPostRedisplay();
}