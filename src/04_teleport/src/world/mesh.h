#include "Eigen/Dense"
#include "raylib.h"
#include <vector>

// TODO: simple grid-based nav mesh

std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
slice_voxel_space(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &,
    int, int);

Mesh merge_voxels(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &,
    int);
