#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

struct Vec3 
{
	float x, y, z;
};

struct Color
{
	float r, g, b, a;
};
struct Vertex
{
	Vec3 position;
	Color color;
};


//void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);

//TODO: Vertex shader source code
const char* vertexShaderSource =
"#version 450							\n"
"layout (location = 0) in vec3 vPos;    \n"
"layout (location = 1) in vec4 vColor;  \n"
"out vec4 Color;						\n"
"uniform float _Time;					\n"
"void main(){							\n"
"Color = vColor;						\n"
"gl_Position = vec4(vPos.x * abs(sin(_Time + vPos.x)), vPos.y * abs(sin(_Time)), vPos.z, 1.0); \n"
"}										\0"; // \0 is a null character and shows the end of a none character array


//TODO: Fragment shader source code
const char* fragmentShaderSource =
"#version 450					\n"
"out vec4 FragColor;			\n"
"in vec4 Color;					\n"
"uniform float _Time;			\n"
"void main(){					\n"
"float t = abs(sin(_Time));		\n"
"FragColor = vec4(Color * t);   \n"
"}								\0";

//TODO: Vertex data array
Vertex vertices[] = { 
	//x y z           r g b a
	-0.5, -0.5, 0.0,  1.0, 0.0, 0.0, 1.0, //Bottom left
	+0.5, -0.5, 0.0,  0.0, 1.0, 0.0, 1.0, //Bottom right
	+0.0, +0.5, 0.0,  0.0, 0.0, 1.0, 1.0, //Top 

	-1.0, -1.0, 0.0,  0.0, 1.0, 0.0, 1.0,
	-0.5, +0.5, 0.0,  1.0, 0.0, 1.0, 1.0,
	+0.0, 0.25, 0.0,  0.0, 0.0, 1.0, 1.0
};

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGLExample", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);


	//TODO: Create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//TODO: Get vertex shader compilation status and output info log
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		//Dump logs into a char array - 512 is an abritrary length
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Failed to compile vertex Shader: %s", infoLog);
	}
	
	//TODO: Create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	////TODO: Get fragment shader compilation status and output info log
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		//Dump logs into a char array - 512 is an abritrary length
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("Failed to compile vertex Shader: %s", infoLog);
	}

	//TODO: Create shader program
	GLuint shaderProgram = glCreateProgram();

	//TODO: Attach vertex and fragment shaders to shader program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//TODO: Link shader program
	glLinkProgram(shaderProgram);

	//TODO: Check for link status and output errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("Failed to link shader program: %s", infoLog);
	}

	//TODO: Delete vertex + fragment shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//TODO: Create and bind Vertex Array Object (VAO)
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	//TODO: Create and bind Vertex Buffer Object (VBO), fill with vertexData
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//TODO: Define vertex ATTRIBUTES layout

	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);  

	//COLOR
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
	glEnableVertexAttribArray(1);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//TODO:Use shader program
		glUseProgram(shaderProgram);

		//Set uniform
		float time = (float)glfwGetTime();
		GLint timeLocation = glGetUniformLocation(shaderProgram, "_Time");
		glUniform1f(timeLocation, time);
		
		//TODO: Draw triangle (3 indices!)
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

