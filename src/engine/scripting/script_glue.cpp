#include "pch.h"
#include "script_glue.h"
#include "script_engine.h"

#include "core/uuid.h"
#include "events/key_codes.h"
#include "core/input.h"

#include "scene/scene.h"
#include "scene/entity.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/box2d.h"

namespace ck {

static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_entity_has_component_funcs;

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

static bool Entity_HasComponent(UUID entity_id, MonoReflectionType* component_type) {
  Scene* scene = ScriptEngine::GetSceneContext();
  CK_ENGINE_ASSERT(scene, "No scene context");
  Entity entity = scene->GetEntityByUUID(entity_id);
  CK_ENGINE_ASSERT(entity, "Entity not found");

  MonoType* managed_type = mono_reflection_type_get_type(component_type);
  CK_ENGINE_ASSERT(s_entity_has_component_funcs.find(managed_type) != s_entity_has_component_funcs.end(), "Component type not registered");
  return s_entity_has_component_funcs.at(managed_type)(entity);
}

static void TransformComponent_GetTranslation(UUID entity_id, glm::vec3* out_translation) {
  Scene* scene = ScriptEngine::GetSceneContext();
  CK_ENGINE_ASSERT(scene, "No scene context");
  Entity entity = scene->GetEntityByUUID(entity_id);
  CK_ENGINE_ASSERT(entity, "Entity not found");

  *out_translation = entity.GetComponent<TransformComponent>().position;
}

static void TransformComponent_SetTranslation(UUID entity_id, glm::vec3* translation) {
  Scene* scene = ScriptEngine::GetSceneContext();
  CK_ENGINE_ASSERT(scene, "No scene context");
  Entity entity = scene->GetEntityByUUID(entity_id);
  CK_ENGINE_ASSERT(entity, "Entity not found");

  entity.GetComponent<TransformComponent>().position = *translation;
}

static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entity_id, glm::vec2* impulse, glm::vec2* point, bool wake) {
  Scene* scene = ScriptEngine::GetSceneContext();
  CK_ENGINE_ASSERT(scene, "No scene context");
  Entity entity = scene->GetEntityByUUID(entity_id);
  CK_ENGINE_ASSERT(entity, "Entity not found");

  auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
  b2BodyId body_id = b2LoadBodyId(rb2d.runtime_body_id);
  b2Body_ApplyLinearImpulse(body_id, {impulse->x, impulse->y}, {point->x, point->y}, wake);
}

static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entity_id, glm::vec2* impulse, bool wake) {
  Scene* scene = ScriptEngine::GetSceneContext();
  CK_ENGINE_ASSERT(scene, "No scene context");
  Entity entity = scene->GetEntityByUUID(entity_id);
  CK_ENGINE_ASSERT(entity, "Entity not found");

  auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
  b2BodyId body_id = b2LoadBodyId(rb2d.runtime_body_id);
  b2Body_ApplyLinearImpulseToCenter(body_id, {impulse->x, impulse->y}, wake);
}

static bool Input_IsKeyDown(KeyCode keycode) {
  return Input::IsKeyPressed(keycode);
}

template <typename... Component>
static void RegisterComponent() {
  ([]() {
    std::string_view type_name = typeid(Component).name();
    size_t pos = type_name.find_last_of(':');
    std::string_view struct_name = type_name.substr(pos + 1);
    std::string managed_typename = fmt::format("CK.{}", struct_name);

    MonoType* managed_type = mono_reflection_type_from_name(managed_typename.data(), ScriptEngine::GetCoreAssemblyImage());
    if (!managed_type) {
      CK_ENGINE_ERROR("Could not find component type {}", managed_typename);
      return;
    }
    s_entity_has_component_funcs[managed_type] = [](Entity entity) { return entity.HasComponent<Component>(); };
  }(), ...);
}

template <typename... Component>
static void RegisterComponent(ComponentGroup<Component...>) {
  RegisterComponent<Component...>();
}

void ScriptGlue::RegisterComponents() {
  RegisterComponent(AllComponents{});
}

void ScriptGlue::RegisterFunctions() {
  CK_ADD_INTERNAL_CALL(NativeLog);
  CK_ADD_INTERNAL_CALL(NativeLog_Vector);
  CK_ADD_INTERNAL_CALL(NativeLog_VectorDot);

  CK_ADD_INTERNAL_CALL(Entity_HasComponent);
  CK_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
  CK_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

  CK_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
  CK_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

  CK_ADD_INTERNAL_CALL(Input_IsKeyDown);
}

}  // namespace ck
