#version 120

attribute vec4 aPos;
attribute vec3 aNor;
attribute vec2 aTex;
attribute vec4 dPos1;
attribute vec4 dPos2;
attribute vec3 dNor1;
attribute vec3 dNor2;

uniform float weight1;
uniform float weight2;
uniform mat4 P;
uniform mat4 MV;


varying vec3 vPos;
varying vec3 vNor;
varying vec2 vTex;

void main()
{
	vec4 newPos = aPos + (weight1 * dPos1) + (weight2 * dPos2);
	newPos[3] = 1.0f;
	vec3 newNor = aNor + weight1 * dNor1 + weight2 * dNor2;
	newNor = normalize(newNor);
	vec4 posCam = MV * newPos;
	vec3 norCam = (MV * vec4(newNor, 0.0)).xyz;
	gl_Position = P * posCam;
	vPos = posCam.xyz;
	vNor = norCam;
	vTex = aTex;
}
