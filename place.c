#include "game.h"
#include <stddef.h>

/* disgusting code in another file so i dont have to look at it
 * it's used for placing blocks and stuff like finding which face is being looked at */

Face get_face_collisions(Ray crosshair_ray, Block *target_block, Camera *camera) {
    RayCollision collides_top, collides_bottom, collides_left, collides_right, collides_front, collides_back;
    collides_top = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                (Vector3) {
                    target_block->collision_box.min.x,
                    target_block->collision_box.max.y - 0.1,
                    target_block->collision_box.min.z,
		    }, target_block->collision_box.max
            });
    collides_bottom = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                target_block->collision_box.min, (Vector3) {
                    target_block->collision_box.max.x,
                    target_block->collision_box.min.y+0.1,
                    target_block->collision_box.max.z,
                }
            });
    collides_back = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                target_block->collision_box.min, (Vector3) {
                    target_block->collision_box.max.x,
                    target_block->collision_box.max.y,
                    target_block->collision_box.min.z+0.1,
                }
            });
    collides_front = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                (Vector3) {
                    target_block->collision_box.min.x,
                    target_block->collision_box.min.y,
                    target_block->collision_box.max.z - 0.1,
                }, target_block->collision_box.max
            });
    collides_left = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                target_block->collision_box.min, (Vector3) {
                    target_block->collision_box.min.x+0.1,
                    target_block->collision_box.max.y,
                    target_block->collision_box.max.z,
                }
            });
    collides_right = GetRayCollisionBox(crosshair_ray,
            (BoundingBox) {
                (Vector3) {
                    target_block->collision_box.max.x - 0.1,
                    target_block->collision_box.min.y,
                    target_block->collision_box.min.z,
                }, target_block->collision_box.max
            });
    RayCollision arr[] = {collides_top, collides_bottom,
                          collides_left, collides_right,
                          collides_front, collides_back};
    float min_dist = -1;
    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) {
        if (arr[i].distance < min_dist || min_dist == -1)
            min_dist = arr[i].distance;
    }
    if      (collides_top.hit    && min_dist == collides_top.distance)    return FACE_TOP;
    else if (collides_bottom.hit && min_dist == collides_bottom.distance) return FACE_BOTTOM;
    else if (collides_left.hit   && min_dist == collides_left.distance)   return FACE_LEFT;
    else if (collides_right.hit  && min_dist == collides_right.distance)  return FACE_RIGHT;
    else if (collides_front.hit  && min_dist == collides_front.distance)  return FACE_FRONT;
    else if (collides_back.hit   && min_dist == collides_back.distance)   return FACE_BACK;
    else return FACE_NONE;
}

Block *place_on_face(Face collision_face, Chunk *chunk, Block *target_block, BlockType type) {
    Block *new_block;
    switch (collision_face) {
    case FACE_TOP:
        new_block = &chunk->cubes[(((int)target_block->loc_cube.y+1)*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.y++;
        new_block->loc.y += BLOCK_SIZE;
        break;
    case FACE_BOTTOM:
        new_block = &chunk->cubes[(((int)target_block->loc_cube.y-1)*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.y--;
        new_block->loc.y -= BLOCK_SIZE;
        break;
    case FACE_FRONT:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z+1];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.z++;
        new_block->loc.z += BLOCK_SIZE;
        break;
    case FACE_BACK:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+((int)target_block->loc_cube.x*CHUNK_WIDTH)+(int)target_block->loc_cube.z-1];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.z--;
        new_block->loc.z -= BLOCK_SIZE;
        break;
    case FACE_RIGHT:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+(((int)target_block->loc_cube.x+1)*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.x++;
        new_block->loc.x += BLOCK_SIZE;
        break;
    case FACE_LEFT:
        new_block = &chunk->cubes[((int)target_block->loc_cube.y*CHUNK_AREA)+(((int)target_block->loc_cube.x-1)*CHUNK_WIDTH)+(int)target_block->loc_cube.z];
        new_block->loc_cube = target_block->loc_cube;
        new_block->loc = target_block->loc;
        new_block->loc_cube.x--;
        new_block->loc.x -= BLOCK_SIZE;
        break;
    default:
        return NULL;
    }
    new_block->type = type;
    new_block->collision_box = 
        (BoundingBox) {
            (Vector3){new_block->loc.x-BLOCK_SIZE/2, new_block->loc.y-BLOCK_SIZE/2, new_block->loc.z-BLOCK_SIZE/2},
            (Vector3){new_block->loc.x+BLOCK_SIZE/2, new_block->loc.y+BLOCK_SIZE/2, new_block->loc.z+BLOCK_SIZE/2}
        };
    return new_block;
}
