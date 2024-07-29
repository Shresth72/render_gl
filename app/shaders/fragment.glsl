#version 330 core

in vec2 fragmentTexCoord;

out vec4 finalColor;

layout (std140) uniform diskParameters {
  vec4 InnerColor;
  vec4 OuterColor;
  float InnerRadius;
  float OuterRadius;
};

void main() {
  float dx = fragmentTexCoord.x - 0.5;
  float dy = fragmentTexCoord.y - 0.5;
  float dist = sqrt(dx * dx + dy * dy);

  finalColor = mix(InnerColor, OuterColor, smoothstep(InnerRadius, OuterRadius, dist));
}
