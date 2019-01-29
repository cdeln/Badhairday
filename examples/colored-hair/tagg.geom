#version 150

layout(triangles) in;
layout(line_strip, max_vertices = 27) out; // 3 * (numOfSegs + 1)

in vec3 exNormal[3];
out vec3 f_color;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform float t;

void main()
{
	//f_exNormal = exNormal[0];
	//f_exSurface = exSurface[0];
	float omega = 0.001;
	float hairLength = 0.01;
	int numOfSegs = 10;
	f_color = normalize(exNormal[0]);
	mat4 M = projectionMatrix * modelviewMatrix;

	for(int i = 0; i < 3; ++i) {

		vec4 lastPos = vec4(0,0,0,0);
		vec3 currDir = exNormal[i];	
	
		for(int n = 0; n < numOfSegs; ++n){

		gl_Position = M * (gl_in[i].gl_Position + lastPos);
		EmitVertex();

		vec3 tangent = normalize(cross(currDir, vec3(0,1,0)));
		vec3 offset = normalize(normalize(currDir) + 0.5*sin(n*omega*t)*tangent);
		
		gl_Position = M * (gl_in[i].gl_Position + lastPos + hairLength*vec4(offset, 0));
		EmitVertex();

		EndPrimitive();
		lastPos = lastPos + hairLength*vec4(offset, 0) ;
		currDir = offset;
		}	
	}
}  
