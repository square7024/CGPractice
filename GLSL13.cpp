
#include <gl/glew.h>
#include <gl/glfw3.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>
#include <random>
#include <cstddef>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

//--- 셰이더
GLuint vertexShader = 0;
GLuint fragmentShader = 0;
GLuint shaderProgramID = 0;

//--- 버퍼
GLuint VAO = 0;
GLuint VBO = 0;

//--- 모델링 변환 행렬 위치
GLint modelLocation = -1;

//--- 각 면의 출력 여부
bool cubeFaces[6] = { false };
bool pyramidFaces[5] = { false };

//--- 랜덤 엔진
std::random_device rd;
std::mt19937 gen(rd());

//--- 함수 프로토타입
std::string filetobuf(const char* file);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

void InitBuffer();

void DrawScene();
void DrawVertices(const std::vector<Vertex>& vertices, GLenum mode);
void DrawAxis();
void DrawCube();
void DrawPyramid();

void KeyCallback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods
);

//----------------
int main()
//----------------
{
    //--- GLFW 초기화
    if (!glfwInit())
    {
        std::cerr << "GLFW 초기화 실패!" << std::endl;
        return -1;
    }

    //--- OpenGL 버전 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //--- 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Practice 13",
        nullptr,
        nullptr
    );

    if (!window)
    {
        std::cerr << "윈도우 생성 실패!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    //--- GLEW 초기화
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패!" << std::endl;

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    //--- 깊이 검사 활성화
    glEnable(GL_DEPTH_TEST);

    //--- 셰이더 생성
    make_vertexShaders();
    make_fragmentShaders();

    shaderProgramID = make_shaderProgram();

    if (shaderProgramID == 0)
    {
        if (vertexShader)
            glDeleteShader(vertexShader);

        if (fragmentShader)
            glDeleteShader(fragmentShader);

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    modelLocation = glGetUniformLocation(
        shaderProgramID,
        "modelTransform"
    );

    //--- 버퍼 생성
    InitBuffer();

    //--- 키보드 콜백
    glfwSetKeyCallback(window, KeyCallback);

    //--- 렌더링 루프
    while (!glfwWindowShouldClose(window))
    {
        DrawScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //--- 메모리 해제
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgramID);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

//--------------------------------------------------
// 셰이더 파일 읽기
//--------------------------------------------------
std::string filetobuf(const char* file)
{
    std::ifstream shaderFile(file);

    if (!shaderFile.is_open())
    {
        std::cerr << file
            << " 파일을 열 수 없습니다."
            << std::endl;
        return "";
    }

    std::string source(
        (std::istreambuf_iterator<char>(shaderFile)),
        std::istreambuf_iterator<char>()
    );

    shaderFile.close();

    return source;
}

//--------------------------------------------------
// 버텍스 셰이더
//--------------------------------------------------
void make_vertexShaders()
{
    std::string vertexSource =
        filetobuf("GLSL13vertex.glsl");

    if (vertexSource.empty())
        return;

    const char* source = vertexSource.c_str();

    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &source, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);

    if (!result)
    {
        glGetShaderInfoLog(
            vertexShader,
            512,
            NULL,
            errorLog
        );

        std::cerr
            << "ERROR: vertex shader 컴파일 실패\n"
            << errorLog
            << std::endl;

        glDeleteShader(vertexShader);
        vertexShader = 0;
    }
}

//--------------------------------------------------
// 프래그먼트 셰이더
//--------------------------------------------------
void make_fragmentShaders()
{
    std::string fragmentSource =
        filetobuf("GLSL13fragment.glsl");

    if (fragmentSource.empty())
        return;

    const char* source = fragmentSource.c_str();

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &source, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);

    if (!result)
    {
        glGetShaderInfoLog(
            fragmentShader,
            512,
            NULL,
            errorLog
        );

        std::cerr
            << "ERROR: fragment shader 컴파일 실패\n"
            << errorLog
            << std::endl;

        glDeleteShader(fragmentShader);
        fragmentShader = 0;
    }
}

//--------------------------------------------------
// 셰이더 프로그램
//--------------------------------------------------
GLuint make_shaderProgram()
{
    if (vertexShader == 0 || fragmentShader == 0)
        return 0;

    GLuint shaderID = glCreateProgram();

    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    GLint result;
    GLchar errorLog[512];

    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);

    if (!result)
    {
        glGetProgramInfoLog(
            shaderID,
            512,
            NULL,
            errorLog
        );

        std::cerr
            << "ERROR: shader program 연결 실패\n"
            << errorLog
            << std::endl;

        glDeleteProgram(shaderID);
        return 0;
    }

    return shaderID;
}

//--------------------------------------------------
// VAO, VBO 생성
//--------------------------------------------------
void InitBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //--- 정점 좌표
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );
    glEnableVertexAttribArray(0);

    //--- 정점 색상
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, color)
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

//--------------------------------------------------
// 정점 데이터 출력
//--------------------------------------------------
void DrawVertices(
    const std::vector<Vertex>& vertices,
    GLenum mode
)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(
        mode,
        0,
        (GLsizei)vertices.size()
    );

    glBindVertexArray(0);
}

//--------------------------------------------------
// 화면 좌표축 그리기
//--------------------------------------------------
void DrawAxis()
{
    glm::mat4 model(1.0f);

    //--- x축 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- y축 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    std::vector<Vertex> axes = {
        //--- x축: 빨간색
        {{-0.9f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.9f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},

        //--- y축: 초록색
        {{0.0f, -0.9f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f,  0.9f, 0.0f}, {0.0f, 1.0f, 0.0f}},

        //--- z축: 파란색
        {{0.0f, 0.0f, -0.9f}, {0.0f, 0.0f, 1.0f}},
        {{0.0f, 0.0f,  0.9f}, {0.0f, 0.0f, 1.0f}}
    };

    DrawVertices(axes, GL_LINES);
}

//--------------------------------------------------
// 육면체 그리기
//--------------------------------------------------
void DrawCube()
{
    const float s = 0.30f;

    //--- 육면체의 8개 꼭짓점
    glm::vec3 p[8] = {
        {-s, -s, -s},
        { s, -s, -s},
        { s,  s, -s},
        {-s,  s, -s},
        {-s, -s,  s},
        { s, -s,  s},
        { s,  s,  s},
        {-s,  s,  s}
    };

    //--- 각 면을 구성하는 꼭짓점
    int faces[6][4] = {
        {4, 5, 6, 7}, // 앞면
        {1, 0, 3, 2}, // 뒷면
        {0, 4, 7, 3}, // 왼쪽 면
        {5, 1, 2, 6}, // 오른쪽 면
        {3, 7, 6, 2}, // 윗면
        {0, 1, 5, 4}  // 아랫면
    };

    //--- 각 면의 색상
    glm::vec3 colors[6] = {
        {1.0f, 0.0f, 0.0f}, // 빨강
        {1.0f, 1.0f, 0.0f}, // 노랑
        {0.0f, 0.0f, 1.0f}, // 파랑
        {0.0f, 1.0f, 0.0f}, // 초록
        {0.0f, 1.0f, 1.0f}, // 청록
        {1.0f, 0.0f, 1.0f}  // 자홍
    };

    //--- 모델링 변환
    glm::mat4 model(1.0f);

    //--- 화면 왼쪽으로 이동
    model = glm::translate(
        model,
        glm::vec3(-0.45f, 0.0f, 0.0f)
    );

    //--- x축 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- y축 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    //--- 선택된 면만 출력
    for (int i = 0; i < 6; i++)
    {
        if (!cubeFaces[i])
            continue;

        int* f = faces[i];
        glm::vec3 c = colors[i];

        //--- 사각형을 삼각형 2개로 구성
        std::vector<Vertex> vertices = {
            {p[f[0]], c},
            {p[f[1]], c},
            {p[f[2]], c},

            {p[f[0]], c},
            {p[f[2]], c},
            {p[f[3]], c}
        };

        DrawVertices(vertices, GL_TRIANGLES);
    }
}

//--------------------------------------------------
// 사각뿔 그리기
//--------------------------------------------------
void DrawPyramid()
{
    const float s = 0.30f;

    //--- 사각뿔의 5개 꼭짓점
    glm::vec3 p[5] = {
        {-s, -s, -s},
        { s, -s, -s},
        { s, -s,  s},
        {-s, -s,  s},
        {0.0f, s, 0.0f}
    };

    //--- 삼각형 옆면
    int faces[4][3] = {
        {0, 1, 4},
        {1, 2, 4},
        {2, 3, 4},
        {3, 0, 4}
    };

    //--- 각 면의 색상
    glm::vec3 colors[5] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f}
    };

    //--- 모델링 변환
    glm::mat4 model(1.0f);

    //--- 화면 오른쪽으로 이동
    model = glm::translate(
        model,
        glm::vec3(0.45f, 0.0f, 0.0f)
    );

    //--- x축 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- y축 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    //--- 삼각형 옆면 그리기
    for (int i = 0; i < 4; i++)
    {
        if (!pyramidFaces[i])
            continue;

        std::vector<Vertex> vertices = {
            {p[faces[i][0]], colors[i]},
            {p[faces[i][1]], colors[i]},
            {p[faces[i][2]], colors[i]}
        };

        DrawVertices(vertices, GL_TRIANGLES);
    }

    //--- 바닥면 그리기
    if (pyramidFaces[4])
    {
        std::vector<Vertex> vertices = {
            {p[0], colors[4]},
            {p[3], colors[4]},
            {p[2], colors[4]},

            {p[0], colors[4]},
            {p[2], colors[4]},
            {p[1], colors[4]}
        };

        DrawVertices(vertices, GL_TRIANGLES);
    }
}

//--------------------------------------------------
// 화면 그리기
//--------------------------------------------------
void DrawScene()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //--- 색상 버퍼와 깊이 버퍼 초기화
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    DrawAxis();
    DrawCube();
    DrawPyramid();
}

//--------------------------------------------------
// 키보드 입력
//--------------------------------------------------
void KeyCallback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods
)
{
    if (action != GLFW_PRESS)
        return;

    //--- 1~6: 육면체의 각 면
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6)
    {
        int index = key - GLFW_KEY_1;
        cubeFaces[index] = true;
    }

    //--- 7~9: 사각뿔의 각 면
    if (key >= GLFW_KEY_7 && key <= GLFW_KEY_9)
    {
        int index = key - GLFW_KEY_7;
        pyramidFaces[index] = true;
    }

    //--- 0: 사각뿔의 네 번째 옆면
    if (key == GLFW_KEY_0)
    {
        pyramidFaces[3] = true;
    }

    //--- c: 육면체 랜덤 2개 면
    if (key == GLFW_KEY_C)
    {
        std::uniform_int_distribution<int> dist(0, 5);

        int a = dist(gen);
        int b = dist(gen);

        while (a == b)
            b = dist(gen);

        cubeFaces[a] = true;
        cubeFaces[b] = true;
    }

    //--- t: 사각뿔 바닥과 랜덤 옆면
    if (key == GLFW_KEY_T)
    {
        std::uniform_int_distribution<int> dist(0, 3);

        pyramidFaces[4] = true;
        pyramidFaces[dist(gen)] = true;
    }

    //--- r: 모든 면 초기화
    if (key == GLFW_KEY_R)
    {
        for (int i = 0; i < 6; i++)
            cubeFaces[i] = false;

        for (int i = 0; i < 5; i++)
            pyramidFaces[i] = false;
    }

    //--- q: 프로그램 종료
    if (key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(window, true);
    }
}
