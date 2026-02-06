#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Mat4 {
    float m[16];
};

static float clamp(float v, float minV, float maxV) {
    return (v < minV) ? minV : (v > maxV) ? maxV : v;
}

static Vec3 add(const Vec3& a, const Vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static Vec3 sub(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vec3 mul(const Vec3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }
static float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static Vec3 normalize(const Vec3& v) {
    float len = std::sqrt(dot(v, v));
    if (len <= 0.00001f) return {0.0f, 0.0f, 0.0f};
    return {v.x / len, v.y / len, v.z / len};
}

static Mat4 identity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

static Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            r.m[col + row * 4] =
                a.m[0 + row * 4] * b.m[col + 0 * 4] +
                a.m[1 + row * 4] * b.m[col + 1 * 4] +
                a.m[2 + row * 4] * b.m[col + 2 * 4] +
                a.m[3 + row * 4] * b.m[col + 3 * 4];
        }
    }
    return r;
}

static Mat4 translate(const Vec3& t) {
    Mat4 r = identity();
    r.m[12] = t.x;
    r.m[13] = t.y;
    r.m[14] = t.z;
    return r;
}

static Mat4 scale(const Vec3& s) {
    Mat4 r = identity();
    r.m[0] = s.x;
    r.m[5] = s.y;
    r.m[10] = s.z;
    return r;
}

static Mat4 rotateX(float radians) {
    Mat4 r = identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    r.m[5] = c;  r.m[9] = -s;
    r.m[6] = s;  r.m[10] = c;
    return r;
}

static Mat4 rotateY(float radians) {
    Mat4 r = identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    r.m[0] = c;  r.m[8] = s;
    r.m[2] = -s; r.m[10] = c;
    return r;
}

static Mat4 rotateZ(float radians) {
    Mat4 r = identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    r.m[0] = c;  r.m[4] = -s;
    r.m[1] = s;  r.m[5] = c;
    return r;
}

static Mat4 perspective(float fovRadians, float aspect, float nearZ, float farZ) {
    Mat4 r{};
    float f = 1.0f / std::tan(fovRadians / 2.0f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (farZ + nearZ) / (nearZ - farZ);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return r;
}

static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = normalize(sub(center, eye));
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);

    Mat4 r = identity();
    r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
    r.m[12] = -dot(s, eye);
    r.m[13] = -dot(u, eye);
    r.m[14] = dot(f, eye);
    return r;
}

static GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info);
        std::cerr << "Shader compile error: " << info << std::endl;
    }
    return shader;
}

static GLuint createProgram(const std::string& vs, const std::string& fs) {
    GLuint vsId = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fsId = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint program = glCreateProgram();
    glAttachShader(program, vsId);
    glAttachShader(program, fsId);
    glLinkProgram(program);
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(program, 1024, nullptr, info);
        std::cerr << "Program link error: " << info << std::endl;
    }
    glDeleteShader(vsId);
    glDeleteShader(fsId);
    return program;
}

static bool sphereAabbCollision(const Vec3& center, float radius, const Vec3& min, const Vec3& max) {
    float x = clamp(center.x, min.x, max.x);
    float y = clamp(center.y, min.y, max.y);
    float z = clamp(center.z, min.z, max.z);
    Vec3 closest = {x, y, z};
    Vec3 diff = sub(center, closest);
    return dot(diff, diff) < (radius * radius);
}

static float lastX = 400.0f;
static float lastY = 300.0f;
static bool firstMouse = true;
static float yaw = -90.0f;
static float pitch = 0.0f;
static Vec3 cameraPos{0.0f, 0.6f, 4.0f};  // Start at ground + playerHeight
static Vec3 cameraFront{0.0f, 0.0f, -1.0f};
static Vec3 cameraUp{0.0f, 1.0f, 0.0f};
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

// Player physics
static float velocityY = 0.0f;           // Vertical velocity
static const float gravity = -20.0f;     // Gravity acceleration
static const float jumpSpeed = 8.0f;     // Initial jump velocity
static const float playerHeight = 1.6f;  // Eye height above feet
static const float groundLevel = -1.0f;  // Y position of ground plane
static bool isGrounded = false;          // Is player on ground?

static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    pitch = clamp(pitch, -89.0f, 89.0f);

    float yawRad = yaw * 3.14159265f / 180.0f;
    float pitchRad = pitch * 3.14159265f / 180.0f;

    Vec3 front{
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)
    };
    cameraFront = normalize(front);
}

static void processInput(GLFWwindow* window, Vec3& cubeRotation, float& cubeRotationSpeed) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cubeRotation.y -= cubeRotationSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cubeRotation.y += cubeRotationSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) cubeRotation.x -= cubeRotationSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) cubeRotation.x += cubeRotationSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) cubeRotation.z -= cubeRotationSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) cubeRotation.z += cubeRotationSpeed * deltaTime;
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "3D Overworld", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    float vertices[] = {
        // positions          // normals           // texcoords
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f
    };

    GLuint VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    float groundVertices[] = {
        // positions            // normals        // texcoords
        -1.0f, 0.0f, -1.0f,      0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, 0.0f, -1.0f,      0.0f, 1.0f, 0.0f,  8.0f, 0.0f,
         1.0f, 0.0f,  1.0f,      0.0f, 1.0f, 0.0f,  8.0f, 8.0f,

        -1.0f, 0.0f, -1.0f,      0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, 0.0f,  1.0f,      0.0f, 1.0f, 0.0f,  8.0f, 8.0f,
        -1.0f, 0.0f,  1.0f,      0.0f, 1.0f, 0.0f,  0.0f, 8.0f
    };

    GLuint groundVAO = 0, groundVBO = 0;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    std::string vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTex;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;

        void main() {
            FragPos = vec3(uModel * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            TexCoord = aTex;
            gl_Position = uProjection * uView * vec4(FragPos, 1.0);
        }
    )";

    std::string fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;

        uniform vec3 uLightDir;
        uniform vec3 uViewPos;
        uniform sampler2D uTexture;
        uniform samplerCube uEnvMap;
        uniform vec3 uColorTint;
        uniform vec3 uFogColor;
        uniform float uFogDensity;
        uniform mat4 uView;

        // Cinematic lighting parameters
        const vec3 sunColor = vec3(1.0, 0.95, 0.85);       // Warm sunlight
        const vec3 groundColor = vec3(0.3, 0.25, 0.2);     // Warm ground bounce
        const vec3 rimColor = vec3(0.9, 0.85, 0.8);        // Subtle warm rim
        const float sunIntensity = 1.4;
        const float ambientIntensity = 0.25;
        const float rimPower = 3.0;
        const float rimIntensity = 0.5;

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(-uLightDir);
            vec3 viewDir = normalize(uViewPos - FragPos);
            
            // Blinn-Phong halfway vector for better specular
            vec3 halfwayDir = normalize(lightDir + viewDir);
            
            // Hemisphere ambient lighting (sky above, ground below)
            float hemisphereBlend = norm.y * 0.5 + 0.5;
            vec3 envDir = normalize(mat3(uView) * norm);
            vec3 skyAmbient = texture(uEnvMap, envDir).rgb;
            vec3 ambient = mix(groundColor, skyAmbient, hemisphereBlend) * ambientIntensity;
            
            // Wrapped diffuse for softer shadows
            float NdotL = dot(norm, lightDir);
            float wrappedDiff = max((NdotL + 0.3) / 1.3, 0.0);
            vec3 diffuse = wrappedDiff * sunColor * sunIntensity;
            
            // Blinn-Phong specular with roughness
            float NdotH = max(dot(norm, halfwayDir), 0.0);
            float shininess = 64.0;
            float spec = pow(NdotH, shininess);
            
            // Fresnel-Schlick approximation for realistic specular falloff
            float fresnel = pow(1.0 - max(dot(viewDir, halfwayDir), 0.0), 5.0);
            float F0 = 0.04;  // Base reflectivity for dielectrics
            float fresnelFactor = F0 + (1.0 - F0) * fresnel;
            
            // Apply specular only on lit surfaces
            float specMask = smoothstep(0.0, 0.1, NdotL);
            vec3 specular = spec * fresnelFactor * sunColor * specMask * 0.8;
            
            // Rim lighting (backlight effect)
            float rimDot = 1.0 - max(dot(viewDir, norm), 0.0);
            float rimAmount = pow(rimDot, rimPower);
            // Enhance rim on surfaces facing away from light (silhouette effect)
            float rimShadow = 1.0 - max(NdotL, 0.0);
            vec3 rim = rimAmount * rimShadow * rimColor * rimIntensity;
            
            // Sample albedo texture
            vec3 albedo = texture(uTexture, TexCoord).rgb * uColorTint;
            
            // Energy conservation: reduce diffuse where specular is strong
            vec3 diffuseContrib = diffuse * (1.0 - fresnelFactor * 0.5);
            
            // Combine lighting
            vec3 lit = (ambient + diffuseContrib) * albedo + specular + rim * albedo;
            
            // Subtle tone mapping for HDR-like feel
            lit = lit / (lit + vec3(1.0));
            
            // Atmospheric fog with distance
            float distanceToCamera = length(uViewPos - FragPos);
            float fogFactor = clamp(exp(-pow(distanceToCamera * uFogDensity, 1.5)), 0.0, 1.0);
            vec3 fogged = mix(uFogColor, lit, fogFactor);
            
            // Final gamma correction hint (slight contrast boost)
            fogged = pow(fogged, vec3(0.95));

            FragColor = vec4(fogged, 1.0);
        }
    )";

    GLuint program = createProgram(vertexShader, fragmentShader);

    // Skybox shader
    std::string skyboxVS = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        out vec3 TexCoords;
        
        uniform mat4 uProjection;
        uniform mat4 uView;
        
        void main() {
            TexCoords = aPos;
            // Remove translation from view matrix (only rotation)
            mat4 viewNoTranslation = mat4(mat3(uView));
            vec4 pos = uProjection * viewNoTranslation * vec4(aPos, 1.0);
            gl_Position = pos.xyww;  // Set z = w so skybox is at max depth
        }
    )";

    std::string skyboxFS = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 TexCoords;
        
        uniform samplerCube uSkybox;
        
        void main() {
            vec3 color = texture(uSkybox, TexCoords).rgb;
            // HDR tone mapping
            color = color / (color + vec3(1.0));
            // Gamma correction
            color = pow(color, vec3(1.0/2.2));
            FragColor = vec4(color, 1.0);
        }
    )";

    GLuint skyboxProgram = createProgram(skyboxVS, skyboxFS);

    // Skybox cube vertices (inside-out cube)
    float skyboxVertices[] = {
        // Back face
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        // Front face
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        // Left face
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        // Right face
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        // Top face
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        // Bottom face
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f
    };

    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create procedural HDR cubemap texture
    GLuint skyboxTexture;
    glGenTextures(1, &skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

    const int skySize = 256;
    std::vector<float> skyFaceData(skySize * skySize * 3);

    // Sun direction (normalized)
    float sunDirX = 0.4f, sunDirY = 0.6f, sunDirZ = -0.7f;
    float sunLen = std::sqrt(sunDirX*sunDirX + sunDirY*sunDirY + sunDirZ*sunDirZ);
    sunDirX /= sunLen; sunDirY /= sunLen; sunDirZ /= sunLen;

    // Generate each cubemap face
    for (int face = 0; face < 6; ++face) {
        for (int y = 0; y < skySize; ++y) {
            for (int x = 0; x < skySize; ++x) {
                // Convert pixel to direction vector for this face
                float u = (x + 0.5f) / skySize * 2.0f - 1.0f;
                float v = (y + 0.5f) / skySize * 2.0f - 1.0f;
                float dx, dy, dz;
                
                switch (face) {
                    case 0: dx =  1.0f; dy = -v;    dz = -u;    break; // +X
                    case 1: dx = -1.0f; dy = -v;    dz =  u;    break; // -X
                    case 2: dx =  u;    dy =  1.0f; dz =  v;    break; // +Y
                    case 3: dx =  u;    dy = -1.0f; dz = -v;    break; // -Y
                    case 4: dx =  u;    dy = -v;    dz =  1.0f; break; // +Z
                    case 5: dx = -u;    dy = -v;    dz = -1.0f; break; // -Z
                }
                
                // Normalize direction
                float len = std::sqrt(dx*dx + dy*dy + dz*dz);
                dx /= len; dy /= len; dz /= len;
                
                // Calculate sky gradient based on elevation
                float elevation = dy;  // -1 (down) to 1 (up)
                
                // HDR sky colors (values > 1 for HDR)
                float r, g, b;
                
                if (elevation > 0.0f) {
                    // Sky gradient: horizon to zenith
                    float t = std::pow(elevation, 0.5f);
                    // Horizon: warm golden (1.8, 1.4, 0.9)
                    // Zenith: deep blue (0.2, 0.4, 1.2)
                    r = 1.8f * (1.0f - t) + 0.2f * t;
                    g = 1.4f * (1.0f - t) + 0.4f * t;
                    b = 0.9f * (1.0f - t) + 1.2f * t;
                } else {
                    // Ground gradient
                    float t = std::min(1.0f, -elevation * 2.0f);
                    // Horizon: warm (1.0, 0.8, 0.6)
                    // Nadir: dark earth (0.15, 0.12, 0.1)
                    r = 1.0f * (1.0f - t) + 0.15f * t;
                    g = 0.8f * (1.0f - t) + 0.12f * t;
                    b = 0.6f * (1.0f - t) + 0.10f * t;
                }
                
                // Add sun glow
                float sunDot = dx * sunDirX + dy * sunDirY + dz * sunDirZ;
                if (sunDot > 0.0f) {
                    // Sun disc (very bright HDR)
                    float sunDisc = std::pow(std::max(0.0f, sunDot), 256.0f);
                    r += sunDisc * 30.0f;
                    g += sunDisc * 28.0f;
                    b += sunDisc * 20.0f;
                    
                    // Sun glow (softer)
                    float sunGlow = std::pow(std::max(0.0f, sunDot), 8.0f);
                    r += sunGlow * 1.5f;
                    g += sunGlow * 1.2f;
                    b += sunGlow * 0.6f;
                }
                
                int idx = (y * skySize + x) * 3;
                skyFaceData[idx + 0] = r;
                skyFaceData[idx + 1] = g;
                skyFaceData[idx + 2] = b;
            }
        }
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F,
                     skySize, skySize, 0, GL_RGB, GL_FLOAT, skyFaceData.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const int texSize = 64;
    std::vector<unsigned char> texData(texSize * texSize * 3);
    for (int y = 0; y < texSize; ++y) {
        for (int x = 0; x < texSize; ++x) {
            int idx = (y * texSize + x) * 3;
            int checker = ((x / 8) + (y / 8)) % 2;
            unsigned char value = checker ? 220 : 60;
            texData[idx] = value;
            texData[idx + 1] = value;
            texData[idx + 2] = value;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, texData.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    std::vector<Vec3> cubePositions = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, -3.0f},
        {-2.0f, 0.0f, -4.0f},
        {0.0f, 1.5f, -2.0f},
        {3.5f, 0.0f, 1.5f},
        {-3.5f, 0.0f, 2.0f},
        {1.5f, 0.0f, 3.5f},
        {-1.5f, 0.0f, 3.0f},
        {4.0f, 0.0f, -5.0f},
        {-4.0f, 0.0f, -5.5f}
    };

    std::vector<Vec3> colorPalette = {
        {1.0f, 0.95f, 0.9f},
        {0.8f, 0.9f, 1.0f},
        {0.9f, 1.0f, 0.8f},
        {1.0f, 0.85f, 0.7f}
    };

    Vec3 groundScale{40.0f, 1.0f, 40.0f};
    Vec3 groundTint{0.65f, 0.85f, 0.65f};

    Vec3 cubeRotation{0.0f, 0.0f, 0.0f};
    float cubeRotationSpeed = 1.8f;
    float cameraSpeed = 3.0f;
    float cameraRadius = 0.35f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cubeRotation, cubeRotationSpeed);

        // Calculate horizontal movement direction (WASD keys)
        Vec3 movement{0.0f, 0.0f, 0.0f};
        Vec3 right = normalize(cross(cameraFront, cameraUp));
        
        // Get forward direction projected onto XZ plane (horizontal only)
        Vec3 forward = normalize({cameraFront.x, 0.0f, cameraFront.z});
        Vec3 strafeRight = normalize(cross(forward, cameraUp));
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement = add(movement, forward);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement = sub(movement, forward);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement = sub(movement, strafeRight);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement = add(movement, strafeRight);

        // Apply horizontal movement with collision detection
        if (dot(movement, movement) > 0.0f) {
            movement = normalize(movement);
            Vec3 nextPos = add(cameraPos, mul(movement, cameraSpeed * deltaTime));

            bool collided = false;
            for (const auto& pos : cubePositions) {
                Vec3 min = sub(pos, {0.6f, 0.6f, 0.6f});
                Vec3 max = add(pos, {0.6f, 0.6f, 0.6f});
                if (sphereAabbCollision(nextPos, cameraRadius, min, max)) {
                    collided = true;
                    break;
                }
            }
            if (!collided) {
                cameraPos = nextPos;
            }
        }

        // Jump input (only when grounded)
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isGrounded) {
            velocityY = jumpSpeed;
            isGrounded = false;
        }

        // Apply gravity to vertical velocity
        velocityY += gravity * deltaTime;

        // Apply vertical velocity to camera position
        cameraPos.y += velocityY * deltaTime;

        // Ground collision - player's feet position is cameraPos.y - playerHeight
        float feetY = cameraPos.y - playerHeight;
        if (feetY <= groundLevel) {
            cameraPos.y = groundLevel + playerHeight;
            velocityY = 0.0f;
            isGrounded = true;
        } else {
            isGrounded = false;
        }

        int width = 1600;
        int height = 900;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.05f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = static_cast<float>(width) / static_cast<float>(height);
        Mat4 projection = perspective(45.0f * 3.14159265f / 180.0f, aspect, 0.1f, 140.0f);
        Mat4 view = lookAt(cameraPos, add(cameraPos, cameraFront), cameraUp);

        // Render skybox first (with depth write disabled, depth test LEQUAL)
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxProgram);
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "uView"), 1, GL_FALSE, view.m);
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "uProjection"), 1, GL_FALSE, projection.m);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glUniform1i(glGetUniformLocation(skyboxProgram, "uSkybox"), 0);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);  // Restore default depth function

        glUseProgram(program);
        glUniform3f(glGetUniformLocation(program, "uLightDir"), -0.25f, -1.0f, -0.35f);
        glUniform3f(glGetUniformLocation(program, "uViewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(program, "uFogColor"), 0.35f, 0.45f, 0.65f);
        glUniform1f(glGetUniformLocation(program, "uFogDensity"), 0.03f);
        glUniformMatrix4fv(glGetUniformLocation(program, "uView"), 1, GL_FALSE, view.m);
        glUniformMatrix4fv(glGetUniformLocation(program, "uProjection"), 1, GL_FALSE, projection.m);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glUniform1i(glGetUniformLocation(program, "uEnvMap"), 1);

        glBindVertexArray(groundVAO);
        Mat4 groundModel = multiply(translate({0.0f, -1.0f, 0.0f}), scale(groundScale));
        glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, groundModel.m);
        glUniform3f(glGetUniformLocation(program, "uColorTint"), groundTint.x, groundTint.y, groundTint.z);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(VAO);
        for (size_t i = 0; i < cubePositions.size(); ++i) {
            Vec3 pos = cubePositions[i];
            Vec3 tint = colorPalette[i % colorPalette.size()];
            Mat4 model = multiply(translate(pos), multiply(rotateY(cubeRotation.y + static_cast<float>(i) * 0.6f),
                             multiply(rotateX(cubeRotation.x), rotateZ(cubeRotation.z))));
            glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, model.m);
            glUniform3f(glGetUniformLocation(program, "uColorTint"), tint.x, tint.y, tint.z);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteProgram(program);
    glDeleteProgram(skyboxProgram);
    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &skyboxTexture);
    glfwTerminate();
    return 0;
}
