
#include <gl/glew.h>
#include <gl/glfw3.h>

#include <gl/glm/glm/glm.hpp>
#include <gl/glm/glm/ext.hpp>
#include <gl/glm/glm/gtc/matrix_transform.hpp>
#include <gl/glm/glm/gtc/type_ptr.hpp>

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

//--- 도형 종류
enum class ShapeType
{
    NONE,
    CUBE,
    PYRAMID
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

//--- 현재 도형
ShapeType currentShape = ShapeType::NONE;

//--- 각 면의 출력 여부
bool cubeFaces[6] = { false };
bool pyramidFaces[5] = { false };

//--- 랜덤 엔진
std::random_device rd;
std::default_random_engine gen(rd());

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

void ChangeShape(ShapeType shape);
void ResetShapes();

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

    //--- 은면 제거
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

    //--- 모델링 변환 행렬 위치 가져오기
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

//--- 셰이더 파일 읽기
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

//--- 버텍스 셰이더
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

//--- 프래그먼트 셰이더
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

//--- 셰이더 프로그램
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

//--- VAO, VBO 생성
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

//--- 정점 데이터 출력
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

//--- 화면 좌표축 그리기
void DrawAxis()
{
    //--- 모델링 변환
    glm::mat4 model(1.0f);

    //--- X축에 대하여 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- Y축에 대하여 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    //--- 모델링 변환 행렬 전송
    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    //--- 좌표축 정점
    std::vector<Vertex> axes = {
        //--- X축: 빨간색
        {{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},

        //--- Y축: 초록색
        {{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},

        //--- Z축: 파란색
        {{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {{0.0f, 0.0f,  1.0f}, {0.0f, 0.0f, 1.0f}}
    };

    DrawVertices(axes, GL_LINES);
}

//--- 육면체 그리기
void DrawCube()
{
    const float s = 0.5f;

    //--- 육면체의 8개 꼭짓점
    glm::vec3 p[8] = {
        {-s, -s, -s}, // 0
        { s, -s, -s}, // 1
        { s,  s, -s}, // 2
        {-s,  s, -s}, // 3
        {-s, -s,  s}, // 4
        { s, -s,  s}, // 5
        { s,  s,  s}, // 6
        {-s,  s,  s}  // 7
    };

    //--- 각 면을 구성하는 꼭짓점 (CCW)
    int faces[6][4] = {
        {4, 5, 6, 7}, // 앞면 (+Z)
        {1, 0, 3, 2}, // 뒷면 (-Z)
        {0, 4, 7, 3}, // 왼쪽 면 (-X)
        {5, 1, 2, 6}, // 오른쪽 면 (+X)
        {3, 7, 6, 2}, // 윗면 (+Y)
        {0, 1, 5, 4}  // 아랫면 (-Y)
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

    //--- X축에 대하여 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- Y축에 대하여 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    //--- 모델링 변환 행렬 전송
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

//--- 사각뿔 그리기
void DrawPyramid()
{
    const float s = 0.5f;

    //--- 사각뿔의 5개 꼭짓점
    glm::vec3 p[5] = {
        {-s, -s, -s},         // 0
        { s, -s, -s},         // 1
        { s, -s,  s},         // 2
        {-s, -s,  s},         // 3
        {0.0f, s, 0.0f}       // 4: 꼭짓점
    };

    //--- 삼각형 옆면 (CCW)
    int faces[4][3] = {
        {1, 0, 4}, // 뒷면 (-Z)
        {2, 1, 4}, // 오른쪽 면 (+X)
        {3, 2, 4}, // 앞면 (+Z)
        {0, 3, 4}  // 왼쪽 면 (-X)
    };

    //--- 각 면의 색상
    glm::vec3 colors[5] = {
        {1.0f, 0.0f, 0.0f}, // 빨강
        {0.0f, 0.0f, 1.0f}, // 파랑
        {0.0f, 1.0f, 0.0f}, // 초록
        {1.0f, 1.0f, 0.0f}, // 노랑
        {1.0f, 0.0f, 1.0f}  // 자홍
    };

    //--- 모델링 변환
    glm::mat4 model(1.0f);

    //--- X축에 대하여 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- Y축에 대하여 30도 회전
    model = glm::rotate(
        model,
        glm::radians(30.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    //--- 모델링 변환 행렬 전송
    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    //--- 선택된 삼각형 옆면 출력
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

    //--- 바닥면 출력 (CCW)
    if (pyramidFaces[4])
    {
        std::vector<Vertex> vertices = {
            {p[0], colors[4]},
            {p[1], colors[4]},
            {p[2], colors[4]},

            {p[0], colors[4]},
            {p[2], colors[4]},
            {p[3], colors[4]}
        };

        DrawVertices(vertices, GL_TRIANGLES);
    }
}

//--- 현재 도형 변경
void ChangeShape(ShapeType shape)
{
    //--- 같은 종류의 도형이면 기존 면 유지
    if (currentShape == shape)
        return;

    //--- 도형이 변경되면 모든 면 초기화
    for (int i = 0; i < 6; i++)
        cubeFaces[i] = false;

    for (int i = 0; i < 5; i++)
        pyramidFaces[i] = false;

    currentShape = shape;
}

//--- 모든 도형 초기화
void ResetShapes()
{
    for (int i = 0; i < 6; i++)
        cubeFaces[i] = false;

    for (int i = 0; i < 5; i++)
        pyramidFaces[i] = false;

    currentShape = ShapeType::NONE;
}

//--- 화면 그리기
void DrawScene()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //--- 색상 버퍼와 깊이 버퍼 초기화
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    //--- 좌표축 출력
    DrawAxis();

    //--- 선택된 도형만 출력
    if (currentShape == ShapeType::CUBE)
    {
        DrawCube();
    }
    else if (currentShape == ShapeType::PYRAMID)
    {
        DrawPyramid();
    }
}

//--- 키보드 입력
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
        ChangeShape(ShapeType::CUBE);

        int index = key - GLFW_KEY_1;
        cubeFaces[index] = true;
    }

    //--- 7~9: 사각뿔의 삼각형 면
    else if (key >= GLFW_KEY_7 && key <= GLFW_KEY_9)
    {
        ChangeShape(ShapeType::PYRAMID);

        int index = key - GLFW_KEY_7;
        pyramidFaces[index] = true;
    }

    //--- 0: 사각뿔의 네 번째 삼각형 면
    else if (key == GLFW_KEY_0)
    {
        ChangeShape(ShapeType::PYRAMID);

        pyramidFaces[3] = true;
    }

    //--- C: 육면체 랜덤 2개 면
    else if (key == GLFW_KEY_C)
    {
        ChangeShape(ShapeType::CUBE);

        std::uniform_int_distribution<int> dist(0, 5);

        int a = dist(gen);
        int b = dist(gen);

        //--- 서로 다른 두 면 선택
        while (a == b)
            b = dist(gen);

        cubeFaces[a] = true;
        cubeFaces[b] = true;
    }

    //--- T: 사각뿔 바닥과 랜덤 옆면
    else if (key == GLFW_KEY_T)
    {
        ChangeShape(ShapeType::PYRAMID);

        std::uniform_int_distribution<int> dist(0, 3);

        pyramidFaces[4] = true;
        pyramidFaces[dist(gen)] = true;
    }

    //--- R: 전체 초기화
    else if (key == GLFW_KEY_R)
    {
        ResetShapes();
    }

    //--- Q: 프로그램 종료
    else if (key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(window, true);
    }
}
