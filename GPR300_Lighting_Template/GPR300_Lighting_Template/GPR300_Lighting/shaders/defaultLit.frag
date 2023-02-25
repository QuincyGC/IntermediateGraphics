#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct DirectionLight{
    vec3 color;
    vec3 direction;
    float intensity;
};

struct PointLight{
    vec3 color;
    vec3 pos;
    float intensity;
    float linearFallOff;
    float quadFallOff;
};

struct SpotLight{
    vec3 color;
    vec3 direction;
    vec3 pos;
    float intensity;
    float linAttenuation;
    float minAngle;
    float maxAngle;
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
#define MAX_LIGHTS 2

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform Material material;
uniform Camera camera;

uniform DirectionLight dLit;
uniform PointLight pLit[MAX_LIGHTS];
uniform SpotLight sLit;

float Ambient(float intensity)
{
    float ambient = material.AmbientK * intensity;

    return ambient;
}

float Diffuse(float intensity, vec3 lightDir)
{
    vec3 normal = normalize(v_out.WorldNormal);

    float dotProd = max(dot(lightDir, normal), 0.0);
    float diffuse = material.DiffuseK * dotProd * intensity;

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

//Get Directional light
vec3 dirLit(DirectionLight dirLight, vec3 normal)
{
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 reflectDir = reflect(-lightDir, normal);

    float ambient = Ambient(dirLight.intensity);
    float diffuse = Diffuse(dirLight.intensity, lightDir);
    float specular = Specular(reflectDir, dirLight.intensity);

    vec3 result = (ambient + diffuse + specular) * dirLight.color;

    return result;
};

vec3 pointLit(PointLight pointLight, vec3 normal)
{
    float constCoefficient = 1;
   
    vec3 lightDir = normalize(pointLight.pos - v_out.WorldPosition);
    float dist = length(pointLight.pos - v_out.WorldPosition);
    float attenuation = 1 / (constCoefficient + (pointLight.linearFallOff * dist) + (pointLight.quadFallOff * pow(dist,2)));

    vec3 reflectDir = reflect(-lightDir, normal);

    float ambient = Ambient(pointLight.intensity);
    float diffuse = Diffuse(pointLight.intensity, lightDir);
    float specular = Specular(reflectDir, pointLight.intensity);

     vec3 result = (ambient + diffuse + specular) * pointLight.color * attenuation;

     return result;
};

vec3 spotLit()
{
    vec3 color;
    vec3 dir;
    vec3 pos;
    vec3 intensity;
    float linAttenuation;
    float minAngle;
    float MaxAngle;
     return vec3(0);
};

void main(){      
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 cameraDir = normalize(camera.pos - v_out.WorldPosition);

    vec3 result = dirLit(dLit, normal);
   
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        result += pointLit(pLit[i], normal);
    }
    

    FragColor = vec4((result * material.color),1.0f); //abs(normal)
}