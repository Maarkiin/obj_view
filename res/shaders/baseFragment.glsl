#version 330 core
#define PI 3.14159265359
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;


void main() {
    float x = abs(sin(u_time));
    gl_FragColor = vec4( x, x, x, 1.0f );
}