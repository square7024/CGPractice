#include <gl/glew.h>
#include <gl/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_SHAPE_COUNT 50

enum class ShapeType
{
	Point,
	Line,
	Triangle,
	Rect
};

struct Shape
{
	ShapeType type;

	float x;
	float y;

	float size;

	float r;
	float g;
	float b;
};

Shape shapes[MAX_SHAPE_COUNT];

int shapeCount{ 0 };
int selectedIndex{ -1 };

GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO;
GLuint VBO;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<float> positionDist(-0.8f, 0.8f);
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

const float MOVE_STEP{ 0.05f };

std::string filetobuf(const char* file);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

void InitBuffer();

void AddShape(ShapeType type);
void DrawScene();
void DrawShape(const Shape& shape, bool selected);

void MoveSelected(float dx, float dy);
void MoveAll(float dx, float dy);
void ClampShape(Shape& shape);

bool HitTest(const Shape& shape, float x, float y);
bool PointInTriangle(
	float px,
	float py,
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3
);

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
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		"Practice 7",
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

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	while (!glfwWindowShouldClose(window))
	{
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

// 버텍스 셰이더 만들기
void make_vertexShaders()
{
	std::string vertexSource = filetobuf("GLSL7vertex.glsl");
	const char* source = vertexSource.c_str();

	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(
		vertexShader,
		1,
		&source,
		NULL
	);

	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];

	glGetShaderiv(
		vertexShader,
		GL_COMPILE_STATUS,
		&result
	);

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

// 프래그먼트 셰이더 만들기
void make_fragmentShaders()
{
	std::string fragmentSource = filetobuf("GLSL7fragment.glsl");
	const char* source = fragmentSource.c_str();

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(
		fragmentShader,
		1,
		&source,
		NULL
	);

	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];

	glGetShaderiv(
		fragmentShader,
		GL_COMPILE_STATUS,
		&result
	);

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

// 셰이더 프로그램 만들기
GLuint make_shaderProgram()
{
	GLuint shaderID = glCreateProgram();

	glAttachShader(
		shaderID,
		vertexShader
	);

	glAttachShader(
		shaderID,
		fragmentShader
	);

	glLinkProgram(shaderID);

	GLint result;
	GLchar errorLog[512];

	glGetProgramiv(
		shaderID,
		GL_LINK_STATUS,
		&result
	);

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

// VAO와 VBO 생성
void InitBuffer()
{
	glGenVertexArrays(
		1,
		&VAO
	);

	glBindVertexArray(VAO);

	glGenBuffers(
		1,
		&VBO
	);

	glBindBuffer(
		GL_ARRAY_BUFFER,
		VBO
	);

	// 한 도형에서 최대 20개의 정점을 저장할 공간 확보
	// 정점 하나는 위치 3개 + 색상 3개 = float 6개
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(float) * 20 * 6,
		nullptr,
		GL_DYNAMIC_DRAW
	);

	// 위치 속성
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(void*)0
	);

	glEnableVertexAttribArray(0);

	// 색상 속성
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

// 새로운 도형 생성
void AddShape(ShapeType type)
{
	if (shapeCount >= MAX_SHAPE_COUNT)
	{
		return;
	}

	Shape& shape = shapes[shapeCount];

	shape.type = type;

	shape.x = positionDist(gen);
	shape.y = positionDist(gen);

	shape.size = 0.15f;

	shape.r = colorDist(gen);
	shape.g = colorDist(gen);
	shape.b = colorDist(gen);

	ClampShape(shape);

	++shapeCount;
}

// 화면 출력
void DrawScene()
{
	glClearColor(
		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	for (int i = 0; i < shapeCount; ++i)
	{
		DrawShape(
			shapes[i],
			i == selectedIndex
		);
	}
}

// 도형 그리기
void DrawShape(const Shape& shape, bool selected)
{
	std::vector<float> vertices;

	float x{ shape.x };
	float y{ shape.y };
	float size{ shape.size };

	float r{ shape.r };
	float g{ shape.g };
	float b{ shape.b };

	// 선택된 도형의 테두리 색상
	float borderR{ 1.0f };
	float borderG{ 0.0f };
	float borderB{ 0.0f };

	GLenum drawMode{ GL_TRIANGLES };
	int vertexCount{ 0 };

	// 점
	if (shape.type == ShapeType::Point)
	{
		float half{ 0.02f };

		AddVertex(
			vertices,
			x - half,
			y + half,
			r, g, b
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			r, g, b
		);

		AddVertex(
			vertices,
			x - half,
			y + half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y + half,
			r, g, b
		);

		drawMode = GL_TRIANGLES;
		vertexCount = 6;
	}

	// 선
	else if (shape.type == ShapeType::Line)
	{
		float half{ size };

		AddVertex(
			vertices,
			x - half,
			y,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y,
			r, g, b
		);

		drawMode = GL_LINES;
		vertexCount = 2;
	}

	// 삼각형
	else if (shape.type == ShapeType::Triangle)
	{
		float half{ size / 2.0f };

		AddVertex(
			vertices,
			x,
			y + half,
			r, g, b
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			r, g, b
		);

		drawMode = GL_TRIANGLES;
		vertexCount = 3;
	}

	// 사각형
	else if (shape.type == ShapeType::Rect)
	{
		float half{ size / 2.0f };

		// 첫 번째 삼각형
		AddVertex(
			vertices,
			x - half,
			y + half,
			r, g, b
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			r, g, b
		);

		// 두 번째 삼각형
		AddVertex(
			vertices,
			x - half,
			y + half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			r, g, b
		);

		AddVertex(
			vertices,
			x + half,
			y + half,
			r, g, b
		);

		drawMode = GL_TRIANGLES;
		vertexCount = 6;
	}

	glBindBuffer(
		GL_ARRAY_BUFFER,
		VBO
	);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	// 선
	if (shape.type == ShapeType::Line)
	{
		// 선택된 선이면 뒤에 굵은 빨간 선 그리기
		if (selected)
		{
			std::vector<float> borderVertices;

			float half{ size };

			AddVertex(
				borderVertices,
				x - half,
				y,
				1.0f, 0.0f, 0.0f
			);

			AddVertex(
				borderVertices,
				x + half,
				y,
				1.0f, 0.0f, 0.0f
			);

			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				borderVertices.size() * sizeof(float),
				borderVertices.data()
			);

			glLineWidth(7.0f);

			glDrawArrays(
				GL_LINES,
				0,
				2
			);

			// 원래 선의 정점 데이터를 다시 VBO에 넣기
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				vertices.size() * sizeof(float),
				vertices.data()
			);
		}

		// 원래 색상의 선
		glLineWidth(3.0f);

		glDrawArrays(
			GL_LINES,
			0,
			vertexCount
		);
	}

	// 점, 삼각형, 사각형
	else
	{
		glDrawArrays(
			drawMode,
			0,
			vertexCount
		);
	}

	// 선택된 삼각형 테두리
	if (selected && shape.type == ShapeType::Triangle)
	{
		float half{ size / 2.0f };

		vertices.clear();

		AddVertex(
			vertices,
			x,
			y + half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			borderR, borderG, borderB
		);

		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			vertices.size() * sizeof(float),
			vertices.data()
		);

		glLineWidth(2.0f);

		glDrawArrays(
			GL_LINE_LOOP,
			0,
			3
		);
	}

	// 선택된 점 테두리
	if (selected && shape.type == ShapeType::Point)
	{
		float half{ 0.02f };

		vertices.clear();

		AddVertex(
			vertices,
			x - half,
			y + half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x + half,
			y + half,
			borderR, borderG, borderB
		);

		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			vertices.size() * sizeof(float),
			vertices.data()
		);

		glLineWidth(2.0f);

		glDrawArrays(
			GL_LINE_LOOP,
			0,
			4
		);
	}

	// 선택된 사각형 테두리
	if (selected && shape.type == ShapeType::Rect)
	{
		float half{ size / 2.0f };

		vertices.clear();

		AddVertex(
			vertices,
			x - half,
			y + half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			borderR, borderG, borderB
		);

		AddVertex(
			vertices,
			x + half,
			y + half,
			borderR, borderG, borderB
		);

		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			vertices.size() * sizeof(float),
			vertices.data()
		);

		glLineWidth(2.0f);

		glDrawArrays(
			GL_LINE_LOOP,
			0,
			4
		);
	}
}

// 선택한 도형 이동
void MoveSelected(float dx, float dy)
{
	if (selectedIndex < 0 ||
		selectedIndex >= shapeCount)
	{
		return;
	}

	shapes[selectedIndex].x += dx;
	shapes[selectedIndex].y += dy;

	ClampShape(
		shapes[selectedIndex]
	);
}

// 모든 도형 이동
void MoveAll(float dx, float dy)
{
	for (int i = 0; i < shapeCount; ++i)
	{
		shapes[i].x += dx;
		shapes[i].y += dy;

		ClampShape(
			shapes[i]
		);
	}
}

// 화면 밖으로 나가지 않도록 제한
void ClampShape(Shape& shape)
{
	float marginX{ shape.size / 2.0f };
	float marginY{ shape.size / 2.0f };

	//--- 점
	if (shape.type == ShapeType::Point)
	{
		marginX = 0.02f;
		marginY = 0.02f;
	}

	//--- 선
	else if (shape.type == ShapeType::Line)
	{
		// 수평선이므로 좌우에만 길이가 있음
		marginX = shape.size;
		marginY = 0.0f;
	}

	shape.x = std::clamp(
		shape.x,
		-1.0f + marginX,
		1.0f - marginX
	);

	shape.y = std::clamp(
		shape.y,
		-1.0f + marginY,
		1.0f - marginY
	);
}

// 마우스로 도형 선택 검사
bool HitTest(
	const Shape& shape,
	float x,
	float y
)
{
	// 점
	if (shape.type == ShapeType::Point)
	{
		float half{ 0.02f };

		return
			x >= shape.x - half &&
			x <= shape.x + half &&
			y >= shape.y - half &&
			y <= shape.y + half;
	}

	// 선
	if (shape.type == ShapeType::Line)
	{
		float half{
			shape.size
		};

		const float lineRange{ 0.03f };

		return
			x >= shape.x - half &&
			x <= shape.x + half &&
			std::fabs(y - shape.y) <= lineRange;
	}

	// 삼각형
	if (shape.type == ShapeType::Triangle)
	{
		float half{
			shape.size / 2.0f
		};

		return PointInTriangle(
			x,
			y,
			shape.x,
			shape.y + half,
			shape.x - half,
			shape.y - half,
			shape.x + half,
			shape.y - half
		);
	}

	// 사각형
	if (shape.type == ShapeType::Rect)
	{
		float half{
			shape.size / 2.0f
		};

		return
			x >= shape.x - half &&
			x <= shape.x + half &&
			y >= shape.y - half &&
			y <= shape.y + half;
	}

	return false;
}

// 삼각형 내부 검사
bool PointInTriangle(
	float px,
	float py,
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3
)
{
	float d1{
		(px - x2) * (y1 - y2) -
		(x1 - x2) * (py - y2)
	};

	float d2{
		(px - x3) * (y2 - y3) -
		(x2 - x3) * (py - y3)
	};

	float d3{
		(px - x1) * (y3 - y1) -
		(x3 - x1) * (py - y1)
	};

	bool hasNegative{
		d1 < 0.0f ||
		d2 < 0.0f ||
		d3 < 0.0f
	};

	bool hasPositive{
		d1 > 0.0f ||
		d2 > 0.0f ||
		d3 > 0.0f
	};

	return !(hasNegative && hasPositive);
}

// 키보드 입력
void KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods
)
{
	if (action != GLFW_PRESS &&
		action != GLFW_REPEAT)
	{
		return;
	}

	// 도형 생성은 한 번 눌렀을 때만 실행
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_P)
		{
			AddShape(
				ShapeType::Point
			);
		}

		else if (key == GLFW_KEY_E)
		{
			AddShape(
				ShapeType::Line
			);
		}

		else if (key == GLFW_KEY_T)
		{
			AddShape(
				ShapeType::Triangle
			);
		}

		else if (key == GLFW_KEY_R)
		{
			AddShape(
				ShapeType::Rect
			);
		}

		else if (key == GLFW_KEY_C)
		{
			shapeCount = 0;
			selectedIndex = -1;
		}

		// 프로그램 종료
		else if (key == GLFW_KEY_Q)
		{
			glfwSetWindowShouldClose(window, true);
		}
	}

	// 선택한 도형 상하좌우 이동
	if (key == GLFW_KEY_W)
	{
		MoveSelected(
			0.0f,
			MOVE_STEP
		);
	}

	else if (key == GLFW_KEY_A)
	{
		MoveSelected(
			-MOVE_STEP,
			0.0f
		);
	}

	else if (key == GLFW_KEY_S)
	{
		MoveSelected(
			0.0f,
			-MOVE_STEP
		);
	}

	else if (key == GLFW_KEY_D)
	{
		MoveSelected(
			MOVE_STEP,
			0.0f
		);
	}

	// 선택한 도형 대각선 이동
	else if (key == GLFW_KEY_I)
	{
		// 좌상
		MoveSelected(
			-MOVE_STEP,
			MOVE_STEP
		);
	}

	else if (key == GLFW_KEY_L)
	{
		// 우상
		MoveSelected(
			MOVE_STEP,
			MOVE_STEP
		);
	}

	else if (key == GLFW_KEY_J)
	{
		// 좌하
		MoveSelected(
			-MOVE_STEP,
			-MOVE_STEP
		);
	}

	else if (key == GLFW_KEY_K)
	{
		// 우하
		MoveSelected(
			MOVE_STEP,
			-MOVE_STEP
		);
	}

	// 모든 도형 이동
	else if (key == GLFW_KEY_1)
	{
		// 좌
		MoveAll(
			-MOVE_STEP,
			0.0f
		);
	}

	else if (key == GLFW_KEY_2)
	{
		// 우
		MoveAll(
			MOVE_STEP,
			0.0f
		);
	}

	else if (key == GLFW_KEY_3)
	{
		// 상
		MoveAll(
			0.0f,
			MOVE_STEP
		);
	}

	else if (key == GLFW_KEY_4)
	{
		// 하
		MoveAll(
			0.0f,
			-MOVE_STEP
		);
	}
}

// 마우스 클릭으로 도형 선택
void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT ||
		action != GLFW_PRESS)
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

	selectedIndex = -1;

	// 나중에 그린 도형부터 검사
	for (int i = shapeCount - 1; i >= 0; --i)
	{
		if (HitTest(
			shapes[i],
			x,
			y
		))
		{
			selectedIndex = i;
			break;
		}
	}
}

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