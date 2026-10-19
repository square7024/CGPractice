
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

//--- 은면 제거 및 출력 방식
bool depthEnabled = true;
bool wireframe = false;

//--- 객체 위치
float moveX = 0.0f;
float moveY = 0.0f;

//--- 객체 회전 각도
float angleX = 30.0f;
float angleY = 30.0f;

//--- 회전 속도 (도/초)
float rotateSpeedX = 0.0f;
float rotateSpeedY = 0.0f;

const float ROTATE_SPEED = 60.0f;
const float MOVE_STEP = 0.1f;

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

void UpdateAnimation(float deltaTime);
void ResetTransform();

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
        "Practice 14",
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

    //--- 화면 설정
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSwapInterval(1);

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

    //--- 모델링 변환 행렬 위치
    modelLocation = glGetUniformLocation(
        shaderProgramID,
        "modelTransform"
    );

    //--- 버퍼 생성
    InitBuffer();

    //--- 키보드 콜백
    glfwSetKeyCallback(window, KeyCallback);

    double lastTime = glfwGetTime();

    //--- 렌더링 루프
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        float deltaTime = static_cast<float>(
            currentTime - lastTime
            );

        lastTime = currentTime;

        //--- 애니메이션 갱신
        UpdateAnimation(deltaTime);

        //--- 화면 출력
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
        filetobuf("GLSL14vertex.glsl");

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
            vertexShader, 512, NULL, errorLog
        );

        std::cerr
            << "ERROR: vertex shader 컴파일 실패\n"
            << errorLog << std::endl;

        glDeleteShader(vertexShader);
        vertexShader = 0;
    }
}

//--- 프래그먼트 셰이더
void make_fragmentShaders()
{
    std::string fragmentSource =
        filetobuf("GLSL14fragment.glsl");

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
            fragmentShader, 512, NULL, errorLog
        );

        std::cerr
            << "ERROR: fragment shader 컴파일 실패\n"
            << errorLog << std::endl;

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
            shaderID, 512, NULL, errorLog
        );

        std::cerr
            << "ERROR: shader program 연결 실패\n"
            << errorLog << std::endl;

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
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );
    glEnableVertexAttribArray(0);

    //--- 정점 색상
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
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
        mode, 0, (GLsizei)vertices.size()
    );

    glBindVertexArray(0);
}

//--- 좌표축 그리기
void DrawAxis()
{
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

    glUniformMatrix4fv(
        modelLocation, 1, GL_FALSE,
        glm::value_ptr(model)
    );

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

    //--- 각 면의 꼭짓점 순서 (CCW)
    int faces[6][4] = {
        {4, 5, 6, 7}, // 앞면 (+Z)
        {1, 0, 3, 2}, // 뒷면 (-Z)
        {0, 4, 7, 3}, // 왼쪽 (-X)
        {5, 1, 2, 6}, // 오른쪽 (+X)
        {3, 7, 6, 2}, // 윗면 (+Y)
        {0, 1, 5, 4}  // 아랫면 (-Y)
    };

    //--- 각 꼭짓점의 색상
    glm::vec3 colors[8] = {
        {1.0f, 0.0f, 0.0f}, // 0: 빨강
        {0.0f, 1.0f, 0.0f}, // 1: 초록
        {0.0f, 0.0f, 1.0f}, // 2: 파랑
        {1.0f, 1.0f, 0.0f}, // 3: 노랑
        {0.0f, 1.0f, 1.0f}, // 4: 청록
        {1.0f, 0.0f, 1.0f}, // 5: 자홍
        {1.0f, 0.5f, 0.0f}, // 6: 주황
        {0.5f, 0.5f, 0.5f}  // 7: 회색
    };

    //--- 모델링 변환
    glm::mat4 model(1.0f);

    //--- 객체 이동
    model = glm::translate(
        model,
        glm::vec3(moveX, moveY, 0.0f)
    );

    //--- X축에 대하여 회전
    model = glm::rotate(
        model,
        glm::radians(angleX),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- Y축에 대하여 회전
    model = glm::rotate(
        model,
        glm::radians(angleY),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(
        modelLocation, 1, GL_FALSE,
        glm::value_ptr(model)
    );

    //--- 육면체의 모든 면 그리기
    for (int i = 0; i < 6; i++)
    {
        int* f = faces[i];

        std::vector<Vertex> vertices = {
            {p[f[0]], colors[f[0]]},
            {p[f[1]], colors[f[1]]},
            {p[f[2]], colors[f[2]]},

            {p[f[0]], colors[f[0]]},
            {p[f[2]], colors[f[2]]},
            {p[f[3]], colors[f[3]]}
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
        {-s, -s, -s},     // 0
        { s, -s, -s},     // 1
        { s, -s,  s},     // 2
        {-s, -s,  s},     // 3
        {0.0f, s, 0.0f}   // 4: 꼭짓점
    };

    //--- 삼각형 옆면의 정점 순서 (CCW)
    int faces[4][3] = {
        {1, 0, 4}, // 뒷면 (-Z)
        {2, 1, 4}, // 오른쪽 (+X)
        {3, 2, 4}, // 앞면 (+Z)
        {0, 3, 4}  // 왼쪽 (-X)
    };

    //--- 각 꼭짓점의 색상
    glm::vec3 colors[5] = {
        {1.0f, 0.0f, 0.0f}, // 0: 빨강
        {0.0f, 1.0f, 0.0f}, // 1: 초록
        {0.0f, 0.0f, 1.0f}, // 2: 파랑
        {1.0f, 1.0f, 0.0f}, // 3: 노랑
        {1.0f, 0.0f, 1.0f}  // 4: 자홍
    };

    //--- 모델링 변환
    glm::mat4 model(1.0f);

    //--- 객체 이동
    model = glm::translate(
        model,
        glm::vec3(moveX, moveY, 0.0f)
    );

    //--- X축에 대하여 회전
    model = glm::rotate(
        model,
        glm::radians(angleX),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    //--- Y축에 대하여 회전
    model = glm::rotate(
        model,
        glm::radians(angleY),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(
        modelLocation, 1, GL_FALSE,
        glm::value_ptr(model)
    );

    //--- 삼각형 옆면 그리기
    for (int i = 0; i < 4; i++)
    {
        int* f = faces[i];

        std::vector<Vertex> vertices = {
            {p[f[0]], colors[f[0]]},
            {p[f[1]], colors[f[1]]},
            {p[f[2]], colors[f[2]]}
        };

        DrawVertices(vertices, GL_TRIANGLES);
    }

    //--- 바닥면 그리기
    std::vector<Vertex> bottom = {
        {p[0], colors[0]},
        {p[1], colors[1]},
        {p[2], colors[2]},

        {p[0], colors[0]},
        {p[2], colors[2]},
        {p[3], colors[3]}
    };

    DrawVertices(bottom, GL_TRIANGLES);
}

//--- 회전 애니메이션 갱신
void UpdateAnimation(float deltaTime)
{
    angleX += rotateSpeedX * deltaTime;
    angleY += rotateSpeedY * deltaTime;
}

//--- 초기 위치로 리셋
void ResetTransform()
{
    moveX = 0.0f;
    moveY = 0.0f;

    angleX = 30.0f;
    angleY = 30.0f;

    rotateSpeedX = 0.0f;
    rotateSpeedY = 0.0f;
}

//--- 화면 그리기
void DrawScene()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //--- 색상 버퍼와 깊이 버퍼 초기화
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    //--- 좌표축은 항상 고정
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    DrawAxis();

    //--- 와이어프레임 / 솔리드 설정
    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //--- 선택된 도형 출력
    if (currentShape == ShapeType::CUBE)
    {
        DrawCube();
    }
    else if (currentShape == ShapeType::PYRAMID)
    {
        DrawPyramid();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

    bool shift = (mods & GLFW_MOD_SHIFT) != 0;

    //--- C: 육면체
    if (key == GLFW_KEY_C)
    {
        currentShape = ShapeType::CUBE;
    }

    //--- P: 사각뿔
    else if (key == GLFW_KEY_P)
    {
        currentShape = ShapeType::PYRAMID;
    }

    //--- H: 은면 제거 적용 / 해제
    else if (key == GLFW_KEY_H)
    {
        depthEnabled = !depthEnabled;

        if (depthEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    //--- w/W: 와이어프레임 / 솔리드
    else if (key == GLFW_KEY_W)
    {
        wireframe = !shift;
    }

    //--- x/X: X축 양/음 방향 자전
    else if (key == GLFW_KEY_X)
    {
        float speed = shift ? -ROTATE_SPEED : ROTATE_SPEED;

        if (rotateSpeedX == speed)
            rotateSpeedX = 0.0f;
        else
            rotateSpeedX = speed;
    }

    //--- y/Y: Y축 양/음 방향 자전
    else if (key == GLFW_KEY_Y)
    {
        float speed = shift ? -ROTATE_SPEED : ROTATE_SPEED;

        if (rotateSpeedY == speed)
            rotateSpeedY = 0.0f;
        else
            rotateSpeedY = speed;
    }

    //--- 왼쪽 이동
    else if (key == GLFW_KEY_LEFT)
    {
        moveX -= MOVE_STEP;
    }

    //--- 오른쪽 이동
    else if (key == GLFW_KEY_RIGHT)
    {
        moveX += MOVE_STEP;
    }

    //--- 위쪽 이동
    else if (key == GLFW_KEY_UP)
    {
        moveY += MOVE_STEP;
    }

    //--- 아래쪽 이동
    else if (key == GLFW_KEY_DOWN)
    {
        moveY -= MOVE_STEP;
    }

    //--- S: 초기 위치 및 애니메이션 리셋
    else if (key == GLFW_KEY_S)
    {
        ResetTransform();
    }
}   