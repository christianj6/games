#include "Eigen/Dense"
#include "raylib.h"
#include <vector>

std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
slice_voxel_space(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &,
    int, int);

Mesh merge_voxels(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &,
    int);
