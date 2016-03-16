#version 150

attribute vec2 osc;
uniform float y_pos;
uniform float len;
varying float position;
varying float height;

void main(void)
{
	float x = (osc.x - len) / len;
	gl_PointSize = 0.2+abs(osc.y)*10;
	gl_Position = vec4(x, osc.y / 2.0 + y_pos, 0.0, 1.0);
	position = x;
	height = osc.y;
}
