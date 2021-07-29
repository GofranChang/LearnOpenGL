#include "src/gl_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __cplusplus
}
#endif

using namespace gofran;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static void init_opengl_env();

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static void process_input(GLFWwindow *window);

void my_fun();

int main(int argc, const char* argv[]) {
    init_opengl_env();

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 加载GLFW函数指针，在调用任何OpenGL函数之前必须调用该函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const char *vertexShaderSource =
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "layout (location = 1) in vec3 aColor;\n"
            "layout (location = 2) in vec2 aTexCoord;\n"
            "out vec3 ourColor;\n"
            "out vec2 TexCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(aPos, 1.0);\n"
            "    ourColor = aColor;\n"
            "    TexCoord = aTexCoord;\n"
            "}\n\0";

    const char* fragmentShaderSource =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec3 ourColor;\n"
            "in vec2 TexCoord;\n"
            "uniform sampler2D ourTexture;\n"
            "void main()\n"
            "{\n"
            "    FragColor = texture(ourTexture, TexCoord);\n"
            "}\n\0";

    GLPipeline pipeline;

    int res = 0;
    std::string vertex_str(vertexShaderSource);
    res = pipeline.set_vertex_shader(vertex_str);

    std::string fragment_str(fragmentShaderSource);
    res = pipeline.set_fragment_shader(fragment_str);
    res = pipeline.link();

    float vertices[] = {
        // ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // 左上
    };
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    GLVertexArray vao;
    vao.generate();
    vao.bind();

    GLBuffer vbo(gli_buffertype::GLI_ARRAY_BUFFER);
    vbo.generate();
    vbo.bind();
    vbo.set_data(vertices, sizeof(vertices));

    GLBuffer ebo(gli_buffertype::GLI_ELEMENT_ARRAY_BUFFER);
    ebo.generate();
    ebo.bind();
    ebo.set_data(indices, sizeof(indices));

    vao.set_attribute(0, 3, gli_type::GLI_FLOAT, false, 8 * sizeof(float), 0);
    vao.set_attribute(1, 3, gli_type::GLI_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao.set_attribute(2, 3, gli_type::GLI_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));

    GLTextures texture(gli_texturetype::GLI_TEXTURE_2D);
    texture.generate();
    texture.bind();
    texture.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_WRAP_S, gli_textureparams::GLI_REPEAT);
    texture.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_WRAP_T, gli_textureparams::GLI_REPEAT);
    texture.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_MIN_FILTER, gli_textureparams::GLI_LINEAR);
    texture.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_MAG_FILTER, gli_textureparams::GLI_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        texture.load_texture(width, height, data);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    while (!glfwWindowShouldClose(window)) {
        // input
        // -----
        process_input(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind Texture
        // glBindTexture(GL_TEXTURE_2D, texture1);
        texture.bind();

        // render container
        // ourShader.use();
        pipeline.use();
        // glBindVertexArray(VAO);
        vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}

void init_opengl_env() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
