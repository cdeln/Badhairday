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
    const float specular_exp = 50.0;
    float diffuse, specular, shade;

    vec3 l = normalize(light);
    vec3 n = normalize(f_Normal);
    vec3 v = normalize(- f_View );

    // Diffuse
    diffuse = dot(n, l); 
    diffuse = max(0.0, diffuse); // No negative light

    // Specular
    vec3 r = reflect(- l, n);
    specular = max(dot(r, v), 0);
    specular = max(specular, 0.0);
    specular = pow(specular, specular_exp);
    shade = diffuse_factor*diffuse + specular_factor*specular;

    outColor = vec4(shade, shade, shade, 1.0);
}
