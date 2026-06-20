#pragma once
#include "raylib.h"
#include <deque>

struct BlinkCommand {
  Vector3 from;
  Vector3 to;
};

class JumpList {
public:
  static constexpr int CAPACITY = 5;

  void push(BlinkCommand cmd) {
    history_.push_front(cmd);
    if ((int)history_.size() > CAPACITY)
      history_.pop_back();
  }

  BlinkCommand pop() {
    BlinkCommand cmd = history_.front();
    history_.pop_front();
    return cmd;
  }

  bool empty() const { return history_.empty(); }
  int size() const { return (int)history_.size(); }
  const BlinkCommand &peek(int i) const { return history_[i]; }

private:
  std::deque<BlinkCommand> history_;
};
