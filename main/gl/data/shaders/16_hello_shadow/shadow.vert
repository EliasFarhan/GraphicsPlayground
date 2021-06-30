#version 300 es
precision highp float;

layout (location = 0) in vec3 aPos; // local space 
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 transposeInverseModel; //normal matrix
uniform mat4 lightSpaceMatrix; // camera from light point of view with view and projection

void main()
{    
    FragPos = vec3(model * vec4(aPos, 1.0)); // world space
    Normal = mat3(transposeInverseModel) * aNormal;
    TexCoords = aTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); // light pass, clip space
    gl_Position = projection * view * vec4(FragPos, 1.0);
}