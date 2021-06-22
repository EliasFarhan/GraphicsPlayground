#version 300 es
precision highp float;
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform bool reinhard;
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    if(hdr)
    {
        vec3 result;
        if(reinhard)
        {
            result = hdrColor / (hdrColor + vec3(1.0));
        }
        else 
        {
            // exposure tonemapping
            result = vec3(1.0) - exp(-hdrColor * exposure);
        }
        // also gamma correct while we're at it
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        // simple gamma correction
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}