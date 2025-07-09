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

vec3 hsv2hsv(float h, float s, float v) {
    return vec3(h,s,v);
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

vec3 iterationsToRainbow (float mb) {
    return hsv2rgb(mb,1.0,0.9);
}

vec3 iterationsToInvertedRainbow (float mb) {
    return hsv2rgb(1-mb,1.0,0.9);
}

// function mostly written by DeepSeek AI
vec3 iterationsToInferno(float mb) {
    // Clamp the value between 0 and 1
    mb = clamp(mb, 0.0, 1.0);
    
    // Key points in the inferno colormap (mb, R, G, B)
    // Note: GLSL doesn't support 2D arrays like C++, so we'll use separate arrays
    const float positions[9] = float[9](
        0.0, 0.13, 0.25, 0.38, 0.5, 0.63, 0.75, 0.88, 1.00
    );
    
    const vec3 colors[9] = vec3[9](
        vec3(0.0, 0.0, 0.0) / 255.0,
        vec3(27.0, 11.0, 120.0) / 255.0,
        vec3(81.0, 18.0, 123.0) / 255.0,
        vec3(134.0, 22.0, 110.0) / 255.0,
        vec3(185.0, 39.0, 88.0) / 255.0,
        vec3(225.0, 69.0, 41.0) / 255.0,
        vec3(243.0, 114.0, 22.0) / 255.0,
        vec3(252.0, 193.0, 50.0) / 255.0,
        vec3(252.0, 255.0, 164.0) / 255.0
    );
    
    // Find the interval where the value lies
    for (int i = 0; i < 8; i++) {
        if (mb >= positions[i] && mb <= positions[i+1]) {
            // Linear interpolation between the two colors
            float t = (mb - positions[i]) / (positions[i+1] - positions[i]);
            return mix(colors[i], colors[i+1], t);
        }
    }
    
    // Fallback (shouldn't be reached if value is properly clamped)
    return colors[8];
}

vec3 iterationsToInvertedInferno(float mb) { return iterationsToInferno(1-mb); }


uniform int colorFuncIndex;

void main()
{
    vec2 ndc = (TexCoord * 2.0 - 1.0);// * vec2(aspectRatio, 1.0); // normalized coordinates [-1,1]
    ndc /= zoom;
    vec2 uv = ndc * vec2(aspectRatio, 1.0) * 0.5 + 0.5 + center;
    float mb = iterateMandelbrot(uv, maxRepetitions); // * 255;
    vec3 color;
    switch (colorFuncIndex) {
        case 0: color = iterationsToRainbow(mb); break;
        case 1: color = iterationsToInvertedRainbow(mb); break;
        case 2: color = iterationsToInferno(mb); break;
        case 3: color = iterationsToInvertedInferno(mb); break;
        default: color = iterationsToRainbow(mb);
    }
    // vec3 color = colorFuncs[colorFuncIndex](mb);
    FragColor = vec4(color, 1.0);
}