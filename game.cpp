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
static Vec3 cameraPos{0.0f, 0.0f, 4.0f};
static Vec3 cameraFront{0.0f, 0.0f, -1.0f};
static Vec3 cameraUp{0.0f, 1.0f, 0.0f};
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

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

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Game", nullptr, nullptr);
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

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(-uLightDir);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 ambient = vec3(0.2);
            vec3 diffuse = diff * vec3(0.8);
            vec3 color = (ambient + diffuse) * texture(uTexture, TexCoord).rgb;
            FragColor = vec4(color, 1.0);
        }
    )";

    GLuint program = createProgram(vertexShader, fragmentShader);

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
        {0.0f, 1.5f, -2.0f}
    };

    Vec3 cubeRotation{0.0f, 0.0f, 0.0f};
    float cubeRotationSpeed = 1.8f;
    float cameraSpeed = 3.0f;
    float cameraRadius = 0.35f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cubeRotation, cubeRotationSpeed);

        Vec3 movement{0.0f, 0.0f, 0.0f};
        Vec3 right = normalize(cross(cameraFront, cameraUp));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement = add(movement, cameraFront);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement = sub(movement, cameraFront);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement = sub(movement, right);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement = add(movement, right);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) movement = add(movement, cameraUp);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) movement = sub(movement, cameraUp);

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

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width = 800;
        int height = 600;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        Mat4 projection = perspective(45.0f * 3.14159265f / 180.0f, aspect, 0.1f, 100.0f);
        Mat4 view = lookAt(cameraPos, add(cameraPos, cameraFront), cameraUp);

        glUseProgram(program);
        glUniform3f(glGetUniformLocation(program, "uLightDir"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(program, "uViewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(glGetUniformLocation(program, "uView"), 1, GL_FALSE, view.m);
        glUniformMatrix4fv(glGetUniformLocation(program, "uProjection"), 1, GL_FALSE, projection.m);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(program, "uTexture"), 0);

        glBindVertexArray(VAO);
        for (size_t i = 0; i < cubePositions.size(); ++i) {
            Vec3 pos = cubePositions[i];
            Mat4 model = multiply(translate(pos), multiply(rotateY(cubeRotation.y + static_cast<float>(i) * 0.6f),
                             multiply(rotateX(cubeRotation.x), rotateZ(cubeRotation.z))));
            glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, model.m);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glDeleteTextures(1, &texture);
    glfwTerminate();
    return 0;
}
