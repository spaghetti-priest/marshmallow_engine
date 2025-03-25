#define TILE_WIDTH 64
#define TILE_HEIGHT 64
#define TILE_COUNT_X 12
#define TILE_COUNT_Y 8

global_variable V4 player_tile_color       = v4(1.0, 1.0, 0.0, 1.0);
global_variable V4 traversable_tile_color  = v4(1.0, 0.0, 0.5, 0.0);
global_variable V4 boundry_tile_color      = v4(0.7, 0.7, 0.7, 1.0);
global_variable V4 click_tile_color        = v4(0.0, 1.0, 0.0, 1.0);
global_variable V4 highlight_tile_color    = v4(1.0, 1.0, 1.0, 1.0);

inline Tile
get_tile (s32 x, s32 y, V4 color, V2 thickness, s32 tile_x, s32 tile_y, int tilevalue)
{
   Tile new_tile        = {};
   new_tile.min         = v2s(x, y);
   new_tile.max         = v2s((x + TILE_WIDTH) - thickness.x, (y + TILE_HEIGHT) - thickness.y);
   new_tile.color       = color;
   new_tile.grid_pos.x  = tile_x;
   new_tile.grid_pos.y  = tile_y;
   new_tile.tilevalue   = tilevalue;
   return new_tile;
}

inline Tile
get_tile (s32 x, s32 y, f32 r, f32 g, f32 b, f32 a, V2 thickness, s32 tile_x, s32 tile_y, int tilevalue)
{
   Tile new_tile        = {};
   new_tile.min         = v2s(x, y);
   new_tile.max         = v2s((x + TILE_WIDTH) - thickness.x, (y + TILE_HEIGHT) - thickness.y);
   new_tile.color       = v4(r, g, b, a);
   new_tile.grid_pos.x  = tile_x;
   new_tile.grid_pos.y  = tile_y;
   new_tile.tilevalue   = tilevalue;
   return new_tile;
}

// inline void
// draw_tile (GameBackbuffer *buffer, Tile *tile)
// {
//    assert(tile && buffer);
//    debug_draw_rect(buffer, tile->min, tile->max, tile->color.x, tile->color.y, tile->color.z, tile->color.w);
// }

inline void
update_tile (Tilemap *tilemap, s32 tile_x, s32 tile_y, V4 color)
{
   assert(tilemap);
   assert(tile_y < tilemap->tile_count_y || tile_y > 0);
   assert(tile_x < tilemap->tile_count_x || tile_x > 0);
   // if ((tile_y > tilemap->tile_count_y || tile_y < 0) || (tile_x > tilemap->tile_count_x || tile_x < 0))
      // return;

   // @Note: All of these bugs revolve around the fact that the bounds check is not correct
   // @Bug: When the player position is at X == 0 or X == tile_count_x, the left or right neighbor will equal -1 or tile_count + 1,
   // @Bug: When the player access the top tile row (Y == 0) the game crashes presumably due to the path tile updating out of bounds
   u32 tile_index = tile_y * tilemap->tile_count_x + tile_x;
   tilemap->tiles[tile_index].color = color;
}

inline void
update_tile (Tilemap *tilemap, V2S pos, V4 color)
{
   assert(tilemap);
   update_tile(tilemap, pos.x, pos.y, color);
}

// inline void
// update_tile_position(Tilemap *tilemap, V2S *src_pos)
// {
//     TilemapPosition *dst_pos = src_pos;
// }

inline s32
get_tileposition(Tilemap *tilemap, int tile_x, int tile_y)
{
    assert(tilemap);
    u32 grid_dimensions = tilemap->tile_count_x;
    u32 current_tile_id = tile_y * grid_dimensions + tile_x;
    // s32 r               = tilemap->tiles[current_tile_id].tilevalue;
    return current_tile_id;
}

inline s32
get_tilevalue (Tilemap *tilemap, s32 tile_x, s32 tile_y)
{
   assert(tilemap);
   s32 tilevalue       = 0;
   u32 grid_dimensions = tilemap->tile_count_x;
   u32 current_tile_id = tile_y * grid_dimensions + tile_x;
   tilevalue           = tilemap->tiles[current_tile_id].tilevalue;
   return tilevalue;
}

// @Cleanup: Change from color to the tilevalue map
inline void
set_tilevalue(Tilemap *tilemap, s32 tile_x, s32 tile_y, int tilevalue, V4 color)
{
   assert(tilemap);
   u32 grid_dimensions                          = tilemap->tile_count_x;
   u32 current_tile_id                          = tile_y * grid_dimensions + tile_x;
   tilemap->tiles[current_tile_id].tilevalue    = tilevalue;

   update_tile(tilemap, tile_x, tile_y, color);
}

// @Cleanup: A tile is a square so the tile width/height should just be one variable/dimension
// @Implementation: If the user creates a tilemap the editor should determine where the tilemap starts by the direction of the vector of the mouse
// So if the direction vector is +X +Y, the (0,0) of the tilemap should be at the bottom right and if +X, -Y, it should start at the top-right hmm...
function Tilemap*
initialize_tilemap (MemoryArena *arena, World *world, u32 tile_width, u32 tile_height, u32 tile_count_x,
                    u32 tile_count_y, int start_x, int start_y)
{
   Tilemap *res        = world->tilemap;
   // Tilemap *res        = push_struct(arena, Tilemap);
   res->tile_width     = tile_width;
   res->tile_height    = tile_height;
   res->tile_count_x   = tile_count_x;
   res->tile_count_y   = tile_count_y;
   res->tiles          = push_array(arena, tile_count_x * tile_count_y, Tile);
   int initial_start_x = start_x;

   for (int y = 0; y <= tile_count_y - 1; ++y) {
      // res->grid_pos.tile_y = y;
      for (int x = 0; x <= tile_count_x - 1; ++x) {
         // res->grid_pos.tile_x    = x;
         u32 current_tile_id           = y * tile_count_x + x;
         res->tiles[current_tile_id]   = get_tile(start_x, start_y,
                                                  traversable_tile_color, v2(2.0, 2.0),
                                                  x, y,
                                                  1); // Make all of the tiles traversaable at init time
         start_x += TILE_WIDTH;
      }
      start_y += TILE_HEIGHT;
      start_x = initial_start_x;
   }

   return res;
}

inline Tile*
get_neighbors_for_tile (Tile *tile,  u32 neighbor_count)
{
   // @Incomplete: The neighbor count should always either equal 2, 4, 6, or 8 directions
   // @Incomplete: Check the if any of the neighbors are outside of the tilemap bounds
   // and then discard them
   // @Cleanup: Create a dir struct that resembles the position + 1 and loop over the neighbors
   // array. This is much more simplier and can allow us to check if the neighbor tile is out of bounds
   Tile *neighbors       = (Tile*)malloc(sizeof(Tile) * neighbor_count);

   neighbors[0].grid_pos = {tile->grid_pos.x - 1,     tile->grid_pos.y};      // left
   neighbors[1].grid_pos = {tile->grid_pos.x + 1,     tile->grid_pos.y};      // right
   neighbors[2].grid_pos = {tile->grid_pos.x,         tile->grid_pos.y - 1};  // top
   neighbors[3].grid_pos = {tile->grid_pos.x,         tile->grid_pos.y + 1};  // bottom

   neighbors[4].grid_pos = {tile->grid_pos.x - 1,     tile->grid_pos.y - 1};  // top left
   neighbors[5].grid_pos = {tile->grid_pos.x + 1,     tile->grid_pos.y - 1};  // top right
   neighbors[6].grid_pos = {tile->grid_pos.x - 1,     tile->grid_pos.y + 1};  // bottom left
   neighbors[7].grid_pos = {tile->grid_pos.x + 1,     tile->grid_pos.y + 1};  // bottom right

   return neighbors;
}


// function void
// draw_tilemap (GameBackbuffer *buffer, Tilemap *tilemap, bool clearmap)
// {
//    if (clearmap) {
//       // @Speed: Instead of clearing the entire tilemap. Just clear the tiles that arent being affected
//       // OR just have a tilemap dirty flag for the tiles that have being marked for update then draw those
//       for (int y = 0; y <= TILE_COUNT_Y; ++y) {
//          for (int x = 0; x <= TILE_COUNT_X; ++x) {
//             u32 tile_index = y * TILE_COUNT_X + x;
//             update_tile(tilemap, x, y, traversable_tile_color);
//          }
//       }
//    } else {
//       for (int y = 0; y <= TILE_COUNT_Y; ++y) {
//          for (int x = 0; x <= TILE_COUNT_X; ++x) {
//             // @Speed: Instead of clearing the entire tilemap. Just clear the tiles that arent being affected
//             // OR just have a tilemap dirty flag for the tiles that have being marked for update then draw those
//             u32 tile_index = y * TILE_COUNT_X + x;
//             draw_tile(buffer, &tilemap->tiles[tile_index]);
//          }
//       }
//    }
// }