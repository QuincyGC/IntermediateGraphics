#version 450                          
out vec4 FragColor;

in vec3 Normal;

in struct Vertex{
    vec3 WorldNormal;
}vs_out; 

struct Material{
    vec3 Color;
    float ambientK;
    float diffuseK;
    float SpecularK; //ranges from 0-1
    float Shininess; //(1-512)
};

struct DirectionLight{
    vec3 dir;
    vec3 color;
    float intensity;
};

struct PointLight{
    vec3 color;
    vec3 pos;
    float intensity;
    float linAtt; //0-1 linear attenuation
};

struct SpotLight{
    vec3 Color;
    vec3 pos;
    vec3 dir;
    float intensity;
    float linAtt;
    float minAngle;//Penumbra
    float maxAngle;//Umbra
};

void main(){         

    vec3 normal = normalize(vs_out.WorldNormal); //MIGHT CAUSE FUTURE PROBLEMS

    FragColor = vec4(abs(normal),1.0f);
}
