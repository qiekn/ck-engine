#include "layer_stack.h"

namespace ck {
LayerStack::LayerStack() {}

LayerStack::~LayerStack() {
  // Mirror Push order in reverse so overlays detach before regular layers.
  // Must run before the unique_ptrs themselves destruct, so OnDetach can
  // still touch sibling subsystems (ImGuiLayer reaches into Renderer).
  for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
    (*it)->OnDetach();
  }
}

void LayerStack::push_layer(Scope<Layer> layer) {
  layers_.emplace(layers_.begin() + layer_insert_index_, std::move(layer));
  layer_insert_index_++;
}

void LayerStack::pop_layer(Scope<Layer> layer) {
  auto it = std::find(layers_.begin(), layers_.end(), layer);
  if (it != layers_.end()) {
    layers_.erase(it);
    layer_insert_index_--;
  }
}

void LayerStack::push_overlay(Scope<Layer> layer) { layers_.emplace_back(std::move(layer)); }

void LayerStack::pop_overlay(Scope<Layer> overlay) {
  auto it = std::find(layers_.begin(), layers_.end(), overlay);
  if (it != layers_.end()) {
    layers_.erase(it);
  }
}
}  // namespace ck
