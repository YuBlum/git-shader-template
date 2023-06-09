#version 460 core

out vec4 out_color;

uniform vec2  resolution;
uniform vec2  mouse;
uniform float time;
vec2 uv, ms;

void
main() {
  uv = (gl_FragCoord.xy * 2 - resolution) / resolution.y;
  ms = (mouse * 2 - resolution) / resolution.y;
  out_color = vec4(0.2);
}
