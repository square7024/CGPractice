#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;

out vec3 fragColor;

uniform mat4 modelTransform;

void main()
{
    gl_Position = modelTransform * vec4(vPos, 1.0);
    fragColor = vColor;
}
