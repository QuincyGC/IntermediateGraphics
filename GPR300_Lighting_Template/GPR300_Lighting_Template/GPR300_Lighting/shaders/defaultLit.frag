#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct DirectionLight{
    vec3 color;
    vec3 dir;
    float intesity;
};

struct PointLight{
    vec3 color;
    vec3 pos;
    float intesity;
    float linAttenuation;
};

struct SpotLight{
    vec3 color;
    vec3 dir;
    vec3 pos;
    vec3 intesity;
    float linAttenuation;
    float minAngle;
    float MaxAngle;
};

//For the materials being effected by light
struct Material{
    vec3 color;
    float AmbientK;
    float DiffuseK;
    float SpecularK; //0-1
    float Shininess; //1-512
};

struct Camera{
    vec3 pos;
    vec3 dir;
};
#define MAX_LIGHTS 8

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform Material material;
uniform Camera camera;

float Ambient(float intensity)
{
    float ambient = material.AmbientK * intensity;

    return ambient;
}

float Diffuse(float intensity, vec3 lightDir)
{
    vec3 normal = normalize(v_out.WorldNormal);

    float diff = max(dot(normal, lightDir), 0.0);
    float diffuse = material.DiffuseK * diff * intensity;

    return diffuse;
}

float Specular(vec3 reflectDir, float intensity)
{
    vec3 viewDir = normalize(camera.pos - v_out.WorldPosition);
    float specDot = max(dot(reflectDir, viewDir), 0.0); //max is helping clamp
    float specPow = pow(specDot, material.Shininess);
    float specular = material.SpecularK * specPow * intensity;

    return specular;
}


void main(){      
    vec3 normal = normalize(v_out.WorldNormal);
    
    //vec3 result = (Ambient() + Diffuse()) * ObjectColor;

    vec3 cameraDir = normalize(camera.pos - v_out.WorldPosition);

    FragColor = vec4(abs(normal),1.0f); //abs(normal)
}