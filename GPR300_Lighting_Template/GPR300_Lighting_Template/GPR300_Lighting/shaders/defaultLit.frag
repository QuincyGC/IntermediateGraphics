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

#define MAX_LIGHTS 8
//const int MAX_LIGHTS = 8;
uniform Light _Lights[MAX_LIGHTS];

vec3 Ambient(vec3 LightColor, vec3 ObjectColor)
{
    float coefficient = .1;
    vec3 ambient = coefficient * LightColor;
    vec3 result = ambient * ObjectColor;

    return result;
}

vec3 Diffuse()
{

    return vec3(0);
}

vec3 Specular()
{
    return vec3(0);
}

void main(){      
    vec3 normal = normalize(v_out.WorldNormal);
    FragColor = vec4(abs(normal),1.0f); //abs(normal)
}