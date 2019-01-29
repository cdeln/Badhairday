#version 150

in  vec3 inPosition;
in  vec3 inNormal;
out float shade;

// NY
//uniform mat4 myMatrix;
uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
	const vec3 light = vec3(0.58, 0.58, 0.58);
	
	vec3 normal = mat3(modelviewMatrix) * inNormal; // Rotate normal (simple normal matrix, uniform scaling only)
	shade = dot(normalize(normal), light);

	gl_Position = projectionMatrix * modelviewMatrix * vec4(inPosition, 1.0);
}
