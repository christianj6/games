#include "world.h"
#include "fmt/core.h"
#include "raymath.h"
#include <random>

void World::get_initial_world_state() {
  // TODO: try to create this elsewhere
  voxel_space.resize(
      VOXEL_SIZE, Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>::Constant(
                      VOXEL_SIZE, VOXEL_SIZE, false));

  // TODO: extract number generation
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  std::uniform_int_distribution<int> height_dist(1, 12);

  const int plane_y = 0;
  int active_voxel_count = 0;

  for (int i = 0; i < VOXEL_SIZE; i++) {
    for (int j = 0; j < VOXEL_SIZE; j++) {
      // create the flat plane
      voxel_space[plane_y](i, j) = true;
      active_voxel_count++;

      // randomly create columns
      if (dist(gen) < VOXEL_DENSITY) {
        for (int k = 0; k < height_dist(gen); k++) {
          voxel_space[plane_y + k](i, j) = true;
          active_voxel_count++;
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

World::World() : renderer() {
  get_initial_world_state();
  configure_materials();
  merge_voxels();
  renderer.configure_lighting();
}

void World::update(float dt, Camera player_camera) {
  float player_camera_position[3] = {player_camera.position.x,
                                     player_camera.position.y,
                                     player_camera.position.z};
  renderer.update(player_camera_position);
}

void World::merge_voxels() {
  // TODO: try to fix issue where mesh size cannot grow too large
  // TODO: if increasing plane size or n columns, mesh cuts off in render
  // TODO: extract this logic into mesh.h/cpp
  std::vector<Vector3> vertices;
  std::vector<Vector3> normals;
  std::vector<unsigned short> indices;
  unsigned short currentIndex = 0;

  // Helper lambda to check if a voxel exists at given coordinates
  auto hasVoxel = [this](int x, int y, int z) -> bool {
    if (x < 0 || x >= VOXEL_SIZE || y < 0 || y >= VOXEL_SIZE || z < 0 ||
        z >= VOXEL_SIZE)
      return false;
    return voxel_space[y](x, z);
  };

  // For each voxel position
  for (int x = 0; x < VOXEL_SIZE; x++) {
    for (int y = 0; y < VOXEL_SIZE; y++) {
      for (int z = 0; z < VOXEL_SIZE; z++) {
        if (!hasVoxel(x, y, z))
          continue;

        // Check each face and only add if it's exposed
        // Front face
        if (!hasVoxel(x, y, z + 1)) {
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 1)});
          for (int i = 0; i < 4; i++)
            normals.push_back({0, 0, 1});
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        }

        // Back face
        if (!hasVoxel(x, y, z - 1)) {
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 0)});
          for (int i = 0; i < 4; i++)
            normals.push_back({0, 0, -1});
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        }

        // Top face
        if (!hasVoxel(x, y + 1, z)) {
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 0)});
          for (int i = 0; i < 4; i++)
            normals.push_back({0, 1, 0});
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        }

        // Bottom face
        if (!hasVoxel(x, y - 1, z)) {
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 1)});
          for (int i = 0; i < 4; i++)
            normals.push_back({0, -1, 0});
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        }

        // Right face
        if (!hasVoxel(x + 1, y, z)) {
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 1),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 1)});
          for (int i = 0; i < 4; i++)
            normals.push_back({1, 0, 0});
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        }

        // Left face
        if (!hasVoxel(x - 1, y, z)) {
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 0)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 0),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 1)});
          vertices.push_back({static_cast<float>(x + 0),
                              static_cast<float>(y + 1),
                              static_cast<float>(z + 0)});
          for (int i = 0; i < 4; i++)
            normals.push_back({-1, 0, 0});
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 1);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex);
          indices.push_back(currentIndex + 2);
          indices.push_back(currentIndex + 3);
          currentIndex += 4;
        }
      }
    }
  }

  // Create the mesh
  merged_mesh = {0};
  merged_mesh.vertexCount = vertices.size();
  merged_mesh.triangleCount = indices.size() / 3;
  merged_mesh.vertices =
      (float *)RL_MALLOC(vertices.size() * 3 * sizeof(float));
  merged_mesh.normals = (float *)RL_MALLOC(normals.size() * 3 * sizeof(float));
  merged_mesh.indices =
      (unsigned short *)RL_MALLOC(indices.size() * sizeof(unsigned short));

  // Add texture coordinates
  merged_mesh.texcoords =
      (float *)RL_MALLOC(vertices.size() * 2 * sizeof(float));
  for (size_t i = 0; i < vertices.size(); i++) {
    merged_mesh.texcoords[i * 2] = 0.0f;
    merged_mesh.texcoords[i * 2 + 1] = 0.0f;
  }

  // Copy vertex data
  for (size_t i = 0; i < vertices.size(); i++) {
    merged_mesh.vertices[i * 3] = vertices[i].x;
    merged_mesh.vertices[i * 3 + 1] = vertices[i].y;
    merged_mesh.vertices[i * 3 + 2] = vertices[i].z;

    merged_mesh.normals[i * 3] = normals[i].x;
    merged_mesh.normals[i * 3 + 1] = normals[i].y;
    merged_mesh.normals[i * 3 + 2] = normals[i].z;
  }

  // Copy indices
  for (size_t i = 0; i < indices.size(); i++) {
    merged_mesh.indices[i] = indices[i];
  }

  UploadMesh(&merged_mesh, false);
}

void World::draw() {
  DrawMesh(merged_mesh, material_default, MatrixIdentity());
}
