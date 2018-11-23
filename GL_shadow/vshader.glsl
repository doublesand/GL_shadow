#version 330 core 

in vec3 vPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() 
{
	vec4 v1 = modelMatrix*vec4(vPosition, 1.0);  
	// 由于modelMatrix有可能为阴影矩阵，为了得到正确位置，我们需要做一次透视除法
	vec4 v2 = vec4(v1.xyz / v1.w, 1.0);
	// 考虑相机和投影
	vec4 v3 = projMatrix*viewMatrix*v2;
	// 再做一次透视除法 (为了明确概念，我们显式做一次透视除法)
	vec4 v4 = vec4(v3.xyz / v3.w, 1.0);
	gl_Position = v4;
}