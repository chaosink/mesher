#version 400 core

in Attribute {
	vec3 position;
	vec3 normal;
} vertexIn;

uniform mat4 mv;

out vec3 color;

vec3 Lighting(vec3 light_position, float LightPower, int light_n) {
	vec3 LightColor = vec3(1.f, 1.f, 1.f);

	vec3 MaterialDiffuseColor = vec3(0.9f, 0.3f, 0.3f);
	vec3 MaterialAmbientColor = vec3(0.4f, 0.4f, 0.4f) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3f, 0.3f, 0.3f);

	vec3 light_direction = light_position - vertexIn.position;
	float distance = length(light_direction);

	vec3 n = normalize(vertexIn.normal);
	vec3 l = normalize(light_direction);
	float cosTheta = clamp(dot(n, l), 0.f, 1.f);

	vec3 E = normalize(-vertexIn.position);
	vec3 R = reflect(-l, n);
	float cosAlpha = clamp(dot(E, R), 0.f, 1.f);

	color =
		MaterialAmbientColor / light_n +
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance * distance) +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha, 5.f) / (distance * distance);

	return color;
}

void main() {
	vec3 light_position_0 = (mv * vec4( 20.f, 20.f, 10.f, 1.f)).xyz;
	vec3 light_position_1 = (mv * vec4(-20.f, 10.f,-10.f, 1.f)).xyz;

	vec3 c = vec3(0.f);

	c += Lighting(light_position_0, 40.f, 2);
	c += Lighting(light_position_1, 30.f, 2);
	// c = vec4(1.f, 1.f, 1.f, 1.f);
	// c = vertexIn.normal;

	color = c;
}
