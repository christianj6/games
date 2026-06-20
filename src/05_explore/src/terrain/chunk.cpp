#include "chunk.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/graphics/renderer.h"

// Directions for neighbor checking (6 faces)
static const int DIRS[6][3] = {
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
};

// Vertex positions for a unit cube face (+X, -X, +Y, -Y, +Z, -Z)
static const Vector3 FACE_VERTS[6][4] = {
    {{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}}, // +X
    {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 0}}, // -X
    {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}}, // +Y
    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}, // -Y
    {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}, // +Z
    {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}, // -Z
};

inline uint32_t pack(int x, int y, int z) {
  return (x & 0x3FF) | ((y & 0x3FF) << 10) | ((z & 0x3FF) << 20);
}

inline void unpack(uint32_t k, int &x, int &y, int &z) {
  x = k & 0x3FF;
  y = (k >> 10) & 0x3FF;
  z = (k >> 20) & 0x3FF;
}

Material configure_material(Renderer *renderer) {
  Material material_default = LoadMaterialDefault();
  material_default.shader = renderer->get_shader();
  // material_default.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

  return material_default;
}

Chunk::Chunk(Vector2 position, int size, Renderer *renderer)
    : position_(position), size_(size) {
  mesh_ = {0};
  if (renderer == nullptr) {
    static Material default_material = LoadMaterialDefault();
    material_ = default_material;
  } else {
    material_ = configure_material(renderer);
  }
}

Chunk::~Chunk() { unload(); }

bool Chunk::is_in_bounds(int x, int y, int z) const {
  return x >= 0 && y >= 0 && z >= 0 && x < size_ && y < size_ && z < size_;
}

void Chunk::set_voxel(int x, int y, int z) {
  if (is_in_bounds(x, y, z))
    voxels_.insert(pack(x, y, z));
}

void Chunk::clear_voxel(int x, int y, int z) { voxels_.erase(pack(x, y, z)); }

bool Chunk::is_filled(int x, int y, int z) const {
  return voxels_.contains(pack(x, y, z));
}

void Chunk::generate_mesh() {
  if (state != ChunkState::GENERATING)
    return;

  std::vector<Vector3> verts;
  std::vector<Vector3> norms;
  std::vector<Vector2> uvs;
  std::vector<unsigned short> indices;

  unsigned short index = 0;

  for (uint32_t k : voxels_) {
    int x, y, z;
    unpack(k, x, y, z);

    for (int f = 0; f < 6; f++) {
      int nx = x + DIRS[f][0];
      int ny = y + DIRS[f][1];
      int nz = z + DIRS[f][2];

      if (!is_in_bounds(nx, ny, nz) || !is_filled(nx, ny, nz)) {
        // Add 4 vertices
        for (int i = 0; i < 4; i++) {
          Vector3 v = FACE_VERTS[f][i];
          v.x += x;
          v.y += y;
          v.z += z;
          verts.push_back(v);

          Vector3 n = {float(DIRS[f][0]), float(DIRS[f][1]), float(DIRS[f][2])};
          norms.push_back(n);

          Vector2 uv = {(i == 1 || i == 2) ? 1.0f : 0.0f,
                        (i >= 2) ? 1.0f : 0.0f};
          uvs.push_back(uv);
        }

        indices.push_back(index);
        indices.push_back(index + 1);
        indices.push_back(index + 2);
        indices.push_back(index);
        indices.push_back(index + 2);
        indices.push_back(index + 3);
        index += 4;
      }
    }
  }

  // Store mesh data for later upload
  {
    std::lock_guard<std::mutex> lock(mesh_data_mutex_);
    mesh_data_.vertices.resize(verts.size() * 3);
    mesh_data_.normals.resize(norms.size() * 3);
    mesh_data_.texcoords.resize(uvs.size() * 2);
    mesh_data_.indices = indices;

    for (size_t i = 0; i < verts.size(); i++) {
      mesh_data_.vertices[i * 3] = verts[i].x;
      mesh_data_.vertices[i * 3 + 1] = verts[i].y;
      mesh_data_.vertices[i * 3 + 2] = verts[i].z;

      mesh_data_.normals[i * 3] = norms[i].x;
      mesh_data_.normals[i * 3 + 1] = norms[i].y;
      mesh_data_.normals[i * 3 + 2] = norms[i].z;

      mesh_data_.texcoords[i * 2] = uvs[i].x;
      mesh_data_.texcoords[i * 2 + 1] = uvs[i].y;
    }
  }

  state = ChunkState::READY_TO_UPLOAD;
}

void Chunk::upload_mesh() {
  if (state != ChunkState::READY_TO_UPLOAD)
    return;

  {
    std::lock_guard<std::mutex> lock(mesh_data_mutex_);

    mesh_.vertexCount = mesh_data_.vertices.size() / 3;
    mesh_.triangleCount = mesh_data_.indices.size() / 3;

    mesh_.vertices =
        (float *)MemAlloc(mesh_data_.vertices.size() * sizeof(float));
    mesh_.normals =
        (float *)MemAlloc(mesh_data_.normals.size() * sizeof(float));
    mesh_.texcoords =
        (float *)MemAlloc(mesh_data_.texcoords.size() * sizeof(float));
    mesh_.indices = (unsigned short *)MemAlloc(mesh_data_.indices.size() *
                                               sizeof(unsigned short));

    memcpy(mesh_.vertices, mesh_data_.vertices.data(),
           mesh_data_.vertices.size() * sizeof(float));
    memcpy(mesh_.normals, mesh_data_.normals.data(),
           mesh_data_.normals.size() * sizeof(float));
    memcpy(mesh_.texcoords, mesh_data_.texcoords.data(),
           mesh_data_.texcoords.size() * sizeof(float));
    memcpy(mesh_.indices, mesh_data_.indices.data(),
           mesh_data_.indices.size() * sizeof(unsigned short));

    // Clear temporary data
    mesh_data_.vertices.clear();
    mesh_data_.normals.clear();
    mesh_data_.texcoords.clear();
    mesh_data_.indices.clear();
  }

  UploadMesh(&mesh_, false);

  state = ChunkState::LOADED;
  loaded = true;
}

void Chunk::unload() {
  if (state == ChunkState::LOADED) {
    UnloadMesh(mesh_);
    mesh_ = {0}; // Reset mesh structure
  }
  state = ChunkState::UNLOADED;
  loaded = false;
}

void Chunk::draw() {
  DrawMesh(mesh_, material_,
           MatrixTranslate(position_.x * size_, 0, position_.y * size_));
}
