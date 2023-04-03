#version 450                          
//Quincy Code

out vec4 FragColor;
in vec2 UV;

uniform sampler2D _texture;

void main(){         
    
    //greyscale
    vec4 color = texture(_texture, UV);

    float average = (color.x+color.y+color.z) / 3;

    FragColor = vec4(vec3(average), 1.0);

}
