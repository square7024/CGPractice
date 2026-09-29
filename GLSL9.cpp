#include <gl/glew.h>
#include <gl/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PI 3.1415926535f

struct Triangle
{
	float x;
	float y;

	float size;

	float r;
	float g;
	float b;

	bool used;

	// 이동 방향
	float dx;
	float dy;

	// 이동 속도
	float speed;

	// 스파이럴 이동용
	float centerX;
	float centerY;
	float angle;
	float radius;
};

Triangle triangles[4];

GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO;
GLuint VBO;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<float> sizeDist(0.10f, 0.20f);
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_real_distribution<float> speedDist(0.01f, 0.025f);
std::uniform_real_distribution<float> directionDist(-1.0f, 1.0f);

// true: 면
// false: 선
bool fillMode{ true };

// 1: 튕기기
// 2: 좌우 지그재그
// 3: 상하 지그재그
// 4: 원 스파이럴
int moveMode{ 1 };

std::string filetobuf(const char* file);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

void InitBuffer();

void DrawScene();
void DrawAxis();
void DrawTriangle(const Triangle& triangle);
void DrawPath();

void CreateTriangle(int area, float x, float y);
void ChangeTriangleSize(int area);

void UpdateTriangles();

void UpdateBounce(Triangle& triangle);
void UpdateHorizontalZigzag(Triangle& triangle);
void UpdateVerticalZigzag(Triangle& triangle);
void UpdateSpiral(Triangle& triangle);

void ResetMovement();

int GetArea(float x, float y);

void KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods
);

void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
);

void AddVertex(
	std::vector<float>& vertices,
	float x,
	float y,
	float r,
	float g,
	float b
);

//----------------
int main()
//----------------
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
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		"Practice 9",
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

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!" << std::endl;

		glfwDestroyWindow(window);
		glfwTerminate();

		return -1;
	}

	make_vertexShaders();
	make_fragmentShaders();

	shaderProgramID = make_shaderProgram();

	if (shaderProgramID == 0)
	{
		glfwDestroyWindow(window);
		glfwTerminate();

		return -1;
	}

	InitBuffer();

	for (int i = 0; i < 4; ++i)
	{
		triangles[i].used = false;
	}

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	while (!glfwWindowShouldClose(window))
	{
		UpdateTriangles();
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

// 셰이더 파일 읽기
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

// 버텍스 셰이더
void make_vertexShaders()
{
	std::string vertexSource = filetobuf("GLSL9vertex.glsl");
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
	}
}

// 프래그먼트 셰이더
void make_fragmentShaders()
{
	std::string fragmentSource = filetobuf("GLSL9fragment.glsl");
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
	}
}

// 셰이더 프로그램
GLuint make_shaderProgram()
{
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

		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderID);

	return shaderID;
}

// VAO / VBO
void InitBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// 경로를 그리기 위해 넉넉하게 공간 확보
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(float) * 1000 * 6,
		nullptr,
		GL_DYNAMIC_DRAW
	);

	// 위치
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(void*)0
	);
	glEnableVertexAttribArray(0);

	// 색상
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(void*)(3 * sizeof(float))
	);
	glEnableVertexAttribArray(1);
}

// 화면 출력
void DrawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	// 이동 경로
	// DrawPath();

	// 삼각형
	for (int i = 0; i < 4; ++i)
	{
		if (triangles[i].used)
		{
			DrawTriangle(triangles[i]);
		}
	}
}

// 축
void DrawAxis()
{
	std::vector<float> vertices;

	AddVertex(vertices, -1.0f, 0.0f, 0.5f, 0.7f, 1.0f);
	AddVertex(vertices, 1.0f, 0.0f, 0.5f, 0.7f, 1.0f);

	AddVertex(vertices, 0.0f, -1.0f, 0.5f, 0.7f, 1.0f);
	AddVertex(vertices, 0.0f, 1.0f, 0.5f, 0.7f, 1.0f);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	glDrawArrays(GL_LINES, 0, 4);
}

// 삼각형 그리기
void DrawTriangle(const Triangle& triangle)
{
	std::vector<float> vertices;

	float half{
		triangle.size / 2.0f
	};

	// 삼각형의 중심을 맞추기 위한 보정
	float centerY{
		triangle.size / 4.0f
	};

	float angle{ 0.0f };

	// 2번: 좌우 이동 방향
	if (moveMode == 2)
	{
		if (triangle.dx > 0.0f)
		{
			angle = -PI / 2.0f;
		}
		else
		{
			angle = PI / 2.0f;
		}
	}

	// 3번: 상하 이동 방향
	else if (moveMode == 3)
	{
		if (triangle.dy > 0.0f)
		{
			angle = 0.0f;
		}
		else
		{
			angle = PI;
		}
	}

	float cosAngle{
		std::cos(angle)
	};

	float sinAngle{
		std::sin(angle)
	};

	// 위쪽 꼭짓점
	float x1{ 0.0f };
	float y1{ triangle.size - centerY };

	// 왼쪽 아래
	float x2{ -half };
	float y2{ -half - centerY };

	// 오른쪽 아래
	float x3{ half };
	float y3{ -half - centerY };

	// 회전
	float rx1{
		x1 * cosAngle - y1 * sinAngle
	};

	float ry1{
		x1 * sinAngle + y1 * cosAngle
	};

	float rx2{
		x2 * cosAngle - y2 * sinAngle
	};

	float ry2{
		x2 * sinAngle + y2 * cosAngle
	};

	float rx3{
		x3 * cosAngle - y3 * sinAngle
	};

	float ry3{
		x3 * sinAngle + y3 * cosAngle
	};

	AddVertex(
		vertices,
		triangle.x + rx1,
		triangle.y + ry1,
		triangle.r,
		triangle.g,
		triangle.b
	);

	AddVertex(
		vertices,
		triangle.x + rx2,
		triangle.y + ry2,
		triangle.r,
		triangle.g,
		triangle.b
	);

	AddVertex(
		vertices,
		triangle.x + rx3,
		triangle.y + ry3,
		triangle.r,
		triangle.g,
		triangle.b
	);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	if (fillMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// 이동 경로
void DrawPath()
{
	std::vector<float> vertices;

	float r{ 1.0f };
	float g{ 0.0f };
	float b{ 0.0f };

	// 1 : 튕기기
	if (moveMode == 1)
	{
		AddVertex(vertices, -0.9f, -0.5f, r, g, b);
		AddVertex(vertices, 0.6f, 0.9f, r, g, b);

		AddVertex(vertices, 0.6f, 0.9f, r, g, b);
		AddVertex(vertices, 0.9f, 0.3f, r, g, b);

		AddVertex(vertices, 0.9f, 0.3f, r, g, b);
		AddVertex(vertices, -0.1f, -0.9f, r, g, b);

		AddVertex(vertices, -0.1f, -0.9f, r, g, b);
		AddVertex(vertices, -0.9f, -0.5f, r, g, b);
	}

	// 2 : 좌우 지그재그
	else if (moveMode == 2)
	{
		float y{ 0.8f };
		bool right{ true };

		for (int i = 0; i < 7; ++i)
		{
			float x1{
				right ? -0.9f : 0.9f
			};

			float x2{
				right ? 0.9f : -0.9f
			};

			AddVertex(vertices, x1, y, r, g, b);
			AddVertex(vertices, x2, y, r, g, b);

			if (i < 6)
			{
				AddVertex(vertices, x2, y, r, g, b);
				AddVertex(vertices, x2, y - 0.25f, r, g, b);
			}

			y -= 0.25f;
			right = !right;
		}
	}

	// 3 : 상하 뾰족 지그재그
	else if (moveMode == 3)
	{
		float x{ -0.8f };
		float step{ 0.4f };

		AddVertex(vertices, x, 0.9f, r, g, b);

		for (int i = 0; i < 4; ++i)
		{
			AddVertex(
				vertices,
				x + step / 2.0f,
				-0.9f,
				r,
				g,
				b
			);

			AddVertex(
				vertices,
				x + step,
				0.9f,
				r,
				g,
				b
			);

			x += step;
		}
	}

	// 4 : 원 스파이럴
	else if (moveMode == 4)
	{
		const int POINT_COUNT{ 200 };

		for (int i = 1; i < POINT_COUNT; ++i)
		{
			float angle1{
				(i - 1) * 0.12f
			};

			float radius1{
				0.0035f * (i - 1)
			};

			float angle2{
				i * 0.12f
			};

			float radius2{
				0.0035f * i
			};

			float x1{
				radius1 * std::cos(angle1)
			};

			float y1{
				radius1 * std::sin(angle1)
			};

			float x2{
				radius2 * std::cos(angle2)
			};

			float y2{
				radius2 * std::sin(angle2)
			};

			AddVertex(vertices, x1, y1, r, g, b);
			AddVertex(vertices, x2, y2, r, g, b);
		}
	}

	if (vertices.empty())
	{
		return;
	}

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	glLineWidth(1.0f);

	glDrawArrays(
		GL_LINES,
		0,
		static_cast<GLsizei>(vertices.size() / 6)
	);
}

// 삼각형 생성
void CreateTriangle(int area, float x, float y)
{
	Triangle& triangle = triangles[area];

	triangle.x = x;
	triangle.y = y;

	triangle.size = sizeDist(gen);

	triangle.r = colorDist(gen);
	triangle.g = colorDist(gen);
	triangle.b = colorDist(gen);

	triangle.speed = speedDist(gen);

	triangle.dx = directionDist(gen);
	triangle.dy = directionDist(gen);

	// 방향이 너무 작아지는 경우 방지
	if (std::abs(triangle.dx) < 0.2f)
	{
		triangle.dx = 0.5f;
	}

	if (std::abs(triangle.dy) < 0.2f)
	{
		triangle.dy = 0.5f;
	}

	triangle.centerX = x;
	triangle.centerY = y;

	triangle.angle = 0.0f;
	triangle.radius = 0.0f;

	triangle.used = true;
}

// 크기 변경
void ChangeTriangleSize(int area)
{
	if (!triangles[area].used)
	{
		return;
	}

	std::uniform_real_distribution<float> scaleDist(0.7f, 1.3f);

	triangles[area].size *= scaleDist(gen);

	triangles[area].size = std::clamp(
		triangles[area].size,
		0.05f,
		0.30f
	);
}

// 전체 삼각형 이동
void UpdateTriangles()
{
	for (int i = 0; i < 4; ++i)
	{
		if (!triangles[i].used)
		{
			continue;
		}

		if (moveMode == 1)
		{
			UpdateBounce(triangles[i]);
		}
		else if (moveMode == 2)
		{
			UpdateHorizontalZigzag(triangles[i]);
		}
		else if (moveMode == 3)
		{
			UpdateVerticalZigzag(triangles[i]);
		}
		else if (moveMode == 4)
		{
			UpdateSpiral(triangles[i]);
		}
	}
}

// 1 : 튕기기
void UpdateBounce(Triangle& triangle)
{
	float half{
		triangle.size / 2.0f
	};

	triangle.x += triangle.dx * triangle.speed;
	triangle.y += triangle.dy * triangle.speed;

	// 좌우 충돌
	if (triangle.x + half >= 1.0f)
	{
		triangle.x = 1.0f - half;
		triangle.dx = -std::abs(triangle.dx);
	}
	else if (triangle.x - half <= -1.0f)
	{
		triangle.x = -1.0f + half;
		triangle.dx = std::abs(triangle.dx);
	}

	// 상하 충돌
	if (triangle.y + half >= 1.0f)
	{
		triangle.y = 1.0f - half;
		triangle.dy = -std::abs(triangle.dy);
	}
	else if (triangle.y - half <= -1.0f)
	{
		triangle.y = -1.0f + half;
		triangle.dy = std::abs(triangle.dy);
	}
}

// 2 : 좌우 지그재그
void UpdateHorizontalZigzag(Triangle& triangle)
{
	float half{
		triangle.size / 2.0f
	};

	triangle.x += triangle.dx * triangle.speed;

	// 오른쪽 충돌
	if (triangle.x + half >= 1.0f)
	{
		triangle.x = 1.0f - half;
		triangle.dx = -std::abs(triangle.dx);
		triangle.y -= 0.10f;
	}

	// 왼쪽 충돌
	else if (triangle.x - half <= -1.0f)
	{
		triangle.x = -1.0f + half;
		triangle.dx = std::abs(triangle.dx);
		triangle.y -= 0.10f;
	}

	// 아래까지 도착하면 위로 이동
	if (triangle.y - half <= -1.0f)
	{
		triangle.y = 1.0f - half;
	}
}

// 3 : 상하 뾰족 지그재그
void UpdateVerticalZigzag(Triangle& triangle)
{
	float half{
		triangle.size / 2.0f
	};

	triangle.x += triangle.dx * triangle.speed;
	triangle.y += triangle.dy * triangle.speed * 3.0f;

	// 위 충돌
	if (triangle.y + half >= 1.0f)
	{
		triangle.y = 1.0f - half;
		triangle.dy = -std::abs(triangle.dy);
		triangle.x += 0.05f;
	}

	// 아래 충돌
	else if (triangle.y - half <= -1.0f)
	{
		triangle.y = -1.0f + half;
		triangle.dy = std::abs(triangle.dy);
		triangle.x += 0.05f;
	}

	// 오른쪽 끝
	if (triangle.x + half >= 1.0f)
	{
		triangle.x = -1.0f + half;
	}
}

// 4 : 원 스파이럴
void UpdateSpiral(Triangle& triangle)
{
	float half{
		triangle.size / 2.0f
	};

	triangle.angle += triangle.speed * 15.0f;
	triangle.radius += triangle.speed * 0.15f;

	triangle.x =
		triangle.centerX +
		triangle.radius * std::cos(triangle.angle);

	triangle.y =
		triangle.centerY +
		triangle.radius * std::sin(triangle.angle);

	// 가장자리에 닿으면 방향 변경
	if (triangle.x + half >= 1.0f ||
		triangle.x - half <= -1.0f ||
		triangle.y + half >= 1.0f ||
		triangle.y - half <= -1.0f)
	{
		// 현재 위치를 새로운 중심으로 사용
		triangle.centerX = std::clamp(
			triangle.x,
			-0.8f,
			0.8f
		);

		triangle.centerY = std::clamp(
			triangle.y,
			-0.8f,
			0.8f
		);

		triangle.radius = 0.0f;
		triangle.angle += PI;
	}
}

// 이동 상태 초기화
void ResetMovement()
{
	for (int i = 0; i < 4; ++i)
	{
		if (!triangles[i].used)
		{
			continue;
		}

		triangles[i].dx = directionDist(gen);
		triangles[i].dy = directionDist(gen);

		if (std::abs(triangles[i].dx) < 0.2f)
		{
			triangles[i].dx = 0.5f;
		}

		if (std::abs(triangles[i].dy) < 0.2f)
		{
			triangles[i].dy = 0.5f;
		}

		triangles[i].centerX = triangles[i].x;
		triangles[i].centerY = triangles[i].y;

		triangles[i].angle = 0.0f;
		triangles[i].radius = 0.0f;
	}
}

// 사분면 검사
int GetArea(float x, float y)
{
	if (x >= 0.0f &&
		y >= 0.0f)
	{
		return 0;
	}

	if (x < 0.0f &&
		y >= 0.0f)
	{
		return 1;
	}

	if (x < 0.0f &&
		y < 0.0f)
	{
		return 2;
	}

	return 3;
}

// 키보드
void KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods
)
{
	if (action != GLFW_PRESS)
	{
		return;
	}

	// 면
	if (key == GLFW_KEY_A)
	{
		fillMode = true;
	}

	// 선
	else if (key == GLFW_KEY_B)
	{
		fillMode = false;
	}

	// 모두 삭제
	else if (key == GLFW_KEY_C)
	{
		for (int i = 0; i < 4; ++i)
		{
			triangles[i].used = false;
		}
	}

	// 종료
	else if (key == GLFW_KEY_Q)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// 1 : 튕기기
	else if (key == GLFW_KEY_1)
	{
		moveMode = 1;
		ResetMovement();
	}

	// 2 : 좌우 지그재그
	else if (key == GLFW_KEY_2)
	{
		moveMode = 2;
		ResetMovement();
	}

	// 3 : 상하 지그재그
	else if (key == GLFW_KEY_3)
	{
		moveMode = 3;
		ResetMovement();
	}

	// 4 : 원 스파이럴
	else if (key == GLFW_KEY_4)
	{
		moveMode = 4;
		ResetMovement();
	}
}

// 마우스
void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
)
{
	if (action != GLFW_PRESS)
	{
		return;
	}

	double mouseX;
	double mouseY;

	glfwGetCursorPos(
		window,
		&mouseX,
		&mouseY
	);

	int width;
	int height;

	glfwGetWindowSize(
		window,
		&width,
		&height
	);

	float x{
		static_cast<float>(
			mouseX / width * 2.0 - 1.0
		)
	};

	float y{
		static_cast<float>(
			1.0 - mouseY / height * 2.0
		)
	};

	int area{
		GetArea(x, y)
	};

	// 왼쪽 클릭
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		CreateTriangle(
			area,
			x,
			y
		);
	}

	// 오른쪽 클릭
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		ChangeTriangleSize(area);
	}
}

// 정점 추가
void AddVertex(
	std::vector<float>& vertices,
	float x,
	float y,
	float r,
	float g,
	float b
)
{
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(0.0f);

	vertices.push_back(r);
	vertices.push_back(g);
	vertices.push_back(b);
}