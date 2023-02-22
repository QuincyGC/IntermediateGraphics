#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct Light{
    vec3 position;
    float intensity;
    vec3 color;
};

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

vec3 Ambient()
{
    float coefficient = .1;
    vec3 ambient = coefficient * LightColor;

    return ambient;
}

vec3 Diffuse()
{
    float coefficient = .1;

    vec3 lightDir = normalize(lightPos - v_out.WorldPosition);

    vec3 surfaceNormal = normalize(v_out.WorldNormal);

    float intensity;

    float diff = max(dot(surfaceNormal, lightDir), 0.0);
    vec3 diffuse = diff * LightColor;


    return diffuse;
}

vec3 Specular()
{
    
    return vec3(0);
}

#define MAX_LIGHTS 8
//const int MAX_LIGHTS = 8;
uniform Light _Lights[MAX_LIGHTS];

uniform vec3 lightPos;

void main(){      
    vec3 normal = normalize(v_out.WorldNormal);
    
    //vec3 result = (Ambient() + Diffuse()) * ObjectColor;

    FragColor = vec4(abs(normal),1.0f); //abs(normal)
}