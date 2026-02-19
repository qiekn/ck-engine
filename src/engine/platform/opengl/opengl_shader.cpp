#include "opengl_shader.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/log.h"
#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"

namespace ck {

// Helper Method
static GLenum ShaderTypeFromString(const std::string& type) {
  if (type == "vertex") {
    return GL_VERTEX_SHADER;
  } else if (type == "fragment" || type == "pixel") {
    return GL_FRAGMENT_SHADER;
  }

  CK_ENGINE_ASSERT(false, "unknown shader type");
  return 0;
}

OpenGLShader::OpenGLShader(const std::string& filepath) {
  CK_PROFILE_FUNCTION();
  std::string origin_source = ReadFile(filepath);
  std::unordered_map<GLenum, std::string> shader_sources = Parse(origin_source);
  Compile(shader_sources);

  // Set shader name base on filepath
  name_ = std::filesystem::path(filepath).stem().string();
};

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertex_source,
                           const std::string& fragment_source)
    : name_(name) {
  CK_PROFILE_FUNCTION();
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

OpenGLShader::~OpenGLShader() {
  CK_PROFILE_FUNCTION();
  glDeleteProgram(renderer_id_);
}

void OpenGLShader::Bind() const {
  CK_PROFILE_FUNCTION();
  glUseProgram(renderer_id_);
}

void OpenGLShader::Unbind() const {
  CK_PROFILE_FUNCTION();
  glUseProgram(0);
}

const std::string& OpenGLShader::Name() const { return name_; }

/*─────────────────────────────────────┐
│      Uniform Functions Helpers       │
└──────────────────────────────────────*/

void OpenGLShader::SetInt(const std::string& name, int value) const {
  CK_PROFILE_FUNCTION();
  UploadUniformInt(name, value);
}

void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count) {
  UploadUniformIntArray(name, values, count);
}

void OpenGLShader::SetFloat(const std::string& name, float value) const {
  CK_PROFILE_FUNCTION();
  UploadUniformFloat(name, value);
}

void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value) const {
  CK_PROFILE_FUNCTION();
  UploadUniformFloat2(name, value);
}

void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) const {
  CK_PROFILE_FUNCTION();
  UploadUniformFloat3(name, value);
}

void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) const {
  CK_PROFILE_FUNCTION();
  UploadUniformFloat4(name, value);
}

void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) const {
  CK_PROFILE_FUNCTION();
  UploadUniformMat4(name, value);
}

/*─────────────────────────────────────┐
│          Uniform Functions           │
└──────────────────────────────────────*/

void OpenGLShader::UploadUniformInt(const std::string& name, int value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniform1i(location, value);
}

void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values,
                                         uint32_t count) const {
  GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniform1iv(location, count, values);
}

void OpenGLShader::UploadUniformFloat(const std::string& name, float value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniform1f(location, value);
}

void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniform2f(location, value.x, value.y);
}

void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& value) const {
  const GLint location = glGetUniformLocation(renderer_id_, name.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

/*─────────────────────────────────────┐
│           Private Methods            │
└──────────────────────────────────────*/

std::string OpenGLShader::ReadFile(const std::string& filepath) {
  CK_PROFILE_FUNCTION();
  auto result = std::string();
  auto file = std::ifstream(filepath, std::ios::in | std::ios::binary);
  if (!file) {
    CK_ENGINE_ERROR("could not open file: {}", filepath);
    return result;
  }

  // Get file size
  file.seekg(0, std::ios::end);
  result.resize(file.tellg());
  file.seekg(0, std::ios::beg);  // beg --> begin

  file.read(&result[0], result.size());
  return result;
}

std::unordered_map<GLenum, std::string> OpenGLShader::Parse(const std::string& source) {
  CK_PROFILE_FUNCTION();
  auto shader_sources = std::unordered_map<GLenum, std::string>();

  const char* type_token = "#type";
  size_t type_token_length = strlen(type_token);
  size_t pos = source.find(type_token, 0);

  while (pos != std::string::npos) {
    size_t eol = source.find_first_of("\r\n", pos);
    CK_ENGINE_ASSERT(eol != std::string::npos, "syntax error");

    // pos of the begin of type_token name
    //       ↓ <-- here
    // #type vertex
    size_t begin = pos + type_token_length + 1;
    std::string type = source.substr(begin, eol - begin);
    CK_ENGINE_ASSERT(type == "vertex" || type == "fragment", "invalid shader type specified");

    begin = source.find_first_not_of("\r\n", eol);  // begin of the glsl shader source
    pos = source.find(type_token, begin);           // end of the source

    // If not found, which means current shader souce block is the last one
    size_t length = (pos == std::string::npos) ? source.size() - begin : pos - begin;

    shader_sources[ShaderTypeFromString(type)] = source.substr(begin, length);
  }
  CK_ENGINE_INFO("shader sources size={}", shader_sources.size());
  return shader_sources;
}

void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shader_sources) {
  CK_PROFILE_FUNCTION();
  GLuint program = glCreateProgram();
  CK_ENGINE_ASSERT(shader_sources.size() <= 2, "only 2 shader sources in a single file supported");
  ;
  auto shader_ids = std::vector<GLenum>(shader_sources.size());

  // Compile
  for (auto const& [shader_type, source] : shader_sources) {
    GLuint shader = glCreateShader(shader_type);

    const GLchar* source_cstr = source.c_str();
    glShaderSource(shader, 1, &source_cstr, 0);
    glCompileShader(shader);

    // Check compile
    GLint is_compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {
      GLint max_length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
      std::vector<GLchar> info_log(max_length);
      glGetShaderInfoLog(shader, max_length, &max_length, &info_log[0]);
      glDeleteShader(shader);
      CK_CLIENT_ERROR("{}", info_log.data());
      CK_ENGINE_ASSERT(false, "shader compilation failure");
      return;
    }

    glAttachShader(program, shader);
    shader_ids.push_back(shader);
  }

  // Link
  glLinkProgram(program);
  GLint is_linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, (int*)&is_linked);

  // Check link
  if (is_linked == GL_FALSE) {
    GLint max_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
    std::vector<GLchar> info_log(max_length);
    glGetProgramInfoLog(program, max_length, &max_length, &info_log[0]);
    glDeleteProgram(program);
    for (auto shader_id : shader_ids) {
      glDeleteShader(shader_id);
    }

    CK_ENGINE_ERROR("{}", info_log.data());
    CK_ENGINE_ASSERT(false, "shader link failure");
    return;
  }

  for (auto shader_id : shader_ids) {
    glDeleteShader(shader_id);
  }
  renderer_id_ = program;
}
}  // namespace ck
