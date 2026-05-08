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

#include "renderer/renderer_2d.h"

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

// imgui
using ::ck::ImGuiLayer;

// renderer
using ::ck::Renderer2D;

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

// ImGui re-exports — minimal set chosen for phase 6.A.2 stats panels.
// Add as new client call sites need them; printf-style varargs (Text)
// and inline default-argument helpers are the known compatibility
// risks called out in phase-6-plan.md.
export namespace ImGui {
using ::ImGui::Begin;
using ::ImGui::End;
using ::ImGui::Text;
using ::ImGui::GetIO;
}  // export namespace ImGui

export using ::ImGuiIO;
export using ::ImGuiWindowFlags;
