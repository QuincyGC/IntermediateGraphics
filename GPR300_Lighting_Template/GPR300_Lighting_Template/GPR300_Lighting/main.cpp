#include "Lighttypes.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

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
#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"
#include <iostream>

using namespace std;

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

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
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

//IMPORTANT QUINCY!!!*************************************************
glm::vec3 bgColor = glm::vec3(0);
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 3.0f, 0.0f);

bool wireFrame = false;

SpotLight spotLit;
DirectionalLight dirLit;
PointLight pointLit[2];
Material material;

//Texture Bool
struct TextureChose
{
	int ChosenTxtr = 0;
};

struct Controls
{
	bool onePointLight = true;
};

TextureChose txtChoice; 
Controls controls;

//Normal Mapping

float normMapIntensity;

//*********************************

//Filename 
const char* woodFile = "./WoodFloor.png";
const char* marleFile = "./Marble.png";
const char* woodNormal = "./WoodFloor_NormalGL.png";

GLuint createTexture(const char* filePath)
{
	//Generate a texture name
	GLuint txtr;
	glGenTextures(1, &txtr);
	glBindTexture(GL_TEXTURE_2D, txtr);

	int width, height, compNum;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* txtrData = stbi_load(filePath, &width, &height, &compNum, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, txtrData);
	
	//Wrap 
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//horiz
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // vertical

	//Zoom
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//magnify
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//min

	glGenerateMipmap(GL_TEXTURE_2D);

	return txtr;

}

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	//TEXTURE OF WOOD from File
	GLuint wood = createTexture(woodFile);
	GLuint marble = createTexture(marleFile);
	GLuint woodNormaltxtr = createTexture(woodNormal);

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform;

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	lightTransform.scale = glm::vec3(0.5f);
	lightTransform.position = glm::vec3(0.0f, 5.0f, 0.0f);


	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//TIMER
		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());

		//Choose texture
		litShader.setInt("_ChosenTexture", txtChoice.ChosenTxtr);

		//Controls
		litShader.setInt("_OnlyPoint", controls.onePointLight);

		//Activate Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodNormaltxtr);
		litShader.setInt("_NormalMap", 0); //this brings it to the shaders (It is the sampler2D

		//Activate Texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wood);
		litShader.setInt("_WoodTexture", 1); //this brings it to the shaders (It is the sampler2D

		//Animate texture
		litShader.setFloat("_Time", time);

		//NORMAL MAP
		litShader.setFloat("_normMapIntensity", normMapIntensity);

		//Camere and Material
		litShader.setVec3("camera.pos", camera.getPosition());

		litShader.setFloat("material.AmbientK", material.AmbientK);
		litShader.setFloat("material.DiffuseK", material.DiffuseK);
		litShader.setFloat("material.SpecularK", material.SpecularK);
		litShader.setFloat("material.Shininess", material.Shininess);
		litShader.setVec3("material.color", material.color);

		//Directional Light
		litShader.setVec3("dLit.color", dirLit.color);
		litShader.setVec3("dLit.direction", dirLit.dir);
		litShader.setFloat("dLit.intensity", dirLit.intensity);


		//Point Light
		for (size_t i = 0; i < 2; i++)
		{
			litShader.setVec3("pLit[" + std::to_string(i) + "].color", pointLit[i].color);
			litShader.setVec3("pLit[" + std::to_string(i) + "].pos", pointLit[i].pos);
			litShader.setFloat("pLit[" + std::to_string(i) + "].intensity", pointLit[i].intensity);
			litShader.setFloat("pLit[" + std::to_string(i) + "].linearFallOff", pointLit[i].linearFallOff);
			litShader.setFloat("pLit[" + std::to_string(i) + "].quadFallOff", pointLit[i].quadFallOff);
		}

		//Spot Light
		litShader.setFloat("sLit.minAngle", spotLit.minAngle);
		litShader.setFloat("sLit.maxAngle", spotLit.maxAngle);
		litShader.setVec3("sLit.pos", spotLit.pos);
		litShader.setVec3("sLit.direction", spotLit.direction);
		litShader.setVec3("sLit.color", spotLit.color);
		litShader.setFloat("sLit.intensity", spotLit.intensity);
		litShader.setFloat("sLit.fallOffCurve", spotLit.fallOffCurve);
		
		//Draw cube
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//Draw plane
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		unlitShader.setMat4("_Model", lightTransform.getModelMatrix());
		unlitShader.setVec3("_Color", lightColor);
		sphereMesh.draw();


		//Draw UI
		//Material
		ImGui::Begin("Material");
		ImGui::SliderFloat("AmbientK", &material.AmbientK, 0,1);
		ImGui::SliderFloat("DiffuseK", &material.DiffuseK, 0, 1);
		ImGui::SliderFloat("SpecularK", &material.SpecularK,.1, 1);
		ImGui::SliderFloat("Direction", &material.Shininess,1, 512);
		ImGui::ColorEdit3("Color", &material.color.r);
		ImGui::End();

		//Directional
		ImGui::Begin("Directional Light");
		ImGui::DragFloat3("Direction", &dirLit.dir.x);
		ImGui::SliderFloat("Intensity", &dirLit.intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("Color", &dirLit.color.r);
		ImGui::End();

		ImGui::Begin("Point Light 1");
		ImGui::DragFloat3("Position", &pointLit[0].pos.x);
		ImGui::SliderFloat("linearFallOff", &pointLit[0].linearFallOff, 0, 1);
		ImGui::SliderFloat("quadFallOff", &pointLit[0].quadFallOff, 0, 1);
		ImGui::SliderFloat("Intensity", &pointLit[0].intensity, 0.0f, 10.0f);
		ImGui::ColorEdit3("Color", &pointLit[0].color.r);
		ImGui::End();

		ImGui::Begin("Point Light 2");
		ImGui::DragFloat3("Position", &pointLit[1].pos.x);
		ImGui::SliderFloat("linearFallOff", &pointLit[1].linearFallOff, 0, 1);
		ImGui::SliderFloat("quadFallOff", &pointLit[1].quadFallOff, 0, 1);
		ImGui::SliderFloat("Intensity", &pointLit[1].intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("Color", &pointLit[1].color.r);
		ImGui::End();

		//Spot
		ImGui::Begin("Spot Light");
		ImGui::DragFloat3("Direction", &spotLit.direction.x);
		ImGui::DragFloat3("Position", &spotLit.pos.x);
		ImGui::SliderFloat("Intensity", &spotLit.intensity, 0.0f, 1.0f);
		ImGui::SliderFloat("fallOffCurve", &spotLit.fallOffCurve, 1.0f, 4.0f);
		ImGui::SliderFloat("MinAngle", &spotLit.minAngle, 0.0f, 120.0f);
		ImGui::SliderFloat("MaxAngle", &spotLit.maxAngle, 0.0f, 120.0f);
		ImGui::ColorEdit3("Color", &spotLit.color.r);
		ImGui::End();

		//Controls
		ImGui::Begin("Controls");
		ImGui::SliderInt("Switch Texture", &txtChoice.ChosenTxtr, 0.0f, 1.0f);
		ImGui::Checkbox("Point Light Only", &controls.onePointLight);
		ImGui::SliderFloat("Normal Map Intensity", &normMapIntensity, 0.0f, 1.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
