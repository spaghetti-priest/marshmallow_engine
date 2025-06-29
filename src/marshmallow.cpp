// #define TINYGLTF_IMPLEMENTATION
// #define TINYGLTF_NO_STB_IMAGE

//#ifdef (WIN32)
//#define STBI_WINDOWS_UTF8
//#endif

// @Temporary: Should this be moved to marshmallow_gl
// #include "tinygltf/tiny_gltf.h"

#include "marshmallow.h"
#include "marshmallow_draw.h"
#include "marshmallow_draw.cpp"
#include "marshmallow_tile.h"
#include "marshmallow_tile.cpp"
#include "marshmallow_asset.h"
#include "marshmallow_asset.cpp"

typedef struct Player {
   V2    pos;
   Tile  tile;
   Tile  *next;
} Player;

typedef struct World {
   Tilemap  *tilemap;
   Player   *player;
} World;

typedef struct GameState {
   Arena arena; // @Cleanup: Might change this into arena
   World       *world;
   void        *thing1;
   GameModes   game_modes;
} GameState;

// @Cleanup: Make this into an enum or table
V4 RED      = v4(1.0f, 0.0f, 0.0f, 1.0f);
V4 GREEN    = v4(0.0f, 1.0f, 0.0f, 1.0f);
V4 BLUE     = v4(0.0f, 0.0f, 1.0f, 1.0f);
V4 YELLOW   = v4(1.0f, 1.0f, 0.0f, 1.0f);
V4 WHITE    = v4(1.0f, 1.0f, 1.0f, 1.0f);
V4 GRAY     = v4(0.5f, 0.5f, 0.5f, 0.5f);

global_variable V4 player_tile_color       = v4(1.0f, 1.0f, 0.0f, 1.0f);
global_variable V4 traversable_tile_color  = v4(1.0f, 0.0f, 0.5f, 0.0f);
global_variable V4 boundry_tile_color      = v4(0.7f, 0.7f, 0.7f, 1.0f);
global_variable V4 click_tile_color        = v4(0.0f, 1.0f, 0.0f, 1.0f);
global_variable V4 highlight_tile_color    = v4(1.0f, 1.0f, 1.0f, 1.0f);

std::queue<Tile> check_path_queue;
std::queue<Tile> display_path_queue;
GameModes modes;

void
game_update_and_render (GameMemory *memory, GameInput *input, GameBackbuffer *backbuffer)
{
   GameState *gamestate = (GameState*)memory->permanent_storage;
   assert(sizeof(gamestate) <= memory->permanent_storage_size);

   if (!memory->is_initialized) {
      // If in editor mode the user creates an tilegrid. We should start from the mouse position
      // and then the tile count should be dictated by the distance from the initial mouse position
      // and the end mouse position
      int start_y = 0;
      int start_x = 0;
      u32 tile_width = 1;
      u32 tile_height = 1;
      // @Hardocded: This will be hardcoded to 32 x 32. In the software renderer the whole thing bricks
      u32 tile_count_x = 32;
      u32 tile_count_y = 32;
      arena_alloc(&gamestate->arena,
                  memory->permanent_storage_size - sizeof(GameState),
                  (u8*)memory->permanent_storage + sizeof(GameState));

      gamestate->world        = push_struct(&gamestate->arena, World);
      World *world            = gamestate->world;
      world->player           = push_struct(&gamestate->arena, Player);
      Tilemap *tilemap        = initialize_tilemap(&gamestate->arena,
                                                   tile_width, tile_height,
                                                   tile_count_x, tile_count_y,
                                                   start_x, start_y,
                                                   traversable_tile_color);
      world->tilemap          = tilemap;

      Player *player          = world->player;
      player->tile.grid_pos   = {5, 3};
      player->tile.color      = player_tile_color;
      player->tile.tilevalue  = 2;

      // bool s = asset_load_gltf_model("C:\\marshmallow\\extern\\tinygltf\\models\\Cube\\Cube.gltf");
      //Model cube_gltf{};
      bool s = asset_load_gltf_model(&gamestate->arena, &cube_gltf, "C:\\marshmallow\\extern\\tinygltf\\models\\Cube\\Cube.gltf");

      memory->is_initialized  = true;
      debug_push_tilemap_instanced(backbuffer, tilemap);
   }

   // @Todo: Create a function that draws a rectangle outline
   RectS32 rect1 = {v2s(0, 0),     v2s(1280, 20)}; // TOP
   RectS32 rect2 = {v2s(0, 0),     v2s(15, 700)}; // LEFT
   RectS32 rect3 = {v2s(1265, 0),  v2s(1280, 715)}; // RIGHT
   RectS32 rect4 = {v2s(0, 700),   v2s(1280, 720)}; // BOTOMM

   if (input->editor_button_pressed) {
      modes = EDITOR_MODE;
      debug_draw_rect(backbuffer, rect1.min , rect1.max, 0, 0, 1.0);
      debug_draw_rect(backbuffer, rect2.min , rect2.max, 0, 0, 1.0);
      debug_draw_rect(backbuffer, rect3.min , rect3.max, 0, 0, 1.0);
      debug_draw_rect(backbuffer, rect4.min , rect4.max, 0, 0, 1.0);
   } else {
      modes = RUNTIME_MODE;
   }

   World   *world       = gamestate->world;
   Tilemap *tilemap     = world->tilemap;
   Player  *player      = world->player;

   Tile dest_tile       = {};
   dest_tile.grid_pos   = {2, 6};
   dest_tile.color      = v4(0.2f, 0.5f, 0.5f, 0.0f);
   dest_tile.tilevalue  = 1;

   // @Speed: We loop through the tilemap three times
   clear_tilemap(backbuffer, tilemap, traversable_tile_color);

   // set_tilevalue(tilemap, 7,  4 ,0,  boundry_tile_color);
   // set_tilevalue(tilemap, 5,  4, 0,  boundry_tile_color);
   update_tile(tilemap, dest_tile.grid_pos, dest_tile.color);

// const u32 cube_instance_count = 1;
// V3S cube_positions[cube_instance_count] = {v3s(8.0f, 2.0f, -4.0f)};//,
//                                            // v3s(4.0f, 2.2f, -4.5f),
//                                            // v3s(12.0f, 2.2f, -4.5f)};

   // draw_cube(cube_positions[0], v3(1.0, 1.0, 1.0f), BLUE);

   // draw_cube_mesh_instanced(positions, scale, colors);
   // @Speed: We loop through the tilemap three times
   std::queue<Tile> check_path_queue;
   std::queue<Tile> display_path_queue;
   const u32 NEIGHBOR_COUNT = 8;

/*****************************************************
*  Process Inputs
*****************************************************/
#if 0
   for (int i = 0; i <= tilemap->tile_count_y * tilemap->tile_count_x; ++i) {
      Tile mouse_tile = tilemap->tiles[i];

      if ((input->mouse_state.pos.x >= mouse_tile.min.x && input->mouse_state.pos.x <= mouse_tile.max.x)
      &&  (input->mouse_state.pos.y >= mouse_tile.min.y && input->mouse_state.pos.y <= mouse_tile.max.y)) {
         s32 chebshev = chebyshev_distance(player->tile.grid_pos, mouse_tile.grid_pos);

         // // @TODO (low priority): This could be a bool since we only need the sign bit
         s32 x_dir = (mouse_tile.grid_pos.x - player->tile.grid_pos.x);
         s32 y_dir = (mouse_tile.grid_pos.y - player->tile.grid_pos.y);

         // @Incomplete: A shitty version of A* search for now
         int dx = -1, dy = -1;
         display_path_queue.push(player->tile);
         for (int i = 0; i <= chebshev; ++i) {
            Tile check_path_tile = display_path_queue.back();
            check_path_tile.color = GRAY;
            if (x_dir > 0) dx = 1;
            if (y_dir > 0) dy = 1;

            if (check_path_tile.grid_pos.x != mouse_tile.grid_pos.x) {
               check_path_tile.grid_pos.x += dx;
            }
            if (check_path_tile.grid_pos.y != mouse_tile.grid_pos.y) {
                  check_path_tile.grid_pos.y += dy;
            }
            display_path_queue.push(check_path_tile);
            update_tile(tilemap, check_path_tile.grid_pos, check_path_tile.color);
         }

         update_tile(tilemap, mouse_tile.grid_pos, highlight_tile_color);
         // @Cleanup: Sloppy way of displaying the path and then copying it
         check_path_queue = display_path_queue;

         if (input->left_button_pressed) {
            update_tile(tilemap, mouse_tile.grid_pos, click_tile_color);
         } else if (input->left_button_double_pressed) {
            if (tilemap->tiles[i].tilevalue == 1) {
               check_path_queue.pop();
               Tile check_path_tile = check_path_queue.front();
               Tile *neighbors = get_neighbors_for_tile(&gamestate->arena,
                                                        &player->tile,
                                                        NEIGHBOR_COUNT);
               for (int i = 0; i <= NEIGHBOR_COUNT - 1; ++i) {
                  Tile next_tile_in_path = neighbors[i];
                  if (next_tile_in_path.grid_pos == check_path_tile.grid_pos) {
                     player->tile.grid_pos = check_path_tile.grid_pos;
                     break;
                  }
               }
               free(neighbors);
            }
         }
      }
   }
#endif

   update_tile(tilemap, player->tile.grid_pos, player_tile_color);
   // @Speed: We loop through the three times
   debug_push_tilemap_instanced(backbuffer, tilemap);

   // gl_draw_model(&cube_gltf);

   // debug_draw_tilemap(backbuffer, tilemap);
   // draw_tilemap(backbuffer, tilemap);

   return;
}