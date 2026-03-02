#include "pch.h"
#include "script_engine.h"

#include "script_glue.h"

#include <fstream>

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace ck {

namespace utils {

static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* out_size) {
  std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
  if (!stream) return nullptr;

  std::streampos end = stream.tellg();
  stream.seekg(0, std::ios::beg);
  uint64_t size = end - stream.tellg();
  if (size == 0) return nullptr;

  char* buffer = new char[size];
  stream.read(buffer, size);
  stream.close();

  *out_size = static_cast<uint32_t>(size);
  return buffer;
}

static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assembly_path) {
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

  std::string path_string = assembly_path.string();
  MonoAssembly* assembly = mono_assembly_load_from_full(image, path_string.c_str(), &status, 0);
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

}  // namespace utils

struct ScriptEngineData {
  MonoDomain* root_domain = nullptr;
  MonoDomain* app_domain = nullptr;

  MonoAssembly* core_assembly = nullptr;
  MonoImage* core_assembly_image = nullptr;

  ScriptClass entity_class;
};

static ScriptEngineData* s_data = nullptr;

void ScriptEngine::Init() {
  s_data = new ScriptEngineData();

  InitMono();
  LoadAssembly("assets/scripts/CK-ScriptCore.dll");

  ScriptGlue::RegisterFunctions();

  // Retrieve and instantiate class
  s_data->entity_class = ScriptClass("CK", "Entity");

  MonoObject* instance = s_data->entity_class.Instantiate();

  // Call method
  MonoMethod* print_message_func = s_data->entity_class.GetMethod("PrintMessage", 0);
  s_data->entity_class.InvokeMethod(instance, print_message_func);

  // Call method with param
  MonoMethod* print_int_func = s_data->entity_class.GetMethod("PrintInt", 1);
  int value = 5;
  void* param = &value;
  s_data->entity_class.InvokeMethod(instance, print_int_func, &param);

  MonoMethod* print_ints_func = s_data->entity_class.GetMethod("PrintInts", 2);
  int value2 = 508;
  void* params[2] = {&value, &value2};
  s_data->entity_class.InvokeMethod(instance, print_ints_func, params);

  MonoString* mono_string = mono_string_new(s_data->app_domain, "Hello World from C++!");
  MonoMethod* print_custom_message_func = s_data->entity_class.GetMethod("PrintCustomMessage", 1);
  void* string_param = mono_string;
  s_data->entity_class.InvokeMethod(instance, print_custom_message_func, &string_param);
}

void ScriptEngine::Shutdown() {
  ShutdownMono();
  delete s_data;
}

void ScriptEngine::InitMono() {
  mono_set_assemblies_path("deps/mono/lib/mono/4.5");

  MonoDomain* root_domain = mono_jit_init("CKJITRuntime");
  CK_ENGINE_ASSERT(root_domain, "Failed to initialize Mono JIT runtime");
  s_data->root_domain = root_domain;
}

void ScriptEngine::ShutdownMono() {
  // NOTE: Mono shutdown is tricky — revisit this later
  // mono_domain_unload(s_data->app_domain);
  s_data->app_domain = nullptr;

  // mono_jit_cleanup(s_data->root_domain);
  s_data->root_domain = nullptr;
}

void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath) {
  // Create an App Domain
  char app_domain_name[] = "CKScriptRuntime";
  s_data->app_domain = mono_domain_create_appdomain(app_domain_name, nullptr);
  mono_domain_set(s_data->app_domain, true);

  s_data->core_assembly = utils::LoadMonoAssembly(filepath);
  s_data->core_assembly_image = mono_assembly_get_image(s_data->core_assembly);
  // utils::PrintAssemblyTypes(s_data->core_assembly);
}

MonoObject* ScriptEngine::InstantiateClass(MonoClass* mono_class) {
  MonoObject* instance = mono_object_new(s_data->app_domain, mono_class);
  mono_runtime_object_init(instance);
  return instance;
}

// --- ScriptClass ---

ScriptClass::ScriptClass(const std::string& class_namespace, const std::string& class_name)
    : class_namespace_(class_namespace), class_name_(class_name) {
  mono_class_ = mono_class_from_name(s_data->core_assembly_image, class_namespace.c_str(), class_name.c_str());
}

MonoObject* ScriptClass::Instantiate() {
  return ScriptEngine::InstantiateClass(mono_class_);
}

MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameter_count) {
  return mono_class_get_method_from_name(mono_class_, name.c_str(), parameter_count);
}

MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params) {
  return mono_runtime_invoke(method, instance, params, nullptr);
}

}  // namespace ck
