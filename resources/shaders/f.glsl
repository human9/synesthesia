#version 150

varying float position;
varying float height;

void main()
{
	gl_FragColor = vec4((-1*height+1), 0.0, (height+1),  1.0);
}
