#include "shader.h"

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"
#include "log.h"

namespace ck {

Shader::Shader(const std::string& vertex_source, const std::string& fragment_source) {
  // https://wikis.khronos.org/opengl/Shader_Compilation#Example

  // Create an empty vertex shader handle
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

  // Send the vertex shader source code to GL
  // Note that std::string's .c_str is NULL character terminated.
  const GLchar* source = (const GLchar*)vertex_source.c_str();
  glShaderSource(vertex_shader, 1, &source, 0);

  // Compile the vertex shader
  glCompileShader(vertex_shader);

  GLint is_compiled = 0;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled == GL_FALSE) {
    GLint max_length = 0;
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &max_length);

    // The maxLength includes the NULL character
    std::vector<GLchar> info_log(max_length);
    glGetShaderInfoLog(vertex_shader, max_length, &max_length, &info_log[0]);

    // We don't need the shader anymore.
    glDeleteShader(vertex_shader);
    CK_ENGINE_ERROR("Failed to compile vertex shader");
    CK_ENGINE_ERROR("{}", info_log.data());

    return;
  }

  // Create an empty fragment shader handle
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Send the fragment shader source code to GL
  // Note that std::string's .c_str is NULL character terminated.
  source = (const GLchar*)fragment_source.c_str();
  glShaderSource(fragment_shader, 1, &source, 0);

  // Compile the fragment shader
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled == GL_FALSE) {
    GLint max_length = 0;
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);

    // The maxLength includes the NULL character
    std::vector<GLchar> info_log(max_length);
    glGetShaderInfoLog(fragment_shader, max_length, &max_length, &info_log[0]);

    // We don't need the shader anymore.
    glDeleteShader(fragment_shader);
    // Either of them. Don't leak shaders.
    glDeleteShader(vertex_shader);

    CK_ENGINE_ERROR("Failed to compile fragment shader");
    CK_ENGINE_ERROR("{}", info_log.data());
    return;
  }

  // Vertex and fragment shaders are successfully compiled.
  // Now time to link them together into a program.
  // Get a program object.
  GLuint program = glCreateProgram();
  renderer_id_ = program;

  // Attach our shaders to our program
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  // Link our program
  glLinkProgram(program);

  // Note the different functions here: glGetProgram* instead of glGetShader*.
  GLint is_linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, (int*)&is_linked);
  if (is_linked == GL_FALSE) {
    GLint max_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

    // The maxLength includes the NULL character
    std::vector<GLchar> info_log(max_length);
    glGetProgramInfoLog(program, max_length, &max_length, &info_log[0]);

    // We don't need the program anymore.
    glDeleteProgram(program);
    // Don't leak shaders either.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    CK_ENGINE_ERROR("Failed to link shader");
    CK_ENGINE_ERROR("{}", info_log.data());
    return;
  }

  // Always detach shaders after a successful link.
  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);
}

Shader::~Shader() { glDeleteProgram(renderer_id_); }

void Shader::Bind() const { glUseProgram(renderer_id_); }

void Shader::Unbind() const { glUseProgram(0); }

void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const {
  GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}
}  // namespace ck
