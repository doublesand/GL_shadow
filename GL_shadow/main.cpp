#include "include/Angel.h"

#pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>

using namespace std;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint modelMatrixID;
GLuint viewMatrixID;
GLuint projMatrixID;
GLuint fColorID;

// �Ӿ��壬��ͷ�ܿ����ĵط�
float l = -3.0, r = 3.0;    // left and right
float b = -3.0, t = 3.0;    // bottom and top
float n = -3.0, f = 3.0;    // zNear and zFar

float rotationAngle = -5.0;    //��ת�Ƕ�

vec4 red(1.0, 0.0, 0.0, 1.0);          //������ɫ
vec4 black(0.0, 0.0, 0.0, 1.0);        //��Ӱ��ɫ�����ˣ���ѧ��phone����ģ�ͺ�ͻ���

float lightPos[3] = { -0.5, 2.0, 0.5 };  //���Դ

vec3 points[3] = {  //���������ε�������
	vec3(-0.5, 0.5, 0.5),
	vec3(0.5, 0.5, 0.5),
	vec3(0.0, 0.75, 0.0)
};

namespace Camera  //���������ϵ��
{
	mat4 modelMatrix; //ģ�;���
	mat4 viewMatrix;  //�۲����
	mat4 projMatrix;  //ͶӰ����

	mat4 ortho(const GLfloat left, const GLfloat right,  //����ͶӰ
 		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar)
	{
		mat4 c;
		c[0][0] = 2.0 / (right - left);
		c[1][1] = 2.0 / (top - bottom);
		c[2][2] = 2.0 / (zNear - zFar);
		c[3][3] = 1.0;
		c[0][3] = -(right + left) / (right - left);
		c[1][3] = -(top + bottom) / (top - bottom);
		c[2][3] = -(zFar + zNear) / (zFar - zNear);
		return c;
	}

	mat4 perspective(const GLfloat fovy, const GLfloat aspect,  //͸��ͶӰ�任
		const GLfloat zNear, const GLfloat zFar)
	{
		GLfloat top = tan(fovy*DegreesToRadians / 2) * zNear;
		GLfloat right = top * aspect;

		mat4 c;
		c[0][0] = zNear / right;
		c[1][1] = zNear / top;
		c[2][2] = -(zFar + zNear) / (zFar - zNear);
		c[2][3] = (-2.0*zFar*zNear) / (zFar - zNear);
		c[3][2] = -1.0;
		c[3][3] = 0.0;
		return c;
	}

	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)  //�۲����任
	{
		vec4 n = normalize(eye - at);
		vec4 u = normalize(vec4(cross(up, n), 0.0));
		vec4 v = normalize(vec4(cross(n, u), 0.0));
		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 R = mat4(u, v, n, t);
		mat4 T = Translate(-eye.x, -eye.y, -eye.z);
		return R*T;
	}
}

// ��ʼ����
void init()
{
	// ���������ɫ��������Ӱ�Ǻ�ɫ�ģ��������ｫ������Ϊ��ɫ
	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
	programID = InitShader("vshader.glsl", "fshader.glsl");  //���붥����ɫ����ƬԪ��ɫ��

	vPositionID = glGetAttribLocation(programID, "vPosition");  //����Ķ����ʶ

	//�ֱ�õ�ģ�;��󡢹۲����ͶӰ������ɫ������
	modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
	viewMatrixID = glGetUniformLocation(programID, "viewMatrix");
	projMatrixID = glGetUniformLocation(programID, "projMatrix");
	fColorID = glGetUniformLocation(programID, "fColor");

	// ��������������󲢰�
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// �������㻺����󲢰�
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//��Ȳ���
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

// ��ɫ��Ⱦ
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);


	// ����ʹ��Ĭ���������ʱ��������ķ�����ͶӰƽ��ƽ�У���ˣ�����һ������ķ����λ��
	vec4 eye(0.5, 2.0, -0.5, 1.0); // ��Դ����y-zƽ��ĶԳƷ���
	vec4 at(0, 0, 0, 1);   // ԭ��
	vec4 up(0, 1, 0, 0);      // Ĭ�Ϸ���

	Camera::viewMatrix = Camera::lookAt(eye, at, up);
	Camera::projMatrix = Camera::ortho(l, r, b, t, n, f);
	glUniformMatrix4fv(viewMatrixID, 1, GL_TRUE, Camera::viewMatrix);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);

	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(
		vPositionID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	// ����ԭʼ�����Σ����ô���ȥ��������
	Camera::modelMatrix = RotateY(rotationAngle);
	glUniformMatrix4fv(modelMatrixID, 1, GL_TRUE, Camera::modelMatrix);
	glUniform4fv(fColorID, 1, red);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// ��������Ӱ����
	float lx = lightPos[0];
	float ly = lightPos[1];
	float lz = lightPos[2];
	// ���ݹ�Դλ�ã�������ӰͶӰ����,��ᷢ�����½ǵ�һ��Ԫ�ؾ�Ȼ����1��
	mat4 shadowProjMatrix(-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// ����ͨ������������y����ת�Ƕ�
	Camera::modelMatrix = shadowProjMatrix*RotateY(rotationAngle);
	//�ٴ������������������Ӱ
	glUniformMatrix4fv(modelMatrixID, 1, GL_TRUE, Camera::modelMatrix);
	glUniform4fv(fColorID, 1, black);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Clean up
	glDisableVertexAttribArray(vPositionID);
	glUseProgram(0);

	glutSwapBuffers();
}

// ���»��ƴ���
void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}

// Control mouse actions
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		lightPos[0] = x;
		lightPos[1] = y;
	}
}

// ���̼���
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033: // Both escape key and 'q' cause the game to exit
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
	case 'y':
		rotationAngle -= 1.0;
		break;
	case 'Y':
		rotationAngle += 1.0;
		break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

void printHelp()
{
	printf("%s\n\n", "Camera");
	printf("Keyboard options:\n");
	printf("y: Decrease rotation angle (aroud Y axis)\n");
	printf("Y: Increase rotation angle (aroud Y axis)\n");
}

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("��Ӱ");

	glewInit();
	init();

	// �ص�����
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	printHelp();
	glutMainLoop();

	clean();

	return 0;
}