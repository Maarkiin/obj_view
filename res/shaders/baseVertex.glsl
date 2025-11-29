#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
uniform float u_time;

void main()
{
    gl_Position = vec4(aPos.x * sin(u_time), aPos.y * cos(u_time), aPos.z, 1.0); // see how we directly give a vec3 to vec4's constructor
}