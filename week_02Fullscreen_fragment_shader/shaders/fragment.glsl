#version 330

in vec4 vColour;

out vec4 fragColour;

//use uniform to get cpu side data into shader
uniform float uTime;
uniform vec2 uResolution;

const float PI = 3.14159265;

float Circle(in vec2 a_uv, in float a_radius)
{
	vec2 pos = vec2(0.5) - a_uv; 
	a_radius *= 0.75;
	return 1.0 - smoothstep(a_radius - (a_radius * 0.05), a_radius + (a_radius * 0.05), dot(pos, pos) * PI);

}


//from book of shaders (edited to work with current code)
float box(vec2 _st, vec2 _size){

    _size = vec2(0.5) -_size * 0.5;
    vec2 uv = smoothstep(_size,_size + (_size * 0.1),_st);
    uv *= smoothstep(_size,_size + (_size * 0.1),vec2(1.0)-_st);
    return uv.x*uv.y;
}
vec2 rotate2D(vec2 _st, float _angle){ //from book of shaders
    _st -= 0.5;
    _st =  mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle)) * _st;
    _st += 0.5;
    return _st;
}

void main()
{

	vec2 uv = gl_FragCoord.xy / uResolution;
	uv *= 24;
	uv = fract(uv);
	float s = sin(uTime);
	vec4 tColour = vColour;
	if(s < 0)
	{
		tColour = tColour.gbra;
	}
	uv = rotate2D(uv, PI * 0.25);
	//fragColour= mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), tColour, Circle(uv, abs(s)));
	fragColour= mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), tColour, box(uv, vec2(abs(s), abs(s))));
}
