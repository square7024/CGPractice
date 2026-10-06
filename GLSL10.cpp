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

enum ShapeType
{
	RECTANGLE,
	EQUILATERAL_TRIANGLE,
	RIGHT_TRIANGLE
};

struct Shape
{
	ShapeType type;

	float x;
	float y;

	float size;
	float angle;

	float r;
	float g;
	float b;

	bool used;
	bool fixed;

	int boardIndex;
	int slotIndex;
};

struct Slot
{
	ShapeType type;

	float x;
	float y;

	float size;
	float angle;

	bool filled;
	int shapeIndex;
};

struct Board
{
	Slot slots[8];
	int slotCount;

	bool complete;
};

struct PieceSpec
{
	ShapeType type;
	float size;
	float angle;
};

Shape shapes[24];
int shapeCount{ 0 };

Board boards[5];

GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO;
GLuint VBO;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
std::uniform_real_distribution<float> xDist(-0.90f, 0.10f);
std::uniform_real_distribution<float> yDist(-0.90f, 0.90f);

const float DIVIDER_X{ 0.25f };

int selectedShape{ -1 };

bool dragging{ false };

float dragOffsetX{ 0.0f };
float dragOffsetY{ 0.0f };

float dragStartX{ 0.0f };
float dragStartY{ 0.0f };

int dragOldBoard{ -1 };
int dragOldSlot{ -1 };

std::string filetobuf(const char* file);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

void InitBuffer();

void InitBoards();
void AddSlot(
	Board& board,
	ShapeType type,
	float x,
	float y,
	float size,
	float angle
);

void CreateShapes();
void RandomizeShapePosition(int index);
void ResetGame();

void DrawScene();
void DrawDivider();
void DrawBoards();
void DrawShapes();

void DrawShape(
	ShapeType type,
	float x,
	float y,
	float size,
	float angle,
	float r,
	float g,
	float b,
	bool filled
);

bool IsInsideShape(
	const Shape& shape,
	float x,
	float y
);

bool PointInTriangle(
	float px,
	float py,
	float ax,
	float ay,
	float bx,
	float by,
	float cx,
	float cy
);

bool AngleSame(float a, float b);

bool FindSnapTarget(
	const Shape& shape,
	int& boardIndex,
	int& slotIndex
);

void CheckBoardComplete(int boardIndex);

void GetMousePosition(
	GLFWwindow* window,
	double mouseX,
	double mouseY,
	float& x,
	float& y
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

void CursorPositionCallback(
	GLFWwindow* window,
	double xpos,
	double ypos
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
	glfwWindowHint(
		GLFW_OPENGL_PROFILE,
		GLFW_OPENGL_CORE_PROFILE
	);

	GLFWwindow* window{
		glfwCreateWindow(
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			"Practice 10",
			nullptr,
			nullptr
		)
	};

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
	ResetGame();

	glfwSetKeyCallback(
		window,
		KeyCallback
	);

	glfwSetMouseButtonCallback(
		window,
		MouseButtonCallback
	);

	glfwSetCursorPosCallback(
		window,
		CursorPositionCallback
	);

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

// 버텍스 셰이더
void make_vertexShaders()
{
	std::string vertexSource{
		filetobuf("GLSL10vertex.glsl")
	};

	const char* source{
		vertexSource.c_str()
	};

	vertexShader =
		glCreateShader(GL_VERTEX_SHADER);

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

// 프래그먼트 셰이더
void make_fragmentShaders()
{
	std::string fragmentSource{
		filetobuf("GLSL10fragment.glsl")
	};

	const char* source{
		fragmentSource.c_str()
	};

	fragmentShader =
		glCreateShader(GL_FRAGMENT_SHADER);

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

// 셰이더 프로그램
GLuint make_shaderProgram()
{
	GLuint shaderID{
		glCreateProgram()
	};

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

// VAO / VBO
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

	glBufferData(
		GL_ARRAY_BUFFER,
		20000 * sizeof(float),
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

// 모양판에 슬롯 추가
void AddSlot(
	Board& board,
	ShapeType type,
	float x,
	float y,
	float size,
	float angle
)
{
	int index{
		board.slotCount
	};

	board.slots[index].type = type;

	board.slots[index].x = x;
	board.slots[index].y = y;

	board.slots[index].size = size;
	board.slots[index].angle = angle;

	board.slots[index].filled = false;
	board.slots[index].shapeIndex = -1;

	board.slotCount++;
}

// 모양판 초기화
void InitBoards()
{
	for (int i = 0; i < 5; ++i)
	{
		boards[i].slotCount = 0;
		boards[i].complete = false;
	}

	// 1번 모양판 : 사각형 4개
	AddSlot(
		boards[0],
		RECTANGLE,
		0.60f,
		0.82f,
		0.065f,
		0.0f
	);

	AddSlot(
		boards[0],
		RECTANGLE,
		0.68f,
		0.82f,
		0.065f,
		0.0f
	);

	AddSlot(
		boards[0],
		RECTANGLE,
		0.60f,
		0.74f,
		0.065f,
		0.0f
	);

	AddSlot(
		boards[0],
		RECTANGLE,
		0.68f,
		0.74f,
		0.065f,
		0.0f
	);

	// 2번 모양판 : 정삼각형 4개
	AddSlot(
		boards[1],
		EQUILATERAL_TRIANGLE,
		0.64f,
		0.51f,
		0.10f,
		PI
	);

	AddSlot(
		boards[1],
		EQUILATERAL_TRIANGLE,
		0.64f,
		0.35f,
		0.10f,
		0.0f
	);

	AddSlot(
		boards[1],
		EQUILATERAL_TRIANGLE,
		0.56f,
		0.43f,
		0.10f,
		-PI / 2.0f
	);

	AddSlot(
		boards[1],
		EQUILATERAL_TRIANGLE,
		0.72f,
		0.43f,
		0.10f,
		PI / 2.0f
	);

	// 3번 모양판 : 직각삼각형 2개
	AddSlot(
		boards[2],
		RIGHT_TRIANGLE,
		0.64f,
		0.09f,
		0.17f,
		0.0f
	);

	AddSlot(
		boards[2],
		RIGHT_TRIANGLE,
		0.64f,
		0.09f,
		0.17f,
		PI
	);

	// 4번 모양판 : 집
	AddSlot(
		boards[3],
		RECTANGLE,
		0.64f,
		-0.27f,
		0.10f,
		0.0f
	);

	AddSlot(
		boards[3],
		EQUILATERAL_TRIANGLE,
		0.64f,
		-0.17f,
		0.11f,
		0.0f
	);

	// 5번 모양판 : 화살표
	AddSlot(
		boards[4],
		RECTANGLE,
		0.59f,
		-0.65f,
		0.08f,
		0.0f
	);

	AddSlot(
		boards[4],
		EQUILATERAL_TRIANGLE,
		0.69f,
		-0.65f,
		0.11f,
		-PI / 2.0f
	);
}

// 작은 도형 생성
void CreateShapes()
{
	shapeCount = 0;

	std::vector<PieceSpec> specs;

	// 모든 모양판을 완성할 수 있는 도형 생성
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0;
			j < boards[i].slotCount;
			++j)
		{
			PieceSpec spec;

			spec.type =
				boards[i].slots[j].type;

			spec.size =
				boards[i].slots[j].size;

			spec.angle =
				boards[i].slots[j].angle;

			specs.push_back(spec);
		}
	}

	// 추가 도형 개수 랜덤
	std::uniform_int_distribution<int> extraDist(
		0,
		2
	);

	std::uniform_int_distribution<int> angleDist(
		0,
		3
	);

	std::uniform_real_distribution<float> extraSize(
		0.055f,
		0.09f
	);

	// 종류별로 0~2개 추가
	for (int type = 0;
		type < 3;
		++type)
	{
		int extraCount{
			extraDist(gen)
		};

		for (int i = 0;
			i < extraCount;
			++i)
		{
			PieceSpec spec;

			spec.type =
				static_cast<ShapeType>(type);

			spec.size =
				extraSize(gen);

			spec.angle = 0.0f;

			if (type != RECTANGLE)
			{
				int direction{
					angleDist(gen)
				};

				spec.angle =
					direction *
					PI / 2.0f;
			}

			specs.push_back(spec);
		}
	}

	// 생성 순서도 랜덤
	std::shuffle(
		specs.begin(),
		specs.end(),
		gen
	);

	for (int i = 0;
		i < static_cast<int>(specs.size());
		++i)
	{
		if (shapeCount >= 24)
		{
			break;
		}

		Shape& shape{
			shapes[shapeCount]
		};

		shape.type =
			specs[i].type;

		shape.size =
			specs[i].size;

		shape.angle =
			specs[i].angle;

		shape.r = colorDist(gen);
		shape.g = colorDist(gen);
		shape.b = colorDist(gen);

		shape.used = true;
		shape.fixed = false;

		shape.boardIndex = -1;
		shape.slotIndex = -1;

		RandomizeShapePosition(
			shapeCount
		);

		shapeCount++;
	}
}

// 왼쪽 영역 랜덤 위치
void RandomizeShapePosition(int index)
{
	for (int attempt = 0;
		attempt < 300;
		++attempt)
	{
		float x{
			xDist(gen)
		};

		float y{
			yDist(gen)
		};

		bool overlap{ false };

		for (int i = 0;
			i < index;
			++i)
		{
			float dx{
				x - shapes[i].x
			};

			float dy{
				y - shapes[i].y
			};

			float distance{
				std::sqrt(
					dx * dx +
					dy * dy
				)
			};

			float minDistance{
				(shapes[index].size +
					shapes[i].size) *
				0.65f
			};

			if (distance < minDistance)
			{
				overlap = true;
				break;
			}
		}

		if (!overlap)
		{
			shapes[index].x = x;
			shapes[index].y = y;

			return;
		}
	}

	// 위치를 찾지 못했을 경우
	shapes[index].x = xDist(gen);
	shapes[index].y = yDist(gen);
}

// 전체 리셋
void ResetGame()
{
	selectedShape = -1;
	dragging = false;

	dragOldBoard = -1;
	dragOldSlot = -1;

	InitBoards();
	CreateShapes();
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

	glBindVertexArray(VAO);

	DrawDivider();
	DrawBoards();
	DrawShapes();
}

// 화면 구분선
void DrawDivider()
{
	std::vector<float> vertices;

	AddVertex(
		vertices,
		DIVIDER_X,
		-0.95f,
		0.2f,
		0.5f,
		0.8f
	);

	AddVertex(
		vertices,
		DIVIDER_X,
		0.95f,
		0.2f,
		0.5f,
		0.8f
	);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	glLineWidth(1.5f);

	glDrawArrays(
		GL_LINES,
		0,
		2
	);
}

// 모양판 출력
void DrawBoards()
{
	for (int i = 0;
		i < 5;
		++i)
	{
		for (int j = 0;
			j < boards[i].slotCount;
			++j)
		{
			Slot& slot{
				boards[i].slots[j]
			};

			DrawShape(
				slot.type,
				slot.x,
				slot.y,
				slot.size,
				slot.angle,
				0.2f,
				0.5f,
				0.8f,
				false
			);
		}
	}
}

// 작은 도형 출력
void DrawShapes()
{
	for (int i = 0;
		i < shapeCount;
		++i)
	{
		if (!shapes[i].used)
		{
			continue;
		}

		DrawShape(
			shapes[i].type,
			shapes[i].x,
			shapes[i].y,
			shapes[i].size,
			shapes[i].angle,
			shapes[i].r,
			shapes[i].g,
			shapes[i].b,
			true
		);
	}
}

// 도형 출력
void DrawShape(
	ShapeType type,
	float x,
	float y,
	float size,
	float angle,
	float r,
	float g,
	float b,
	bool filled
)
{
	std::vector<float> vertices;

	std::vector<float> localX;
	std::vector<float> localY;

	// 사각형
	if (type == RECTANGLE)
	{
		float half{
			size / 2.0f
		};

		localX.push_back(-half);
		localY.push_back(half);

		localX.push_back(-half);
		localY.push_back(-half);

		localX.push_back(half);
		localY.push_back(-half);

		localX.push_back(half);
		localY.push_back(half);
	}

	// 정삼각형
	else if (type == EQUILATERAL_TRIANGLE)
	{
		float height{
			size * 0.8660254f
		};

		localX.push_back(0.0f);
		localY.push_back(
			height * 2.0f / 3.0f
		);

		localX.push_back(
			-size / 2.0f
		);

		localY.push_back(
			-height / 3.0f
		);

		localX.push_back(
			size / 2.0f
		);

		localY.push_back(
			-height / 3.0f
		);
	}

	// 직각삼각형
	else if (type == RIGHT_TRIANGLE)
	{
		float half{
			size / 2.0f
		};

		localX.push_back(-half);
		localY.push_back(-half);

		localX.push_back(-half);
		localY.push_back(half);

		localX.push_back(half);
		localY.push_back(-half);
	}

	float cosAngle{
		std::cos(angle)
	};

	float sinAngle{
		std::sin(angle)
	};

	for (int i = 0;
		i < static_cast<int>(localX.size());
		++i)
	{
		float rx{
			localX[i] * cosAngle -
			localY[i] * sinAngle
		};

		float ry{
			localX[i] * sinAngle +
			localY[i] * cosAngle
		};

		AddVertex(
			vertices,
			x + rx,
			y + ry,
			r,
			g,
			b
		);
	}

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	if (filled)
	{
		if (type == RECTANGLE)
		{
			glDrawArrays(
				GL_TRIANGLE_FAN,
				0,
				4
			);
		}
		else
		{
			glDrawArrays(
				GL_TRIANGLES,
				0,
				3
			);
		}
	}
	else
	{
		glLineWidth(1.5f);

		glDrawArrays(
			GL_LINE_LOOP,
			0,
			static_cast<GLsizei>(
				localX.size()
				)
		);
	}
}

// 점이 도형 안에 있는지 검사
bool IsInsideShape(
	const Shape& shape,
	float x,
	float y
)
{
	float dx{
		x - shape.x
	};

	float dy{
		y - shape.y
	};

	float cosAngle{
		std::cos(shape.angle)
	};

	float sinAngle{
		std::sin(shape.angle)
	};

	// 회전을 반대로 적용
	float localX{
		dx * cosAngle +
		dy * sinAngle
	};

	float localY{
		-dx * sinAngle +
		dy * cosAngle
	};

	if (shape.type == RECTANGLE)
	{
		float half{
			shape.size / 2.0f
		};

		return
			std::abs(localX) <= half &&
			std::abs(localY) <= half;
	}

	if (shape.type ==
		EQUILATERAL_TRIANGLE)
	{
		float height{
			shape.size * 0.8660254f
		};

		return PointInTriangle(
			localX,
			localY,

			0.0f,
			height * 2.0f / 3.0f,

			-shape.size / 2.0f,
			-height / 3.0f,

			shape.size / 2.0f,
			-height / 3.0f
		);
	}

	float half{
		shape.size / 2.0f
	};

	return PointInTriangle(
		localX,
		localY,

		-half,
		-half,

		-half,
		half,

		half,
		-half
	);
}

// 삼각형 내부 검사
bool PointInTriangle(
	float px,
	float py,
	float ax,
	float ay,
	float bx,
	float by,
	float cx,
	float cy
)
{
	float d1{
		(px - bx) * (ay - by) -
		(ax - bx) * (py - by)
	};

	float d2{
		(px - cx) * (by - cy) -
		(bx - cx) * (py - cy)
	};

	float d3{
		(px - ax) * (cy - ay) -
		(cx - ax) * (py - ay)
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

// 두 각도가 같은지 검사
bool AngleSame(float a, float b)
{
	float difference{
		std::abs(a - b)
	};

	while (difference >
		2.0f * PI)
	{
		difference -=
			2.0f * PI;
	}

	if (difference > PI)
	{
		difference =
			2.0f * PI -
			difference;
	}

	return difference < 0.05f;
}

// 가장 가까운 맞는 슬롯 찾기
bool FindSnapTarget(
	const Shape& shape,
	int& boardIndex,
	int& slotIndex
)
{
	float bestDistance{
		1000.0f
	};

	boardIndex = -1;
	slotIndex = -1;

	for (int i = 0;
		i < 5;
		++i)
	{
		if (boards[i].complete)
		{
			continue;
		}

		for (int j = 0;
			j < boards[i].slotCount;
			++j)
		{
			Slot& slot{
				boards[i].slots[j]
			};

			if (slot.filled)
			{
				continue;
			}

			if (slot.type != shape.type)
			{
				continue;
			}

			if (!AngleSame(
				slot.angle,
				shape.angle
			))
			{
				continue;
			}

			if (std::abs(
				slot.size -
				shape.size
			) > 0.01f)
			{
				continue;
			}

			float dx{
				shape.x - slot.x
			};

			float dy{
				shape.y - slot.y
			};

			float distance{
				std::sqrt(
					dx * dx +
					dy * dy
				)
			};

			float snapDistance{
				slot.size * 0.9f +
				0.04f
			};

			if (distance <= snapDistance &&
				distance < bestDistance)
			{
				bestDistance = distance;

				boardIndex = i;
				slotIndex = j;
			}
		}
	}

	return
		boardIndex != -1 &&
		slotIndex != -1;
}

// 모양판 완성 검사
void CheckBoardComplete(int boardIndex)
{
	bool complete{ true };

	for (int i = 0;
		i < boards[boardIndex].slotCount;
		++i)
	{
		if (!boards[boardIndex].
			slots[i].filled)
		{
			complete = false;
			break;
		}
	}

	boards[boardIndex].complete =
		complete;

	if (!complete)
	{
		return;
	}

	// 완성되면 도형 고정
	for (int i = 0;
		i < shapeCount;
		++i)
	{
		if (shapes[i].boardIndex ==
			boardIndex)
		{
			shapes[i].fixed = true;
		}
	}
}

// 마우스 좌표를 OpenGL 좌표로 변경
void GetMousePosition(
	GLFWwindow* window,
	double mouseX,
	double mouseY,
	float& x,
	float& y
)
{
	int width;
	int height;

	glfwGetWindowSize(
		window,
		&width,
		&height
	);

	x =
		static_cast<float>(
			mouseX /
			width *
			2.0 -
			1.0
			);

	y =
		static_cast<float>(
			1.0 -
			mouseY /
			height *
			2.0
			);
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

	// 리셋
	if (key == GLFW_KEY_R)
	{
		ResetGame();
	}

	// 종료
	else if (key == GLFW_KEY_Q)
	{
		glfwSetWindowShouldClose(
			window,
			true
		);
	}
}

// 마우스 버튼
void MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods
)
{
	if (button !=
		GLFW_MOUSE_BUTTON_LEFT)
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

	float x;
	float y;

	GetMousePosition(
		window,
		mouseX,
		mouseY,
		x,
		y
	);

	// 클릭
	if (action == GLFW_PRESS)
	{
		selectedShape = -1;

		// 뒤쪽부터 검사
		for (int i = shapeCount - 1;
			i >= 0;
			--i)
		{
			if (shapes[i].fixed)
			{
				continue;
			}

			if (IsInsideShape(
				shapes[i],
				x,
				y
			))
			{
				selectedShape = i;
				break;
			}
		}

		if (selectedShape == -1)
		{
			return;
		}

		Shape& shape{
			shapes[selectedShape]
		};

		dragging = true;

		dragOffsetX =
			shape.x - x;

		dragOffsetY =
			shape.y - y;

		dragStartX =
			shape.x;

		dragStartY =
			shape.y;

		dragOldBoard =
			shape.boardIndex;

		dragOldSlot =
			shape.slotIndex;

		// 미완성 모양판에 놓여 있던 도형이면 슬롯 해제
		if (dragOldBoard >= 0 &&
			!boards[dragOldBoard].complete)
		{
			boards[dragOldBoard].
				slots[dragOldSlot].
				filled = false;

			boards[dragOldBoard].
				slots[dragOldSlot].
				shapeIndex = -1;

			shape.boardIndex = -1;
			shape.slotIndex = -1;
		}
	}

	// 마우스 놓기
	else if (action == GLFW_RELEASE)
	{
		if (!dragging ||
			selectedShape == -1)
		{
			return;
		}

		Shape& shape{
			shapes[selectedShape]
		};

		int boardIndex;
		int slotIndex;

		// 맞는 슬롯 발견
		if (FindSnapTarget(
			shape,
			boardIndex,
			slotIndex
		))
		{
			Slot& slot{
				boards[boardIndex].
				slots[slotIndex]
			};

			shape.x = slot.x;
			shape.y = slot.y;

			shape.boardIndex =
				boardIndex;

			shape.slotIndex =
				slotIndex;

			slot.filled = true;

			slot.shapeIndex =
				selectedShape;

			CheckBoardComplete(
				boardIndex
			);
		}

		// 맞지 않는 위치에 놓음
		else
		{
			// 원래 슬롯에 있던 도형
			if (dragOldBoard >= 0)
			{
				Slot& oldSlot{
					boards[dragOldBoard].
						slots[dragOldSlot]
				};

				shape.x =
					oldSlot.x;

				shape.y =
					oldSlot.y;

				shape.boardIndex =
					dragOldBoard;

				shape.slotIndex =
					dragOldSlot;

				oldSlot.filled = true;

				oldSlot.shapeIndex =
					selectedShape;
			}

			// 일반 도형
			else
			{
				shape.x =
					dragStartX;

				shape.y =
					dragStartY;
			}
		}

		selectedShape = -1;
		dragging = false;

		dragOldBoard = -1;
		dragOldSlot = -1;
	}
}

// 마우스 드래그
void CursorPositionCallback(
	GLFWwindow* window,
	double xpos,
	double ypos
)
{
	if (!dragging ||
		selectedShape == -1)
	{
		return;
	}

	float x;
	float y;

	GetMousePosition(
		window,
		xpos,
		ypos,
		x,
		y
	);

	shapes[selectedShape].x =
		x + dragOffsetX;

	shapes[selectedShape].y =
		y + dragOffsetY;
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