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

// 视景体，镜头能看见的地方
float l = -3.0, r = 3.0;    // left and right
float b = -3.0, t = 3.0;    // bottom and top
float n = -3.0, f = 3.0;    // zNear and zFar

float rotationAngle = -5.0;    //旋转角度

vec4 red(1.0, 0.0, 0.0, 1.0);          //物体颜色
vec4 black(0.0, 0.0, 0.0, 1.0);        //阴影颜色，简化了，等学了phone光照模型后就会了

float lightPos[3] = { -0.5, 2.0, 0.5 };  //点光源

vec3 points[3] = {  //构造三角形的三个点
	vec3(-0.5, 0.5, 0.5),
	vec3(0.5, 0.5, 0.5),
	vec3(0.0, 0.75, 0.0)
};

namespace Camera  //设置照相机系数
{
	mat4 modelMatrix; //模型矩阵
	mat4 viewMatrix;  //观察矩阵
	mat4 projMatrix;  //投影矩阵

	mat4 ortho(const GLfloat left, const GLfloat right,  //正交投影
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

	mat4 perspective(const GLfloat fovy, const GLfloat aspect,  //透视投影变换
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

	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)  //观察矩阵变换
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

// 初始函数
void init()
{
	// 清除背景颜色，由于阴影是黑色的，我们这里将其设置为灰色
	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
	programID = InitShader("vshader.glsl", "fshader.glsl");  //引入顶点着色器和片元着色器

	vPositionID = glGetAttribLocation(programID, "vPosition");  //传入的顶点标识

	//分别得到模型矩阵、观察矩阵、投影矩阵、颜色的索引
	modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
	viewMatrixID = glGetUniformLocation(programID, "viewMatrix");
	projMatrixID = glGetUniformLocation(programID, "projMatrix");
	fColorID = glGetUniformLocation(programID, "fColor");

	// 创建顶点数组对象并绑定
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// 创建顶点缓存对象并绑定
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//深度测试
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

// 着色渲染
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);


	// 由于使用默认相机参数时，相机看的方向与投影平面平行，因此，调整一下相机的方向和位置
	vec4 eye(0.5, 2.0, -0.5, 1.0); // 光源关于y-z平面的对称方向
	vec4 at(0, 0, 0, 1);   // 原点
	vec4 up(0, 1, 0, 0);      // 默认方向

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

	// 绘制原始三角形，利用传进去的三个点
	Camera::modelMatrix = RotateY(rotationAngle);
	glUniformMatrix4fv(modelMatrixID, 1, GL_TRUE, Camera::modelMatrix);
	glUniform4fv(fColorID, 1, red);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// 三角形阴影绘制
	float lx = lightPos[0];
	float ly = lightPos[1];
	float lz = lightPos[2];
	// 根据光源位置，计算阴影投影矩阵,你会发现右下角的一个元素居然不是1！
	mat4 shadowProjMatrix(-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// 可以通过交互控制绕y轴旋转角度
	Camera::modelMatrix = shadowProjMatrix*RotateY(rotationAngle);
	//再次利用那三个点绘制阴影
	glUniformMatrix4fv(modelMatrixID, 1, GL_TRUE, Camera::modelMatrix);
	glUniform4fv(fColorID, 1, black);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Clean up
	glDisableVertexAttribArray(vPositionID);
	glUseProgram(0);

	glutSwapBuffers();
}

// 重新绘制窗口
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

// 键盘监听
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
	glutCreateWindow("阴影");

	glewInit();
	init();

	// 回调函数
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