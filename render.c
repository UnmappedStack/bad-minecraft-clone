#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "game.h"

static Block *get_block_in_chunks(Chunk *chunks[3][3], long x, long y, long z) {
	long chunk_x = 1;
	long chunk_z = 1;

	// check chuncks around
	if (x < 0) {
		x += CHUNK_WIDTH;
		chunk_x--;
	} else if (x >= CHUNK_WIDTH) {
		x -= CHUNK_WIDTH;
		chunk_x++;
	}
	if (z < 0) {
		z += CHUNK_WIDTH;
		chunk_z--;
	} else if (z >= CHUNK_WIDTH) {
		z -= CHUNK_WIDTH;
		chunk_z++;
	}

	return get_block_in_chunk(chunks[chunk_x][chunk_z], x, y, z);
}

static bool check_air(Chunk *chunks[3][3], long x, long y, long z) {
	Block *block = get_block_in_chunks(chunks, x, y, z);
	if (!block) {
		// no block so it's air
		return true;
	}
	return block->type == BLOCK_AIR;
}

static float ambiant_occlusion(Chunk *chunks[3][3], Vector3 *vertex, Vector3 *normal, long x, long y, long z) {
	float value = 0;

	// corner
	long cor_x = vertex->x == 0.0f ? x - 1 : x + 1;
	long cor_y = vertex->y == 0.0f ? y - 1 : y + 1;
	long cor_z = vertex->z == 0.0f ? z - 1 : z + 1;
	if (!check_air(chunks, cor_x, cor_y, cor_z)) {
		value += 1.0f / 4.0f;
	}

	// sides
	if (normal->x == 0.0f) {
		if (!check_air(chunks, x, cor_y, cor_z)) {
			value += 1.0f / 4.0f;
		}
	}
	if (normal->y == 0.0f) {
		if (!check_air(chunks, cor_x, y, cor_z)) {
			value += 1.0f / 4.0f;
		}
	}
	if (normal->z == 0.0f) {
		if (!check_air(chunks, cor_x, cor_y, z)) {
			value += 1.0f / 4.0f;
		}
	}
	return value;
}

static void add_face(Mesh *mesh, Chunk *chunks[3][3], Face face, Vector3 pos, long x, long y, long z, Vector2 *uv1, Vector2 *uv2) {
	Vector3 normal;
	Vector3 v[4];
	switch (face) {
	case FACE_LEFT:
		normal = (Vector3){-1.0f, 0.0f, 0.0f};
		v[0] = (Vector3){0.0f, 0.0f, 0.0f};
		v[1] = (Vector3){0.0f, 0.0f, 1.0f};
		v[2] = (Vector3){0.0f, 1.0f, 1.0f};
		v[3] = (Vector3){0.0f, 1.0f, 0.0f};
		break;
	case FACE_RIGHT:
		normal = (Vector3){ 1.0f, 0.0f, 0.0f};
		v[0] = (Vector3){1.0f, 0.0f, 1.0f};
		v[1] = (Vector3){1.0f, 0.0f, 0.0f};
		v[2] = (Vector3){1.0f, 1.0f, 0.0f};
		v[3] = (Vector3){1.0f, 1.0f, 1.0f};
		break;
	case FACE_BOTTOM:
		normal = (Vector3){0.0f, -1.0f, 0.0f};
		v[0] = (Vector3){0.0f, 0.0f, 1.0f};
		v[1] = (Vector3){0.0f, 0.0f, 0.0f};
		v[2] = (Vector3){1.0f, 0.0f, 0.0f};
		v[3] = (Vector3){1.0f, 0.0f, 1.0f};
		break;
	case FACE_TOP:
		normal = (Vector3){0.0f,  1.0f, 0.0f};
		v[0] = (Vector3){0.0f, 1.0f, 0.0f};
		v[1] = (Vector3){0.0f, 1.0f, 1.0f};
		v[2] = (Vector3){1.0f, 1.0f, 1.0f};
		v[3] = (Vector3){1.0f, 1.0f, 0.0f};
		break;
	case FACE_BACK:
		normal = (Vector3){0.0f, 0.0f, -1.0f};
		v[0] = (Vector3){1.0f, 0.0f, 0.0f};
		v[1] = (Vector3){0.0f, 0.0f, 0.0f};
		v[2] = (Vector3){0.0f, 1.0f, 0.0f};
		v[3] = (Vector3){1.0f, 1.0f, 0.0f};
		break;
	case FACE_FRONT:
		normal = (Vector3){0.0f, 0.0f,  1.0f};
		v[0] = (Vector3){0.0f, 0.0f, 1.0f};
		v[1] = (Vector3){1.0f, 0.0f, 1.0f};
		v[2] = (Vector3){1.0f, 1.0f, 1.0f};
		v[3] = (Vector3){0.0f, 1.0f, 1.0f};
		break;
	case FACE_NONE:
		return;
	}

	Vector2 uvs[4] = {
		(Vector2){uv1->x, uv2->y},
		(Vector2){uv2->x, uv2->y},
		(Vector2){uv2->x, uv1->y},
		(Vector2){uv1->x, uv1->y},
	};

	size_t v_index = mesh->vertexCount;
	size_t t_index = mesh->triangleCount;
	mesh->vertexCount += 4;
	mesh->triangleCount += 2;
	mesh->vertices  = realloc(mesh->vertices , mesh->vertexCount * 3 * sizeof(float));
	mesh->normals   = realloc(mesh->normals  , mesh->vertexCount * 3 * sizeof(float));
	mesh->texcoords = realloc(mesh->texcoords, mesh->vertexCount * 2 * sizeof(float));
	mesh->colors    = realloc(mesh->colors   , mesh->vertexCount * 4 * sizeof(float));
	mesh->indices = realloc(mesh->indices, mesh->triangleCount * 3 * sizeof(unsigned short));

	// add vertices
	for (int i=0; i<4; i++) {
		float occlusion = ambiant_occlusion(chunks, &v[i], &normal, x, y, z);
		mesh->vertices[(v_index + i) * 3 + 0] = v[i].x * BLOCK_SIZE + pos.x;
		mesh->vertices[(v_index + i) * 3 + 1] = v[i].y * BLOCK_SIZE + pos.y;
		mesh->vertices[(v_index + i) * 3 + 2] = v[i].z * BLOCK_SIZE + pos.z;

		mesh->normals[(v_index + i) * 3 + 0] = normal.x;
		mesh->normals[(v_index + i) * 3 + 1] = normal.y;
		mesh->normals[(v_index + i) * 3 + 2] = normal.z;

		mesh->texcoords[(v_index + i) * 2 + 0] = uvs[i].x;
		mesh->texcoords[(v_index + i) * 2 + 1] = uvs[i].y;

		mesh->colors[(v_index + i) * 4 + 0] = 255.0f * (1.0f - occlusion);
		mesh->colors[(v_index + i) * 4 + 1] = 255.0f * (1.0f - occlusion);
		mesh->colors[(v_index + i) * 4 + 2] = 255.0f * (1.0f - occlusion);
		mesh->colors[(v_index + i) * 4 + 3] = 255;
	}

	mesh->indices[t_index * 3 + 0] = v_index + 0;
	mesh->indices[t_index * 3 + 1] = v_index + 1;
	mesh->indices[t_index * 3 + 2] = v_index + 2;
	mesh->indices[t_index * 3 + 3] = v_index + 0;
	mesh->indices[t_index * 3 + 4] = v_index + 2;
	mesh->indices[t_index * 3 + 5] = v_index + 3;
}

static void generate_block_mesh(Chunk *chunk, Chunk *chunks[3][3], Block *block, long x, long y, long z){
    if (block->type == BLOCK_AIR) return;
    Texture2D *texture = &block_material.maps[MATERIAL_MAP_DIFFUSE].texture;
    float tex_width = (float)texture->width;
    float tex_height = (float)texture->height;

    bool sides_air[6];
    sides_air[FACE_LEFT]    = check_air(chunks, x-1, y, z);
    sides_air[FACE_RIGHT]   = check_air(chunks, x+1, y, z);
    sides_air[FACE_BOTTOM]  = check_air(chunks, x, y-1, z);
    sides_air[FACE_TOP]     = check_air(chunks, x, y+1, z);
    sides_air[FACE_BACK]    = check_air(chunks, x, y, z-1);
    sides_air[FACE_FRONT]   = check_air(chunks, x, y, z+1);

    Vector3 position = {x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE};

    for (int i=0; i<6; i++) {
    	if (sides_air[i]) {
	    // We calculate the normalized texture coordinates for the desired texture-source-rectangle
	    // It means converting from (tex.width, tex.height) coordinates to [0.0f, 1.0f] equivalent
	    Vector2 uv1 = {
		texture_uvs[block->type][i].x / tex_width,
	        texture_uvs[block->type][i].y / tex_height,
	    };
	    Vector2 uv2 = {
		(texture_uvs[block->type][i].x + texture_uvs[block->type][i].width) / tex_width,
	        (texture_uvs[block->type][i].y + texture_uvs[block->type][i].height) / tex_height,
	    };
	    add_face(&chunk->mesh, chunks, i, position, x, y, z, &uv1, &uv2);
	}
    }
}

static void generate_chunk_mesh(Chunk *chunk) {
	printf("regenerate mesh for chunk %ld %ld\n", chunk->x, chunk->z);
	UnloadMesh(chunk->mesh);
	chunk->mesh.vertexCount = 0;
	chunk->mesh.triangleCount = 0;

	// get chunk on the sides
	Chunk *chunks[3][3];
	for (int x=0; x<3; x++) {
		for (int z=0; z<3; z++) {
			chunks[x][z] = get_chunk(chunk->x + x - 1, chunk->z + z - 1);
		}
	}

	for (int z=0; z<CHUNK_WIDTH; z++) {
		for (int x=0; x<CHUNK_WIDTH; x++) {
			for (int y=0; y<CHUNK_HEIGHT; y++) {
				Block *block = get_block_in_chunk(chunk, x, y, z);
				generate_block_mesh(chunk, chunks, block, x, y, z);
			}
		}
	}
	UploadMesh(&chunk->mesh, false);
	chunk->flags &= ~CHUNK_DIRTY;
	return;
}

static Mesh *get_chunk_mesh(Chunk *chunk) {
	if (chunk->flags & CHUNK_DIRTY) {
		generate_chunk_mesh(chunk);
	}
	return &chunk->mesh;
}

void draw_chunk(Chunk *chunk) {
    Mesh *mesh = get_chunk_mesh(chunk);
    // TODO : apply position ?
    Matrix transform = MatrixTranslate(
	chunk->x * CHUNK_WIDTH * BLOCK_SIZE,
	0.0f,
	chunk->z * CHUNK_WIDTH * BLOCK_SIZE
    );
    DrawMesh(*mesh, block_material, transform);
}
