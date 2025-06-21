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

typedef struct MarshFile {
  void *contents;
  u32 size;
} MarshFile;

// @Compile-Error: SDL already reserves the function keyword here so
// we use static for now
static MarshFile debug_read_file(const char *filename);
static MarshFile debug_read_null_terminated_file(const char *filename);
static bool debug_write_file(MarshFile *file, const char *filename);
static bool debug_list_all_directories(const char *dirname);


#define SDL_MARSHMALLOW_H
#endif
