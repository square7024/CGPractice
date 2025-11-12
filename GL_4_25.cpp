#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLvoid make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);

//--- 필요한 변수 선언
GLint width, height;
GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

glm::vec3 cameraPos;

GLuint vao, vbo;
//const GLfloat triShape[3][3] = { //--- 삼각형 위치 값
//{ -0.5, -0.5, 0.0 }, { 0.5, -0.5, 0.0 }, { 0.0, 0.5, 0.0} };
//const GLfloat colors[3][3] = { //--- 삼각형 꼭지점 색상
//{ 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };

float vertices[] = { //--- 버텍스 속성: 좌표값(FragPos), 노말값 (Normal)
-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,     0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,     0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,     -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,     -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,     0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,     -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,     -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,     -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,     -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,     -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,     0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,     0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,     0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,     0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,     -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,     -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,     0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,     -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
};


char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	width = 800;
	height = 800;
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("GLSL_4_25");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_shaderProgram();
	InitBuffer();

	glEnable(GL_DEPTH_TEST);

	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMainLoop();
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	vertexSource = filetobuf("vertex_25.glsl");

	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);

	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment_25.glsl");

	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);

	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLvoid make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기

	//-- shader Program
	shaderProgramID = glCreateProgram();

	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}

GLvoid InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glGenBuffers(1, &vbo); //--- 1개의 VBO를 지정하고 할당하기

	glBindVertexArray(vao); //--- VAO를 바인드하기
	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// 아직 데이터가 없으므로 크기 0의 빈 버퍼를 생성.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//--- 좌표값을 attribute 인덱스 0번에 명시한다: 버텍스 당 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//--- attribute 인덱스 0번을 사용가능하게 함
	glEnableVertexAttribArray(0);
	//--- 색상값을 attribute 인덱스 1번에 명시한다: 버텍스 당 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//--- attribute 인덱스 1번을 사용 가능하게 함.
	glEnableVertexAttribArray(1);

	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos"); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
	glUniform3f(lightPosLocation, 0.0, 0.0, 5.0);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor"); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색
	glUniform3f(objColorLocation, 1.0, 0.5, 0.3);
	//unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치
	//glUniform3f(viewPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);
}

//--- 출력 콜백 함수
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	GLfloat rColor, gColor, bColor;

	rColor = bColor = gColor = 0.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	int modelLoc = glGetUniformLocation(shaderProgramID, "model"); //--- 버텍스 세이더에서 모델링 변환 행렬 변수값을 받아온다.
	int viewLoc = glGetUniformLocation(shaderProgramID, "view"); //--- 버텍스 세이더에서 뷰잉 변환 행렬 변수값을 받아온다.
	int projLoc = glGetUniformLocation(shaderProgramID, "projection"); //--- 버텍스 세이더에서 투영 변환 행렬 변수값을 받아온다.

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f); //--- 카메라 위치
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
	glm::mat4 view = glm::mat4(1.0f);

	//--- 모델링 변환, 뷰잉 변환, 투영 변환 행렬을 설정한 후, 버텍스 세이더에 저장한다.
	glm::mat4 mTransform = glm::mat4(1.0f);
	//mTransform = glm::rotate(mTransform, glm::radians(0), glm::vec3(0.0f, 1.0, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);


	//--- 삼각형 그리기
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glutSwapBuffers(); //--- 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	width = (h == 0) ? w : w;   // 0 보호
	height = (h == 0) ? 1 : h;
	glViewport(0, 0, width, height);
}

GLvoid Keyboard(unsigned char key, int, int)
{

	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{

}