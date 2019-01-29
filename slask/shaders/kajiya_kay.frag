#version 150

// Simplified Phong: No materials, only one, hard coded light source
// (in view coordinates) and no ambient

// Note: Simplified! In particular, the light source is given in view
// coordinates, which means that it will follow the camera.
// You usually give light sources in world coordinates.

in vec3 f_Normal; // Phong
in vec3 f_View; // Phong (specular)

out vec4 outColor;

uniform vec3 light;

void main(void)
{
    const float diffuse_factor = 0.7;
    const float specular_factor = 0.8;
    const float specular_exp = 2; 
	float diffuse, specular, shade;
	
	// Diffuse
	diffuse = length(cross(normalize(f_Normal), light)); // Hair shading
	//diffuse = dot(normalize(f_Normal), light);
	diffuse = max(diffuse, 0);
	
	// Specular
	vec3 v = normalize(f_View); 
    vec3 half_Vector = normalize(light + v); 
	specular = max(sqrt(1 - pow(dot(half_Vector, f_Normal),2)), 0); //Kajiya-Kay Model

    specular = pow(specular, specular_exp);
	shade = diffuse_factor * diffuse + specular_factor * specular;

	outColor = vec4(shade, shade, shade, 1.0);
}
