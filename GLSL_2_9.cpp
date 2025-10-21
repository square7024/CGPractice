#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

constexpr int MAX_PER_QUAD = 4;

// 삼각형 1개의 정보
struct Tri {
	float cx, cy;    // 중심 (NDC)
	float size;      // 크기(한 변 길이 기준 스케일)
	float r, g, b;   // 색
};

//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLvoid make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
static void RebuildBuffers();
static void rebuildAllFromQuadrants();
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);

//--- 필요한 변수 선언
GLint width, height;
GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

GLuint vao, vbo[2];
//const GLfloat triShape[3][3] = { //--- 삼각형 위치 값
//{ -0.5, -0.5, 0.0 }, { 0.5, -0.5, 0.0 }, { 0.0, 0.5, 0.0} };
//const GLfloat colors[3][3] = { //--- 삼각형 꼭지점 색상
//{ 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };

std::vector<GLfloat> gPos;   // x,y,z…
std::vector<GLfloat> gCol;   // r,g,b…
GLsizei gVertexCount;        // 총 정점 수

enum QuadrantIdx { Q1 = 0, Q2 = 1, Q3 = 2, Q4 = 3 };

static std::vector<Tri> gTris[4];   // gTris[0]~[3]

static bool gWireframe = false; // false면 면(FILL), true면 선(LINE)

// 편의 푸시
static inline void pushPos(float x, float y, float z = 0.0f) {
	gPos.push_back(x); gPos.push_back(y); gPos.push_back(z);
}
static inline void pushCol(float r, float g, float b) {
	gCol.push_back(r); gCol.push_back(g); gCol.push_back(b);
}

static inline float pix2ndcX(int x) { return  2.0f * x / width - 1.0f; }
static inline float pix2ndcY(int y) { return  1.0f - 2.0f * y / height; } // 위쪽이 +Y

static inline int quadrantOf(float x, float y)
{
	if (x >= 0.f && y > 0.f) return Q1; // 1사분면
	if (x < 0.f && y >= 0.f) return Q2; // 2사분면
	if (x <= 0.f && y < 0.f) return Q3; // 3사분면
	if (x > 0.f && y <= 0.f) return Q4; // 4사분면
	return -1;
}

static void randColor(float& r, float& g, float& b) {
	r = 0.3f + 0.7f * (float)rand() / RAND_MAX;
	g = 0.3f + 0.7f * (float)rand() / RAND_MAX;
	b = 0.3f + 0.7f * (float)rand() / RAND_MAX;
}

// 보기 좋은 크기(분면 기준)에서 무작위로 살짝 변주
static float randSize(float base = 0.18f) {
	float k = 0.75f + 0.5f * (float)rand() / RAND_MAX; // 0.75~1.25배
	return base * k;
}

// 삼각형 1개(정삼각형) → 정점 3개 push
static void emitFilledTriangle(float cx, float cy, float size, float r, float g, float b)
{
	const float h = size * 0.8660254f; // √3/2
	const float half = size * 0.5f;
	const float v[3][3] = {
		{ cx,        cy + (2.0f / 3.0f) * h, 0.0f },
		{ cx - half, cy - (1.0f / 3.0f) * h, 0.0f },
		{ cx + half, cy - (1.0f / 3.0f) * h, 0.0f }
	};
	for (int i = 0;i < 3;i++) { pushPos(v[i][0], v[i][1], v[i][2]); pushCol(r, g, b); }
	gVertexCount += 3;
}

static inline Tri makeTriAt(float nx, float ny) {
	Tri t;
	t.cx = nx; t.cy = ny;
	t.size = randSize();
	randColor(t.r, t.g, t.b);
	return t;
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
	glutCreateWindow("GLSL_2_9");

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

	Tri t{ 0.5f, 0.5f, randSize(), 0.2f, 0.9f, 0.3f };
	gTris[Q1].push_back(t);
	rebuildAllFromQuadrants();

	gPos.reserve(3000);
	gCol.reserve(3000);
}

// CPU→GPU 업로드
static void RebuildBuffers()
{
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // position
	glBufferData(GL_ARRAY_BUFFER, gPos.size() * sizeof(GLfloat), gPos.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // color
	glBufferData(GL_ARRAY_BUFFER, gCol.size() * sizeof(GLfloat), gCol.data(), GL_DYNAMIC_DRAW);
}

// 분면 컨테이너 전체 → gPos/gCol로 직렬화 후 업로드
static void rebuildAllFromQuadrants()
{
	gPos.clear(); gCol.clear();
	gVertexCount = 0;

	for (int q = 0; q < 4; ++q) {
		for (const Tri& t : gTris[q]) {
			emitFilledTriangle(t.cx, t.cy, t.size, t.r, t.g, t.b);
		}
	}
	RebuildBuffers();
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

	// 면/선 모드
	glPolygonMode(GL_FRONT_AND_BACK, gWireframe ? GL_LINE : GL_FILL);

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
	switch (key)
	{
	case 'a': // 면으로
	case 'A':
		gWireframe = false;
		break;

	case 'b': // 선으로
	case 'B':
		gWireframe = true;
		break;

	case 'c': // 전체 삭제
	case 'C':
		for (int q = 0; q < 4; ++q) gTris[q].clear();
		gPos.clear(); gCol.clear(); gVertexCount = 0;
		RebuildBuffers(); // 빈 버퍼 업로드
		break;

	case 'q': // 종료
	case 'Q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN) return;

	float nx = pix2ndcX(x);
	float ny = pix2ndcY(y);
	int q = quadrantOf(nx, ny);
	if (q < 0) return; // 축 위면 무시

	if (button == GLUT_LEFT_BUTTON) {
		// 왼쪽 클릭: 해당 분면 "교체" (기존 것 지우고 1개만)
		gTris[q].clear();
		gTris[q].push_back(makeTriAt(nx, ny));
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		// 오른쪽 클릭: 해당 분면 "추가" (최대 MAX_PER_QUAD)
		if ((int)gTris[q].size() == MAX_PER_QUAD) {
			// 정책 1) 가장 오래된 것 삭제 후 추가 (최근 것 우선)
			gTris[q].erase(gTris[q].begin());
		}
		gTris[q].push_back(makeTriAt(nx, ny));
	}

	rebuildAllFromQuadrants(); // CPU→GPU 재업로드
	glutPostRedisplay();
}