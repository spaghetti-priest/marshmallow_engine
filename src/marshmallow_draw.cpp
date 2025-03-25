function void
debug_draw_rect (GameBackbuffer *buffer, V2S p0, V2S p1, f32 r, f32 g, f32 b, f32 a = 1.0)
{
   s32 x0 = p0.x;
   s32 y0 = p0.y;
   s32 x1 = p1.x;
   s32 y1 = p1.y;

   if (x0 < 0)             x0 = 0;
   if (y0 < 0)             y0 = 0;
   if (x1 > buffer->w)     x1 = buffer->w;
   if (y1 > buffer->h)     y1 = buffer->h;

   u8 *row     = ((u8*)buffer->pixels + x0 * buffer->bytes_pp + y0 * buffer->pitch);
   u32 color   = pack_BGRA(r, g, b, a);

   for (int y = y0; y < y1; ++y) {
      u32 *pixel = (u32*)row;
      for (int x = x0; x < x1; ++x) {
         *pixel++ = color;
      }
      row += buffer->pitch;
   }
}

function void
draw_pixel (GameBackbuffer *buffer, s32 x, s32 y, u32 color)
{
   u8 *pixel    = ((u8*)buffer->pixels + x * buffer->bytes_pp + y * buffer->pitch);
   *(u32*)pixel = color;
}

function void
test_gradient (GameBackbuffer *buffer, s32 x_offset, s32 y_offset)
{
   u8 *row     = (u8 *)buffer->pixels;
   int pitch   = buffer->w * buffer->bytes_pp;

   for (int y = 0; y < buffer->h; ++y) {
      u32 *pixel = (u32 *)row;
      for (int x = 0; x < buffer->w; ++x) {
         u8 blue  = (x + x_offset);
         u8 red   = (y + y_offset);
         // pixel layout is BGRA
         *pixel++ = (blue << 0 | (red << 16) );
      }
      row += pitch;
   }
}

function void
debug_draw_rectf32 (GameBackbuffer *buffer, V2 p0, V2 p1, f32 r, f32 g, f32 b, f32 a = 1.0f)
{
   s32 x0 = round_f32_to_s32(p0.x);
   s32 y0 = round_f32_to_s32(p0.y);
   s32 x1 = round_f32_to_s32(p1.x);
   s32 y1 = round_f32_to_s32(p1.y);

   if (x0 < 0)             x0 = 0;
   if (y0 < 0)             y0 = 0;
   if (x1 > buffer->w)     x1 = buffer->w;
   if (y1 > buffer->h)     y1 = buffer->h;

   u8 *row     = ((u8*)buffer->pixels + x0 * buffer->bytes_pp + y0 * buffer->pitch );
   u32 color   = pack_BGRA(r, g, b, a);

   for (int y = y0; y < y1; ++y) {
      u32 *pixel = (u32*)row;
      for (int x = x0; x < x1; ++x) {
         *pixel++ = color;
      }
      row += buffer->pitch;
   }
}

function void
draw_tile (GameBackbuffer *buffer, Tile *tile)
{
   assert(tile && buffer);
   debug_draw_rect(buffer, tile->min, tile->max, tile->color.x, tile->color.y, tile->color.z, tile->color.w);
}


function void
draw_tilemap (GameBackbuffer *buffer, Tilemap *tilemap, bool clearmap)
{
   if (clearmap) {
      // @Speed: Instead of clearing the entire tilemap. Just clear the tiles that arent being affected
      // OR just have a tilemap dirty flag for the tiles that have being marked for update then draw those
      for (int y = 0; y <= TILE_COUNT_Y; ++y) {
         for (int x = 0; x <= TILE_COUNT_X; ++x) {
            u32 tile_index = y * TILE_COUNT_X + x;
            update_tile(tilemap, x, y, traversable_tile_color);
         }
      }
   } else {
      for (int y = 0; y <= TILE_COUNT_Y; ++y) {
         for (int x = 0; x <= TILE_COUNT_X; ++x) {
            // @Speed: Instead of clearing the entire tilemap. Just clear the tiles that arent being affected
            // OR just have a tilemap dirty flag for the tiles that have being marked for update then draw those
            u32 tile_index = y * TILE_COUNT_X + x;
            draw_tile(buffer, &tilemap->tiles[tile_index]);
         }
      }
   }
}

