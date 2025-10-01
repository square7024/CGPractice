#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

// --- 사용자 정의 함수 원형
void   make_vertexShaders();
void   make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);

// --- 전역 상태
GLint  width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO = 0, VBO = 0;           // 정점 배열/버퍼
GLint  uColorLoc = -1;             // 색상 uniform 위치

// --- 도형 정의
struct Shape {
    enum Type { POINT_QUAD, LINE_SEG, TRIANGLE, RECT } type;
    float cx, cy;     // 중심 좌표(NDC)
    float size;       // 기본 크기
    float r, g, b;    // 색
    int   first;      // VBO 내 시작 정점 인덱스
    int   count;      // 정점 개수
    GLenum prim;      // GL_LINES / GL_TRIANGLES
};
constexpr int MAX_SHAPES = 10;
static Shape gShapes[MAX_SHAPES];
static int   gCount = 0;
static int   gSelected = -1;

// 실시간 정점 풀(모든 도형의 정점(x,y,z)들)
static std::vector<float> gVerts;

// 현재 추가 모드
enum DrawMode { M_POINT, M_LINE, M_TRI, M_RECT };
static DrawMode gMode = M_POINT;

// --- 파일에서 텍스트 읽기 (네가 준 그대로)
char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr) return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}

// --- 수학/유틸
static inline float pix2ndcX(int x) { return  2.0f * x / width - 1.0f; }
static inline float pix2ndcY(int y) { return  1.0f - 2.0f * y / height; }
static inline float d2(float x0, float y0, float x1, float y1) { float dx = x0 - x1, dy = y0 - y1; return dx * dx + dy * dy; }

static void RebuildVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(gVerts.size() * sizeof(float)), gVerts.data(), GL_DYNAMIC_DRAW);
}

static void EmitPointQuad(float cx, float cy, float sz, int& first, int& count)
{
    float hx = sz * 0.5f, hy = sz * 0.5f;
    const float quad[6][3] = {
        {cx - hx, cy - hy, 0}, {cx + hx, cy - hy, 0}, {cx + hx, cy + hy, 0},
        {cx - hx, cy - hy, 0}, {cx + hx, cy + hy, 0}, {cx - hx, cy + hy, 0}
    };
    first = (int)(gVerts.size() / 3);
    for (auto& v : quad) gVerts.insert(gVerts.end(), v, v + 3);
    count = 6;
}
static void EmitLine(float x0, float y0, float x1, float y1, int& first, int& count)
{
    const float seg[2][3] = { {x0,y0,0}, {x1,y1,0} };
    first = (int)(gVerts.size() / 3);
    for (auto& v : seg) gVerts.insert(gVerts.end(), v, v + 3);
    count = 2;
}
static void EmitTriangle(float cx, float cy, float sz, int& first, int& count)
{
    float h = sz;
    float x0 = cx, y0 = cy + h * 0.57735f;   // 정삼각형 위 꼭짓점
    float x1 = cx - h * 0.5f, y1 = cy - h * 0.288675f;
    float x2 = cx + h * 0.5f, y2 = cy - h * 0.288675f;
    const float tri[3][3] = { {x0,y0,0},{x1,y1,0},{x2,y2,0} };
    first = (int)(gVerts.size() / 3);
    for (auto& v : tri) gVerts.insert(gVerts.end(), v, v + 3);
    count = 3;
}
static void EmitRect(float cx, float cy, float sz, int& first, int& count)
{
    float hx = sz * 0.6f, hy = sz * 0.4f;
    const float quad[6][3] = {
        {cx - hx, cy - hy, 0},{cx + hx, cy - hy, 0},{cx + hx, cy + hy, 0},
        {cx - hx, cy - hy, 0},{cx + hx, cy + hy, 0},{cx - hx, cy + hy, 0}
    };
    first = (int)(gVerts.size() / 3);
    for (auto& v : quad) gVerts.insert(gVerts.end(), v, v + 3);
    count = 6;
}

static void RebuildAllVertices()
{
    gVerts.clear();
    for (int i = 0;i < gCount;++i) {
        Shape& s = gShapes[i];
        s.first = s.count = 0;
        switch (s.type) {
        case Shape::POINT_QUAD: EmitPointQuad(s.cx, s.cy, s.size, s.first, s.count); s.prim = GL_TRIANGLES; break;
        case Shape::LINE_SEG:   EmitLine(s.cx - s.size, s.cy, s.cx + s.size, s.cy, s.first, s.count); s.prim = GL_LINES; break;
        case Shape::TRIANGLE:   EmitTriangle(s.cx, s.cy, s.size, s.first, s.count);  s.prim = GL_TRIANGLES; break;
        case Shape::RECT:       EmitRect(s.cx, s.cy, s.size, s.first, s.count);      s.prim = GL_TRIANGLES; break;
        }
    }
    RebuildVBO();
}

static void AddShape(float nx, float ny)
{
    if (gCount == MAX_SHAPES) {
        // 가장 오래된 것 제거(앞으로 당김)
        for (int i = 1;i < MAX_SHAPES;++i) gShapes[i - 1] = gShapes[i];
        gCount = MAX_SHAPES - 1;
    }
    Shape s{};
    switch (gMode) {
    case M_POINT: s.type = Shape::POINT_QUAD; break;
    case M_LINE:  s.type = Shape::LINE_SEG;   break;
    case M_TRI:   s.type = Shape::TRIANGLE;   break;
    case M_RECT:  s.type = Shape::RECT;       break;
    }
    s.cx = nx; s.cy = ny; s.size = 0.08f;

    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_real_distribution<float> U(0.3f, 1.0f);
    s.r = U(rng); s.g = U(rng); s.b = U(rng);

    gShapes[gCount++] = s;
    gSelected = gCount - 1;
    RebuildAllVertices();
}

static void PickNearest(float nx, float ny)
{
    if (gCount == 0) { gSelected = -1; return; }
    int bi = 0; float bd = d2(nx, ny, gShapes[0].cx, gShapes[0].cy);
    for (int i = 1;i < gCount;++i) {
        float dd = d2(nx, ny, gShapes[i].cx, gShapes[i].cy);
        if (dd < bd) { bd = dd; bi = i; }
    }
    gSelected = bi;
}

static void MoveSelected(float dx, float dy)
{
    if (gSelected < 0 || gSelected >= gCount) return;
    gShapes[gSelected].cx += dx;
    gShapes[gSelected].cy += dy;
    RebuildAllVertices();
}

// --- 메인
void main(int argc, char** argv)
{
    width = 800;
    height = 600;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("GLSL 실습 8");

    // GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // 셰이더
    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();

    // --- VAO/VBO 준비 (정점 속성: 위치만 사용, location=0)
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 uniform 위치
    uColorLoc = glGetUniformLocation(shaderProgramID, "uColor");

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);

    // 콜백
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);

    glutMainLoop();
}

// --- 버텍스 셰이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource = (GLchar*)filetobuf("vertex8.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result; GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(vertexSource); // 파일 버퍼 해제
}

// --- 프래그먼트 셰이더 객체 만들기
void make_fragmentShaders()
{
    GLchar* fragmentSource = (GLchar*)filetobuf("fragment8.glsl");
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result; GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: frag shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(fragmentSource); // 파일 버퍼 해제
}

// --- 세이더 프로그램 만들기 & 링크
GLuint make_shaderProgram()
{
    GLint result;
    GLchar errorLog[512];
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 링크 실패\n" << errorLog << std::endl;
        return 0;
    }
    glUseProgram(shaderID);
    return shaderID;
}

// --- 출력 콜백
GLvoid drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    for (int i = 0;i < gCount;++i) {
        const Shape& s = gShapes[i];
        float r = s.r, g = s.g, b = s.b;
        if (i == gSelected) { // 선택 강조 살짝 밝게
            r = std::min(1.f, r * 0.6f + 0.4f);
            g = std::min(1.f, g * 0.6f + 0.4f);
            b = std::min(1.f, b * 0.6f + 0.4f);
        }
        glUniform4f(uColorLoc, r, g, b, 1.0f);
        glDrawArrays(s.prim, s.first, s.count);
    }
    glutSwapBuffers();
}

// --- 다시 그리기 콜백
GLvoid Reshape(int w, int h)
{
    width = w; height = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

// --- 키보드 콜백
GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        // 모드 전환
    case '1': gMode = M_POINT; break;
    case '2': gMode = M_LINE;  break;
    case '3': gMode = M_TRI;   break;
    case '4': gMode = M_RECT;  break;

        // 모든 도형 삭제
    case 'c':
        gCount = 0; gSelected = -1;
        gVerts.clear();
        RebuildVBO();
        break;

        // 이동 (선택된 도형)
    case 'w': MoveSelected(0.0f, +0.03f); break;
    case 's': MoveSelected(0.0f, -0.03f); break;
    case 'a': MoveSelected(-0.03f, 0.0f); break;
    case 'd': MoveSelected(+0.03f, 0.0f); break;
    case 'i': MoveSelected(-0.03f, +0.03f); break; // 좌상
    case 'o': MoveSelected(+0.03f, +0.03f); break; // 우상
    case 'k': MoveSelected(-0.03f, -0.03f); break; // 좌하
    case 'l': MoveSelected(+0.03f, -0.03f); break; // 우하
    }
    glutPostRedisplay();
}

// --- 마우스 콜백
GLvoid Mouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN) return;
    float nx = pix2ndcX(x), ny = pix2ndcY(y);

    if (gCount > 0) {
        int prevSel = gSelected;
        PickNearest(nx, ny);
        float nearD = d2(nx, ny, gShapes[gSelected].cx, gShapes[gSelected].cy);
        // 충분히 멀면 새 도형 추가(클릭으로 만들기)
        if (nearD > 0.05f * 0.05f) {
            gSelected = prevSel;
            AddShape(nx, ny);
        }
    }
    else {
        AddShape(nx, ny);
    }
    glutPostRedisplay();
}