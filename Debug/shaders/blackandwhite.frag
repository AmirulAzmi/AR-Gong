uniform samplerRect tex;
uniform float threshold;

void main (void)
{
    vec4 lowColor=textureRect( tex, gl_TexCoord[0].st);
    float averageColor= (lowColor.r+lowColor.b+lowColor.g)/3.;
   // float threshold=0.5;
    if (averageColor>threshold)
    {
    gl_FragColor.r=0.;
    gl_FragColor.g=0.;
    gl_FragColor.b=0.;
	}
	else
	{
    gl_FragColor.r=1.;
    gl_FragColor.g=1.;
    gl_FragColor.b=1.;
	}
    gl_FragColor.a=1.0;
}