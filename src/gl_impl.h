#pragma once

#include <string>
#include <iostream>
#include <typeinfo>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace gofran {

enum gli_status {
    gli_success,
    gli_uninitialize,
    gli_compile_shader,
    gli_uninited,
    gli_regenerate,
    gli_notgenerate,
    gli_rebind,
    gli_notbind,
};

enum gli_shadertype {
    gli_vertex_shader,
    gli_fragment_shader,
    gli_unknown_shader,
};

enum class gli_type {
    gli_float,
};

enum class gli_buffertype {
    gli_array_buffer,
    gli_element_array_buffer,
};

enum class gli_texturetype {
    gli_texture_2D,
};

class GLShader {
public:
    GLShader() : _id(0)
            , _type(gli_shadertype::gli_unknown_shader)
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

//    int set_file(ShaderType type, const std::string& path);

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
        // auto res = GL_VERTEX_SHADER;
        switch (type) {
            case gli_shadertype::gli_vertex_shader:
                return GL_VERTEX_SHADER;
                break;
            case gli_fragment_shader:
                return GL_FRAGMENT_SHADER;
                break;
            default:
                break;
        }

        return GL_VERTEX_SHADER;
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
        _vertex_shader.set_src(gli_shadertype::gli_vertex_shader, str);
        return _vertex_shader.compiler();
    }

    gli_status set_fragment_shader(const std::string& str) {
        _fragment_shader.set_src(gli_shadertype::gli_fragment_shader, str);
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

protected:
    static unsigned int type_2_gltype(const gli_type& type) {
        switch (type) {
            case gli_type::gli_float:
                return GL_FLOAT;
                break;
            default:
                break;
        }

        return GL_BYTE;
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
class GLVerterArray : public GLTypeImpl {
public:
    GLVerterArray() = default;

    ~GLVerterArray() {
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

        auto buffer_type = get_buffer_type(_type);
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

        auto buffer_type = get_buffer_type(_type);
        glBufferData(buffer_type, size, data, GL_STATIC_DRAW);

        return gli_success;
    }

private:
    static unsigned int get_buffer_type(const gli_buffertype& type) {
        switch (type) {
            case gli_buffertype::gli_array_buffer:
                return GL_ARRAY_BUFFER;
                break;
            case gli_buffertype::gli_element_array_buffer:
                return GL_ELEMENT_ARRAY_BUFFER;
                break;
            default:
                break;
        }
        
        return GL_ARRAY_BUFFER;
    }

private:
    gli_buffertype _type;
};

// gltextures
class GLTextyres : public GLTypeImpl {
public:
    GLTextyres(const gli_texturetype& type) : _type(type) {
    }

    ~GLTextyres() = default;

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

private:
    static unsigned int texturetype_2_gltexturetype(const gli_texturetype& type) {
        switch (type) {
            case gli_texturetype::gli_texture_2D:
                return GL_TEXTURE_2D;
                break;
            default:
                break;
        }
        
        return GL_TEXTURE_2D;
    }

private:
    gli_texturetype _type;
};

}
