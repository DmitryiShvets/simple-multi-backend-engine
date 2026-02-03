#version 330 core

uniform vec4 customColor;
out vec4 color;
void main() 
{
    color = customColor;
}