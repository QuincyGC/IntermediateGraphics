#pragma once
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
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

struct DirectionalLight
{
    glm::vec3 color;
    glm::vec3 dir;
    float intesity;
};

struct PointLight
{
    glm::vec3 color;
    glm::vec3 pos;
    float intesity;
    float linAttenuation;
};

struct SpotLight
{
    glm::vec3 color;
    glm::vec3 dir;
    glm::vec3 pos;
    glm::vec3 intesity;
    float linAttenuation;
    float minAngle;
    float MaxAngle;
};

struct Material
{
    glm::vec3 color;
    float AmbientK;
    float DiffuseK;
    float SpecularK; //0-1
    float Shininess; //1-512
};