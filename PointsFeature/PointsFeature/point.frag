#version 450

in vec2 tex_coord;

// uniform variables
uniform sampler2D tex;
uniform bool bEnableSprite;
uniform sampler2D fboTex;
uniform bool bBltFBO;

// ps output
out vec4 color;

void main(void)
{
    if (bBltFBO)
        color = texture(fboTex, tex_coord);
    else if (bEnableSprite)
        color = texture(tex, gl_PointCoord);
    else
        color = vec4(0.0, 0.0, 1.0, 1.0);
}
