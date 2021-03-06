#version 330 core

layout (location = 0) in vec3 _pos;
layout (location = 1) in vec3 _normal;
layout (location = 2) in vec2 _texCoord;
//layout (location = 3) in vec3 _vertColor;

out vec2 texCoord;
//out vec3 vertColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(_pos, 1.0);
	//vertColor = _vertColor;
	texCoord = _texCoord;
}