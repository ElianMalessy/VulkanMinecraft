#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
	vec4 quaternion;
	vec4 translate;
	mat4 projectionMatrix;
	vec3 color;
} push;

vec3 qrot(vec4 q, vec3 v) 
{ 
    return v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
}
void main() {
  gl_Position = push.projectionMatrix * vec4(push.translate.w * qrot(push.quaternion, position) + push.translate.xyz, 1.0);
	fragColor = color;
} 