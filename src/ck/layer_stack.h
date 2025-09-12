#pragma once

#include "layer.h"
#include "pch.h"

namespace ck {
class LayerStack {
public:
  LayerStack();
  virtual ~LayerStack();

  // 函数用了小写&下划线，保持 STL 容器相同的使用习惯

  // 主要逻辑层，渲染、UI、游戏逻辑
  void push_layer(std::unique_ptr<Layer> layer);
  void pop_layer(std::unique_ptr<Layer> layer);

  // Overlay 是覆盖在画面最上层的内容
  // 比如调试面板、日志窗口、FPS信息、弹窗
  void push_overlay(std::unique_ptr<Layer> layer);
  void pop_overlay(std::unique_ptr<Layer> overlay);

  auto begin() { return layers_.begin(); }
  auto end() { return layers_.end(); }

private:
  std::vector<std::unique_ptr<Layer>> layers_;
  std::vector<std::unique_ptr<Layer>>::iterator layer_insert_;
};
}  // namespace ck
