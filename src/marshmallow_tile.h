#ifndef MARSHMALLOW_TILE_H
#define MARSHMALLOW_TILE_H

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

#endif