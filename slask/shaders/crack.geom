#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 exNormal[];
out vec4 f_color;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	//f_exNormal = exNormal[0];
	//f_exSurface = exSurface[0];
	mat4 M = projectionMatrix * modelviewMatrix;

	vec3 pos[3];
	pos[0] = gl_in[0].gl_Position.xyz;
	pos[1] = gl_in[1].gl_Position.xyz;
	pos[2] = gl_in[2].gl_Position.xyz + 500*exNormal[2];

	vec3 normal = normalize(cross(pos[1]-pos[0],pos[2]-pos[0]));
	const vec3 light = normalize(vec3(1,1,1));
	float shade = dot(light, normal);
	f_color = vec4(shade,shade,shade,1);

	for(int i = 0; i < gl_in.length(); ++i)
	{ 
		gl_Position = M * vec4(pos[i], 1); 
		EmitVertex();
	}
	
	EndPrimitive();
}  
