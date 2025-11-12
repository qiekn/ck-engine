#include "render_command.h"

#include "platform/opengl/opengl_renderer_api.h"
#include "renderer/renderer_api.h"

namespace ck {
RendererAPI* RenderCommand::renderer_api_ = new OpenGLRendererAPI();
}  // namespace ck
