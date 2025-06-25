#include "world.h"
#include "fmt/core.h"
#include "raymath.h"
#include "utils/random.h"

void World::get_initial_world_state() {
  // TODO: try to create this elsewhere
  voxel_space.resize(
      VOXEL_SIZE, Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>::Constant(
                      VOXEL_SIZE, VOXEL_SIZE, false));

  RandomNumberGenerator<float> random_voxel(0.0f, 1.0f);
  RandomNumberGenerator<int> random_height(1, 20);

  const int plane_y = 0;
  int active_voxel_count = 0;

  // TODO: bro just manipulate the whole space to achieve walls etc.
  // TODO: just make sure you split it up right

  for (int i = 0; i < VOXEL_SIZE; i++) {
    for (int j = 0; j < VOXEL_SIZE; j++) {
      // create the flat plane
      voxel_space[plane_y](i, j) = true;
      // create a ceiling
      voxel_space[VOXEL_SIZE - 60](i, j) = true;
      active_voxel_count++;

      // randomly create columns
      if (random_voxel() < VOXEL_DENSITY) {
        for (int k = 0; k < random_height(); k++) {
          voxel_space[plane_y + k](i, j) = true;
          // columns coming down from above
          voxel_space[VOXEL_SIZE - 1 - 60 - k](i, j) = true;
          active_voxel_count += 2;
        }
      }
    }
  }
}

void World::configure_materials() {
  material_default = LoadMaterialDefault();
  material_default.shader = renderer.get_shader();
  material_default.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
  material_default.maps[MATERIAL_MAP_DIFFUSE].value = 1.0f;
}

std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
slice_voxel_space(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &voxel_space,
    int y_start, int y_end) {

  if (y_start < 0)
    y_start = 0;
  if (y_end > static_cast<int>(voxel_space.size()))
    y_end = voxel_space.size();
  if (y_start >= y_end)
    return {};

  // Simply return the subvector (no merging, no collapsing)
  return std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>(
      voxel_space.begin() + y_start, voxel_space.begin() + y_end);
}

World::World() : renderer() {
  get_initial_world_state();
  configure_materials();
  ground = merge_voxels(slice_voxel_space(voxel_space, 0, 1), 0);
  columns = merge_voxels(
      slice_voxel_space(voxel_space, 1, voxel_space.size() - 60), 1);
  ceiling = merge_voxels(slice_voxel_space(voxel_space, voxel_space.size() - 60,
                                           voxel_space.size()),
                         voxel_space.size() - 60);
  renderer.configure_lighting();
}

void World::update(float dt, Camera player_camera) {
  float player_camera_position[3] = {player_camera.position.x,
                                     player_camera.position.y,
                                     player_camera.position.z};
  renderer.update(player_camera_position);
}

Mesh World::merge_voxels(
    // TODO: extract this logic into mesh.h/cpp*/
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &voxel_space,
    int y_offset = 0) {

  const int Y_SIZE = static_cast<int>(voxel_space.size());
  if (Y_SIZE == 0)
    return Mesh{}; // early exit if empty

  const int X_SIZE = voxel_space[0].rows();
  const int Z_SIZE = voxel_space[0].cols();

  std::vector<Vector3> vertices;
  std::vector<Vector3> normals;
  std::vector<unsigned short> indices;
  unsigned short currentIndex = 0;

  auto hasVoxel = [&](int x, int y, int z) -> bool {
    if (x < 0 || x >= X_SIZE || y < 0 || y >= Y_SIZE || z < 0 || z >= Z_SIZE)
      return false;
    return voxel_space[y](x, z);
  };

  for (int x = 0; x < X_SIZE; x++) {
    for (int y = 0; y < Y_SIZE; y++) {
      for (int z = 0; z < Z_SIZE; z++) {
        if (!hasVoxel(x, y, z))
          continue;

        auto addFace = [&](std::initializer_list<Vector3> faceVertices,
                           Vector3 normal) {
          for (const auto &v : faceVertices)
            vertices.push_back(v);
          for (int i = 0; i < 4; i++)
            normals.push_back(normal);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        };

        float xf = static_cast<float>(x);
        float yf = static_cast<float>(y + y_offset); // apply offset here
        float zf = static_cast<float>(z);

        if (!hasVoxel(x, y, z + 1))
          addFace({{xf, yf, zf + 1},
                   {xf + 1, yf, zf + 1},
                   {xf + 1, yf + 1, zf + 1},
                   {xf, yf + 1, zf + 1}},
                  {0, 0, 1});

        if (!hasVoxel(x, y, z - 1))
          addFace({{xf + 1, yf, zf},
                   {xf, yf, zf},
                   {xf, yf + 1, zf},
                   {xf + 1, yf + 1, zf}},
                  {0, 0, -1});

        if (!hasVoxel(x, y + 1, z))
          addFace({{xf, yf + 1, zf},
                   {xf, yf + 1, zf + 1},
                   {xf + 1, yf + 1, zf + 1},
                   {xf + 1, yf + 1, zf}},
                  {0, 1, 0});

        if (!hasVoxel(x, y - 1, z))
          addFace({{xf, yf, zf + 1},
                   {xf, yf, zf},
                   {xf + 1, yf, zf},
                   {xf + 1, yf, zf + 1}},
                  {0, -1, 0});

        if (!hasVoxel(x + 1, y, z))
          addFace({{xf + 1, yf, zf + 1},
                   {xf + 1, yf, zf},
                   {xf + 1, yf + 1, zf},
                   {xf + 1, yf + 1, zf + 1}},
                  {1, 0, 0});

        if (!hasVoxel(x - 1, y, z))
          addFace({{xf, yf, zf},
                   {xf, yf, zf + 1},
                   {xf, yf + 1, zf + 1},
                   {xf, yf + 1, zf}},
                  {-1, 0, 0});
      }
    }
  }

  Mesh merged_mesh = {0};
  merged_mesh.vertexCount = vertices.size();
  merged_mesh.triangleCount = indices.size() / 3;

  merged_mesh.vertices =
      (float *)RL_MALLOC(vertices.size() * 3 * sizeof(float));
  merged_mesh.normals = (float *)RL_MALLOC(normals.size() * 3 * sizeof(float));
  merged_mesh.indices =
      (unsigned short *)RL_MALLOC(indices.size() * sizeof(unsigned short));
  merged_mesh.texcoords =
      (float *)RL_MALLOC(vertices.size() * 2 * sizeof(float));

  for (size_t i = 0; i < vertices.size(); i++) {
    merged_mesh.vertices[i * 3] = vertices[i].x;
    merged_mesh.vertices[i * 3 + 1] = vertices[i].y;
    merged_mesh.vertices[i * 3 + 2] = vertices[i].z;

    merged_mesh.normals[i * 3] = normals[i].x;
    merged_mesh.normals[i * 3 + 1] = normals[i].y;
    merged_mesh.normals[i * 3 + 2] = normals[i].z;

    merged_mesh.texcoords[i * 2] = 0.0f;
    merged_mesh.texcoords[i * 2 + 1] = 0.0f;
  }

  for (size_t i = 0; i < indices.size(); i++) {
    merged_mesh.indices[i] = indices[i];
  }

  UploadMesh(&merged_mesh, false);
  return merged_mesh;
}

void World::draw() {
  // note: shifting mesh calculation here has a huge performance hit,
  // but would be necessary in some way if we want a dynamic game world
  // (minecraft) we can think about this, and relevance of instancing strategy,
  // for 05_explore
  DrawMesh(ground, material_default, MatrixIdentity());
  DrawMesh(columns, material_default, MatrixIdentity());
  DrawMesh(ceiling, material_default, MatrixIdentity());
}
