uniform samplerRect texture;
uniform samplerRect texture2;
uniform float threshold;

void main (void)
{
    vec4 lowColor=textureRect( texture, gl_TexCoord[0].st);
    vec4 lowColor2=textureRect( texture2, gl_TexCoord[0].st);

   gl_FragColor=threshold*lowColor+(1-threshold)*lowColor2;
}