#version 330 core

#define LightArraySize 1

struct SLight {
    vec3 Position;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

struct SMaterial {
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Shininess;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragOutColor;

uniform SLight LightArray[LightArraySize]; // Array of lights
uniform int LightCount;                   // Actual number of lights
uniform SMaterial Material;               // Material properties
uniform vec3 ViewPos;                     // Camera position
uniform vec3 ObjectColor;                 // Object base color

void main()
{
    vec3 ResultColor = vec3(0.0); // Accumulate light contributions

    // Normalize inputs
    vec3 Norm = normalize(Normal);
    vec3 ViewDir = normalize(ViewPos - FragPos);

     for (int i = 0; i < LightArraySize; ++i) {
             // Calculate lighting for each light source

             // Direction to the light
             vec3 LightDir = normalize(LightArray[i].Position - FragPos);

             // Ambient component
             vec3 Ambient = LightArray[i].Ambient * Material.Ambient * ObjectColor;

             // Diffuse component
             float Diff = max(dot(Norm, LightDir), 0.0);
             vec3 Diffuse = LightArray[i].Diffuse * (Diff * Material.Diffuse * ObjectColor);

             // Specular component
             vec3 ReflectDir = reflect(-LightDir, Norm);
             float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Material.Shininess);
             vec3 Specular = LightArray[i].Specular * (Spec * Material.Specular);

             // Accumulate contributions
             ResultColor += Ambient + Diffuse + Specular;
         }

    FragOutColor = vec4(ResultColor, 1.0);
}
