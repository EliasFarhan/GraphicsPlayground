#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    float v = texture(screenTexture, TexCoords).r;
    FragColor = vec4(v,v,v, 1.0);
}