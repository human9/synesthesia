#version 150

attribute vec2 osc;
uniform float y_pos;

void main(void)
{
	gl_Position = vec4(osc.x, osc.y + y_pos, 0.0, 1.0);
}
