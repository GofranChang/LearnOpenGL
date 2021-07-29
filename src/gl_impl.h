#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace gofran {

#define GLI_CONVERT(enum, name) \
{ \
    if (type == gli_##enum::GLI_##name) { \
        return GL_##name; \
    } \
}

// GLI_CONVERT(status, success)

enum gli_status {
    gli_success,
    gli_uninitialize,
    gli_compile_shader,
    gli_uninited,
    gli_regenerate,
    gli_notgenerate,
    gli_rebind,
    gli_notbind,
    gli_io_failed
};

enum class gli_shadertype {
    GLI_VERTEX_SHADER,
    GLI_FRAGMENT_SHADER,
    GLI_UNKNOWN_SHADER
};

enum class gli_type {
    GLI_FLOAT,
    GLI_UNKNOWN_TYPE
};

enum class gli_buffertype {
    GLI_ARRAY_BUFFER,
    GLI_ELEMENT_ARRAY_BUFFER,
    GLI_UNKNOWN_BUFFER_TYPE
};

enum class gli_texturetype {
    GLI_TEXTURE_2D,
    GLI_UNKNOWN_TEXTURETYPE
};

enum class gli_texturesymbol {
    GLI_TEXTURE_MIN_FILTER,
    GLI_TEXTURE_MAG_FILTER,
    GLI_TEXTURE_WRAP_S,
    GLI_TEXTURE_WRAP_T,
};

enum class gli_textureparams {
    GLI_REPEAT,
    GLI_MIRRORED_REPEAT,
    GLI_NEAREST,
    GLI_LINEAR,
    GLI_LINEAR_MIPMAP_LINEAR,
    // gli_mirrored_repeat,
};

enum class gli_pixelformat {
    GLI_RGB,
    GLI_RGBA,
};

class GLShader {
public:
    GLShader() : _id(0)
            , _type(gli_shadertype::GLI_UNKNOWN_SHADER)
            , _src("") {
    }

    ~GLShader() {
        unuse();
    }

    inline void set_src(const gli_shadertype& type,
            const std::string& src) {
        _type = type;
        _src = src;
    }

    gli_status set_file(const gli_shadertype& type,
            const std::string& path) {
        _type = type;
        std::ifstream shader_file;
        shader_file.open(path);
        if (!shader_file) {
            return gli_io_failed;
        }

        std::stringstream shader_stream;
        shader_stream << shader_file.rdbuf();
        shader_file.close();

        _src = shader_stream.str();
        std::cout << _src << std::endl;

        return gli_success;
    }

    gli_status compiler() {
        auto gl_shader_type = shadertype_2_glshadertype(_type);

        _id = glCreateShader(gl_shader_type);

        const char* c_src = _src.c_str();
        glShaderSource(_id, 1, &c_src, NULL);
        glCompileShader(_id);
        int success;
        char infoLog[1024];
        glGetShaderiv(_id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(_id, 1024, NULL, infoLog);
            _id = 0;
            return gli_compile_shader;
        }

        return gli_success;
    }

    inline const gli_shadertype& type() const {
        return _type;
    }

    inline unsigned int id() const {
        return _id;
    }

    gli_status unuse() {
        if (0 != _id) {
            glDeleteShader(_id);
            _id = 0;
        }

        return gli_success;
    }

private:
    static unsigned int shadertype_2_glshadertype(const gli_shadertype& type) {
        GLI_CONVERT(shadertype, VERTEX_SHADER)
        GLI_CONVERT(shadertype, FRAGMENT_SHADER)

        return 0;
    }

private:
    unsigned int _id;

    gli_shadertype _type;

    std::string _src;
};

class GLPipeline {
public:
    GLPipeline() : _id(0) {
    }

    ~GLPipeline() {
        if (0 != _id) {
            glDeleteProgram(_id);
            _id = 0;
        }
    }
    
private:
    GLPipeline(const GLPipeline&) = delete;

    GLPipeline* operator=(const GLPipeline&) = delete;

public:
    gli_status set_vertex_shader(const std::string& str) {
        _vertex_shader.set_src(gli_shadertype::GLI_VERTEX_SHADER, str);
        return _vertex_shader.compiler();
    }

    gli_status set_vertex_file(const std::string& path) {
        _vertex_shader.set_file(gli_shadertype::GLI_VERTEX_SHADER, path);
        return _vertex_shader.compiler();
    }

    gli_status set_fragment_shader(const std::string& str) {
        _fragment_shader.set_src(gli_shadertype::GLI_FRAGMENT_SHADER, str);
        return _fragment_shader.compiler();
    }

    gli_status set_fragment_file(const std::string& path) {
        _fragment_shader.set_file(gli_shadertype::GLI_FRAGMENT_SHADER, path);
        return _fragment_shader.compiler();
    }

    gli_status link() {
        if (0 == _vertex_shader.id() || 0 == _fragment_shader.id()) {
            return gli_uninitialize;
        }

        _id = glCreateProgram();
        glAttachShader(_id, _vertex_shader.id());
        glAttachShader(_id, _fragment_shader.id());
        glLinkProgram(_id);

        int success;
        char infoLog[1024];
        glGetProgramiv(_id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(_id, 1024, NULL, infoLog);
        }
    }

    inline int use() {
        if (0 == _id) {
            return -1;
        }

        glUseProgram(_id);
        return 0;
    }

    inline gli_status set_uniform1(const std::string &name, int value) {
        // TODO: Set
        glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
    }

    inline gli_status set_uniform1(const std::string &name, float value) {
        // TODO: Set
        glUniform1f(glGetUniformLocation(_id, name.c_str()), value);
    }

private:
    GLShader _vertex_shader;

    GLShader _fragment_shader;

    unsigned int _id;
};

class GLTypeImpl {
public:
    GLTypeImpl() : _id(0)
            , _nums(0)
            , _is_binded(false) {
    }

    virtual ~GLTypeImpl() {
    }

    virtual gli_status generate(size_t n = 1) = 0;

    virtual gli_status remove() = 0;

    virtual gli_status bind() = 0;

    virtual gli_status unbind() = 0;

    inline bool is_generated() const {
        return (_id != 0);
    }

    inline bool is_binded() const {
        return _is_binded;
    }

    inline void set_isbinded() {
        _is_binded = true;
    }

    inline void set_notbinded() {
        _is_binded = false;
    }

    inline const unsigned int id() const {
        return _id;
    }

protected:
    static unsigned int type_2_gltype(const gli_type& type) {
        GLI_CONVERT(type, FLOAT)

        return 0;
    }

    static unsigned int bool_2_glbool(bool b) {
        if (b) {
            return GL_TRUE;
        }

        return GL_FALSE;
    }

protected:
    unsigned int _id;

    size_t _nums;

private:
    bool _is_binded;
};

// glvertexarray
class GLVertexArray : public GLTypeImpl {
public:
    GLVertexArray() = default;

    ~GLVertexArray() {
        if (is_binded()) {
            this->unbind();
        }

        if (is_generated()) {
            this->remove();
        }
    }

public:
    virtual gli_status generate(size_t n = 1) override {
        if (is_generated()) {
            // TODO: print log
            return gli_regenerate;
        }

        glGenVertexArrays(n, &_id);
        _nums = n;
        return gli_success;
    }

    virtual gli_status remove() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        glDeleteVertexArrays(_nums, &_id);
        return gli_success;
    }

    virtual gli_status bind() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        if (is_binded()) {
            // TODO: print log
            return gli_rebind;
        }

        glBindVertexArray(_id);
        set_isbinded();
        return gli_success;
    }

    virtual gli_status unbind() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        if (!is_binded()) {
            // TODO: print log
            return gli_notbind;
        }

        glBindVertexArray(0);
        set_notbinded();
        return gli_success;
    }

    int set_attribute(int location, int size,
            const gli_type& type, bool normalize,
            int stride_len, int offset) {
        if (!is_generated() || !is_binded()) {
            return gli_uninited;
        }

        auto gl_type = type_2_gltype(type);
        auto gl_normalize = GLTypeImpl::bool_2_glbool(normalize);

        glVertexAttribPointer(location, size, 
                gl_type, gl_normalize, stride_len, (void*)offset);
        glEnableVertexAttribArray(location);

        return gli_success;
    }
};

// glbuffer
class GLBuffer : public GLTypeImpl {
public:
    GLBuffer(const gli_buffertype& type) : _type(type) {
    }

    ~GLBuffer() = default;

public:
    virtual gli_status generate(size_t n = 1) override {
        if (is_generated()) {
            // TODO: print log
            return gli_regenerate;
        }

        glGenBuffers(n, &_id);
        _nums = n;
        return gli_success;
    }

    virtual gli_status remove() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        glDeleteBuffers(_nums, &_id);
        return gli_success;
    }

    virtual gli_status bind() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        if (is_binded()) {
            // TODO: print log
            return gli_rebind;
        }

        auto buffer_type = buffertype_2_glbuffertype(_type);
        glBindBuffer(buffer_type, _id);

        set_isbinded();
        return gli_success;
    }

    virtual gli_status unbind() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        if (!is_binded()) {
            // TODO: print log
            return gli_notbind;
        }

        glBindBuffer(0, 0);

        set_notbinded();
        return gli_success;
    }

    template<typename T>
    int set_data(const T* data, size_t size) const {
        if (!is_generated() || !is_binded()) {
            return gli_uninited;
        }

        auto buffer_type = buffertype_2_glbuffertype(_type);
        glBufferData(buffer_type, size, data, GL_STATIC_DRAW);

        return gli_success;
    }

private:
    static unsigned int buffertype_2_glbuffertype(const gli_buffertype& type) {
        GLI_CONVERT(buffertype, ARRAY_BUFFER)
        GLI_CONVERT(buffertype, ELEMENT_ARRAY_BUFFER)
        
        return 0;
    }

private:
    gli_buffertype _type;
};

// gltextures
class GLTextures : public GLTypeImpl {
public:
    GLTextures(const gli_texturetype& type) : _type(type) {
    }

    ~GLTextures() = default;
    
#define GLI_TEXTURE_LOCATION(n) GL_TEXTURE##n

public:
    virtual gli_status generate(size_t n = 1) override {
        if (is_generated()) {
            // TODO: print log
            return gli_regenerate;
        }

        glGenTextures(n, &_id);
        _nums = n;
        return gli_success;
    }

    virtual gli_status remove() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        glDeleteTextures(_nums, &_id);
        return gli_success;
    }
    
    virtual gli_status active(int location) {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        // glActiveTexture(GLI_TEXTURE_LOCATION(0));
        if (0 == location)
            glActiveTexture(GL_TEXTURE0);
        if (1 == location)
            glActiveTexture(GL_TEXTURE1);
        if (2 == location)
            glActiveTexture(GL_TEXTURE2);

        this->bind();
        return gli_success;
    }

    virtual gli_status bind() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        if (is_binded()) {
            // TODO: print log
            return gli_rebind;
        }

        auto type = texturetype_2_gltexturetype(_type);
        glBindTexture(type, _id);
        set_isbinded();
        return gli_success;
    }

    virtual gli_status unbind() override {
        if (!is_generated()) {
            // TODO: print log
            return gli_notgenerate;
        }

        if (!is_binded()) {
            // TODO: print log
            return gli_notbind;
        }

        glBindTexture(0, 0);
        set_notbinded();
        return gli_success;
    }

    gli_status load_texture(int width, int height,
            const unsigned char* data, const gli_pixelformat& type) {
        if (!is_generated() || !is_binded()) {
            return gli_uninited;
        }

        auto texture_type = texturetype_2_gltexturetype(_type);
        // TODO: Modify params later
        auto pixel_type = pixelformat_2_glpixelformat(type);
        glTexImage2D(texture_type, 0, pixel_type, width, height, 0, pixel_type, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(texture_type);

        return gli_success;
    }

    gli_status set_tex_parameteri(const gli_texturesymbol& symbol,
            const gli_textureparams& param) {
        if (!is_generated() || !is_binded()) {
            return gli_uninited;
        }

        auto text_type = texturetype_2_gltexturetype(_type);
        auto text_symbol = texturesymbol_2_gltexturesymbol(symbol);
        auto text_params = textureparams_2_gltextureparams(param);
        glTexParameteri(text_type, text_symbol, text_params);

        return gli_success;
    }

private:
    static unsigned int texturetype_2_gltexturetype(const gli_texturetype& type) {
        GLI_CONVERT(texturetype, TEXTURE_2D)

        return 0;
    }

    static unsigned int texturesymbol_2_gltexturesymbol(const gli_texturesymbol& type) {
        GLI_CONVERT(texturesymbol, TEXTURE_MIN_FILTER)
        GLI_CONVERT(texturesymbol, TEXTURE_MAG_FILTER)
        GLI_CONVERT(texturesymbol, TEXTURE_WRAP_S)
        GLI_CONVERT(texturesymbol, TEXTURE_WRAP_T)

        return 0;
    }

    static unsigned int textureparams_2_gltextureparams(const gli_textureparams& type) {
        GLI_CONVERT(textureparams, REPEAT)
        GLI_CONVERT(textureparams, NEAREST)
        GLI_CONVERT(textureparams, LINEAR)
        GLI_CONVERT(textureparams, LINEAR_MIPMAP_LINEAR)

        return 0;
    }

    static unsigned int pixelformat_2_glpixelformat(const gli_pixelformat& type) {
        GLI_CONVERT(pixelformat, RGB)
        GLI_CONVERT(pixelformat, RGBA)

        return 0;
    }

private:
    gli_texturetype _type;
};

}
