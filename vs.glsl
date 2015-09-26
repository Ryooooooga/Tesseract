
void main(void) {
	vec4 vertex		= gl_ModelViewMatrix * gl_Vertex;
	gl_Position		= gl_ProjectionMatrix * vec4(vertex.xyz, 1.0);
	gl_FrontColor	= vec4(0.0, -(vertex.z-1.0)*0.5, 0.0, 1.0);
}
