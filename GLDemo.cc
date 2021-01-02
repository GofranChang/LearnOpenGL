#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <iostream>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void processInput(GLFWwindow *window);

int main(int argc, const char* argv[]) {
    // 初始化GLFW
    glfwInit();
    
    // 设置为OpenGL版本为3.3+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // 使用核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 将该窗口作为当前线程的主上下文
    glfwMakeContextCurrent(window);
    
    // 加载GLFW函数指针，在调用任何OpenGL函数之前必须调用该函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置窗口渲染大小
    glViewport(0, 0, 800, 600);
    // 设置窗口大小改变回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // 检查一次GLFW是否被要求退出
    while(!glfwWindowShouldClose(window)) {
        // 处理当前窗口输入
        processInput(window);
        
        // 设置清空屏幕所用的颜色
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // 清空屏幕颜色缓冲
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色值的大缓冲），
        // 它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上
        glfwSwapBuffers(window);
        // 检查是否有触发事件（键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数
        glfwPollEvents();
    }

    // 关闭窗口，释放资源
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    // 如果用户按下ESC键，则应该退出
    if(GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
}
