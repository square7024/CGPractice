#version 330 core
in vec4 FragPos;												//--- 노멀값을 계산하기 위해 객체의 위치값을 버텍스 세이더에서 받아온다.
in vec3 Normal;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main ()
{
vec3 ambientLight = 0.3;									//--- 주변 조명 계수: 0.0 ≤ ambientLight ≤ 1.0
vec3 ambient = ambientLight * lightColor;					//--- 주변 조명값
	
vec3 normalVector = normalize (Normal);
vec3 lightDir = normalize (lightPos - FragPos);
float diffuseLight = max (dot (norm, lightDir), 0.0);		//--- N과 L의 내적 값으로 강도 조절: 음수 방지
float diffuse = diffuseLight * lightColor;					//--- 산란 반사 조명값: 산란반사값 * 조명색상값

	//int shininess = 128;										//--- 광택 계수
	//vec3 viewDir = normalize (viewPos - FragPos);				//--- 관찰자의 방향
	//vec3 reflectDir = reflect (-lightDir, normVector);			//--- 반사 방향: reflect 함수 - 입사 벡터의 반사 방향 계산
	//float specularLight = max (dot (viewDir, reflectDir), 0.0); //--- V와 R의 내적값으로 강도 조절: 음수 방지
	//specularLight = pow(specularLight, shininess);				//--- shininess 승을 해주어 하이라이트를 만들어준
	//vec3 specular = specularLight * lightColor;					//--- 거울 반사 조명값: 거울반사값 * 조명색상값

vec3 result = (ambient + diffuse) * objectColor;						//--- 객체의 색과 주변조명값을 곱하여 최종 객체 색상 설정

FragColor = vec4 (result, 1.0);
}