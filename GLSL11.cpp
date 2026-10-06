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
#define WINDOW_HEIGHT 800

#define MAX_OBSTACLES 300

enum ShapeType
{
	TRIANGLE,
	RECTANGLE,
	INVERTED_TRIANGLE
};

struct Obstacle
{
	int row;
	int col;

	ShapeType type;

	float size;

	float r;
	float g;
	float b;

	bool used;
};

struct Player
{
	int row;
	int col;

	ShapeType type;

	float r;
	float g;
	float b;
};

GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO;
GLuint VBO;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<float> colorDist(
	0.0f,
	1.0f
);

std::uniform_real_distribution<float> sizeDist(
	0.45f,
	0.80f
);

std::uniform_int_distribution<int> shapeDist(
	0,
	2
);

const float BOARD_LEFT{ -0.90f };
const float BOARD_RIGHT{ 0.90f };
const float BOARD_TOP{ 0.90f };
const float BOARD_BOTTOM{ -0.90f };

int boardCols{ 20 };
int boardRows{ 20 };

Obstacle obstacles[MAX_OBSTACLES];
int obstacleCount{ 0 };

Player player;

bool moving{ true };
bool finished{ false };

// 한 칸 이동하는 시간
double moveInterval{ 0.20 };
double lastMoveTime{ 0.0 };

// 충돌 효과
double collisionEndTime{ 0.0 };

std::string filetobuf(const char* file);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

void InitBuffer();

void InputBoardSize();

void ResetGame();
void CreateObstacles();

void UpdatePlayer();
void MovePlayerOneCell();

void CheckCollision();

void DrawScene();
void DrawBoard();
void DrawObstacles();
void DrawPlayer();
void DrawCollisionEffect();

void DrawShape(
	ShapeType type,
	float x,
	float y,
	float size,
	float r,
	float g,
	float b
);

void DrawSquare(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b
);

void DrawTriangle(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b,
	bool inverted
);

void GetCellCenter(
	int row,
	int col,
	float& x,
	float& y
);

float GetCellWidth();
float GetCellHeight();
float GetShapeBaseSize();

int FindObstacle(
	int row,
	int col
);

void KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
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
	InputBoardSize();

	if (!glfwInit())
	{
		std::cerr
			<< "Failed to initialize GLFW!"
			<< std::endl;

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

	GLFWwindow* window{
		glfwCreateWindow(
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			"Practice 11",
			nullptr,
			nullptr
		)
	};

	if (!window)
	{
		std::cerr
			<< "Failed to create window!"
			<< std::endl;

		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cerr
			<< "Failed to initialize GLEW!"
			<< std::endl;

		glfwDestroyWindow(window);
		glfwTerminate();

		return -1;
	}

	make_vertexShaders();
	make_fragmentShaders();

	shaderProgramID =
		make_shaderProgram();

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

	while (!glfwWindowShouldClose(window))
	{
		UpdatePlayer();
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

// 보드 크기 입력
void InputBoardSize()
{
	while (true)
	{
		std::cout
			<< "Enter board width (10~30): ";

		std::cin >> boardCols;

		if (boardCols >= 10 &&
			boardCols <= 30)
		{
			break;
		}

		std::cout
			<< "Please enter a value between 10 and 30."
			<< std::endl;
	}

	while (true)
	{
		std::cout
			<< "Enter board height (10~30): ";

		std::cin >> boardRows;

		if (boardRows >= 10 &&
			boardRows <= 30)
		{
			break;
		}

		std::cout
			<< "Please enter a value between 10 and 30."
			<< std::endl;
	}
}

// 셰이더 파일 읽기
std::string filetobuf(const char* file)
{
	std::ifstream shaderFile(file);

	if (!shaderFile.is_open())
	{
		std::cerr
			<< file
			<< " cannot be opened."
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
		filetobuf("GLSL11vertex.glsl")
	};

	const char* source{
		vertexSource.c_str()
	};

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
			<< "ERROR: Vertex shader compilation failed\n"
			<< errorLog
			<< std::endl;
	}
}

// 프래그먼트 셰이더
void make_fragmentShaders()
{
	std::string fragmentSource{
		filetobuf("GLSL11fragment.glsl")
	};

	const char* source{
		fragmentSource.c_str()
	};

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
			<< "ERROR: Fragment shader compilation failed\n"
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
			<< "ERROR: Shader program linking failed\n"
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

// 게임 초기화
void ResetGame()
{
	player.row = 0;
	player.col = 0;

	player.type = RECTANGLE;

	player.r = 0.1f;
	player.g = 0.4f;
	player.b = 1.0f;

	moving = true;
	finished = false;

	moveInterval = 0.20;

	lastMoveTime =
		glfwGetTime();

	collisionEndTime = 0.0;

	CreateObstacles();
}

// 장애물 생성
void CreateObstacles()
{
	obstacleCount = 0;

	int totalCells{
		boardRows * boardCols
	};

	int wantedCount{
		totalCells / 4
	};

	if (wantedCount > MAX_OBSTACLES)
	{
		wantedCount =
			MAX_OBSTACLES;
	}

	std::uniform_int_distribution<int> rowDist(
		0,
		boardRows - 1
	);

	std::uniform_int_distribution<int> colDist(
		0,
		boardCols - 1
	);

	while (obstacleCount < wantedCount)
	{
		int row{
			rowDist(gen)
		};

		int col{
			colDist(gen)
		};

		// 시작 위치에는 장애물을 놓지 않음
		if (row == 0 &&
			col == 0)
		{
			continue;
		}

		// 같은 칸 중복 방지
		if (FindObstacle(
			row,
			col
		) != -1)
		{
			continue;
		}

		Obstacle& obstacle{
			obstacles[obstacleCount]
		};

		obstacle.row = row;
		obstacle.col = col;

		obstacle.type =
			static_cast<ShapeType>(
				shapeDist(gen)
				);

		obstacle.size =
			sizeDist(gen);

		obstacle.r =
			colorDist(gen);

		obstacle.g =
			colorDist(gen);

		obstacle.b =
			colorDist(gen);

		obstacle.used = true;

		obstacleCount++;
	}
}

// 주인공 업데이트
void UpdatePlayer()
{
	if (!moving ||
		finished)
	{
		return;
	}

	double currentTime{
		glfwGetTime()
	};

	if (currentTime -
		lastMoveTime <
		moveInterval)
	{
		return;
	}

	lastMoveTime =
		currentTime;

	MovePlayerOneCell();

	CheckCollision();
}

// 한 칸 이동
void MovePlayerOneCell()
{
	// 짝수 번째 줄은 오른쪽으로 이동
	if (player.row % 2 == 0)
	{
		if (player.col <
			boardCols - 1)
		{
			player.col++;
		}
		else
		{
			if (player.row <
				boardRows - 1)
			{
				player.row++;
			}
			else
			{
				finished = true;
				moving = false;
			}
		}
	}

	// 홀수 번째 줄은 왼쪽으로 이동
	else
	{
		if (player.col > 0)
		{
			player.col--;
		}
		else
		{
			if (player.row <
				boardRows - 1)
			{
				player.row++;
			}
			else
			{
				finished = true;
				moving = false;
			}
		}
	}

	// 마지막 칸 도착 검사
	if (player.row ==
		boardRows - 1)
	{
		if (player.row % 2 == 0)
		{
			if (player.col ==
				boardCols - 1)
			{
				finished = true;
				moving = false;
			}
		}
		else
		{
			if (player.col == 0)
			{
				finished = true;
				moving = false;
			}
		}
	}
}

// 충돌 검사
void CheckCollision()
{
	int index{
		FindObstacle(
			player.row,
			player.col
		)
	};

	if (index == -1)
	{
		return;
	}

	Obstacle& obstacle{
		obstacles[index]
	};

	// 주인공과 장애물 모양 교환
	ShapeType temp{
		player.type
	};

	player.type =
		obstacle.type;

	obstacle.type =
		temp;

	// 충돌 효과 시간
	collisionEndTime =
		glfwGetTime() +
		0.30;
}

// 현재 칸의 장애물 찾기
int FindObstacle(
	int row,
	int col
)
{
	for (int i = 0;
		i < obstacleCount;
		++i)
	{
		if (!obstacles[i].used)
		{
			continue;
		}

		if (obstacles[i].row == row &&
			obstacles[i].col == col)
		{
			return i;
		}
	}

	return -1;
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

	DrawBoard();
	DrawObstacles();
	DrawPlayer();
	DrawCollisionEffect();
}

// 보드판 출력
void DrawBoard()
{
	std::vector<float> vertices;

	float cellWidth{
		GetCellWidth()
	};

	float cellHeight{
		GetCellHeight()
	};

	// 세로선
	for (int col = 0;
		col <= boardCols;
		++col)
	{
		float x{
			BOARD_LEFT +
			cellWidth * col
		};

		AddVertex(
			vertices,
			x,
			BOARD_BOTTOM,
			0.6f,
			0.6f,
			0.6f
		);

		AddVertex(
			vertices,
			x,
			BOARD_TOP,
			0.6f,
			0.6f,
			0.6f
		);
	}

	// 가로선
	for (int row = 0;
		row <= boardRows;
		++row)
	{
		float y{
			BOARD_TOP -
			cellHeight * row
		};

		AddVertex(
			vertices,
			BOARD_LEFT,
			y,
			0.6f,
			0.6f,
			0.6f
		);

		AddVertex(
			vertices,
			BOARD_RIGHT,
			y,
			0.6f,
			0.6f,
			0.6f
		);
	}

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() *
		sizeof(float),
		vertices.data()
	);

	glLineWidth(1.0f);

	glDrawArrays(
		GL_LINES,
		0,
		static_cast<GLsizei>(
			vertices.size() / 6
			)
	);
}

// 장애물 출력
void DrawObstacles()
{
	float baseSize{
		GetShapeBaseSize()
	};

	for (int i = 0;
		i < obstacleCount;
		++i)
	{
		if (!obstacles[i].used)
		{
			continue;
		}

		float x;
		float y;

		GetCellCenter(
			obstacles[i].row,
			obstacles[i].col,
			x,
			y
		);

		DrawShape(
			obstacles[i].type,
			x,
			y,
			baseSize *
			obstacles[i].size,
			obstacles[i].r,
			obstacles[i].g,
			obstacles[i].b
		);
	}
}

// 주인공 출력
void DrawPlayer()
{
	float x;
	float y;

	GetCellCenter(
		player.row,
		player.col,
		x,
		y
	);

	float size{
		GetShapeBaseSize() *
		0.72f
	};

	DrawShape(
		player.type,
		x,
		y,
		size,
		player.r,
		player.g,
		player.b
	);
}

// 충돌 효과
void DrawCollisionEffect()
{
	if (glfwGetTime() >
		collisionEndTime)
	{
		return;
	}

	float x;
	float y;

	GetCellCenter(
		player.row,
		player.col,
		x,
		y
	);

	float size{
		GetShapeBaseSize() *
		0.55f
	};

	std::vector<float> vertices;

	// 가로
	AddVertex(
		vertices,
		x - size,
		y,
		1.0f,
		0.0f,
		0.0f
	);

	AddVertex(
		vertices,
		x + size,
		y,
		1.0f,
		0.0f,
		0.0f
	);

	// 세로
	AddVertex(
		vertices,
		x,
		y - size,
		1.0f,
		0.0f,
		0.0f
	);

	AddVertex(
		vertices,
		x,
		y + size,
		1.0f,
		0.0f,
		0.0f
	);

	// 대각선
	AddVertex(
		vertices,
		x - size,
		y - size,
		1.0f,
		0.0f,
		0.0f
	);

	AddVertex(
		vertices,
		x + size,
		y + size,
		1.0f,
		0.0f,
		0.0f
	);

	AddVertex(
		vertices,
		x - size,
		y + size,
		1.0f,
		0.0f,
		0.0f
	);

	AddVertex(
		vertices,
		x + size,
		y - size,
		1.0f,
		0.0f,
		0.0f
	);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() *
		sizeof(float),
		vertices.data()
	);

	glLineWidth(3.0f);

	glDrawArrays(
		GL_LINES,
		0,
		static_cast<GLsizei>(
			vertices.size() / 6
			)
	);
}

// 도형 출력
void DrawShape(
	ShapeType type,
	float x,
	float y,
	float size,
	float r,
	float g,
	float b
)
{
	if (type == RECTANGLE)
	{
		DrawSquare(
			x,
			y,
			size,
			r,
			g,
			b
		);
	}
	else if (type == TRIANGLE)
	{
		DrawTriangle(
			x,
			y,
			size,
			r,
			g,
			b,
			false
		);
	}
	else
	{
		DrawTriangle(
			x,
			y,
			size,
			r,
			g,
			b,
			true
		);
	}
}

// 사각형 출력
void DrawSquare(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b
)
{
	std::vector<float> vertices;

	float half{
		size / 2.0f
	};

	AddVertex(
		vertices,
		x - half,
		y + half,
		r,
		g,
		b
	);

	AddVertex(
		vertices,
		x - half,
		y - half,
		r,
		g,
		b
	);

	AddVertex(
		vertices,
		x + half,
		y - half,
		r,
		g,
		b
	);

	AddVertex(
		vertices,
		x + half,
		y + half,
		r,
		g,
		b
	);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() *
		sizeof(float),
		vertices.data()
	);

	glDrawArrays(
		GL_TRIANGLE_FAN,
		0,
		4
	);
}

// 삼각형 출력
void DrawTriangle(
	float x,
	float y,
	float size,
	float r,
	float g,
	float b,
	bool inverted
)
{
	std::vector<float> vertices;

	float half{
		size / 2.0f
	};

	if (!inverted)
	{
		AddVertex(
			vertices,
			x,
			y + half,
			r,
			g,
			b
		);

		AddVertex(
			vertices,
			x - half,
			y - half,
			r,
			g,
			b
		);

		AddVertex(
			vertices,
			x + half,
			y - half,
			r,
			g,
			b
		);
	}
	else
	{
		AddVertex(
			vertices,
			x - half,
			y + half,
			r,
			g,
			b
		);

		AddVertex(
			vertices,
			x + half,
			y + half,
			r,
			g,
			b
		);

		AddVertex(
			vertices,
			x,
			y - half,
			r,
			g,
			b
		);
	}

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		vertices.size() *
		sizeof(float),
		vertices.data()
	);

	glDrawArrays(
		GL_TRIANGLES,
		0,
		3
	);
}

// 칸 중심 좌표
void GetCellCenter(
	int row,
	int col,
	float& x,
	float& y
)
{
	float cellWidth{
		GetCellWidth()
	};

	float cellHeight{
		GetCellHeight()
	};

	x =
		BOARD_LEFT +
		cellWidth *
		(col + 0.5f);

	y =
		BOARD_TOP -
		cellHeight *
		(row + 0.5f);
}

// 칸 가로 크기
float GetCellWidth()
{
	return
		(BOARD_RIGHT -
			BOARD_LEFT) /
		boardCols;
}

// 칸 세로 크기
float GetCellHeight()
{
	return
		(BOARD_TOP -
			BOARD_BOTTOM) /
		boardRows;
}

// 도형 기본 크기
float GetShapeBaseSize()
{
	return std::min(
		GetCellWidth(),
		GetCellHeight()
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

	// 정지 / 다시 시작
	else if (key == GLFW_KEY_S)
	{
		if (!finished)
		{
			moving = !moving;
			lastMoveTime = glfwGetTime();
		}
	}

	// 속도 증가
	else if (key == GLFW_KEY_EQUAL ||
		key == GLFW_KEY_KP_ADD)
	{
		moveInterval -= 0.03;

		if (moveInterval < 0.05)
		{
			moveInterval = 0.05;
		}
	}

	// 속도 감소
	else if (key == GLFW_KEY_MINUS ||
		key == GLFW_KEY_KP_SUBTRACT)
	{
		moveInterval += 0.03;

		if (moveInterval > 1.0)
		{
			moveInterval = 1.0;
		}
	}

	// 종료
	else if (key == GLFW_KEY_Q)
	{
		glfwSetWindowShouldClose(window, true);
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