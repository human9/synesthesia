#version 150

attribute vec2 osc;
uniform float y_pos;
uniform float len;

void main(void)
{
	float x = (osc.x - len) / len;
	gl_Position = vec4(x, (osc.y / 32768.0) / 2.0 + y_pos, 0.0, 1.0);
}
