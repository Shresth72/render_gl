#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 texCoords;

uniform vec4 fragColor;
uniform sampler2D fragTexture;

void main() {
  vec4 texColor = texture(fragTexture, texCoords)
  color = fragColor; 
}
