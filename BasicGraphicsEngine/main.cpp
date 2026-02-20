#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace std;
using namespace glm;

// --- Global Constants and Engine State ---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Global variables for windowing and timing
GLFWwindow* window = nullptr;
int width = SCR_WIDTH;
int height = SCR_HEIGHT;
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Input state
bool canClick = true; // Debounce flag to prevent rapid-fire clicking

// Camera Matrices
mat4 view;
mat4 projection;

// --- Shader Class ---
// Handles loading shader source code from files, compiling them, 
// linking them into a program, and managing uniforms.
class Shader {
public:
    unsigned int ID; // The Program ID

    Shader(const char* vs, const char* fs);

    // Activate the shader
    void use() { glUseProgram(ID); }

    // Utility functions to set Uniforms (variables sent from CPU to GPU)
    void setVec3(const char* name, const vec3& v) {
        glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(v));
    }

    void setMat4(const char* name, const mat4& m) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(m));
    }

    void setInt(const char* name, int value) {
        glUniform1i(glGetUniformLocation(ID, name), value);
    }
};

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vCode, fCode;
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);

    vCode.assign(std::istreambuf_iterator<char>(vFile), {});
    fCode.assign(std::istreambuf_iterator<char>(fFile), {});

    const char* vSrc = vCode.c_str();
    const char* fSrc = fCode.c_str();

    // 2. Compile shaders
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vSrc, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fSrc, nullptr);
    glCompileShader(fs);

    // 3. Link shaders into a Program
    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);

    // 4. Delete individual shaders as they're linked into the program now
    glDeleteShader(vs);
    glDeleteShader(fs);
}

// --- Texture Loading ---
// Uses stb_image to load a file and generate an OpenGL texture ID
unsigned int LoadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping (Repeat) and filtering (Linear)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

// --- Geometry Setup ---
unsigned int cubeVAO = 0, cubeVBO = 0;

void InitCube()
{
    // Vertex Data: Position (x,y,z), Texture Coords (u,v), Normals (nx,ny,nz)
    float vertices[] = {
        // ... (Vertex data omitted for brevity, matches original code) ...
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        // ... (Assume remaining faces are here as in your source) ...
         // Back face
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

        // Right face
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,

         // Top face
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,

         // Bottom face
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Layout 0: Position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Layout 1: Texture Coords (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Layout 2: Normals (3 floats)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void DrawCube()
{
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// --- Data Structures ---
struct GameObject
{
    vec3 position;
    vec3 color;
    bool active;
    float radius; // For collision detection
    float rotation;
    float rotationSpeed;
    unsigned int textureID;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life; // 1.0 = full life, 0.0 = dead
};

// UI Tweakables
int targetObjectCount = 2;
float objectSpread = 5.0f;
glm::vec3 lightPos(2.0f, 4.0f, 2.0f);

// Engine Logic State
std::vector<GameObject> objects;
std::vector<Particle> particles;

// Generates objects in random locations within the 'objectSpread' range
void SpawnObjects() {
    objects.clear();
    for (int i = 0; i < targetObjectCount; i++) {
        GameObject obj;
        // Random position between -Spread and +Spread
        obj.position = vec3(
            ((rand() % 100) / 50.0f - 1.0f) * objectSpread,
            ((rand() % 100) / 50.0f - 1.0f) * objectSpread,
            ((rand() % 100) / 50.0f - 1.0f) * objectSpread
        );
        obj.color = vec3(1.0f, 0.5f, 0.2f);
        obj.active = true;
        obj.radius = 0.5f; // Matches cube half-width approx
        obj.rotation = 0.0f;
        obj.rotationSpeed = glm::radians(45.0f + rand() % 90);
        obj.textureID = LoadTexture("stone_texture.bmp");
        objects.push_back(obj);
    }
}

// Rotates objects every frame
void UpdateObjects(float deltaTime)
{
    for (auto& obj : objects) {
        if (!obj.active) continue;
        obj.rotation += obj.rotationSpeed * deltaTime;
        if (obj.rotation > glm::two_pi<float>())
            obj.rotation -= glm::two_pi<float>();
    }
}

// --- Raycasting Logic ---
// 1. Converts 2D mouse coordinates (Screen Space) into a 3D ray (World Space).
// 2. Checks intersections between that ray and the game objects.
int CheckObjectClick(double mouseX, double mouseY, int screenW, int screenH, glm::mat4 view, glm::mat4 projection) {
    // A. Normalized Device Coordinates (NDC): range [-1, 1]
    float x = (2.0f * mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenH;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    // B. Clip Space -> Eye Space (Inverse Projection)
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0); // We only want direction, not point

    // C. Eye Space -> World Space (Inverse View)
    glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    // D. Ray Origin is the Camera's position
    glm::vec3 ray_origin = glm::vec3(glm::inverse(view)[3]);

    // E. Intersection Test (Ray vs Sphere)
    for (int i = 0; i < objects.size(); i++) {
        if (!objects[i].active) continue;

        glm::vec3 m = ray_origin - objects[i].position;
        float b = glm::dot(m, ray_wor);
        float c = glm::dot(m, m) - objects[i].radius * objects[i].radius;

        // Exit if ray origin is outside sphere (c > 0) and ray points away (b > 0)
        if (c > 0.0f && b > 0.0f) continue;

        float discr = b * b - c;
        if (discr < 0.0f) continue; // Ray misses sphere

        return i; // Hit found
    }
    return -1; // No hit
}

void SpawnExplosion(glm::vec3 center) {
    for (int i = 0; i < 20; i++) {
        Particle p;
        p.position = center;
        // Random velocity for explosion burst
        p.velocity = glm::vec3(
            (rand() % 100 - 50) / 100.0f,
            (rand() % 100 - 50) / 100.0f,
            (rand() % 100 - 50) / 100.0f
        ) * 6.0f;
        p.life = 1.0f;
        particles.push_back(p);
    }
}

void UpdateParticles(float deltaTime) {
    for (int i = 0; i < particles.size(); i++) {
        particles[i].position += particles[i].velocity * deltaTime;
        particles[i].life -= deltaTime * 1.5f; // Fade out speed

        if (particles[i].life <= 0.0f) {
            particles.erase(particles.begin() + i);
            i--;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

// --- MAIN FUNCTION ---

int main() {
    // 1. Initialize GLFW (Windowing Library)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 2. Create Window
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Click-and-Destroy", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. Initialize GLAD (Loads OpenGL function pointers)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. Setup Resources
    InitCube(); // Generate VAO/VBO
    Shader shader("basic.vert", "basic.frag");
    glEnable(GL_DEPTH_TEST); // Ensure closer objects cover further ones

    // 5. Setup ImGui (Immediate Mode GUI)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 6. Initial Game State
    SpawnObjects();

    // 7. Initial Camera Position (Up 5, Back 10)
    view = glm::lookAt(
        glm::vec3(0.0f, 5.0f, 10.0f), // Camera Eye
        glm::vec3(0.0f, 0.0f, 0.0f),  // Camera Target
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up Vector
    );
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);


    // --- RENDER LOOP ---
    while (!glfwWindowShouldClose(window)) {
        // Calculate DeltaTime (smooth movement regardless of FPS)
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        UpdateObjects(deltaTime);

        // --- INPUT HANDLING ---
        // Only process game clicks if mouse is NOT hovering over UI
        if (io.WantCaptureMouse) {
            canClick = true;
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (canClick) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                // Perform Raycast
                int hitIndex = CheckObjectClick(xpos, ypos, width, height, view, projection);

                if (hitIndex != -1) {
                    objects[hitIndex].active = false; // "Destroy" object
                    SpawnExplosion(objects[hitIndex].position);
                }
                canClick = false;
            }
        }
        else { canClick = true; } // Reset click when mouse button released

        UpdateParticles(deltaTime);

        // --- RENDER SCENE ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle window resizing for projection
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::vec3 cameraPos = glm::vec3(glm::inverse(view)[3]);

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", cameraPos);

        // Draw active Game Objects
        shader.setInt("isParticle", 0); // Boolean uniform to tell Shader to use Texture
        for (auto& obj : objects) {
            if (!obj.active) continue;

            mat4 model = mat4(1.0f);
            model = translate(model, obj.position);
            model = rotate(model, obj.rotation, vec3(0.0f, 1.0f, 0.0f));

            shader.setMat4("model", model);
            shader.setVec3("objectColor", obj.color);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.textureID);
            shader.setInt("texture1", 0);
            DrawCube();
        }

        // Draw Particles (Explosions)
        shader.setInt("isParticle", 1); // Tell Shader to ignore Texture and use solid color
        for (const auto& p : particles) {
            if (p.life <= 0.0f) continue;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, p.position);
            model = glm::scale(model, glm::vec3(0.3f * p.life)); // Shrink as they die

            shader.setMat4("model", model);

            // Fade color from Orange to Black based on life
            glm::vec3 particleColor = glm::vec3(1.0f, 0.5f * p.life, 0.0f) * p.life;
            shader.setVec3("objectColor", particleColor);

            DrawCube();
        }

        // --- RENDER UI ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Engine Controls");
        ImGui::Text("Tweakable Parameters");
        ImGui::SliderInt("Object Count", &targetObjectCount, 1, 10);
        ImGui::SliderFloat("Displacement", &objectSpread, 1.0f, 5.0f);
        if (ImGui::Button("Regenerate")) {
            SpawnObjects();
        }
        ImGui::Separator();
        ImGui::Text("Light Position");
        ImGui::SliderFloat3("Light XYZ", glm::value_ptr(lightPos), -10.0f, 10.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers to display frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}