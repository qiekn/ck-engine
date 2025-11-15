#pragma once

// clang-format off

#include "core/core.h"                                   // IWYU pragma: export
#include "core/application.h"                            // IWYU pragma: export
#include "core/log.h"                                    // IWYU pragma: export
#include "core/layer.h"                                  // IWYU pragma: export
#include "core/input.h"                                  // IWYU pragma: export
#include "core/deltatime.h"                              // IWYU pragma: export

#include "events/event.h"                                // IWYU pragma: export

#include "events/key_code.h"                             // IWYU pragma: export
#include "events/mouse_button_code.h"                    // IWYU pragma: export

#include "imgui/imgui_layer.h"                           // IWYU pragma: export

/*─────────────────────────────────────┐
│               Renderer               │
└──────────────────────────────────────*/

#include "renderer/renderer.h"                           // IWYU pragma: export
#include "renderer/render_command.h"                     // IWYU pragma: export

#include "renderer/buffer.h"                             // IWYU pragma: export
#include "renderer/shader.h"                             // IWYU pragma: export
#include "renderer/texture.h"                            // IWYU pragma: export
#include "renderer/vertex_array.h"                       // IWYU pragma: export

#include "renderer/orthographic_camera_controller.h"     // IWYU pragma: export
#include "renderer/orthographic_camera.h"                // IWYU pragma: export

/*─────────────────────────────────────┐
│             Entry Point              │
└──────────────────────────────────────*/

#include "core/entry_point.h"                            // IWYU pragma: export

// clang-format on

// use vim q: command to format (plugin "echasnovski/mini.nvim" --> Vggga:)
// %s/\/\/ IWYU/?\/\/ IWYU
// %s/?\/\/ IWYU/\/\/ IWYU
