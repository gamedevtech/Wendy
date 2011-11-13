
in vec3 tePosition;
in vec3 teNormal;
in vec3 tePatchDistance;
out vec4 FragColor;

void main()
{
   const vec3 lightPos = vec3(0, 10, 0);

  vec3 V = tePosition;
  vec3 N = normalize(teNormal);
  vec3 L = normalize(lightPos - V);

  float ambient = 0.1;
  float diffuse = clamp(dot(N, L), 0.0, 1.0);

  float c1 = tePatchDistance.x;
  float c2 = tePatchDistance.y;
  float c3 = tePatchDistance.z;

  bool black = min(min(c1, c2), c3) < 0.04;

  if (black)
    FragColor = vec4(0,0,0,1);
  else
    FragColor = vec4(vec3(c1,c2,c3) * (ambient + diffuse), 1.0);
}