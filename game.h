#pragma once
#include "raylib.h"
#include "list.h"
#include <stdint.h>

#define BLOCK_SIZE       2.0f
#define NUM_CHUNKS       6
#define CHUNK_WIDTH      16*NUM_CHUNKS
#define CHUNK_HEIGHT     10
#define MAX_BUILD_HEIGHT 40
#define CHUNK_AREA       (CHUNK_WIDTH * CHUNK_WIDTH) 
#define CHUNK_SIZE       (CHUNK_AREA * CHUNK_HEIGHT)
#define MAX_CHUNK_SIZE   (CHUNK_AREA * MAX_BUILD_HEIGHT)

typedef struct {
    Vector3 loc;
    Vector3 loc_cube;
    Rectangle *texture;
    BoundingBox collision_box;
    bool not_air;
    uint8_t light_levels[6]; // 0-4, for faces
    uint8_t internal_light_level; // 0-4, for the block overall (for air blocks, mainly)
    bool light_emitting;
} Block;

typedef struct {
    struct list list;
    Block *block;
} BlockList;

typedef struct {
    Block cubes[MAX_CHUNK_SIZE];
} Chunk;

typedef enum {
    FACE_FRONT, FACE_BACK,
    FACE_TOP, FACE_BOTTOM,
    FACE_RIGHT, FACE_LEFT,
    FACE_NONE
} Faces;
