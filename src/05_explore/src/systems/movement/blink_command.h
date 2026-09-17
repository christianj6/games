#pragma once
#include "raylib.h"
#include <deque>
#include <vector>

struct BlinkCommand {
  Vector3 from;
  Vector3 to;
};

// Manually placed anchors for Recall Mode.
// Separate from JumpList (which tracks automatic blink history).
class AnchorList {
public:
  static constexpr int CAPACITY = 5;

  void place(Vector3 pos) {
    if ((int)anchors_.size() >= CAPACITY)
      anchors_.erase(anchors_.begin()); // drop oldest
    anchors_.push_back(pos);
  }

  bool empty() const { return anchors_.empty(); }
  int size() const { return (int)anchors_.size(); }
  Vector3 get(int i) const { return anchors_[i]; }

private:
  std::vector<Vector3> anchors_;
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
