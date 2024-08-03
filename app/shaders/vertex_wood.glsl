#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexCoords;

void main()
{
    gl_Position = vertexPosition;
}
