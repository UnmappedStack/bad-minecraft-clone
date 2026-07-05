#include "raylib.h"
#include "rlgl.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

Face get_face_collisions(Ray crosshair_ray, Block *target_block, Camera *camera);

Rectangle get_texture_rect(Texture2D texture, int x, int y) {
    int cube_sz = texture.width/16;
    return (Rectangle){ cube_sz * x,
                        cube_sz * y,
                        cube_sz,
                        cube_sz
    };
}

// for blocks with all 6 sides with the same texture
void load_basic_block_texture(Texture atlas, int atlas_x, int atlas_y, BlockType type) {
    for (int face = 0; face < 6; face++)
        texture_uvs[type][face] = get_texture_rect(atlas, atlas_x, atlas_y);
}

Material block_material;
Rectangle texture_uvs[BLOCK_TYPES_COUNT][6];

int main(void) {
    const float screen_scale = 1.5f;
    int screen_width = 800 * screen_scale;
    int screen_height = 450 * screen_scale;

    InitWindow(screen_width, screen_height, "game");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, BLOCK_SIZE * 54.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f; 
    camera.projection = CAMERA_PERSPECTIVE;
    Ray crosshair_ray = { 0 };

    Font font = LoadFontEx("Minecraftia-Regular.ttf", 32, 0, 250);
    Texture2D texture   = LoadTexture("atlas.png");
    Texture2D crosshair = LoadTexture("crosshair.png");
    Texture2D hotbar    = LoadTexture("hotbar.png");
    Texture2D selector  = LoadTexture("hotbar_selector.png");
    block_material = LoadMaterialDefault();
    block_material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // textures: stone, dirt, oak planks, cobblestone, sand, gravel, oak logs, bricks, glowstone
    load_basic_block_texture(texture, 3, 0, BLOCK_GRASS);
    load_basic_block_texture(texture, 2, 0, BLOCK_DIRT);
    load_basic_block_texture(texture, 0, 1, BLOCK_COBBLESTONE);
    load_basic_block_texture(texture, 1, 1, BLOCK_BEDROCK);
    load_basic_block_texture(texture, 9, 6, BLOCK_GLOWSTONE);
    load_basic_block_texture(texture, 1, 0, BLOCK_STONE);
    load_basic_block_texture(texture, 4, 0, BLOCK_OAK_PLANK);
    load_basic_block_texture(texture, 2, 1, BLOCK_SAND);
    load_basic_block_texture(texture, 3, 1, BLOCK_GRAVEL);
    load_basic_block_texture(texture, 4, 1, BLOCK_OAK_PLANK);
    load_basic_block_texture(texture, 7, 0, BLOCK_BRICKS);
    
    texture_uvs[BLOCK_GRASS][FACE_TOP] = get_texture_rect(texture, 8, 2);
    texture_uvs[BLOCK_GRASS][FACE_BOTTOM] = get_texture_rect(texture, 2, 0);

    texture_uvs[BLOCK_OAK_LOG][FACE_TOP] = texture_uvs[BLOCK_OAK_LOG][FACE_BOTTOM] = get_texture_rect(texture, 5, 1);
    
    int8_t hotbar_selected = 0; // 0-8
    BlockType hotbar_slots[9] = {
        BLOCK_STONE, BLOCK_DIRT, BLOCK_OAK_PLANK, BLOCK_COBBLESTONE,
        BLOCK_SAND, BLOCK_GRAVEL, BLOCK_OAK_LOG, BLOCK_BRICKS, BLOCK_GLOWSTONE,
    };

    DisableCursor();
    SetTargetFPS(80);

    int blocks_placed = 0;
    int num_faces = CHUNK_SIZE * 6;
    while (!WindowShouldClose()) {
        //UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        UpdateCameraPro(&camera,
            (Vector3){ // wasd
                (IsKeyDown(KEY_W)||IsKeyDown(KEY_UP))*0.1f - IsKeyDown(KEY_S)*0.1f,
                IsKeyDown(KEY_D)*0.1f - IsKeyDown(KEY_A)*0.1f,
                IsKeyDown(KEY_SPACE)*0.3f - IsKeyDown(KEY_LEFT_SHIFT)*0.3f
            },
            (Vector3){ // mouse movement
                GetMouseDelta().x*0.1f,
                GetMouseDelta().y*0.1f,
                0.0f
            }, 0.0f
        );
        hotbar_selected += GetMouseWheelMove();
        for (int i = 0; i < 9; i++) {
            if (IsKeyPressed(KEY_ONE + i))
                hotbar_selected = i;
        }
        if (hotbar_selected > 8)
            hotbar_selected = hotbar_selected % 9;
        if (hotbar_selected < 0)
            hotbar_selected = 9-(-hotbar_selected % 9);
        BeginDrawing();
        ClearBackground((Color){0x78, 0xA7, 0xFF});
        BeginMode3D(camera);

        screen_height = GetScreenHeight();
        screen_width = GetScreenWidth();
        crosshair_ray = GetScreenToWorldRay((Vector2){screen_width/2, screen_height/2}, camera);
        
        int rendered = 0;
        Block *target_block = NULL;
        float target_block_dist = 0;
	// TODO : draw chunks around player
	long block_x = camera.position.x / BLOCK_SIZE;
	long block_y = camera.position.y / BLOCK_SIZE;
	long block_z = camera.position.z / BLOCK_SIZE;
	long chunk_x, chunk_z;
	get_chunk_coord_for_block(block_x, block_y, block_z, &chunk_x, &chunk_z);
	int render_distance = 7;
	for (int x=-render_distance; x<=render_distance; x++) {
		for (int z=-render_distance; z<=render_distance; z++) {
			Chunk *chunk = get_chunk(chunk_x + x, chunk_z + z);
			draw_chunk(chunk);
		}
	}
        if (target_block != NULL && target_block_dist <= 5 * BLOCK_SIZE) {
            DrawCubeWires(target_block->loc, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, WHITE);     
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                num_faces -= 6;
                target_block->type = BLOCK_AIR;
		// TODO : mark dirty
            }

            /*if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && hotbar_slots[hotbar_selected] != BLOCK_NONE) {
                // point a ray at each face, check collision, and place on the face it collides with
                // which is closest to the camera
                Face collision_face = get_face_collisions(crosshair_ray, target_block, &camera);
                Block *block_placed = place_on_face(collision_face, chunk, target_block, hotbar_slots[hotbar_selected]);
                if (block_placed != NULL) {
                    block_placed->light_level = blocks_placed%5;
                    printf("light level placed: %.1f\n", block_placed->light_level);
                }
                blocks_placed++;
                num_faces += 6;
            }*/
        }

        EndMode3D();
        const char *text = TextFormat("%i/%i faces rendered (%.1f%)\n"
                                      "FPS: %i\n"
                                      "XYZ: %.0f,%.0f,%.0f\n",
                                        rendered, num_faces, ((float)rendered)/((float)num_faces)*100, GetFPS(),
                                        camera.position.x/2, camera.position.y/2, camera.position.z/2);

        DrawTextEx(font, text, (Vector2){9, 11}, 24, 1, GRAY);
        DrawTextEx(font, text, (Vector2){8, 12}, 24, 1, GRAY);
        DrawTextEx(font, text, (Vector2){10, 10},  24, 1, RAYWHITE);
       
        // crosshair
        DrawTextureEx(crosshair,
                (Vector2){screen_width/2-crosshair.width*0.05/2,
                          screen_height/2-crosshair.height*0.05/2},
                0, 0.05, GRAY);
        
        DrawTextureEx(hotbar, 
                (Vector2){screen_width/2-hotbar.width*1.5f/2,
                          screen_height-hotbar.height*1.5f-10},
                0, 1.5f, WHITE);
        DrawTextureEx(selector, 
                (Vector2){screen_width/2-hotbar.width*1.5f/2 + hotbar_selected * selector.width*2.48f - 1,
                          screen_height-hotbar.height*1.5f-11},
                0, 3, WHITE);
        for (int i = 0; i < 9; i++) {
            if (hotbar_slots[i] == BLOCK_NONE) continue;
            Rectangle dest_rec = {screen_width/2-hotbar.width*1.5f/2 + i * selector.width*2.5f - 1 + 18, // x
                                 screen_height-hotbar.height*1.5f-11 + 18, // y
                                 35, 35};
            DrawTexturePro(texture, texture_uvs[hotbar_slots[i]][FACE_FRONT], dest_rec, (Vector2){0, 0}, 0.0f, WHITE);
        }
        DrawBillboard(camera, hotbar, (Vector3) {0,CHUNK_HEIGHT*2+5,0}, 1, WHITE); // Draw a billboard texture
        EndDrawing();
    }

    UnloadTexture(texture);
    UnloadTexture(crosshair);
    UnloadTexture(hotbar);
    UnloadTexture(selector);
    UnloadFont(font);
    CloseWindow();
    return 0;
}
