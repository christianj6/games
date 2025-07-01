#include "micropather.h"
#include <Eigen/Dense>

class Map : public micropather::Graph {
private:
  std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
      vector_space_data;

public:
  void set_vector_space_data(
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
          &vector_space_data);
  float LeastCostEstimate(void *stateStart, void *stateEnd);
  void AdjacentCost(void *state, MP_VECTOR<micropather::StateCost> *adjacent);
  void PrintStateInfo(void *state);
};

struct Node {
  int x;
  int y;
  Node(int _x, int _y) : x(_x), y(_y) {}

  // Convert node to state integer (for micropather internal handling)
  void *ToState() const { return (void *)(size_t)(x + y * 1000); }

  // Convert state integer back to node
  static Node FromState(void *state) {
    size_t val = (size_t)state;
    int y = (int)(val / 1000);
    int x = (int)(val % 1000);
    return Node(x, y);
  }

  bool IsValid() const { return x >= 0 && x < 1000 && y >= 0 && y < 1000; }
};
