#include <gl/glew.h>
#include <gl/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <algorithm>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct Triangle
{
	float x;
	float y;

	float size;

	float r;
	float g;
	float b;

	bool used;
};

Triangle triangles[4];

GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO;
GLuint VBO;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<float> sizeDist(0.10f, 0.25f);
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_real_distribution<float> scaleDist(0.5f, 1.8f);

// true: 면으로 그리기
// false: 선으로 그리기
bool fillMode{ true };

std::string filetobuf(const char* file);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

void InitBuffer();

void DrawScene();
void DrawAxis();
void DrawTriangle(const Triangle& triangle);

void CreateTriangle(
	int area,
	float x,
	float y
);

void ChangeTriangleSize(int area);

int GetArea(
	float x,
	float y
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

	glfwWindowHint(
		GLFW_CONTEXT_VERSION_MAJOR,
		3
	);

	glfwWindowHint(
		GLFW_CONTEXT_VERSION_MINOR,
		3
	);

	glfwWindowHint(
		GLFW_OPENGL_PROFILE,
		GLFW_OPENGL_CORE_PROFILE
	);

	GLFWwindow* window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		"Practice 8",
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

	// 처음에는 삼각형이 없음
	for (int i = 0; i < 4; ++i)
	{
		triangles[i].used = false;
	}

	glfwSetKeyCallback(
		window,
		KeyCallback
	);

	glfwSetMouseButtonCallback(
		window,
		MouseButtonCallback
	);

	while (!glfwWindowShouldClose(window))
	{
		DrawScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteBuffers(
		1,
		&VBO
	);

	glDeleteVertexArrays(
		1,
		&VAO
	);

	glDeleteProgram(
		shaderProgramID
	);

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
		std::cerr
			<< file
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

// 버텍스 셰이더 만들기
void make_vertexShaders()
{
	std::string vertexSource =
		filetobuf("GLSL8vertex.glsl");

	const char* source =
		vertexSource.c_str();

	vertexShader =
		glCreateShader(
			GL_VERTEX_SHADER
		);

	glShaderSource(
		vertexShader,
		1,
		&source,
		NULL
	);

	glCompileShader(
		vertexShader
	);

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
	std::string fragmentSource =
		filetobuf("GLSL8fragment.glsl");

	const char* source =
		fragmentSource.c_str();

	fragmentShader =
		glCreateShader(
			GL_FRAGMENT_SHADER
		);

	glShaderSource(
		fragmentShader,
		1,
		&source,
		NULL
	);

	glCompileShader(
		fragmentShader
	);

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
	GLuint shaderID =
		glCreateProgram();

	glAttachShader(
		shaderID,
		vertexShader
	);

	glAttachShader(
		shaderID,
		fragmentShader
	);

	glLinkProgram(
		shaderID
	);

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

	glDeleteShader(
		vertexShader
	);

	glDeleteShader(
		fragmentShader
	);

	glUseProgram(
		shaderID
	);

	return shaderID;
}

// VAO와 VBO 생성
void InitBuffer()
{
	glGenVertexArrays(
		1,
		&VAO
	);

	glBindVertexArray(
		VAO
	);

	glGenBuffers(
		1,
		&VBO
	);

	glBindBuffer(
		GL_ARRAY_BUFFER,
		VBO
	);

	// 최대 4개의 정점을 저장할 공간
	// 정점 하나는 위치 3개 + 색상 3개 = float 6개
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(float) * 4 * 6,
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

	glEnableVertexAttribArray(
		0
	);

	// 색상 속성
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(void*)(3 * sizeof(float))
	);

	glEnableVertexAttribArray(
		1
	);
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

	glClear(
		GL_COLOR_BUFFER_BIT
	);

	glUseProgram(
		shaderProgramID
	);

	glBindVertexArray(
		VAO
	);

	// x축, y축
	DrawAxis();

	// 삼각형 그리기
	for (int i = 0; i < 4; ++i)
	{
		if (triangles[i].used)
		{
			DrawTriangle(
				triangles[i]
			);
		}
	}
}

// x축, y축 그리기
void DrawAxis()
{
	std::vector<float> vertices;

	// x축
	AddVertex(
		vertices,
		-1.0f,
		0.0f,
		0.5f, 0.7f, 1.0f
	);

	AddVertex(
		vertices,
		1.0f,
		0.0f,
		0.5f, 0.7f, 1.0f
	);

	// y축
	AddVertex(
		vertices,
		0.0f,
		-1.0f,
		0.5f, 0.7f, 1.0f
	);

	AddVertex(
		vertices,
		0.0f,
		1.0f,
		0.5f, 0.7f, 1.0f
	);

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

	glLineWidth(
		1.0f
	);

	glDrawArrays(
		GL_LINES,
		0,
		4
	);
}

// 삼각형 그리기
void DrawTriangle(const Triangle& triangle)
{
	std::vector<float> vertices;

	float half{
		triangle.size / 2.0f
	};

	// 위
	AddVertex(
		vertices,
		triangle.x,
		triangle.y + triangle.size,
		triangle.r,
		triangle.g,
		triangle.b
	);

	// 왼쪽 아래
	AddVertex(
		vertices,
		triangle.x - half,
		triangle.y - half,
		triangle.r,
		triangle.g,
		triangle.b
	);

	// 오른쪽 아래
	AddVertex(
		vertices,
		triangle.x + half,
		triangle.y - half,
		triangle.r,
		triangle.g,
		triangle.b
	);

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

	if (fillMode)
	{
		glPolygonMode(
			GL_FRONT_AND_BACK,
			GL_FILL
		);
	}
	else
	{
		glPolygonMode(
			GL_FRONT_AND_BACK,
			GL_LINE
		);

		glLineWidth(
			2.0f
		);
	}

	glDrawArrays(
		GL_TRIANGLES,
		0,
		3
	);

	// 다음 그리기를 위해 기본값으로 복구
	glPolygonMode(
		GL_FRONT_AND_BACK,
		GL_FILL
	);
}

// 새로운 삼각형 생성
void CreateTriangle(
	int area,
	float x,
	float y
)
{
	Triangle& triangle =
		triangles[area];

	triangle.x = x;
	triangle.y = y;

	triangle.size =
		sizeDist(gen);
		
	triangle.r =
		colorDist(gen);

	triangle.g =
		colorDist(gen);

	triangle.b =
		colorDist(gen);

	triangle.used = true;

	// 이등변삼각형이 화면 밖이나
	// 다른 사분면으로 넘어가지 않도록 크기 제한

	// 좌우 방향은 size / 2 만큼 차지하므로 2배
	float maxSizeX{
		2.0f * std::min(
			1.0f - std::abs(x),
			std::abs(x)
		)
	};

	float maxSizeY;

	// 위쪽 사분면
	if (y >= 0.0f)
	{
		// 위쪽은 size만큼 차지
		float top{
			1.0f - y
		};

		// 아래쪽은 size / 2 만큼 차지
		float bottom{
			2.0f * y
		};

		maxSizeY =
			std::min(
				top,
				bottom
			);
	}

	// 아래쪽 사분면
	else
	{
		// 위쪽은 size만큼 차지
		float top{
			-y
		};

		// 아래쪽은 size / 2 만큼 차지
		float bottom{
			2.0f * (1.0f + y)
		};

		maxSizeY =
			std::min(
				top,
				bottom
			);
	}

	float maxSize{
		std::min(
			maxSizeX,
			maxSizeY
		)
	};

	if (triangle.size > maxSize)
	{
		triangle.size = maxSize;
	}
}

// 해당 사분면 삼각형 크기 변경
void ChangeTriangleSize(int area)
{
	if (!triangles[area].used)
	{
		return;
	}

	Triangle& triangle =
		triangles[area];

	float scale{
		scaleDist(gen)
	};

	triangle.size *= scale;

	// 최소 크기
	const float MIN_SIZE{
		0.05f
	};

	// 최대 크기
	const float MAX_SIZE{
		0.45f
	};

	triangle.size =
		std::clamp(
			triangle.size,
			MIN_SIZE,
			MAX_SIZE
		);

	// 좌우 방향 크기 제한
	float maxSizeX{
		2.0f * std::min(
			1.0f - std::abs(triangle.x),
			std::abs(triangle.x)
		)
	};

	float maxSizeY;

	// 위쪽 사분면
	if (triangle.y >= 0.0f)
	{
		float top{
			1.0f - triangle.y
		};

		float bottom{
			2.0f * triangle.y
		};

		maxSizeY =
			std::min(
				top,
				bottom
			);
	}

	// 아래쪽 사분면
	else
	{
		float top{
			-triangle.y
		};

		float bottom{
			2.0f * (1.0f + triangle.y)
		};

		maxSizeY =
			std::min(
				top,
				bottom
			);
	}

	float maxSize{
		std::min(
			maxSizeX,
			maxSizeY
		)
	};

	if (triangle.size > maxSize)
	{
		triangle.size = maxSize;
	}
}

// 좌표가 어느 사분면인지 검사
int GetArea(
	float x,
	float y
)
{
	// 1사분면
	if (x >= 0.0f &&
		y >= 0.0f)
	{
		return 0;
	}

	// 2사분면
	if (x < 0.0f &&
		y >= 0.0f)
	{
		return 1;
	}

	// 3사분면
	if (x < 0.0f &&
		y < 0.0f)
	{
		return 2;
	}

	// 4사분면
	return 3;
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
	if (action != GLFW_PRESS)
	{
		return;
	}

	// 면으로 그리기
	if (key == GLFW_KEY_A)
	{
		fillMode = true;
	}

	// 선으로 그리기
	else if (key == GLFW_KEY_B)
	{
		fillMode = false;
	}

	// 모든 도형 삭제
	else if (key == GLFW_KEY_C)
	{
		for (int i = 0; i < 4; ++i)
		{
			triangles[i].used = false;
		}
	}

	// 프로그램 종료
	else if (key == GLFW_KEY_Q)
	{
		glfwSetWindowShouldClose(
			window,
			true
		);
	}
}

// 마우스 입력
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

	// 마우스 좌표를 OpenGL 좌표로 변환
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
		GetArea(
			x,
			y
		)
	};

	// 왼쪽 클릭
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		// 해당 사분면의 기존 삼각형을
		// 새로운 삼각형으로 교체
		CreateTriangle(
			area,
			x,
			y
		);
	}

	// 오른쪽 클릭
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		// 해당 사분면의 삼각형 크기 변경
		ChangeTriangleSize(
			area
		);
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