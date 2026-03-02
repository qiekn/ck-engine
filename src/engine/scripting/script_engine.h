#pragma once

#include <filesystem>
#include <string>

extern "C" {
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
}

namespace ck {

class ScriptEngine {
public:
  static void Init();
  static void Shutdown();

  static void LoadAssembly(const std::filesystem::path& filepath);

private:
  static void InitMono();
  static void ShutdownMono();

  static MonoObject* InstantiateClass(MonoClass* mono_class);

  friend class ScriptClass;
};

class ScriptClass {
public:
  ScriptClass() = default;
  ScriptClass(const std::string& class_namespace, const std::string& class_name);

  MonoObject* Instantiate();
  MonoMethod* GetMethod(const std::string& name, int parameter_count);
  MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

private:
  std::string class_namespace_;
  std::string class_name_;
  MonoClass* mono_class_ = nullptr;
};

}  // namespace ck
