module;

// Pull in the umbrella's worth of public headers (engine-internal during
// the module's global fragment).
#include "core/core.h"
#include "core/application.h"
#include "core/log.h"
#include "core/layer.h"
#include "core/input.h"
#include "core/deltatime.h"

#include "events/event.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"
#include "events/key_codes.h"
#include "events/mouse_codes.h"

#include "imgui/imgui_layer.h"

#include "renderer/renderer.h"
#include "renderer/renderer_2d.h"
#include "renderer/camera.h"
#include "renderer/perspective_camera.h"

#include "scene/components.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "scene/scene_serializer.h"

#include <imgui.h>

export module ck;

// Re-export the public surface as `ck::*` using-declarations. Engine-only
// types (Renderer, Material, vulkan::*, vk::*) are deliberately left out.
export namespace ck {

// core
using ::ck::Application;
using ::ck::ApplicationSpecification;
using ::ck::ApplicationCommandLineArgs;
using ::ck::CreateApplication;
using ::ck::EntryPoint;
using ::ck::Layer;
using ::ck::DeltaTime;
using ::ck::Window;
using ::ck::WindowProps;
using ::ck::Input;
using ::ck::Log;

// smart-pointer aliases + factories
using ::ck::Scope;
using ::ck::Ref;
using ::ck::CreateScope;
using ::ck::CreateRef;

// events
using ::ck::Event;
using ::ck::EventType;
using ::ck::EventDispatcher;
using ::ck::WindowResizeEvent;
using ::ck::WindowCloseEvent;
using ::ck::KeyPressedEvent;
using ::ck::KeyReleasedEvent;
using ::ck::MouseButtonPressedEvent;
using ::ck::MouseButtonReleasedEvent;
using ::ck::MouseMoveEvent;
using ::ck::MouseScrollEvent;
using ::ck::MouseCode;
using ::ck::Mouse;
using ::ck::KeyCode;

// imgui
using ::ck::ImGuiLayer;

// renderer
using ::ck::Renderer;
using ::ck::Renderer2D;
using ::ck::Camera;
using ::ck::PerspectiveCamera;

// scene
using ::ck::Scene;
using ::ck::Entity;
using ::ck::TagComponent;
using ::ck::TransformComponent;
using ::ck::SpriteRendererComponent;
using ::ck::SceneSerializer;

}  // export namespace ck

// glm bare essentials clients touch (mat4, vecs, transform helpers).
// Re-exported so client TUs don't have to #include glm — including it
// after import ck pulls glm a second time in the client TU and trips
// duplicate-template-default errors against the module-global copies.
export namespace glm {
using ::glm::mat4;
using ::glm::mat3;
using ::glm::vec2;
using ::glm::vec3;
using ::glm::vec4;
using ::glm::translate;
using ::glm::scale;
using ::glm::rotate;
}  // export namespace glm

export namespace ck::log {
using ::ck::log::trace;
using ::ck::log::debug;
using ::ck::log::info;
using ::ck::log::warn;
using ::ck::log::error;
using ::ck::log::fatal;
}  // export namespace ck::log

// ImGui re-exports — minimal set chosen for the phase 6.A panels.
// Add as new client call sites need them; printf-style varargs (Text)
// and inline default-argument helpers are the known compatibility
// risks called out in phase-6-plan.md.
export namespace ImGui {
using ::ImGui::Begin;
using ::ImGui::End;
using ::ImGui::Text;
using ::ImGui::GetIO;
using ::ImGui::Image;
using ::ImGui::GetContentRegionAvail;
using ::ImGui::DockSpaceOverViewport;
using ::ImGui::GetMainViewport;
using ::ImGui::PushStyleVar;
using ::ImGui::PopStyleVar;
// SceneHierarchyPanel (6.B.2): tree nodes + popups + mouse queries.
using ::ImGui::TreeNodeEx;
using ::ImGui::TreePop;
using ::ImGui::IsItemClicked;
using ::ImGui::IsMouseDown;
using ::ImGui::IsWindowHovered;
using ::ImGui::IsWindowFocused;
using ::ImGui::BeginPopupContextItem;
using ::ImGui::BeginPopupContextWindow;
using ::ImGui::EndPopup;
using ::ImGui::MenuItem;
// PropertiesPanel (6.B.3): collapsing headers + scalar/color/text editors.
using ::ImGui::CollapsingHeader;
using ::ImGui::Separator;
using ::ImGui::InputText;
using ::ImGui::DragFloat3;
using ::ImGui::ColorEdit4;
// Editor menu bar (6.B.5): main menu + nested menus.
using ::ImGui::BeginMainMenuBar;
using ::ImGui::EndMainMenuBar;
using ::ImGui::BeginMenu;
using ::ImGui::EndMenu;
}  // export namespace ImGui

export using ::ImGuiIO;
export using ::ImGuiWindowFlags;
export using ::ImGuiViewport;
export using ::ImGuiDockNodeFlags;
export using ::ImGuiStyleVar;
export using ::ImGuiStyleVar_;
export using ::ImGuiTreeNodeFlags;
export using ::ImGuiTreeNodeFlags_;
export using ::ImGuiPopupFlags;
export using ::ImGuiPopupFlags_;
export using ::ImTextureID;
export using ::ImVec2;
export using ::ImVec4;
