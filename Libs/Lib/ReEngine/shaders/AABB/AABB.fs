#version 330 core

out vec4 FragOutColor;

uniform vec3 color;

void main()
{
    FragOutColor = vec4(color, 1.0);
}
