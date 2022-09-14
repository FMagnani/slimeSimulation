#version 150

// this is how we receive the texture
uniform sampler2DRect tex0;
in vec2 texCoordVarying;

vec2 offset[9];
vec4 texel0;
vec4 sum;

out vec4 outputColor;

void main()
{

    offset[0] = vec2( 0.0, -1.0);
    offset[1] = vec2(-1.0,  0.0);
    offset[2] = vec2( 0.0,  0.0);
    offset[3] = vec2( 1.0,  0.0);
    offset[4] = vec2( 0.0,  1.0);

    offset[5] = vec2(-1.0, -1.0);
    offset[6] = vec2( 1.0, -1.0);
    offset[7] = vec2(-1.0,  1.0);
    offset[8] = vec2( 1.0,  1.0);

    sum = vec4(0, 0, 0, 0);
    for(int i=0; i<9; i++){
        sum += texture(tex0, texCoordVarying + offset[i]);
    }
    sum /= 9;

    texel0 = texture(tex0, texCoordVarying);

//    float a = max(0, sum.a*0.1);

/*
    float threshold = decayRate*0.1;
    if(sum.r < threshold){
        outputColor = vec4(0,0,0, 1);
    }else{
        outputColor = sum;
    }
*/

//    outputColor *= 0.1;
    outputColor = sum;
/*
    if(texCoordVarying.x == 0 && texCoordVarying.y == 0){
        outputColor = vec4(255,255,255, 255);
    }
*/
}
