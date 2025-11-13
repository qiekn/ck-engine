#pragma once

// clang-format off

#include "core.h"              // IWYU pragma: export
#include "application.h"       // IWYU pragma: export
#include "log.h"               // IWYU pragma: export


#include "input.h"             // IWYU pragma: export
#include "key_code.h"          // IWYU pragma: export
#include "mouse_button_code.h" // IWYU pragma: export


#include "layer.h"             // IWYU pragma: export
#include "imgui/imgui_layer.h" // IWYU pragma: export

/*─────────────────────────────────────┐
│               Renderer               │
└──────────────────────────────────────*/

#include "renderer/renderer.h"            // IWYU pragma: export
#include "renderer/render_command.h"      // IWYU pragma: export

#include "renderer/buffer.h"              // IWYU pragma: export
#include "renderer/shader.h"              // IWYU pragma: export
#include "renderer/texture.h"             // IWYU pragma: export
#include "renderer/vertex_array.h"        // IWYU pragma: export

#include "renderer/orthographic_camera.h" // IWYU pragma: export

/*─────────────────────────────────────┐
│             Entry Point              │
└──────────────────────────────────────*/

#include "entry_point.h"       // IWYU pragma: export

// clang-format on
