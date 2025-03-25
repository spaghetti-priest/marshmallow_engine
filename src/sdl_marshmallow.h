#ifndef SDL_MARSHMALLOW_H

typedef struct SDL_Backbuffer {
   uint32_t w;
   uint32_t h;
   uint32_t pixel_format;
   uint32_t pitch;
   uint32_t *pixels;
   uint32_t bytes_pp;
} SDL_Backbuffer;

typedef struct _SDL_Context_ {
   SDL_Event       *event;
   SDL_Window      *window;
   SDL_Backbuffer  *backbuffer;
   SDL_Surface     *surface;
   SDL_Texture     *texture;
   SDL_Renderer    *renderer;
   void            *game_memory_block;
   u64             total_size;

   bool            running;
} SDL_Context;

#define SDL_MARSHMALLOW_H
#endif
