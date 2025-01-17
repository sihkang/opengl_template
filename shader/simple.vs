#version 330 core // 3.3 core profile 명시
layout (location = 0) in vec3 aPos;

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(aPos, 1.0);
}