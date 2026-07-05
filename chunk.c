#include <stdlib.h>
#include "game.h"

static Chunk *map = NULL;

Block *get_block_in_chunk(Chunk *chunk, long x, long y, long z) {
	if (x < 0 || x >= CHUNK_WIDTH) return NULL;
	if (y < 0 || y >= CHUNK_HEIGHT) return NULL;
	if (z < 0 || z >= CHUNK_WIDTH) return NULL;
	return &chunk->cubes[(y*CHUNK_AREA)+(x*CHUNK_WIDTH)+z];
}

static Chunk *generate_chunk(long x, long z) {
    Chunk *chunk = (Chunk*) calloc(1, sizeof(Chunk));
    chunk->flags = CHUNK_DIRTY;
    chunk->x = x;
    chunk->z = z;

    Image noise = GenImagePerlinNoise(CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH * x, CHUNK_WIDTH * z, 0.05f);
    Color *pixels = LoadImageColors(noise);

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_WIDTH; z++) {
	    int height = pixels[z * CHUNK_WIDTH + x].r * 64.0f / 255.0f;
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                BlockType type;
		if (y > height) {
	            type = BLOCK_AIR;
	        } else if (y == height) {
	            type = BLOCK_GRASS;
		} else if (y > 0) {
		     type = BLOCK_DIRT;
		} else {
	             type = BLOCK_BEDROCK;
		}
                Block *block = get_block_in_chunk(chunk, x, y, z);
                block->loc = (Vector3){x*BLOCK_SIZE, y*BLOCK_SIZE, z*BLOCK_SIZE};
                block->loc_cube = (Vector3){x, y, z};
                block->type = type;
                block->collision_box = 
                    (BoundingBox) {
                        (Vector3){block->loc.x-BLOCK_SIZE/2, block->loc.y-BLOCK_SIZE/2, block->loc.z-BLOCK_SIZE/2},
                        (Vector3){block->loc.x+BLOCK_SIZE/2, block->loc.y+BLOCK_SIZE/2, block->loc.z+BLOCK_SIZE/2}
                    }; 
            }
        }
    }
    UnloadImageColors(pixels);
    UnloadImage(noise);
    chunk->next = map;
    map = chunk;
    return chunk;
}

Chunk *raw_get_chunk(long x, long z) {
	for (Chunk *current=map; current; current = current->next) {
		if (current->x == x && current->z == z) {
			return current;
		}
	}
	return NULL;
}

Chunk *get_chunk(long x, long z) {
	Chunk *chunk = raw_get_chunk(x, z);
	if (chunk) return chunk;
	return generate_chunk(x, z);
}

void get_chunk_coord_for_block(long x, long y, long z, long *chunk_x, long *chunk_z) {
	(void)y;
	if (x >= 0) {
		*chunk_x = x / CHUNK_WIDTH;
	} else {
		*chunk_x = (x + CHUNK_WIDTH - 1) / CHUNK_WIDTH;
	}
	(void)y;
	if (z >= 0) {
		*chunk_z = z / CHUNK_WIDTH;
	} else {
		*chunk_z = (z + CHUNK_WIDTH - 1) / CHUNK_WIDTH;
	}
}

Chunk *get_chunk_for_block(long x, long y, long z) {
	long chunk_x, chunk_z;
	get_chunk_coord_for_block(x, y, z, &chunk_x, &chunk_z);
	return get_chunk(chunk_x, chunk_z);
}

void mark_chunk_dirty(Chunk *chunk) {
	chunk->flags |= CHUNK_DIRTY;
}

void mark_block_dirty(long x, long y, long z) {
	(void)y;
	long chunk_x, chunk_z;
	get_chunk_coord_for_block(x, y, z, &chunk_x, &chunk_z);
	for (int x_off=-1; x_off<2; x_off++) {
		for (int z_off=-1; z_off<2; z_off++) {
			Chunk *chunk = raw_get_chunk(chunk_x + x_off, chunk_z + z_off);
			if (chunk) mark_chunk_dirty(chunk);
		}
	}
}
