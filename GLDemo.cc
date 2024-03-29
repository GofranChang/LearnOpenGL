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

    GLPipeline pipeline;

    int res = 0;
    // std::string vertex_str(vertexShaderSource);
    // res = pipeline.set_vertex_shader(vertex_str);
    res = pipeline.set_vertex_file("../../shaders/4.2_vertex.glsl");

    // std::string fragment_str(fragmentShaderSource);
    // res = pipeline.set_fragment_shader(fragment_str);
    res = pipeline.set_fragment_file("../../shaders/4.2_fragment.glsl");
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

    GLTextures texture1(gli_texturetype::GLI_TEXTURE_2D);
    texture1.generate();
    texture1.bind();
    texture1.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_WRAP_S, gli_textureparams::GLI_REPEAT);
    texture1.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_WRAP_T, gli_textureparams::GLI_REPEAT);
    texture1.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_MIN_FILTER, gli_textureparams::GLI_LINEAR);
    texture1.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_MAG_FILTER, gli_textureparams::GLI_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        texture1.load_texture(width, height, data, gli_pixelformat::GLI_RGB);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    texture1.unbind();

    GLTextures texture2(gli_texturetype::GLI_TEXTURE_2D);
    texture2.generate();
    texture2.bind();
    texture2.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_WRAP_S, gli_textureparams::GLI_REPEAT);
    texture2.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_WRAP_T, gli_textureparams::GLI_REPEAT);
    texture2.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_MIN_FILTER, gli_textureparams::GLI_LINEAR);
    texture2.set_tex_parameteri(gli_texturesymbol::GLI_TEXTURE_MAG_FILTER, gli_textureparams::GLI_LINEAR);

    // int width, height, nrChannels;
    data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
    if (data) {
        texture2.load_texture(width, height, data, gli_pixelformat::GLI_RGBA);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    texture2.unbind();

    pipeline.use();
    pipeline.set_uniform1("texture1", 0);
    pipeline.set_uniform1("texture2", 1);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        texture1.active(0);
        texture1.unbind();
        texture2.active(1);
        texture2.unbind();

        pipeline.use();
        vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
