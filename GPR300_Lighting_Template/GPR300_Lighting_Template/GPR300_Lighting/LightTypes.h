#pragma once


#include <glm/glm.hpp>

struct DirectionalLight
{
    glm::vec3 color = glm::vec3(1);
    glm::vec3 dir = glm::vec3(0, -1, 0); //Pointing Down
    float intensity = 1;
};

struct PointLight
{
    glm::vec3 color = glm::vec3(1);
    glm::vec3 pos = glm::vec3(1, -1, 0);
    float intensity = 1;
    float linearFallOff = .07;
    float quadFallOff = .017;
};

struct SpotLight
{
    glm::vec3 color = glm::vec3(1);
    glm::vec3 direction = glm::vec3(0, 1, 0);
    glm::vec3 pos = glm::vec3(1, -1, 0);
    float intensity = 1; 
    float fallOffCurve = 2;
    float minAngle = glm::cos(glm::radians(30.0f));
    float maxAngle = glm::cos(glm::radians(60.0f));
};

struct Material
{
    glm::vec3 color = glm::vec3(1);
    float AmbientK = 0.1;
    float DiffuseK = 0.1;
    float SpecularK = 0.1; //0-1
    float Shininess = 150; //1-512
};