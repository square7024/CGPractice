#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

out vec3 FragPos;								//--- 객체의 위치값을 프래그먼트 세이더로 보낸다.
out vec3 Normal;								//--- 노멀값을 프래그먼트 세이더로 보낸다.

uniform mat4 model;								//--- 모델링 변환값
uniform mat4 view;								//--- 뷰잉 변환값
uniform mat4 projection;						//--- 투영 변환값

void main()
{
	vec4 worldPos = model * vec4(vPos, 1.0);
	FragPos = worldPos.xyz;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	Normal = normalize(normalMatrix * vNormal);
	
	gl_Position = projection * view * worldPos;
}
