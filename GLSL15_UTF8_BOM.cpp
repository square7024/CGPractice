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
#include <algorithm>
#include <cmath>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

enum class ShapeType
{
    CUBE,
    PYRAMID
};

enum class GroupAnimation
{
    NONE,
    EDGE_360,
    SWAP_90,
    FOUR_Y,
    FOUR_X
};

//--- 셰이더와 버퍼
GLuint vertexShader = 0;
GLuint fragmentShader = 0;
GLuint shaderProgramID = 0;
GLuint VAO = 0;
GLuint VBO = 0;
GLint modelLocation = -1;

//--- 도형 및 은면 제거
ShapeType currentShape = ShapeType::CUBE;
bool depthEnabled = true;

//--- 도형 전체의 Y축 자전
bool rotatingY = false;
float angleY = 0.0f;

//--- 육면체 한 면 애니메이션
bool topSpinning = false;
bool sideSpinning = false;
float topAngle = 0.0f;
float sideAngle = 0.0f;
float frontOpen = 0.0f;
float frontTarget = 0.0f;
float backScale = 1.0f;
float backTarget = 1.0f;

//--- 육면체 여러 면 애니메이션
GroupAnimation groupAnimation = GroupAnimation::NONE;
bool groupRunning = false;
float groupAngle = 0.0f;
int selectedSide = 0;

//--- 랜덤 엔진
std::random_device rd;
std::default_random_engine gen(rd());

//--- 육면체 옆면의 순서: 앞, 오른쪽, 뒤, 왼쪽
const int SIDE_FACES[4] = { 0, 3, 1, 2 };
const float HALF_SIZE = 0.5f;
const float SPIN_SPEED = 65.0f;

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
void ResetAnimation();
void ResetCubeEffects();
void ClearGroupAnimation();
void StartGroupAnimation(GroupAnimation animation);
float MoveTowards(float value, float target, float amount);
glm::mat4 BaseRotation();
glm::mat4 RotateAt(glm::mat4 model, float degrees,
    glm::vec3 axis, glm::vec3 pivot);
glm::mat4 ScaleAt(glm::mat4 model, float scale, glm::vec3 pivot);

void KeyCallback(GLFWwindow* window, int key, int scancode,
    int action, int mods);

//--- 메인 함수
int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW 초기화 실패!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "Practice 15", nullptr, nullptr);

    if (!window)
    {
        std::cerr << "윈도우 생성 실패!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);

    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();

    if (shaderProgramID == 0)
    {
        if (vertexShader) glDeleteShader(vertexShader);
        if (fragmentShader) glDeleteShader(fragmentShader);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
    InitBuffer();
    glfwSetKeyCallback(window, KeyCallback);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;
        deltaTime = std::min(deltaTime, 0.05f);

        UpdateAnimation(deltaTime);
        DrawScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
        std::cerr << file << " 파일을 열 수 없습니다." << std::endl;
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
    std::string sourceText = filetobuf("GLSL15vertex.glsl");
    if (sourceText.empty()) return;

    const char* source = sourceText.c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &source, NULL);
    glCompileShader(vertexShader);

    GLint result = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        GLchar errorLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n"
            << errorLog << std::endl;
        glDeleteShader(vertexShader);
        vertexShader = 0;
    }
}

//--- 프래그먼트 셰이더
void make_fragmentShaders()
{
    std::string sourceText = filetobuf("GLSL15fragment.glsl");
    if (sourceText.empty()) return;

    const char* source = sourceText.c_str();
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &source, NULL);
    glCompileShader(fragmentShader);

    GLint result = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        GLchar errorLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n"
            << errorLog << std::endl;
        glDeleteShader(fragmentShader);
        fragmentShader = 0;
    }
}

//--- 셰이더 프로그램
GLuint make_shaderProgram()
{
    if (!vertexShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint result = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result)
    {
        GLchar errorLog[512];
        glGetProgramInfoLog(program, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n"
            << errorLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

//--- VAO, VBO 생성
void InitBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

//--- 정점 데이터 출력
void DrawVertices(const std::vector<Vertex>& vertices, GLenum mode)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);
}

//--- 처음 X축 및 Y축에 대하여 30도 기울어진 행렬
//--- GLM 행렬은 오른쪽 변환부터 정점에 적용된다.
glm::mat4 BaseRotation()
{
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
    return model;
}

//--- 주어진 점을 지나는 축에 대하여 회전
glm::mat4 RotateAt(glm::mat4 model, float degrees,
    glm::vec3 axis, glm::vec3 pivot)
{
    model = glm::translate(model, pivot);
    model = glm::rotate(model, glm::radians(degrees), axis);
    model = glm::translate(model, -pivot);
    return model;
}

//--- 면의 중점을 기준으로 확대 및 축소
glm::mat4 ScaleAt(glm::mat4 model, float scale, glm::vec3 pivot)
{
    model = glm::translate(model, pivot);
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    model = glm::translate(model, -pivot);
    return model;
}

//--- 고정된 좌표축 그리기
void DrawAxis()
{
    glm::mat4 model = BaseRotation();
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    std::vector<Vertex> axes = {
        {{-1.0f, 0, 0}, {1, 0, 0}}, {{1.0f, 0, 0}, {1, 0, 0}},
        {{0, -1.0f, 0}, {0, 1, 0}}, {{0, 1.0f, 0}, {0, 1, 0}},
        {{0, 0, -1.0f}, {0, 0, 1}}, {{0, 0, 1.0f}, {0, 0, 1}}
    };

    DrawVertices(axes, GL_LINES);
}

//--- 육면체의 6개 면을 각각 독립적으로 그리기
void DrawCube()
{
    const float s = HALF_SIZE;

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

    //--- 각 면의 바깥쪽에서 반시계 방향으로 배치
    int faces[6][4] = {
        {4, 5, 6, 7}, // 0: 앞면 (+Z)
        {1, 0, 3, 2}, // 1: 뒷면 (-Z)
        {0, 4, 7, 3}, // 2: 왼쪽 (-X)
        {5, 1, 2, 6}, // 3: 오른쪽 (+X)
        {3, 7, 6, 2}, // 4: 윗면 (+Y)
        {0, 1, 5, 4}  // 5: 아랫면 (-Y)
    };

    glm::vec3 colors[6] = {
        {1.0f, 0.1f, 0.1f}, // 앞면
        {1.0f, 0.9f, 0.1f}, // 뒷면
        {0.2f, 0.2f, 1.0f}, // 왼쪽
        {0.1f, 0.9f, 0.2f}, // 오른쪽
        {0.0f, 0.9f, 0.9f}, // 윗면
        {1.0f, 0.1f, 0.9f}  // 아랫면
    };

    //--- 기본 기울기 + 육면체 전체 Y축 자전
    glm::mat4 base = BaseRotation();
    base = glm::rotate(base, glm::radians(angleY), glm::vec3(0, 1, 0));

    //--- 모서리를 공유하는 옆면들의 중심 회전축
    glm::vec3 edgePivots[4] = {
        { s, 0,  s},
        { s, 0, -s},
        {-s, 0, -s},
        {-s, 0,  s}
    };

    int first = SIDE_FACES[selectedSide];
    int second = SIDE_FACES[(selectedSide + 1) % 4];

    for (int i = 0; i < 6; ++i)
    {
        glm::mat4 model = base;

        //--- 1: 공통 세로 모서리를 기준으로 두 면이 360도 회전
        if (groupAnimation == GroupAnimation::EDGE_360 &&
            (i == first || i == second))
        {
            model = RotateAt(model, groupAngle,
                glm::vec3(0, 1, 0), edgePivots[selectedSide]);
        }
        //--- 2: 두 면이 중심 Y축을 기준으로 서로 반대로 90도 공전
        else if (groupAnimation == GroupAnimation::SWAP_90)
        {
            if (i == first)
                model = glm::rotate(model, glm::radians(groupAngle),
                    glm::vec3(0, 1, 0));
            else if (i == second)
                model = glm::rotate(model, glm::radians(-groupAngle),
                    glm::vec3(0, 1, 0));
        }
        //--- 4: 네 개의 옆면이 중심 Y축을 기준으로 공전
        else if (groupAnimation == GroupAnimation::FOUR_Y && i < 4)
        {
            model = glm::rotate(model, glm::radians(groupAngle),
                glm::vec3(0, 1, 0));
        }
        //--- 5: 윗면/아랫면/앞면/뒷면이 중심 X축을 기준으로 공전
        else if (groupAnimation == GroupAnimation::FOUR_X &&
            (i == 4 || i == 5 || i == 0 || i == 1))
        {
            model = glm::rotate(model, glm::radians(groupAngle),
                glm::vec3(1, 0, 0));
        }
        else if (groupAnimation == GroupAnimation::NONE)
        {
            //--- t: 윗면 중점을 통과하는 Z축 기준 회전
            if (i == 4)
                model = RotateAt(model, topAngle,
                    glm::vec3(0, 0, 1), glm::vec3(0, s, 0));

            //--- f: 앞면의 위쪽 모서리를 경첩으로 열기/닫기
            if (i == 0)
                model = RotateAt(model, -90.0f * frontOpen,
                    glm::vec3(1, 0, 0), glm::vec3(0, s, s));

            //--- s: 양쪽 옆면이 각각의 중점을 기준으로 회전
            if (i == 2 || i == 3)
            {
                float x = (i == 2) ? -s : s;

                model = RotateAt(model, sideAngle,
                    glm::vec3(0, 1, 0), glm::vec3(x, 0, 0));
            }

            //--- b: 뒷면 중점 기준으로 작아졌다가 다시 커짐
            if (i == 1)
            {
                if (backScale <= 0.001f) continue;
                model = ScaleAt(model, backScale, glm::vec3(0, 0, -s));
            }
        }

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
            glm::value_ptr(model));

        int* f = faces[i];
        glm::vec3 c = colors[i];
        std::vector<Vertex> vertices = {
            {p[f[0]], c}, {p[f[1]], c}, {p[f[2]], c},
            {p[f[0]], c}, {p[f[2]], c}, {p[f[3]], c}
        };

        DrawVertices(vertices, GL_TRIANGLES);
    }
}

//--- 사각뿔 그리기
void DrawPyramid()
{
    const float s = HALF_SIZE;
    glm::vec3 p[5] = {
        {-s, -s, -s},
        { s, -s, -s},
        { s, -s,  s},
        {-s, -s,  s},
        {0.0f, s, 0.0f}
    };

    int faces[4][3] = {
        {1, 0, 4},
        {2, 1, 4},
        {3, 2, 4},
        {0, 3, 4}
    };

    glm::vec3 colors[5] = {
        {1.0f, 0.1f, 0.1f},
        {0.2f, 0.2f, 1.0f},
        {0.1f, 0.9f, 0.2f},
        {1.0f, 0.9f, 0.1f},
        {1.0f, 0.1f, 0.9f}
    };

    glm::mat4 model = BaseRotation();
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    //--- 옆면 4개
    for (int i = 0; i < 4; ++i)
    {
        int* f = faces[i];
        glm::vec3 c = colors[i];
        std::vector<Vertex> vertices = {
            {p[f[0]], c}, {p[f[1]], c}, {p[f[2]], c}
        };
        DrawVertices(vertices, GL_TRIANGLES);
    }

    //--- 바닥면
    std::vector<Vertex> bottom = {
        {p[0], colors[4]}, {p[1], colors[4]}, {p[2], colors[4]},
        {p[0], colors[4]}, {p[2], colors[4]}, {p[3], colors[4]}
    };
    DrawVertices(bottom, GL_TRIANGLES);
}

//--- 현재 값이 목표값으로 일정 속도로 변하도록 보간
float MoveTowards(float value, float target, float amount)
{
    if (value < target) return std::min(value + amount, target);
    if (value > target) return std::max(value - amount, target);
    return value;
}

//--- 애니메이션 갱신
void UpdateAnimation(float deltaTime)
{
    if (rotatingY)
        angleY = std::fmod(angleY + SPIN_SPEED * deltaTime, 360.0f);

    if (topSpinning)
        topAngle = std::fmod(topAngle + SPIN_SPEED * deltaTime, 360.0f);

    if (sideSpinning)
        sideAngle = std::fmod(sideAngle + SPIN_SPEED * deltaTime, 360.0f);

    frontOpen = MoveTowards(frontOpen, frontTarget, deltaTime * 1.5f);
    backScale = MoveTowards(backScale, backTarget, deltaTime * 1.2f);

    if (!groupRunning) return;

    if (groupAnimation == GroupAnimation::EDGE_360)
    {
        groupAngle += 180.0f * deltaTime;
        if (groupAngle >= 360.0f)
        {
            ClearGroupAnimation();
        }
    }
    else if (groupAnimation == GroupAnimation::SWAP_90)
    {
        groupAngle = std::min(groupAngle + 90.0f * deltaTime, 90.0f);
        if (groupAngle >= 90.0f)
            groupRunning = false;
    }
    else if (groupAnimation == GroupAnimation::FOUR_Y ||
             groupAnimation == GroupAnimation::FOUR_X)
    {
        groupAngle = std::fmod(groupAngle + 65.0f * deltaTime, 360.0f);
    }
}

//--- 여러 면 애니메이션 초기화
void ClearGroupAnimation()
{
    groupAnimation = GroupAnimation::NONE;
    groupRunning = false;
    groupAngle = 0.0f;
}

//--- 육면체의 한 면 애니메이션 초기화
void ResetCubeEffects()
{
    topSpinning = false;
    sideSpinning = false;
    topAngle = 0.0f;
    sideAngle = 0.0f;
    frontOpen = 0.0f;
    frontTarget = 0.0f;
    backScale = 1.0f;
    backTarget = 1.0f;
    ClearGroupAnimation();
}

//--- 전체 초기화하고 육면체 출력
void ResetAnimation()
{
    currentShape = ShapeType::CUBE;
    depthEnabled = true;
    glEnable(GL_DEPTH_TEST);
    rotatingY = false;
    angleY = 0.0f;
    ResetCubeEffects();
}

//--- 여러 면 애니메이션 시작 및 일시 정지
void StartGroupAnimation(GroupAnimation animation)
{
    if (groupAnimation == animation &&
        (animation == GroupAnimation::FOUR_Y ||
         animation == GroupAnimation::FOUR_X))
    {
        groupRunning = !groupRunning;
        return;
    }

    //--- 여러 면 변환 전에 한 면 변환들을 초기화
    ResetCubeEffects();
    groupAnimation = animation;
    groupRunning = true;
    groupAngle = 0.0f;

    if (animation == GroupAnimation::EDGE_360 ||
        animation == GroupAnimation::SWAP_90)
    {
        std::uniform_int_distribution<int> dist(0, 3);
        selectedSide = dist(gen);
    }
}

//--- 화면 출력
void DrawScene()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    DrawAxis();
    if (currentShape == ShapeType::CUBE)
        DrawCube();
    else
        DrawPyramid();
}

//--- 키보드 입력
void KeyCallback(GLFWwindow* window, int key, int scancode,
    int action, int mods)
{
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_H)
    {
        depthEnabled = !depthEnabled;
        if (depthEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
    }
    else if (key == GLFW_KEY_P)
    {
        currentShape = (currentShape == ShapeType::CUBE)
            ? ShapeType::PYRAMID : ShapeType::CUBE;
    }
    else if (key == GLFW_KEY_Y)
    {
        rotatingY = !rotatingY;
    }
    else if (key == GLFW_KEY_R)
    {
        ResetAnimation();
    }
    else if (key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if (currentShape == ShapeType::CUBE)
    {
        if (key == GLFW_KEY_T)
        {
            ClearGroupAnimation();
            topSpinning = !topSpinning;
        }
        else if (key == GLFW_KEY_F)
        {
            ClearGroupAnimation();
            frontTarget = (frontTarget == 0.0f) ? 1.0f : 0.0f;
        }
        else if (key == GLFW_KEY_S)
        {
            ClearGroupAnimation();
            sideSpinning = !sideSpinning;
        }
        else if (key == GLFW_KEY_B)
        {
            ClearGroupAnimation();
            backTarget = (backTarget == 1.0f) ? 0.0f : 1.0f;
        }
        else if (key == GLFW_KEY_1)
        {
            StartGroupAnimation(GroupAnimation::EDGE_360);
        }
        else if (key == GLFW_KEY_2)
        {
            StartGroupAnimation(GroupAnimation::SWAP_90);
        }
        else if (key == GLFW_KEY_4)
        {
            StartGroupAnimation(GroupAnimation::FOUR_Y);
        }
        else if (key == GLFW_KEY_5)
        {
            StartGroupAnimation(GroupAnimation::FOUR_X);
        }
    }
}
