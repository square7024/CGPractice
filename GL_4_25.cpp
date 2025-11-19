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
bool gLightOn = true;
float gAngleY = 0.0f;
float gLightAngle = 70.0f;
float gLightRadius = 5.0f;

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
	glUniform3f(lightPosLocation, 0.0, 0.0, gLightRadius);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor"); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색
	glUniform3f(objColorLocation, 1.0, 0.5, 0.3);
	//unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치
	//glUniform3f(viewPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);
}

/*
//--- 출력 콜백 함수
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	int modelLoc = glGetUniformLocation(shaderProgramID, "model"); //--- 버텍스 세이더에서 모델링 변환 행렬 변수값을 받아온다.
	int viewLoc = glGetUniformLocation(shaderProgramID, "view"); //--- 버텍스 세이더에서 뷰잉 변환 행렬 변수값을 받아온다.
	int projLoc = glGetUniformLocation(shaderProgramID, "projection"); //--- 버텍스 세이더에서 투영 변환 행렬 변수값을 받아온다.

	glm::vec3 cameraPos = glm::vec3(-3.0f, 3.0f, 7.0f); //--- 카메라 위치
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
	glm::mat4 view = glm::mat4(1.0f);

	//--- 모델링 변환, 뷰잉 변환, 투영 변환 행렬을 설정한 후, 버텍스 세이더에 저장한다.
	glm::mat4 mTransform = glm::mat4(1.0f);
	mTransform = glm::rotate(mTransform, glm::radians(gAngleY), glm::vec3(0.0f, 1.0, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	glm::vec3 lightPos;
	lightPos.x = gLightRadius * cosf(gLightAngle);
	lightPos.y = 0.0f;
	lightPos.z = gLightRadius * sinf(gLightAngle);

	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	//--- 큐브 그리기
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//--- 조명 궤도 원 그리기 (xz 평면, 반지름 gLightRadius)
	glUseProgram(0);           // 고정 파이프라인으로 잠깐 전환 (color 사용)
	glColor3f(1.0f, 1.0f, 1.0f); // 흰색 궤도

	glBegin(GL_LINE_LOOP);
	const int segments = 64;
	for (int i = 0; i < segments; ++i) {
		float theta = 2.0f * 3.1415926f * i / segments;
		float x = gLightRadius * cosf(theta);
		float z = gLightRadius * sinf(theta);
		glVertex3f(x, 0.0f, z);   // y=0인 평면에 원
	}
	glEnd();

	glUseProgram(shaderProgramID);   // 다시 셰이더로 복귀

	// --- 조명 위치에 작은 큐브 그리기 ---
	glUseProgram(0);
	glColor3f(1.0f, 1.0f, 0.0f);  // 노란색

	glPushMatrix();
	glTranslatef(lightPos.x, lightPos.y, lightPos.z);
	glScalef(0.1f, 0.1f, 0.1f);   // 작은 크기의 큐브

	// 큐브 1개 그리기
	glBegin(GL_QUADS);

	// Front
	glVertex3f(-0.5, -0.5, 0.5);
	glVertex3f(0.5, -0.5, 0.5);
	glVertex3f(0.5, 0.5, 0.5);
	glVertex3f(-0.5, 0.5, 0.5);

	// Back
	glVertex3f(-0.5, -0.5, -0.5);
	glVertex3f(0.5, -0.5, -0.5);
	glVertex3f(0.5, 0.5, -0.5);
	glVertex3f(-0.5, 0.5, -0.5);

	// Left
	glVertex3f(-0.5, -0.5, -0.5);
	glVertex3f(-0.5, -0.5, 0.5);
	glVertex3f(-0.5, 0.5, 0.5);
	glVertex3f(-0.5, 0.5, -0.5);

	// Right
	glVertex3f(0.5, -0.5, -0.5);
	glVertex3f(0.5, -0.5, 0.5);
	glVertex3f(0.5, 0.5, 0.5);
	glVertex3f(0.5, 0.5, -0.5);

	// Top
	glVertex3f(-0.5, 0.5, 0.5);
	glVertex3f(0.5, 0.5, 0.5);
	glVertex3f(0.5, 0.5, -0.5);
	glVertex3f(-0.5, 0.5, -0.5);

	// Bottom
	glVertex3f(-0.5, -0.5, 0.5);
	glVertex3f(0.5, -0.5, 0.5);
	glVertex3f(0.5, -0.5, -0.5);
	glVertex3f(-0.5, -0.5, -0.5);

	glEnd();
	glPopMatrix();

	glUseProgram(shaderProgramID); // 원래 쉐이더로 복귀

	glutSwapBuffers(); //--- 화면에 출력하기
}
*/
GLvoid drawScene()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint objColorLoc = glGetUniformLocation(shaderProgramID, "objectColor");

	// ----- 카메라 행렬 -----
	glm::vec3 cameraPos(-3.0f, 3.0f, 7.0f);
	glm::mat4 view = glm::lookAt(cameraPos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(60.0f),
		(float)width / (float)height,
		0.1f, 100.0f);

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// ----- 조명 위치 계산 & 유니폼 -----
	glm::vec3 lightPos;
	lightPos.x = gLightRadius * cosf(gLightAngle);
	lightPos.y = 0.0f;
	lightPos.z = gLightRadius * sinf(gLightAngle);

	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	// ----- 1. 메인 큐브 그리기 -----
	glm::mat4 modelCube(1.0f);
	modelCube = glm::rotate(modelCube, glm::radians(gAngleY), glm::vec3(0,1,0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCube));
	glUniform3f(objColorLoc, 1.0f, 0.5f, 0.3f); // 오브젝트 색

	glDrawArrays(GL_TRIANGLES, 0, 36);

	// ----- 2. 조명 위치에 작은 큐브 그리기 -----
	glm::mat4 modelLight(1.0f);
	modelLight = glm::translate(modelLight, lightPos);        // 조명 위치로 이동
	modelLight = glm::scale(modelLight, glm::vec3(0.2f));     // 더 작게 만들기

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelLight));
	glUniform3f(objColorLoc, 1.0f, 1.0f, 0.0f); // 노란색 큐브(조명 표시용)

	glDrawArrays(GL_TRIANGLES, 0, 36); // 같은 VAO를 재사용

	glutSwapBuffers();
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
	switch (key)
	{
	case 'm':
	{
		gLightOn = !gLightOn;  // 상태 토글

		glUseProgram(shaderProgramID);
		GLint lightOnLoc = glGetUniformLocation(shaderProgramID, "lightOn");
		glUniform1i(lightOnLoc, gLightOn ? 1 : 0);

		//GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
		//if (gLightOn) {
		//	// 조명 켜기: 흰색 조명
		//	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		//}
		//else {
		//	// 조명 끄기: 조명 색 0 → ambient/diffuse 둘 다 0이 돼서 완전 어둡게
		//	glUniform3f(lightColorLoc, 0.0f, 0.0f, 0.0f);
		//}
		break;
	}
	case 'y':
	{
		gAngleY += 10.0f;
		if (gAngleY >= 360.0f)   // 360도 넘어가면 0으로 되돌리기 (넘치지 않게)
			gAngleY -= 360.0f;
		break;
	}
	case 'r':
		gLightAngle += glm::radians(10.0f);
		if (gLightAngle > glm::two_pi<float>())
			gLightAngle -= glm::two_pi<float>();
		break;
	case 'R':
		gLightAngle -= glm::radians(10.0f);
		if (gLightAngle < glm::two_pi<float>())
			gLightAngle += glm::two_pi<float>();
		break;
	case 'z':
		gLightRadius -= 0.2f;
		if (gLightRadius < 0.5f) gLightRadius = 0.5f;  // 너무 가까우면 0.5로 제한
		break;
	case 'Z':
		gLightRadius += 0.2f;
		if (gLightRadius > 20.0f) gLightRadius = 20.0f; // 너무 멀면 제한
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{

}