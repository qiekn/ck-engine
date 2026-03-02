#include "pch.h"
#include "script_engine.h"

#include <fstream>

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace ck {

struct ScriptEngineData {
  MonoDomain* root_domain = nullptr;
  MonoDomain* app_domain = nullptr;
  MonoAssembly* core_assembly = nullptr;
};

static ScriptEngineData* s_data = nullptr;

void ScriptEngine::Init() {
  s_data = new ScriptEngineData();
  InitMono();
}

void ScriptEngine::Shutdown() {
  ShutdownMono();
  delete s_data;
}

static char* ReadBytes(const std::string& filepath, uint32_t* out_size) {
  std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
  if (!stream) return nullptr;

  std::streampos end = stream.tellg();
  stream.seekg(0, std::ios::beg);
  uint32_t size = static_cast<uint32_t>(end - stream.tellg());
  if (size == 0) return nullptr;

  char* buffer = new char[size];
  stream.read(buffer, size);
  stream.close();

  *out_size = size;
  return buffer;
}

static MonoAssembly* LoadCSharpAssembly(const std::string& assembly_path) {
  uint32_t file_size = 0;
  char* file_data = ReadBytes(assembly_path, &file_size);

  // NOTE: We can't use this image for anything other than loading the assembly
  // because this image doesn't have a reference to the assembly
  MonoImageOpenStatus status;
  MonoImage* image = mono_image_open_from_data_full(file_data, file_size, 1, &status, 0);
  if (status != MONO_IMAGE_OK) {
    const char* error_message = mono_image_strerror(status);
    CK_ENGINE_ERROR("Failed to load C# assembly image: {}", error_message);
    return nullptr;
  }

  MonoAssembly* assembly = mono_assembly_load_from_full(image, assembly_path.c_str(), &status, 0);
  mono_image_close(image);
  delete[] file_data;

  return assembly;
}

static void PrintAssemblyTypes(MonoAssembly* assembly) {
  MonoImage* image = mono_assembly_get_image(assembly);
  const MonoTableInfo* type_def_table = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
  int32_t num_types = mono_table_info_get_rows(type_def_table);

  for (int32_t i = 0; i < num_types; i++) {
    uint32_t cols[MONO_TYPEDEF_SIZE];
    mono_metadata_decode_row(type_def_table, i, cols, MONO_TYPEDEF_SIZE);

    const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
    const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
    CK_ENGINE_TRACE("{}.{}", name_space, name);
  }
}

void ScriptEngine::InitMono() {
  mono_set_assemblies_path("deps/mono/lib/mono/4.5");

  MonoDomain* root_domain = mono_jit_init("CKJITRuntime");
  CK_ENGINE_ASSERT(root_domain, "Failed to initialize Mono JIT runtime");
  s_data->root_domain = root_domain;

  // Create an App Domain (isolated environment, supports hot-reload later)
  char app_domain_name[] = "CKScriptRuntime";
  s_data->app_domain = mono_domain_create_appdomain(app_domain_name, nullptr);
  mono_domain_set(s_data->app_domain, true);

  // Load C# core assembly
  s_data->core_assembly = LoadCSharpAssembly("resources/scripts/CK-ScriptCore.dll");
  PrintAssemblyTypes(s_data->core_assembly);

  // --- Test: create object and call methods ---
  MonoImage* assembly_image = mono_assembly_get_image(s_data->core_assembly);
  MonoClass* mono_class = mono_class_from_name(assembly_image, "CK", "Main");

  // 1. Create an instance (calls the constructor)
  MonoObject* instance = mono_object_new(s_data->app_domain, mono_class);
  mono_runtime_object_init(instance);

  // 2. Call a parameterless method
  MonoMethod* print_message_func = mono_class_get_method_from_name(mono_class, "PrintMessage", 0);
  mono_runtime_invoke(print_message_func, instance, nullptr, nullptr);

  // 3. Call a method with one int parameter
  MonoMethod* print_int_func = mono_class_get_method_from_name(mono_class, "PrintInt", 1);
  int value = 5;
  void* param = &value;
  mono_runtime_invoke(print_int_func, instance, &param, nullptr);

  // 4. Call a method with two int parameters
  MonoMethod* print_ints_func = mono_class_get_method_from_name(mono_class, "PrintInts", 2);
  int value2 = 508;
  void* params[2] = {&value, &value2};
  mono_runtime_invoke(print_ints_func, instance, params, nullptr);

  // 5. Call a method with a string parameter
  MonoString* mono_string = mono_string_new(s_data->app_domain, "Hello World from C++!");
  MonoMethod* print_custom_message_func = mono_class_get_method_from_name(mono_class, "PrintCustomMessage", 1);
  void* string_param = mono_string;
  mono_runtime_invoke(print_custom_message_func, instance, &string_param, nullptr);
}

void ScriptEngine::ShutdownMono() {
  // NOTE: Mono shutdown is tricky — revisit this later
  // mono_domain_unload(s_data->app_domain);
  s_data->app_domain = nullptr;

  // mono_jit_cleanup(s_data->root_domain);
  s_data->root_domain = nullptr;
}

}  // namespace ck
