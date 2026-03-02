#include "pch.h"
#include "script_glue.h"

#include "mono/metadata/object.h"
#include "mono/metadata/loader.h"

#include "glm/glm.hpp"

namespace ck {

#define CK_ADD_INTERNAL_CALL(Name) \
  mono_add_internal_call("CK.InternalCalls::" #Name, reinterpret_cast<const void*>(Name))

static void NativeLog(MonoString* string, int parameter) {
  char* c_str = mono_string_to_utf8(string);
  std::string str(c_str);
  mono_free(c_str);
  std::cout << str << ", " << parameter << std::endl;
}

static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* out_result) {
  CK_ENGINE_WARN("Value: ({}, {}, {})", parameter->x, parameter->y, parameter->z);
  *out_result = glm::normalize(*parameter);
}

static float NativeLog_VectorDot(glm::vec3* parameter) {
  CK_ENGINE_WARN("Value: ({}, {}, {})", parameter->x, parameter->y, parameter->z);
  return glm::dot(*parameter, *parameter);
}

void ScriptGlue::RegisterFunctions() {
  CK_ADD_INTERNAL_CALL(NativeLog);
  CK_ADD_INTERNAL_CALL(NativeLog_Vector);
  CK_ADD_INTERNAL_CALL(NativeLog_VectorDot);
}

}  // namespace ck
