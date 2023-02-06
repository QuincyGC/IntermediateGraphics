#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;

class Camera
{
	glm::vec3 pos;
	glm::vec3 target;
	float fov; //verticaql field of view

	float orthographicSize; // height of frustum in view space
	bool orthographic;

	glm::mat4 getViewMatrix()
	{
		return glm::mat4(1);
	}

	glm::mat4 getProjectionMatrix()
	{
		return glm::mat4(1);
	}

	glm::mat4 ortho(float height, float aspectRatio, float nearPlane, float farPlane)
	{
		float r = (height * aspectRatio) / 2; //width/2
		float t = height / 2;
		float l = -r;
		float b = -t;

		glm::mat4 ortho = { 2 / (r - l), 0 , 0, -(r + l) / (r - l), //column
							0, 2 / (t - b), 0, -(t + b) / (t - b), //column
							0, 0, -2 / (farPlane - nearPlane), -(farPlane + nearPlane) / (farPlane - nearPlane), //column
							0, 0, 0, 1 }; //column

		return ortho;
	}

	glm::mat4 perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
	{
		float c = glm::tan(fov / 2);
		float a = aspectRatio;
		float n = nearPlane;
		float f = farPlane;

		glm::mat4 perspective = { 1 / (a * c), 0, 0, 0,
								0, 1 / c, 0, 0,
								0, 0, -(f + n) / (f - n), -(2 * f * n) / (f - n),
								0, 0, -1, 1 };
		return perspective;
	}

};

class Transform
{
	glm::vec3 pos = glm::vec3(1);
	//glm::quat rotQ;
	glm::vec3 rotE = glm::vec3(1);
	glm::vec3 scale = glm::vec3(1);

	glm::mat4 Transaltion()
	{
		glm::mat4 tran = { 1, 0, 0, pos.x,
						0, 1, 0, pos.y,
						0, 0, 1, pos.z,
						0, 0, 0, 1};

		return tran;
	}

	glm::mat4 Scale()
	{
		glm::mat4 s = { scale.x, 0, 0, 0,
							0, scale.y, 0, 0,
							0, 0, scale.z, 0,
							0, 0, 0, 1 };

		return s;
	}

	glm::mat4 RotationXEuler()
	{
		glm::mat4 rotX = { 1, 0, 0, 0,
							0, glm::cos(glm::radians(rotE[0])), -glm::sin(glm::radians(rotE[0])), 0,
							0, glm::sin(glm::radians(rotE[0])), glm::cos(glm::radians(rotE[0])), 0,
							0, 0, 0, 1 };

		return rotX;
	}
	
	glm::mat4 RotationYEuler()
	{
		glm::mat4 rotY = { glm::cos(glm::radians(rotE[1])), 0, glm::sin(glm::radians(rotE[1])), 0,
							0,1, 0, 0,
							-glm::sin(glm::radians(rotE[1])), 0, glm::cos(glm::radians(rotE[1])), 0,
							0, 0, 0, 1 };

		return rotY;
	}

	glm::mat4 RotationZEuler()
	{
		glm::mat4 rotZ = { glm::cos(glm::radians(rotE[2])), -glm::sin(glm::radians(rotE[2])), 0, 0,
							glm::sin(glm::radians(rotE[2])), glm::cos(glm::radians(rotE[2])), 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1 };

		return rotZ;
	}

	glm::mat4 getEulerRotationMatrix()
	{
		glm::mat4 rotation = RotationXEuler() * RotationYEuler() * RotationZEuler();
		return rotation;
	}

	glm::mat4 getModelMatrix()
	{
		glm::mat4 modelMatrix = Transaltion() * getEulerRotationMatrix() * Scale();

		return modelMatrix;
	}
};


int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}
	
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag");

	MeshData cubeMeshData;
	createCube(1.0f, 1.0f, 1.0f, cubeMeshData);

	Mesh cubeMesh(&cubeMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		shader.use();
		cubeMesh.draw();

		//Draw UI
		//********************************************************************
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Example slider", &exampleSliderFloat, 0.0f, 10.0f);
		ImGui::End();


		//********************************************************************

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}