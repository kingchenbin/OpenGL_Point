#version 450

//vertex attribs
in vec4 model_postion;
in vec2 tex_coord_in;

//uniform variables
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform float pointSize;

out vec2 tex_coord;

void main(void)
{
    vec4 eye_coord, clip_coord;
    eye_coord = modelViewMatrix * model_postion;
    clip_coord = projectionMatrix * eye_coord;

    tex_coord = tex_coord_in;
    gl_Position = clip_coord;
    gl_PointSize = pointSize;
}
