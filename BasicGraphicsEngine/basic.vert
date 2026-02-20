#version 330 core
layout (location = 0) in vec3 aPos;    // Position from VBO
layout (location = 1) in vec2 aTexCoords; // TexCoords from VBO
layout (location = 2) in vec3 aNormal;  // Normals from VBO

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate the position in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Calculate Fragment Position in World Space
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Calculate the Normal (Crucial for lighting!)
    // We use the inverse transpose of the model matrix to transform normals
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    
    TexCoords = aTexCoords;
}