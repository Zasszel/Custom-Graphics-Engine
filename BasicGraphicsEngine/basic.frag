#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 lightPos;      
uniform sampler2D texture1; 
uniform vec3 viewPos;
uniform vec3 objectColor; // Keep this, but we'll use it differently for particles/base color

void main()
{    
    // 1. Get the texture color FIRST
    vec4 texColor = texture(texture1, TexCoords);
    vec3 materialDiffuseColor = texColor.rgb; // <-- Use the sampled color for lighting
    
    // Fallback: If you want to multiply the texture by the base color (which gives the tint):
    // materialDiffuseColor *= objectColor; 
    // BUT we want to remove the tint, so we will only use texColor.rgb

    // 2. Ambient Lighting
    vec3 ambient = 0.1 * materialDiffuseColor; // Ambient light is now based on stone color

    // 3. Diffuse Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuseColor; // Diffuse light is now based on stone color

    // 4. Final Result
    vec3 result = ambient + diffuse;

    // Apply lighting result directly to the texture color
    FragColor = vec4(result, 1.0); // We've already included the color in 'result'
}