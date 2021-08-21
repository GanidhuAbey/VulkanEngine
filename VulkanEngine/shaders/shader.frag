#version 450

layout(push_constant) uniform PushFragConstant {
  vec3 lightColor;
  vec3 lightPosition;
  vec3 objectColor;
} pfc;

layout(location=0) out vec4 outColor;
layout(location=2) in vec3 surfaceNormal;
layout(location=3) in vec4 vPos;

in vec4 gl_FragCoord;

void main() {
    //get vector of light
    vec3 lightToObject = normalize(pfc.lightPosition - vec3(vPos));

    float lightIntensity = max(0.01f, dot(lightToObject, surfaceNormal));
    //float mapIntensity = (lightIntensity/2) + 0.5;

    vec3 newColor = lightIntensity * pfc.lightColor;


    outColor = vec4(newColor * pfc.objectColor, 1.0);
}
