#version 150

varying float position;
varying float height;

void main()
{
	if (height != 0)
	gl_FragColor = vec4((position+1), 0.0, (-1*position+1),  1.0);
	/*
	gl_FragColor[0] = 0.0;
	gl_FragColor[1] = 1.0;
	gl_FragColor[2] = 1.0;
	gl_FragColor[3] = 1.0;
	*/
}
