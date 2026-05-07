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

#include "renderer/renderer_2d.h"

export module ck;

// Re-export the public surface as `ck::*` using-declarations. Engine-only
// types (Renderer, Material, vulkan::*, vk::*) are deliberately left out.
export namespace ck {

// core
using ::ck::Application;
using ::ck::ApplicationSpecification;
using ::ck::ApplicationCommandLineArgs;
using ::ck::CreateApplication;
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

// renderer
using ::ck::Renderer2D;

}  // export namespace ck

export namespace ck::log {
using ::ck::log::trace;
using ::ck::log::debug;
using ::ck::log::info;
using ::ck::log::warn;
using ::ck::log::error;
using ::ck::log::fatal;
}  // export namespace ck::log
