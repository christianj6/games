#include "chunk.h"
#include "raymath.h"
#include "utils/graphics/renderer.h"

namespace {
constexpr float kFloorTopEpsilon = 0.001f;
constexpr float kHalfChunk = 0.5f;
constexpr float kFloorHeight = 1.0f;
constexpr Color kFloorColor{80, 72, 64, 255};
} // namespace

Chunk::Chunk(Vector2 position, int size, Renderer *renderer)
    : position_(position), size_(size) {
  voxels_.assign(size_ * size_ * size_, 0);
  heights_.assign(size_ * size_, 0);
  mesh_ = {0};
  material_ = LoadMaterialDefault();
  if (renderer != nullptr)
    material_.shader = renderer->get_shader();
}

// Releases the GPU mesh if one was uploaded; safe to call twice because
// unload() only frees while state is LOADED.
Chunk::~Chunk() { unload(); }

bool Chunk::is_in_bounds(int x, int y, int z) const {
  return x >= 0 && y >= 0 && z >= 0 && x < size_ && y < size_ && z < size_;
}

void Chunk::set_voxel(int x, int y, int z) {
  if (is_in_bounds(x, y, z)) {
    voxels_[x * size_ * size_ + y * size_ + z] = 1;
    uint8_t &h = heights_[x * size_ + z];
    if (y + 1 > h)
      h = (uint8_t)(y + 1);
  }
}

void Chunk::clear_voxel(int x, int y, int z) {
  if (is_in_bounds(x, y, z)) {
    voxels_[x * size_ * size_ + y * size_ + z] = 0;
    uint8_t &h = heights_[x * size_ + z];
    if ((uint8_t)(y + 1) == h) {
      h = 0;
      for (int yy = size_ - 1; yy >= 0; --yy) {
        if (voxels_[x * size_ * size_ + yy * size_ + z] != 0) {
          h = (uint8_t)(yy + 1);
          break;
        }
      }
    }
  }
}

bool Chunk::is_filled(int x, int y, int z) const {
  if (!is_in_bounds(x, y, z))
    return false;
  return voxels_[x * size_ * size_ + y * size_ + z] != 0;
}

// Build 2D mask of exposed faces for this slice
void Chunk::build_slice_mask(std::vector<bool> &mask, int dim, int u, int v,
                             int side, int slice) const {
  // At a chunk boundary the neighbor is in an adjacent chunk. We
  // treat it as empty (neighbor=false) so the face is generated — but
  // for the world floor (y=0 side faces only) this creates coplanar
  // duplicates with the adjacent chunk's matching face, causing
  // Z-fighting. For X/Z side faces: dim=0 → u=Y → i is Y; dim=2 → v=Y → j
  // is Y.
  bool at_boundary = (slice + side < 0 || slice + side >= size_);
  bool is_xz_face = (dim != 1);

  for (int j = 0; j < size_; j++) {
    for (int i = 0; i < size_; i++) {
      int a[3], b2[3];
      a[dim] = slice;
      a[u] = i;
      a[v] = j;
      b2[dim] = slice + side;
      b2[u] = i;
      b2[v] = j;
      bool current = is_filled(a[0], a[1], a[2]);
      bool neighbor = at_boundary ? false : is_filled(b2[0], b2[1], b2[2]);

      // Suppress side faces of floor voxels (y=0) at chunk boundaries:
      // the adjacent chunk always has a floor there, so these faces are
      // always hidden and only cause Z-fighting.
      int voxel_y = (dim == 0) ? i : (dim == 2) ? j : slice;
      bool floor_seam = at_boundary && is_xz_face && (voxel_y == 0);

      mask[i + j * size_] = current && !neighbor && !floor_seam;
    }
  }
}

// Compute the 4 quad corners in local voxel space.
// Positive face: sits at slice+1, CCW from outside (+side).
// Negative face: sits at slice,   CCW from outside (-side).
// The floor top face (dim=1, side=+1, slice=0) is raised by a
// tiny epsilon so it wins the depth test against coplanar pillar
// base edges that share the exact y=1 boundary.
void Chunk::compute_quad_corners(float p[4][3], int dim, int u, int v, int side,
                                 int slice, int i, int j, int w, int h) {
  float fp = (float)(slice + (side > 0 ? 1 : 0));
  if (dim == 1 && side > 0 && slice == 0)
    fp += kFloorTopEpsilon;
  if (side > 0) {
    p[0][dim] = fp;
    p[0][u] = i;
    p[0][v] = j;
    p[1][dim] = fp;
    p[1][u] = i + w;
    p[1][v] = j;
    p[2][dim] = fp;
    p[2][u] = i + w;
    p[2][v] = j + h;
    p[3][dim] = fp;
    p[3][u] = i;
    p[3][v] = j + h;
  } else {
    p[0][dim] = fp;
    p[0][u] = i;
    p[0][v] = j + h;
    p[1][dim] = fp;
    p[1][u] = i + w;
    p[1][v] = j + h;
    p[2][dim] = fp;
    p[2][u] = i + w;
    p[2][v] = j;
    p[3][dim] = fp;
    p[3][u] = i;
    p[3][v] = j;
  }
}

void Chunk::generate_mesh() {
  if (state != ChunkState::GENERATING)
    return;

  std::vector<float> verts, norms, uvs;
  // Indices are unsigned short: 65535 / 4 caps a chunk at 16384 quads.
  // Zone generation is sparse pillars in open terrain, whose merged face
  // count stays orders of magnitude below that ceiling, so no runtime
  // check is warranted.
  std::vector<unsigned short> indices;
  unsigned short base = 0;

  // Bake world-space offset into vertices so adjacent chunks produce
  // bit-identical positions at shared edges, eliminating seam Z-fighting.
  float wx = position_.x * size_;
  float wz = position_.y * size_;

  // Emit one quad; winding is CCW from the outside for each face direction
  auto emit_quad = [&](float ax, float ay, float az, float bx, float by,
                       float bz, float cx, float cy, float cz, float dx,
                       float dy, float dz, float nx, float ny, float nz) {
    verts.insert(verts.end(), {ax + wx, ay, az + wz, bx + wx, by, bz + wz,
                               cx + wx, cy, cz + wz, dx + wx, dy, dz + wz});
    norms.insert(norms.end(), {nx, ny, nz, nx, ny, nz, nx, ny, nz, nx, ny, nz});
    uvs.insert(uvs.end(), {0, 0, 1, 0, 1, 1, 0, 1});
    indices.insert(indices.end(),
                   {base, (unsigned short)(base + 1),
                    (unsigned short)(base + 2), base,
                    (unsigned short)(base + 2), (unsigned short)(base + 3)});
    base += 4;
  };

  std::vector<bool> mask(size_ * size_);

  // Greedy meshing over all 6 face directions
  for (int dim = 0; dim < 3; dim++) {
    int u = (dim + 1) % 3;
    int v = (dim + 2) % 3;

    for (int side = -1; side <= 1; side += 2) {
      for (int slice = 0; slice < size_; slice++) {

        // Floor top and bottom faces are handled by the dedicated floor mesh.
        if (dim == 1 && slice == 0)
          continue;

        build_slice_mask(mask, dim, u, v, side, slice);

        // Greedy merge and emit quads
        for (int j = 0; j < size_; j++) {
          for (int i = 0; i < size_;) {
            if (!mask[i + j * size_]) {
              i++;
              continue;
            }

            int w = 1;
            while (i + w < size_ && mask[i + w + j * size_])
              w++;

            int h = 1;
            while (j + h < size_) {
              bool row_ok = true;
              for (int k = i; k < i + w; k++)
                if (!mask[k + (j + h) * size_]) {
                  row_ok = false;
                  break;
                }
              if (!row_ok)
                break;
              h++;
            }

            float p[4][3];
            compute_quad_corners(p, dim, u, v, side, slice, i, j, w, h);

            float n[3] = {0, 0, 0};
            n[dim] = (float)side;
            emit_quad(p[0][0], p[0][1], p[0][2], p[1][0], p[1][1], p[1][2],
                      p[2][0], p[2][1], p[2][2], p[3][0], p[3][1], p[3][2],
                      n[0], n[1], n[2]);

            // Clear merged region from mask
            for (int jj = j; jj < j + h; jj++)
              for (int ii = i; ii < i + w; ii++)
                mask[ii + jj * size_] = false;

            i += w;
          }
        }
      }
    }
  }

  {
    std::lock_guard<std::mutex> lock(mesh_data_mutex_);
    mesh_data_.vertices = std::move(verts);
    mesh_data_.normals = std::move(norms);
    mesh_data_.texcoords = std::move(uvs);
    mesh_data_.indices = std::move(indices);
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

    auto upload_channel = [](float *&dst, const std::vector<float> &src) {
      dst = (float *)MemAlloc(src.size() * sizeof(float));
      memcpy(dst, src.data(), src.size() * sizeof(float));
    };
    upload_channel(mesh_.vertices, mesh_data_.vertices);
    upload_channel(mesh_.normals, mesh_data_.normals);
    upload_channel(mesh_.texcoords, mesh_data_.texcoords);
    mesh_.indices = (unsigned short *)MemAlloc(mesh_data_.indices.size() *
                                               sizeof(unsigned short));
    memcpy(mesh_.indices, mesh_data_.indices.data(),
           mesh_data_.indices.size() * sizeof(unsigned short));

    mesh_data_.vertices.clear();
    mesh_data_.normals.clear();
    mesh_data_.texcoords.clear();
    mesh_data_.indices.clear();
  }

  UploadMesh(&mesh_, false);
  state = ChunkState::LOADED;
}

void Chunk::unload() {
  if (state == ChunkState::LOADED) {
    UnloadMesh(mesh_);
    mesh_ = {0};
  }
  state = ChunkState::UNLOADED;
}

void Chunk::draw() {
  float cx = (position_.x + kHalfChunk) * size_;
  float cz = (position_.y + kHalfChunk) * size_;
  DrawPlane({cx, kFloorHeight, cz}, {(float)size_, (float)size_}, kFloorColor);
  DrawMesh(mesh_, material_, MatrixIdentity());
}
