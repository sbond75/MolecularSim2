shader_type spatial;

// INSTANCE_CUSTOM is float height_scale;

void vertex(){
	float height_scale = INSTANCE_CUSTOM.x;
	vec3 scale = vec3(1.0, height_scale, 1.0);
	VERTEX = VERTEX * scale;
}
