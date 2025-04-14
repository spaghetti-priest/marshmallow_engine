#ifndef MARSHMALLOW_TILE_H

typedef struct Tile {
   V2S min;
   V2S max;
   int tilevalue;  // The tile value tells us whether or not the tile is a boundry or is traversable
                   // Currently tilevalue == 1 is traversable while tilevalue == 0 is a boundry and
                   // tilevalue == 2 is the player
   V4 color;       // @Todo: Tilevalue should be mapped to color (player color, traversable color... etc)

   V2S grid_pos; //These are to store the relative grid number (X,Y,Z) for the tiles

   int id;  // @Remove: This is useless because the way tileid is used currently is as an array index. All tile ids should be renamed to tile index or something
            // The real look into the tile grid should be tilemap oosition
} Tile;

// @Todo: Change the name from tilemap to tilegrid
typedef struct Tilemap {
   int tile_width;
   int tile_height;
   // @Cleanup: this is just a V2s
   int tile_count_x;
   int tile_count_y;
   Tile *tiles;
} Tilemap;

function Tile *get_neighbors_for_tile(Tile *tile,  u32 neighbor_count);
function Tilemap *initialize_tilemap(MemoryArena *arena, s32 tile_width, s32 tile_height, u32 tile_count_x, u32 tile_count_y, int start_x, int start_y, V4 color);
function void update_tile(Tilemap *tilemap, V2S pos, V4 color);
function void set_tilevalue(Tilemap *tilemap, s32 tile_x, s32 tile_y, int tilevalue, V4 color);
function void draw_tile(GameBackbuffer *buffer, Tile *tile);
function void clear_tilemap(GameBackbuffer *buffer, Tilemap *tilemap, V4 color);
function void draw_tilemap(GameBackbuffer *buffer, Tilemap *tilemap);

#define MARSHMALLOW_TILE_H
#endif