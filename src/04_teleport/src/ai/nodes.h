#include "behaviortree_cpp/action_node.h"

using namespace BT;

BT::NodeStatus CheckBattery();

class ApproachObject : public BT::SyncActionNode {
public:
  ApproachObject(const std::string &name) : BT::SyncActionNode(name, {}) {}

  // You must override the virtual function tick()
  BT::NodeStatus tick() override {
    std::cout << "ApproachObject: " << this->name() << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
};

class GripperInterface {
public:
  GripperInterface() : _open(true) {}

  NodeStatus open() {
    _open = true;
    std::cout << "GripperInterface::open" << std::endl;
    return NodeStatus::SUCCESS;
  }

  NodeStatus close() {
    std::cout << "GripperInterface::close" << std::endl;
    _open = false;
    return NodeStatus::SUCCESS;
  }

private:
  bool _open;
};
