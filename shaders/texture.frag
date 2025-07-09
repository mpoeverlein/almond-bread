#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform float aspectRatio = 1.0f; // dummy value
uniform float zoom = 1.0;     // Zoom factor
uniform vec2 center = vec2(0.5); // Pan center
uniform int maxRepetitions = 10;
const float convergence_radius_squared = 4.0f;

vec3 hsv2rgb(float h, float s, float v) {
    vec3 c = vec3(h, s, v);
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0.0,4.0,2.0), 6.0)-3.0)-1.0, 0.0, 1.0);
    return v * mix(vec3(1.0), rgb, c.y);
}


/**
 * @brief
 * For a complex number c = a + bi, count how many iterations it takes
 * until the magnitude of z_n = z^2_n-1 + c is larger than 2. 
 * The result is divided by maxRepetitions.
 * 
 * @param uv real and imaginary value of input complex number
 * @param maxIterations after how many interations to stop
 */
float iterateMandelbrot(vec2 uv, int maxRepetitions) {
    float a = uv.x;
    float b = uv.y;
    float tmp_a = uv.x;
    float tmp_b = uv.y;
    for (int i = 0; i < maxRepetitions; ++i) {
        float original_a = tmp_a;
        float original_b = tmp_b;
        tmp_a = original_a*original_a - original_b*original_b + a;
        tmp_b = 2*original_a*original_b + b;
        if (tmp_a*tmp_a + tmp_b*tmp_b > convergence_radius_squared) {
            return float(i) / maxRepetitions;
        }
    }
    return 1.0;
}

void main()
{
    vec2 ndc = (TexCoord * 2.0 - 1.0);// * vec2(aspectRatio, 1.0); // normalized coordinates [-1,1]
    ndc /= zoom;
    vec2 uv = ndc * vec2(aspectRatio, 1.0) * 0.5 + 0.5 + center;
    // float checker = mod(floor(uv.x * 10.0) + floor(uv.y * 10.0), 2.0);
    float mb = iterateMandelbrot(uv, maxRepetitions); // * 255;
    // int mb = iterateMandelbrot(uv, maxRepetitions); 
    // vec3 color = vec3(mb);
    vec3 color = hsv2rgb(mb,1.0,1.0);
    FragColor = vec4(color, 1.0);
    // FragColor = vec4(uv, 0.0, 1.0);
}