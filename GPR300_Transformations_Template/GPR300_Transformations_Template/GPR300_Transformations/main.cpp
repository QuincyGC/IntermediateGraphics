#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <time.h>

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

//******************************
float nearPlane = .1f;
float farPlane = 80.0f;

float sRadius = 0.1f;
float sSpeed = 0.0f;
float sFov = 60.0f;
float sHeight = 0.0f;
bool toggle = 0.0f;

int d = 5;
const int NUM_CUBES = 6;
//******************************

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
	public:

		glm::vec3 camPos = glm::vec3(0, 0, 8);
		glm::vec3 target = glm::vec3(0);

		float orthographicSize = 0; // height of frustum in view space
		bool orthographic = true;
		float fov = 0;


		glm::mat4 getViewMatrix()
		{
			glm::vec3 u = glm::vec3(0, 1, 0);
			glm::vec3 forward = glm::normalize( target - camPos);
			glm::vec3 right = glm::normalize(glm::cross(forward, u));
			glm::vec3 up = glm::cross(right, forward);

			forward *= -1;

				glm::mat4 invRotateCam = {
					right.x, up.x, forward.x, 0,
					right.y, up.y, forward.y, 0,
					right.z, up.z, forward.z, 0,
					0,		 0,		0,		  1
				};

				glm::mat4 invTransformCam = {
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					-camPos.x, -camPos.y, -camPos.z, 1
				};

				glm::mat4 viewMatrix = invRotateCam * invTransformCam;

				return viewMatrix;
			
		}

		glm::mat4 getProjectionMatrix()
		{
			glm::mat4 projMat = glm::mat4(1);

			if (orthographic)
			{
				projMat = orthogonal();
			}
			else
			{
				projMat = perspective();
			}

			return projMat;
		}

		glm::mat4 orthogonal()
		{
			float aspectRatio = SCREEN_WIDTH / SCREEN_HEIGHT;

			float r = (orthographicSize * aspectRatio) / 2; //width/2
			float t = orthographicSize / 2;
			float l = -r;
			float b = -t;

			glm::mat4 ortho; 
		
			ortho[0][0] = 2 / (r - l);
			ortho[1][0] = 0;
			ortho[2][0] = 0;
			ortho[3][0] = -(r + l) / (r - l);

			ortho[0][1] = 0;
			ortho[1][1] = 2 / (t - b);
			ortho[2][1] = 0;
			ortho[3][1] = -(t + b) / (t - b);

			ortho[0][2] = 0;
			ortho[1][2] = 0;
			ortho[2][2] = -2 / (farPlane - nearPlane);
			ortho[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);

			ortho[0][3] = 0;
			ortho[1][3] = 0;
			ortho[2][3] = 0;
			ortho[3][3] = 1;

			return ortho;
		}

		glm::mat4 perspective()
		{
			float aspectRatio = SCREEN_WIDTH / SCREEN_HEIGHT;
			float c = glm::tan(glm::radians(fov) / 2);
			float a = aspectRatio;
			float n = nearPlane;
			float f = farPlane;

			glm::mat4 perspective;
		
			perspective[0][0] = 1 / (a * c);
			perspective[1][0] = 0;
			perspective[2][0] = 0;
			perspective[3][0] = 0;

			perspective[0][1] = 0;
			perspective[1][1] = 1 / c;
			perspective[2][1] = 0;
			perspective[3][1] = 0;

			perspective[0][2] = 0;
			perspective[1][2] = 0;
			perspective[2][2] = -(f + n) / (f - n);
			perspective[3][2] = -(2 * f * n) / (f - n);

			perspective[0][3] = 0;
			perspective[1][3] = 0;
			perspective[2][3] = -1;
			perspective[3][3] = 1;

			return perspective;
		}

};

class Transform
{
	public:
		glm::vec3 pos = glm::vec3(0);
		glm::vec3 rotE = glm::vec3(0);
		glm::vec3 scale = glm::vec3(1);

		glm::mat4 Transaltion()
		{
			glm::mat4 tran;

			tran[0][0] = 1;
			tran[1][0] = 0;
			tran[2][0] = 0;
			tran[3][0] = pos.x;

			tran[0][1] = 0;
			tran[1][1] = 1;
			tran[2][1] = 0;
			tran[3][1] = pos.y;

			tran[0][2] = 0;
			tran[1][2] = 0;
			tran[2][2] = 1;
			tran[3][2] = pos.z;

			tran[0][3] = 0;
			tran[1][3] = 0;
			tran[2][3] = 0;
			tran[3][3] = 1;

			return tran;
		}

		glm::mat4 Scale()
		{
			glm::mat4 s;

			s[0][0] = scale.x;
			s[1][0] = 0;
			s[2][0] = 0;
			s[3][0] = 0;

			s[0][1] = 0;
			s[1][1] = scale.y;
			s[2][1] = 0;
			s[3][1] = 0;

			s[0][2] = 0;
			s[1][2] = 0;
			s[2][2] = scale.z;
			s[3][2] = 0;

			s[0][3] = 0;
			s[1][3] = 0;
			s[2][3] = 0;
			s[3][3] = 1;

			return s;
		}

		glm::mat4 RotationXEuler()
		{
			glm::mat4 rotX;

			rotX[0][0] = 1;
			rotX[1][0] = 0;
			rotX[2][0] = 0;
			rotX[3][0] = 0;

			rotX[0][1] = 0;
			rotX[1][1] = glm::cos(rotE.x);
			rotX[2][1] = -glm::sin(rotE.x);
			rotX[3][1] = 0;

			rotX[0][2] = 0;
			rotX[1][2] = glm::sin(rotE.x);
			rotX[2][2] = glm::cos(rotE.x);
			rotX[3][2] = 0;

			rotX[0][3] = 0;
			rotX[1][3] = 0;
			rotX[2][3] = 0;
			rotX[3][3] = 1;

			return rotX;
		}
	
		glm::mat4 RotationYEuler()
		{
			glm::mat4 rotY;

			rotY[0][0] = glm::cos(rotE.y);
			rotY[1][0] = 0;
			rotY[2][0] = glm::sin(rotE.y);
			rotY[3][0] = 0;

			rotY[0][1] = 0;
			rotY[1][1] = 1;
			rotY[2][1] = 0;
			rotY[3][1] = 0;

			rotY[0][2] = -glm::sin(rotE.y);
			rotY[1][2] = 0;
			rotY[2][2] = glm::cos(rotE.y);
			rotY[3][2] = 0;

			rotY[0][3] = 0;
			rotY[1][3] = 0;
			rotY[2][3] = 0;
			rotY[3][3] = 1;

			return rotY;
		}

		glm::mat4 RotationZEuler()
		{
			glm::mat4 rotZ;

			rotZ[0][0] = glm::cos(rotE.z);
			rotZ[1][0] = -glm::sin(rotE.z);
			rotZ[2][0] = 0;
			rotZ[3][0] = 0;

			rotZ[0][1] = glm::sin(rotE.z);
			rotZ[1][1] = glm::cos(rotE.z);
			rotZ[2][1] = 0;
			rotZ[3][1] = 0;

			rotZ[0][2] = 0;
			rotZ[1][2] = 0;
			rotZ[2][2] = 1;
			rotZ[3][2] = 0;

			rotZ[0][3] = 0;
			rotZ[1][3] = 0;
			rotZ[2][3] = 0;
			rotZ[3][3] = 1;

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
	Camera cam;
	Transform trans[NUM_CUBES];

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

	// random time
	srand(time(NULL));

	// create a random scale, rotation, and translation for each cube
	for (size_t i = 0; i < NUM_CUBES; i++)
	{
		
		float transX = (rand() % 10 + 1);
		float transY = (rand() % 10 + 1);
		float transZ = (rand() % 10 + 1);

		float rotXE = rand() % 10 + 1;
		float rotYE = rand() % 10 + 1;
		float rotZE = rand() % 10 + 1;

		float scale = rand() % 5 + 1;

		trans[i].pos = glm::vec3(transX, transY, transZ);
		/*trans[i].rotE = glm::vec3(rotXE, rotYE, rotZE);*/
		trans[i].scale = glm::vec3(scale);

	}

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
		for (size_t i = 0; i < NUM_CUBES; i++)
		{
			cam.camPos.x = sRadius * (glm::sin(sSpeed * time));
			cam.camPos.z = sRadius * (glm::cos(sSpeed * time));
			cam.fov = sFov;
			cam.orthographicSize = sHeight;
			cam.orthographic = toggle;

			shader.use();
			shader.setMat4("_Projection", cam.getProjectionMatrix());
			shader.setMat4("_View", cam.getViewMatrix());
			shader.setMat4("_Model", trans[i].getModelMatrix());

			cubeMesh.draw();
		}

		//Draw UI
		//********************************************************************
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Orbit Speed", &sSpeed, 0.0f, 10.0f);
		ImGui::SliderFloat("Orbit Radius", &sRadius, 0.0f, 100.0f);
		ImGui::SliderFloat("Field of View", &sFov, 60.0f, 200.0f);
		ImGui::SliderFloat("Orthographic Height", &sHeight, 0.0f, 25.0f);
		ImGui::SliderFloat("Y Height", &cam.camPos.y, -10.0f, 20.0f);
		ImGui::Checkbox("Orthographic Toggle", &toggle);

		ImGui::End();
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