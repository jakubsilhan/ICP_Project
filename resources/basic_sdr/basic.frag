#version 460 core

uniform bool useUniformColor;
uniform vec4 uniformColor;

in vec3 color; // input from vertex stage of graphics pipeline, automatically interpolated
out vec4 FragColor; // output color of current fragment: MUST be written

void main()
{
    if (useUniformColor) {
        FragColor = uniformColor;
    } else {
        FragColor = vec4(color, 1.0f); // copy RGB color, add Alpha=1.0 (not transparent)
    }
}
