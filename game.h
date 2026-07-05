#pragma once
#include "raylib.h"
#include "stdint.h"

#define BLOCK_SIZE       2.0f
#define NUM_CHUNKS       6
#define CHUNK_WIDTH      16
#define CHUNK_HEIGHT     128
#define CHUNK_AREA       (CHUNK_WIDTH * CHUNK_WIDTH) 
#define CHUNK_SIZE       (CHUNK_AREA * CHUNK_HEIGHT)
#define FLOOR_HEIGHT 10

typedef enum {
    BLOCK_AIR,
    BLOCK_STONE,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_OAK_PLANK,
    BLOCK_COBBLESTONE,
    BLOCK_SAND,
    BLOCK_GRAVEL,
    BLOCK_OAK_LOG,
    BLOCK_BRICKS,
    BLOCK_GLOWSTONE,
    BLOCK_BEDROCK,
    BLOCK_NONE,
    BLOCK_TYPES_COUNT,
} BlockType;
	
typedef struct {
    Vector3 loc;
    Vector3 loc_cube;
    BoundingBox collision_box;
    float light_level; // 0-15
    BlockType type;
} Block;

typedef struct chunk {
    Block cubes[CHUNK_SIZE];
    long x;
    long z;
    Mesh mesh;
    struct chunk *next;
    uint8_t flags;
} Chunk;

#define CHUNK_DIRTY (1 << 0)

typedef enum {
    FACE_FRONT, FACE_BACK,
    FACE_TOP, FACE_BOTTOM,
    FACE_RIGHT, FACE_LEFT,
    FACE_NONE
} Face;

extern Material block_material;
extern Rectangle texture_uvs[BLOCK_TYPES_COUNT][6];
Block *get_block_in_chunk(Chunk *chunk, long x, long y, long z);
Chunk *get_chunk(long x, long z);
Chunk *raw_get_chunk(long x, long z);
void mark_chunk_dirty(Chunk *chunk);
void mark_block_dirty(long x, long y, long z);
void get_chunk_coord_for_block(long x, long y, long z, long *chunk_x, long *chunk_z);
Chunk *get_chunk_for_block(long x, long y, long z);
void draw_chunk(Chunk *chunk);
Block *place_on_face(Face collision_face, Chunk *chunk, Block *target_block, BlockType type);
