#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform float zoom = 1.0;     // Zoom factor
uniform vec2 offset = vec2(0.0); // Pan offset
vec3 hsv2rgb(float h, float s, float v) {
    vec3 c = vec3(h, s, v);
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0.0,4.0,2.0), 6.0)-3.0)-1.0, 0.0, 1.0);
    return v * mix(vec3(1.0), rgb, c.y);
}

void main()
{
    vec2 uv = TexCoord * zoom + offset;
    float checker = mod(floor(uv.x * 10.0) + floor(uv.y * 10.0), 2.0);
    vec3 color = vec3(checker);
    FragColor = vec4(color, 1.0);
}