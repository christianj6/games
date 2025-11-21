#include "chunk.h"
#include "raymath.h"

// Directions for neighbor checking (6 faces)
static const int DIRS[6][3] = {
    { 1, 0, 0 }, {-1, 0, 0 },
    { 0, 1, 0 }, { 0,-1, 0 },
    { 0, 0, 1 }, { 0, 0,-1 },
};

// Vertex positions for a unit cube face (+X, -X, +Y, -Y, +Z, -Z)
static const Vector3 FACE_VERTS[6][4] = {
    {{1,0,0},{1,1,0},{1,1,1},{1,0,1}}, // +X
    {{0,0,1},{0,1,1},{0,1,0},{0,0,0}}, // -X
    {{0,1,1},{1,1,1},{1,1,0},{0,1,0}}, // +Y
    {{0,0,0},{1,0,0},{1,0,1},{0,0,1}}, // -Y
    {{0,0,1},{1,0,1},{1,1,1},{0,1,1}}, // +Z
    {{0,1,0},{1,1,0},{1,0,0},{0,0,0}}, // -Z
};

inline uint32_t pack(int x, int y, int z) {
    return (x & 0x3FF) | ((y & 0x3FF) << 10) | ((z & 0x3FF) << 20);
}

inline void unpack(uint32_t k, int &x, int &y, int &z) {
    x =  k        & 0x3FF;
    y = (k >> 10) & 0x3FF;
    z = (k >> 20) & 0x3FF;
}

Chunk::Chunk(Vector2 position, int size) : position_(position), size_(size) {
  mesh_ = {0};
}

Chunk::~Chunk() {
  unload();
}

bool Chunk::is_in_bounds(int x, int y, int z) const {
    return x >= 0 && y >= 0 && z >= 0 && x < size_ && y < size_ && z < size_;
}

void Chunk::set_voxel(int x, int y, int z) {
  if (is_in_bounds(x,y,z)) voxels_.insert(pack(x,y,z));
}

void Chunk::clear_voxel(int x, int y, int z) {
  voxels_.erase(pack(x,y,z));
}

bool Chunk::is_filled(int x, int y, int z) const {
  return voxels_.contains(pack(x,y,z));
}

void Chunk::load() {
    if (loaded) return;

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

                    Vector3 n = { float(DIRS[f][0]), float(DIRS[f][1]), float(DIRS[f][2]) };
                    norms.push_back(n);

                    Vector2 uv = { (i==1||i==2)?1.0f:0.0f, (i>=2)?1.0f:0.0f };
                    uvs.push_back(uv);
                }

                indices.push_back(index); indices.push_back(index+1); indices.push_back(index+2);
                indices.push_back(index); indices.push_back(index+2); indices.push_back(index+3);
                index += 4;
            }
        }
    }

    mesh_.vertexCount = verts.size();
    mesh_.triangleCount = indices.size()/3;

    mesh_.vertices = (float*)MemAlloc(mesh_.vertexCount * 3 * sizeof(float));
    mesh_.normals  = (float*)MemAlloc(mesh_.vertexCount * 3 * sizeof(float));
    mesh_.texcoords= (float*)MemAlloc(mesh_.vertexCount * 2 * sizeof(float));
    mesh_.indices  = (unsigned short*)MemAlloc(indices.size() * sizeof(unsigned short));

    memcpy(mesh_.vertices, verts.data(), verts.size()*3*sizeof(float));
    memcpy(mesh_.normals, norms.data(), norms.size()*3*sizeof(float));
    memcpy(mesh_.texcoords, uvs.data(), uvs.size()*2*sizeof(float));
    memcpy(mesh_.indices, indices.data(), indices.size()*sizeof(unsigned short));

    UploadMesh(&mesh_, false);
    loaded = true;
}

void Chunk::unload() {
    if (!loaded) return;
    UnloadMesh(mesh_);
    loaded = false;
}

void Chunk::draw() {
    DrawMesh(mesh_, LoadMaterialDefault(), MatrixTranslate(position_.x*size_,0,position_.y*size_));
}

