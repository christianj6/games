#include "world.h"
#include "raylib.h"
#include "raymath.h"
#include <Eigen/Dense>
#include <random>

#define MAX_INSTANCES 10000

void World::get_initial_world_state() {
  // initialize 2d eigen array to represent the 3d voxel space
  voxel_space.resize(VOXEL_SIZE * VOXEL_SIZE, VOXEL_SIZE);
  voxel_space.setConstant(false);

  // random number generation
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  // Create a flat plane 10 units below the player's starting position
  const int plane_y =
      VOXEL_SIZE / 2 - 10; // Assuming player starts at VOXEL_SIZE/2
  active_voxel_count = 0;

  // Create the flat plane
  for (int x = VOXEL_SIZE / 4; x < 3 * VOXEL_SIZE / 4; x++) {
    for (int z = VOXEL_SIZE / 4; z < 3 * VOXEL_SIZE / 4; z++) {
      voxel_space(x * VOXEL_SIZE + plane_y, z) = true;
      active_voxel_count++;
    }
  }

  // Create random height columns around the player
  std::uniform_int_distribution<int> height_dist(1, 12);
  for (int x = VOXEL_SIZE / 4; x < 3 * VOXEL_SIZE / 4; x += 5) {
    for (int z = VOXEL_SIZE / 4; z < 3 * VOXEL_SIZE / 4; z += 5) {
      int column_height = height_dist(gen);
      for (int y = plane_y + 1; y < plane_y + 1 + column_height; y++) {
        voxel_space(x * VOXEL_SIZE + y, z) = true;
        active_voxel_count++;
      }
    }
  }

  // allocate transform matrix
  transforms = (Matrix *)RL_CALLOC(active_voxel_count, sizeof(Matrix));

  const float scale = 1.0f; // Size of each voxel
  // TODO: extract or inject from player
  const Vector3 player_start = {-125.0f, 125.0f, -125.0f};
  const Vector3 offset = {player_start.x - (VOXEL_SIZE * scale / 2.0f),
                          player_start.y - (VOXEL_SIZE * scale / 2.0f),
                          player_start.z - (VOXEL_SIZE * scale / 2.0f)};

  // Create a matrix of all positions where voxels are active
  Eigen::MatrixXf positions(4, active_voxel_count);
  int current_voxel = 0;

  for (int x = 0; x < VOXEL_SIZE; x++) {
    for (int y = 0; y < VOXEL_SIZE; y++) {
      for (int z = 0; z < VOXEL_SIZE; z++) {
        if (voxel_space(x * VOXEL_SIZE + y, z)) {
          positions.col(current_voxel) << x * scale + offset.x,
              y * scale + offset.y, z * scale + offset.z, 1.0f;
          current_voxel++;
        }
      }
    }
  }

  // Create transformation matrix (just translation in this case)
  Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();

  // Apply transform to all positions at once
  Eigen::MatrixXf transformed = transform * positions;

  // Convert to array of raylib matrices
  for (int i = 0; i < active_voxel_count; i++) {
    transforms[i] = {
        transform(0, 0), transform(0, 1), transform(0, 2), transformed(0, i),
        transform(1, 0), transform(1, 1), transform(1, 2), transformed(1, i),
        transform(2, 0), transform(2, 1), transform(2, 2), transformed(2, i),
        transform(3, 0), transform(3, 1), transform(3, 2), transformed(3, i)};
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
  std::vector<Vector3> vertices;
  std::vector<Vector3> normals;
  std::vector<unsigned short> indices;
  unsigned short currentIndex = 0;

  // Helper lambda to check if a voxel exists at given coordinates
  auto hasVoxel = [this](int x, int y, int z) -> bool {
    if (x < 0 || y < 0 || z < 0 || x >= VOXEL_SIZE || y >= VOXEL_SIZE ||
        z >= VOXEL_SIZE)
      return false;
    return voxel_space(x * VOXEL_SIZE + y, z);
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
