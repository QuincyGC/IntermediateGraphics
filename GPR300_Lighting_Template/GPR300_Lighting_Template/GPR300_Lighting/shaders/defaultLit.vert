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
out vec3 normal;
out mat3 TBN;



void main(){    
    v_out.WorldPosition = vec3(_Model * vec4(vPos,1));

    vec3 normalCalc = vNormal;

    v_out.Tangent = normalize(vec3(_Model * vec4(vTangent, 0)));
    uv = vUV;

    vec3 btCalc = cross(normalCalc, v_out.Tangent);
    
    vec3 BT = normalize(vec3(_Model * vec4(btCalc, 0)));
    normal = normalize(vec3(_Model * vec4(normalCalc, 0)));

    TBN = mat3(
        v_out.Tangent.x, BT.x, normal.x,
        v_out.Tangent.y, BT.y, normal.y,
        v_out.Tangent.z, BT.z, normal.z
        );

    TBN = transpose(inverse(mat3(_Model))) * TBN;
    normal = TBN * normal;

    v_out.WorldNormal = normal;

    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
