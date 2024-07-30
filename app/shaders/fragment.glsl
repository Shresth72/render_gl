#version 330 core

in vec2 fragmentTexCoord;

out vec4 finalColor;

layout (std140) uniform diskParameters {
  vec4 InnerColor;
  vec4 OuterColor;
  float InnerRadius;
  float OuterRadius;
};

float distToPoint(vec2 p1, vec2 p2) {
  float dx = p1.x - p2.x;
  float dy = p1.y - p2.y;
  return sqrt(dx * dx + dy * dy);
}

void main() {
  float dist = distToPoint(fragmentTexCoord, vec2(0.5, 0.5));

  finalColor = mix(InnerColor, OuterColor, smoothstep(InnerRadius, OuterRadius, dist));
}
