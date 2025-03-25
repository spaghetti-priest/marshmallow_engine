#include "marshmallow.h"
#include "marshmallow_tile.cpp"
#include "marshmallow_draw.h"
#include "marshmallow_draw.cpp"

// @Cleanup: Make this into an enum or table
V4 RED      = v4(1.0, 0.0, 0.0, 1.0);
V4 GREEN    = v4(0.0, 1.0, 0.0, 1.0);
V4 BLUE     = v4(0.0, 0.0, 1.0, 1.0);
V4 YELLOW   = v4(1.0, 1.0, 0.0, 1.0);
V4 WHITE    = v4(1.0, 1.0, 1.0, 1.0);
V4 GRAY     = v4(0.5, 0.5, 0.5, 0.5);

// global_variable V4 player_tile_color       = v4(1.0, 1.0, 0.0, 1.0);
// global_variable V4 traversable_tile_color  = v4(1.0, 0.0, 0.5, 0.0);
// global_variable V4 boundry_tile_color      = v4(0.7, 0.7, 0.7, 1.0);
// global_variable V4 click_tile_color        = v4(0.0, 1.0, 0.0, 1.0);
// global_variable V4 highlight_tile_color    = v4(1.0, 1.0, 1.0, 1.0);

std::queue<Tile> check_path_queue;
std::queue<Tile> display_path_queue;
GameModes modes;

void
game_update_and_render (GameMemory *memory, GameInput *input, GameBackbuffer *backbuffer)
{
   GameState *gamestate = (GameState*)memory->permanent_storage;
   assert(sizeof(gamestate) <= memory->permanent_storage_size);

   // If in editor mode the user creates an tilegrid. We should start from the mouse position
   // and then the tile count should be dictated by the distance from the initial mouse position
   // and the end mouse position
   int start_y = 100;
   int start_x = 100;

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

   if (!memory->is_initialized) {
      arena_alloc(&gamestate->arena,
                  memory->permanent_storage_size - sizeof(GameState),
                  (u8*)memory->permanent_storage + sizeof(GameState));
      gamestate->world        = push_struct(&gamestate->arena, World);

      World *world            = gamestate->world;
      world->tilemap          = push_struct(&gamestate->arena, Tilemap);
      world->player           = push_struct(&gamestate->arena, Player);
      Tilemap *tilemap        = initialize_tilemap(&gamestate->arena, world, 64, 64, 12, 8, 100, 100);

      Player *player          = world->player;
      player->tile.grid_pos   = {5, 3};
      player->tile.color      = player_tile_color;
      player->tile.tilevalue  = 2;

      memory->is_initialized  = true;
   }

   World   *world       = gamestate->world;
   Tilemap *tilemap     = world->tilemap;
   Player  *player      = world->player;

   Tile dest_tile       = {};
   dest_tile.grid_pos   = {2, 6};
   dest_tile.color      = v4(0.2, 0.5, 0.5, 0.0);
   dest_tile.tilevalue  = 1;

   // @Speed: We loop through the tilemap three times
   draw_tilemap(backbuffer, tilemap, true);

   // set_tilevalue(tilemap, 7,  4 ,0,  boundry_tile_color);
   // set_tilevalue(tilemap, 5,  4, 0,  boundry_tile_color);
   update_tile(tilemap, dest_tile.grid_pos, dest_tile.color);

   // @Speed: We loop through the tilemap three times
   std::queue<Tile> check_path_queue;
   std::queue<Tile> display_path_queue;
   const u32 NEIGHBOR_COUNT = 8;

   //
   // Process Inputs
   //
   for (int i = 0; i <= TILE_COUNT_Y * TILE_COUNT_X; ++i) {
      Tile mouse_tile = tilemap->tiles[i];

      if ((input->mouse_pos.x >= mouse_tile.min.x && input->mouse_pos.x <= mouse_tile.max.x)
      &&  (input->mouse_pos.y >= mouse_tile.min.y && input->mouse_pos.y <= mouse_tile.max.y)) {
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
            if(x_dir > 0) dx = 1;
            if(y_dir > 0) dy = 1;

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
               Tile *neighbors = get_neighbors_for_tile(&player->tile, NEIGHBOR_COUNT);
               for (int i = 0; i <= NEIGHBOR_COUNT - 1; ++i) {
                  Tile next_tile_in_path = neighbors[i];
                  if (next_tile_in_path.grid_pos == check_path_tile.grid_pos) {
                     player->tile.grid_pos = check_path_tile.grid_pos;
                     break;
                  }
               }
            }
         }
      }
   }
   update_tile(tilemap, player->tile.grid_pos, player_tile_color);

   // @Speed: We loop through the three times
   draw_tilemap(backbuffer, tilemap, false);

   return;
}