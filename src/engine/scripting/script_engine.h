#pragma once

#include "scene/scene.h"
#include "scene/entity.h"

#include <filesystem>
#include <string>
#include <unordered_map>

extern "C" {
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
}

namespace ck {

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

class ScriptInstance {
public:
  ScriptInstance(Ref<ScriptClass> script_class, Entity entity);

  void InvokeOnCreate();
  void InvokeOnUpdate(float ts);

private:
  Ref<ScriptClass> script_class_;

  MonoObject* instance_ = nullptr;
  MonoMethod* constructor_ = nullptr;
  MonoMethod* on_create_method_ = nullptr;
  MonoMethod* on_update_method_ = nullptr;
};

class ScriptEngine {
public:
  static void Init();
  static void Shutdown();

  static void LoadAssembly(const std::filesystem::path& filepath);

  static void OnRuntimeStart(Scene* scene);
  static void OnRuntimeStop();

  static bool EntityClassExists(const std::string& full_class_name);
  static void OnCreateEntity(Entity entity);
  static void OnUpdateEntity(Entity entity, DeltaTime ts);

  static Scene* GetSceneContext();
  static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();

  static MonoImage* GetCoreAssemblyImage();

private:
  static void InitMono();
  static void ShutdownMono();

  static MonoObject* InstantiateClass(MonoClass* mono_class);
  static void LoadAssemblyClasses(MonoAssembly* assembly);

  friend class ScriptClass;
  friend class ScriptGlue;
};

}  // namespace ck
