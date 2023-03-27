#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vTangent;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec3 Tangent;
}v_out;

out vec2 uv;
vec3 mapNormal;
out mat3 TBN;



void main(){    
    v_out.WorldPosition = vec3(_Model * vec4(vPos,1));

    uv = vUV;
    vec3 normalCalc = vNormal;

    //https://learnopengl.com/Advanced-Lighting/Normal-Mapping 
    v_out.Tangent = normalize(vec3(_Model * vec4(vTangent, 0)));


    vec3 btCalc = cross(normalCalc, v_out.Tangent);
    
    vec3 BT = normalize(vec3(_Model * vec4(btCalc, 0)));
    mapNormal = normalize(vec3(_Model * vec4(normalCalc, 0)));

    //local space
    TBN = mat3(
        v_out.Tangent.x, BT.x, mapNormal.x,
        v_out.Tangent.y, BT.y, mapNormal.y,
        v_out.Tangent.z, BT.z, mapNormal.z
        );

    //TBN to world space
    TBN = transpose(inverse(mat3(_Model))) * TBN;
    mapNormal = TBN * mapNormal;

    v_out.WorldNormal = mapNormal;

    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
