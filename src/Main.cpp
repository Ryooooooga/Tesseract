#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#pragma warning(disable: 4838)
#include <xnamath.h>
#pragma warning(default: 4838)

#ifdef _DEBUG
#	pragma comment(lib, "GL/glew32.lib")
#	pragma comment(lib, "GLFW/GLFW_d.lib")
#	pragma comment(lib, "OpenGL32.lib")
#else
#	pragma comment(lib, "GL/glew32s.lib")
#	pragma comment(lib, "GLFW/GLFW.lib")
#	pragma comment(lib, "OpenGL32.lib")
#endif

using namespace std::literals;

const std::string	title	= u8"Tesseract"s;
constexpr int		width	= 1024;
constexpr int		height	= 576;
constexpr float		depth	= 200.f;
constexpr float		fov		= XM_PI/2;

GLFWwindow*	window;
XMMATRIX	model;
XMMATRIX	view;
XMMATRIX	proj;
XMMATRIX	proj2d;
XMVECTOR	eye;
XMVECTOR	at;
XMVECTOR	up;

float angleXY;
float angleXZ;
float angleYZ;
float angleWX;
float angleWY;
float angleWZ;

XMVECTOR vertices[] = {
	XMVectorSet(-1, -1, -1, -1),
	XMVectorSet(+1, -1, -1, -1),
	XMVectorSet(-1, +1, -1, -1),
	XMVectorSet(+1, +1, -1, -1),
	XMVectorSet(-1, -1, +1, -1),
	XMVectorSet(+1, -1, +1, -1),
	XMVectorSet(-1, +1, +1, -1),
	XMVectorSet(+1, +1, +1, -1),
	XMVectorSet(-1, -1, -1, +1),
	XMVectorSet(+1, -1, -1, +1),
	XMVectorSet(-1, +1, -1, +1),
	XMVectorSet(+1, +1, -1, +1),
	XMVectorSet(-1, -1, +1, +1),
	XMVectorSet(+1, -1, +1, +1),
	XMVectorSet(-1, +1, +1, +1),
	XMVectorSet(+1, +1, +1, +1),
};

constexpr unsigned indices1[] = {
	 0, 1, 0, 2, 3, 1, 3, 2,
	 4, 5, 4, 6, 7, 5, 7, 6,
	 0, 4, 1, 5, 2, 6, 3, 7,
	
	 8, 9, 8,10,11, 9,11,10,
	12,13,12,14,15,13,15,14,
	 8,12, 9,13,10,14,11,15,
	
	 0, 8, 1, 9, 2,10, 3,11,
	 4,12, 5,13, 6,14, 7,15,
};

constexpr unsigned indices2[] = {
	 0, 1, 3, 2,
	 4, 5, 7, 6,
	 0, 1, 5, 4,
	 1, 3, 7, 5,
	 3, 2, 6, 7,
	 2, 0, 4, 6,
	
	 8, 9,11,10,
	12,13,15,14,
	 8, 9,13,12,
	 9,11,15,13,
	11,10,14,15,
	10, 8,12,14,
};

//	get array size
template <typename T, std::size_t N>
constexpr std::size_t arraySize(T (&a)[N]) {
	return N;
}


//	load matrix
void loadMatrix(GLenum mode, const XMMATRIX& matrix) {
	glMatrixMode(mode);
	glLoadMatrixf(matrix.m[0]);
}


//	compile shader
GLuint compileShader(const char* path, GLenum type) {
	//	open file
	std::ifstream file{ path };

	if(!file)
	{
		throw std::exception{ path };
	}

	//	get file size
	file.seekg(0, std::ios::end);
	auto size = file.tellg();

	file.seekg(0, std::ios::beg);

	//	read text
	std::string source;
	source.resize(static_cast<std::size_t>(size) + 1);
	source.back() = '\0';

	file.read(&source[0], size);

	//	compile shader
	GLuint shader = glCreateShader(type);

	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE)
	{
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

		char log[1024];
		glGetShaderInfoLog(shader, 1024, &len, log);

		std::cout << log << std::endl;

		throw std::exception{ "glCompileShader" };
	}

	return shader;
}


//	draw objects
void draw() {
	float t = static_cast<float>(glfwGetTime());
	angleXY = t*XM_PI / 7 *0;
	angleXZ = t*XM_PI / 11*0;
	angleYZ = t*XM_PI / 13*0;
	angleWX = t*XM_PI / 17*0;
	angleWY = t*XM_PI / 19*0;
	angleWZ = t*XM_PI / 23*0;

	float c1 = std::cos(angleXY);
	float c2 = std::cos(angleXZ);
	float c3 = std::cos(angleYZ);
	float c4 = std::cos(angleWX);
	float c5 = std::cos(angleWY);
	float c6 = std::cos(angleWZ);

	float s1 = std::sin(angleXY);
	float s2 = std::sin(angleXZ);
	float s3 = std::sin(angleYZ);
	float s4 = std::sin(angleWX);
	float s5 = std::sin(angleWY);
	float s6 = std::sin(angleWZ);
	
	std::cout << "XY : " << std::setw(7) << angleXY << '\n';
	std::cout << "XZ : " << std::setw(7) << angleXZ << '\n';
	std::cout << "YZ : " << std::setw(7) << angleYZ << '\n';
	std::cout << "WX : " << std::setw(7) << angleWX << '\n';
	std::cout << "WY : " << std::setw(7) << angleWY << '\n';
	std::cout << "WZ : " << std::setw(7) << angleWZ << '\n';

	model = XMMatrixIdentity();
	//	XY
	model *= XMMatrixSet(
		+c1,-s1,  0,  0,
		+s1,+c1,  0,  0,
		  0,  0,  1,  0,
		  0,  0,  0,  1);
	//	XZ
	model *= XMMatrixSet(
		+c2,  0,-s2,  0,
		  0,  1,  0,  0,
		+s2,  0,+c2,  0,
		  0,  0,  0,  1);
	//	YZ
	model *= XMMatrixSet(
		  1,  0,  0,  0,
		  0,+c3,-s3,  0,
		  0,+s3,+c3,  0,
		  0,  0,  0,  1);
	//	XW
	model *= XMMatrixSet(
		+c4,  0,  0,-s4,
		  0,  1,  0,  0,
		  0,  0,  1,  0,
		+s4,  0,  0,+c4);
	//	YW
	model *= XMMatrixSet(
		  1,  0,  0,  0,
		  0,+c5,  0,-s5,
		  0,  0,  1,  0,
		  0,+s5,  0,+c5);
	//	XW
	model *= XMMatrixSet(
		  1,  0,  0,  0,
		  0,  1,  0,  0,
		  0,  0,+c6,-s6,
		  0,  0,+s6,+c6);

	//	set matrix
	loadMatrix(GL_MODELVIEW, model);
	loadMatrix(GL_PROJECTION, view*proj);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, 0, vertices);
	
	//	frame
	glDrawElements(GL_LINES, arraySize(indices1), GL_UNSIGNED_INT, indices1);
	
	//	face
/*	glEnable(GL_DEPTH_TEST);
	glDrawElements(GL_QUADS, arraySize(indices2), GL_UNSIGNED_INT, indices2);*/

	glDisableClientState(GL_VERTEX_ARRAY);
}


//	entry point
int main() {
	using namespace std;

	try {
		//	init glfw/glew
		if(!glfwInit())
		{
			throw exception{ "glfwInit" };
		}

		if(!(window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr)))
		{
			throw exception{ "glfwCreateWindow" };
		}

		glfwMakeContextCurrent(window);

		if(glewInit())
		{
			throw exception{ "glewInit" };
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//	compile shaders
		GLuint vertexShader		= compileShader("vs.glsl", GL_VERTEX_SHADER);
		GLuint fragmentShader	= compileShader("fs.glsl", GL_FRAGMENT_SHADER);

		GLuint program = glCreateProgram();
		
		glAttachShader(program, vertexShader	);
		glAttachShader(program, fragmentShader	);

		glLinkProgram(program);
		glUseProgram(program);

		//	init matrix
		eye		= XMVectorSet(0.f, 0.f, -3.f, 1.f);
		at		= XMVectorZero();
		up		= XMVectorSet(0.f, 1.f, 0.f, 1.f);

		view	= XMMatrixLookAtLH(eye, at, up);
		proj	= XMMatrixPerspectiveFovLH(fov, float(width)/height, 0.001f, depth);
		proj2d	= XMMatrixOrthographicLH(float(width), float(height), 0.f, 1.f);

		//	main loop
		while(!glfwWindowShouldClose(window))
		{
//			system("cls");
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			draw();

			glFlush();
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	catch(const exception& e) {
		cout << typeid(e).name()	<< endl;
		cout << e.what()			<< endl;
	}

	//	terminate glfw
	glfwDestroyWindow(window);
	glfwTerminate();
}
