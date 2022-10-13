shader_type spatial;

void fragment() {
    ALBEDO = COLOR.xyz;
}

// uniform float time_scale;
// uniform float side_to_side;

// void vertex(){
// 	//time_scale is a uniform float
// 	float time = TIME * 1.0* time_scale;
	
// 	//side_to_side is a uniform float
// 	VERTEX.x += cos(time) * 0.01;
	
// 	float body = (VERTEX.z + 1.0) / 2.0; //for a fish centered at (0, 0) with a length of 2
// 	VERTEX.x += cos(time + body) * 1.0;
	
// 	//twist is a uniform float
// 	float twist_angle = cos(time + body) * 0.3 * 1.0;
// 	mat2 twist_matrix = mat2(vec2(cos(twist_angle), -sin(twist_angle)), vec2(sin(twist_angle), cos(twist_angle)));

// 	VERTEX.xy = twist_matrix * VERTEX.xy;
// }
