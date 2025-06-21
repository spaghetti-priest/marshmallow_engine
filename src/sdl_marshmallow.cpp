/*
*   This is the SDL platform layer. Im using SDL because it is quicker to set the platform layer this way and is easier to integrate vulkan with,
*   aswell as is cross platform viable with MACOS/Linux. The development of the Win32 platform layer will come later.
*/

#include "SDL.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "iostream"
#include <assert.h>

#include <AL/al.h>
#include <AL/alc.h>

//@Remove:
#include <queue>
#include <unordered_map>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
// #include "dirent.h"

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef int                 b32;
typedef int                 b32x;
typedef float               f32;
typedef double              f64;

// #include "marshmallow_vulkan.h"
// #include "marshmallow_vulkan.cpp"

#define SCREEN_FPS 60
#define SCREEN_TICKS_PER_FRAME 1000 / SCREEN_FPS

#define LEVEL_WIDTH 2560
#define LEVEL_HEIGHT 1440

#include "pepsimania_math.h"
#include "sdl_marshmallow.h"
#include "marshmallow_base.h"

global_variable const u32 screen_w = 1280;
global_variable const u32 screen_h = 720;

#include "marshmallow_gl.h"
#include "marshmallow_gl.cpp"

#include "marshmallow.cpp"

typedef struct Table {
   int key;
   void *value;
} Table;

struct Item {
   V2S index; // In this case it is the Tile grid position
   int priority;
};

// typedef struct PriorityQueue {
//    Item *item;
//    int capacity;
//    int size;
// } PriorityQueue;

// PriorityQueue
// init_priorrity_queue (int size)
// {
//    PriorityQueue pq  = {};
//    pq.capacity       = -1;
//    pq.size           = size;
//    pq.item           = (Item*)calloc(size, sizeof(Item));
//    pq.item->index    = v2s(0,0);
//    pq.item->priority = 0;

//    return pq;
// }

// // @Note: This enqueue is for min priority queue
// void
// pq_enqueue (PriorityQueue *pq, V2S index, int priority)
// {
//    assert(pq->capacity < pq->size);
//    pq->item[++pq->capacity] = {index, priority};
// }

// bool
// pq_is_empty (PriorityQueue *pq)
// {
//    return pq.capacity < 0;
// }

// int
// pq_peek (PriorityQueue *pq)
// {
//    int lowest_priority_value = INT_MIN;
//    int index = -1;
//    for (int i = 0; i < pq->size; ++i) {

//    }
// }

inline const char*
strip_file_path (const char *s)
{
   const char *last = " ";
   if (strrchr(s, '\\') != nullptr) {
      last = strrchr(s, '\\');
   } else if (strrchr(s, '/') != nullptr) {
      last = strrchr(s, '/');
   }
   last += 1;

   return last;
}

function void
deinitialize_memory_block (void *address)
{
   bool success = VirtualFree(address, 0, MEM_RELEASE);
   if (!success) {
      printf("[ERROR]: Failed to deallocate memory!\n");
   }
}

function inline void*
initialize_memory_block (size_t size)
{
   void *memory = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   if(!memory) {
     printf("[ERROR]: Failed to allocate memory!\n");
     return nullptr;
   }
   return memory;
}

function inline void*
commit_memory_block (size_t size)
{
   void *memory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
   if(!memory) {
      printf("[ERROR]: Failed to allocate memory!\n");
      return nullptr;
   }
   return memory;
}

function inline void*
reserve_memory_block (size_t size)
{
   void *memory = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
   if(!memory) {
      printf("[ERROR]: Failed to allocate memory!\n");
      return nullptr;
   }
   return memory;
}

#if 1
function MarshFile
debug_read_file (const char *filename)
{
   MarshFile res = {};
   FILE *f = nullptr;
   f = fopen(filename, "rb");
   const char *strip_name = strip_file_path(filename);

   if (!f) {
     printf("[ERROR]: Failed to open file: [%s]\n", strip_name);
     return res;
   }

   fseek(f, 0, SEEK_END);
   u32 filesize = ftell(f);
   fseek(f, 0, SEEK_SET);

   void *filecontents  = malloc(sizeof(void*) * filesize);
   u32 bytes_read      = fread(filecontents, sizeof(void*), filesize, f);

/*    if(bytes_read != filesize) {
        printf("[ERROR]: Bytes read does not equal total filesize, possibly corrupted file!\n");
        return res;
    }
*/
   res.contents    = filecontents;
   res.size        = filesize;
   fclose(f);
   return res;
}

// @Note: debug_read_file does not check for null termination. So this function
// exists
function MarshFile
debug_read_null_terminated_file (const char *filename)
{
   MarshFile res = {};
   FILE *f = fopen(filename, "rb");
   const char *strip_name = strrchr(filename, '/');

   if (!f) {
      printf("[ERROR]: Failed to open file: [%s]\n", strip_name);
      return res;
   }

   fseek(f, 0, SEEK_END);
   u32 filesize = ftell(f);
   fseek(f, 0, SEEK_SET);

   char *filecontents  = (char*)malloc(sizeof(char*) * filesize + 1);
   u32 bytes_read      = fread((void*)filecontents, sizeof(char*), filesize, f);

   // if (bytes_read != (filesize / 8)) {
   //    printf("[ERROR]: Bytes read does not equal total filesize, possibly corrupted file!\n");
   //    return res;
   // }

   filecontents[filesize]  = '\0';
   res.contents            = (void*)filecontents;
   res.size                = filesize;
   fclose(f);

   return res;
}

function bool
debug_write_file (MarshFile *file, const char *filename)
{
   FILE *f = fopen(filename, "wb");
   if(!f) {
     printf("[ERROR]: Failed to open file");
     return 0;
   }

   u32 bytes_written = fwrite(file->contents, sizeof(void*), file->size, f);
   fclose(f);

   return 1;
}
/*
function bool
debug_list_all_directories(const char *dirname)
{
   dirent *directory_entry = {};
   DIR *dir = opendir(dirname);
   if (dir == NULL) printf("Could not open current directory\n");

   while ((directory_entry = readdir(dir)) != NULL)
      printf("%s\n", directory_entry->d_name);
   closedir(dir);
   return 1;
}
*/
#else

function MarshFile
debug_read_file (const char *filename)
{
   assert(strcmp(filename," "));

   MarshFile result                = {0};
   LARGE_INTEGER file_size_read    = {0};
   void *file_contents             = nullptr;
   u32 total_file_size             = 0;
   DWORD bytes_read                = 0;

   HANDLE file_handle = CreateFileA(filename, GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING, 0 , 0);

   if (file_handle == INVALID_HANDLE_VALUE) {
      printf("[ERROR]: Invalid File handle %d\n", GetLastError());
      CloseHandle(file_handle);
      return result;
   }

   GetFileSizeEx(file_handle, &file_size_read);
   total_file_size = (u32)file_size_read.QuadPart;

   if (total_file_size == 0) {
      printf("[ERROR]: Size of file read is/less than 0.\n");
      return result;
   }

   file_contents = initialize_memory_block(total_file_size);
   ReadFile(file_handle, file_contents, total_file_size, &bytes_read, 0);

   if (bytes_read == total_file_size) {
      result.contents = file_contents;
      result.size     = total_file_size;
   } else {
      deinitialize_memory_block(file_contents);
      result.contents = 0;
   }

   CloseHandle(file_handle);
   return result;
}

function bool
debug_write_file (MarshFile *file, const char *filename)
{
   HANDLE filehandle   = CreateFileA(filename, GENERIC_WRITE , NULL , NULL , CREATE_ALWAYS , 0 , 0);
   bool success        = false;
   DWORD bytes_written;

   if (filehandle == INVALID_HANDLE_VALUE) {
      printf("[ERROR]: Unable to open file\n");
      CloseHandle(filehandle);
      return false;
   }

   success = WriteFile(filehandle, file->contents, file->size, &bytes_written, 0) && (bytes_written == file->size);

   if (!success) {
      printf("[ERROR]: Unable to write to file, file was possibly corrupted during writing\n");
      CloseHandle(filehandle);
      return false;
   }

   CloseHandle(filehandle);
   return(success);
}

function bool
debug_list_all_directories (const wchar_t *dirname)
{
   WIN32_FIND_DATAW find_data = {};
   LARGE_INTEGER filesize;
   DWORD dwError = 0;
   HANDLE dir_handle = FindFirstFileW((LPCWSTR)dirname, &find_data);

   if (dir_handle == INVALID_HANDLE_VALUE) {
      dwError = GetLastError();
      printf("[ERROR]: Failed to find directory or file!\n");
      return 0;
   }
    // @Incomplete: This does not loop through all the files and directories within the main directory. It only stops after the first (main) directory read
   do {
      if (find_data.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY) {
         wprintf(L"%s <DIR>\n", find_data.cFileName);
      } else {
         filesize.LowPart = find_data.nFileSizeLow;
         filesize.HighPart = find_data.nFileSizeHigh;
         wprintf((L"%s %l1d bytes\n"), find_data.cFileName, filesize.QuadPart);
      }
   } while (FindNextFileW(dir_handle, &find_data) != 0);

   dwError = GetLastError();
   if (dwError != ERROR_NO_MORE_FILES) {
      printf("[ERROR]: %d\n",dwError);
      return 0;
   }
   FindClose(dir_handle);

   return 1;
}
#endif
typedef struct Image {
   int         x;
   int         y;
   int         num_channels;
   u8          *data;
   const char  *name;
} Image;

Image
load_image_in_pixels(const char *filename)
{
   // @Memory: Dont forget to reserve this struct in the memory arena rather than malloc it
   Image i = {};
   int x, y, num_channels;

   u8 *data = stbi_load(filename, &x, &y, &num_channels, 0);
   if (data) {
      i.x             = x;
      i.y             = y;
      i.num_channels  = num_channels;
      i.data          = data;
      i.name          = strip_file_path(filename);
   } else {
      printf("[ERROR] stbi_load failed to load image, Possibly bad filename");
      return i;
   }

   stbi_image_free(data);
   return i;
}

function void
SDL_swap_framebuffers (SDL_Context *context)
{
   SDL_UnlockSurface(context->surface);

   u32 mem_size = context->backbuffer->w * context->backbuffer->h * sizeof(u32);
   memcpy(context->surface->pixels, context->backbuffer->pixels, mem_size);

   SDL_LockSurface(context->surface);
   SDL_UpdateWindowSurface(context->window);
}

function inline u64
SDL_get_tick_value()
{
   u64 ticks = 0;
   ticks = SDL_GetTicks();
   return ticks;
}

function inline u64
SDL_get_seconds_elapsed (u64 start, u64 end)
{
   u64 delta = end - start;
   return delta;
}

function inline u64
SDL_get_performance_count()
{
   u64 count = 0;
   count = SDL_GetPerformanceCounter();
   return count;
}

function void
SDL_process_keyboard_and_mouse_input (ButtonState *input, int button_is_down)
{
   if (input->ends_down != button_is_down) {
      input->ends_down = button_is_down;
      input->half_transition_count++;
   }
   return;
}

// @Incomplete: Use an explicit string comparision or create a string lib
function bool
is_whitespace(char s)
{
   bool r = (s == ' ' || s == '\n' || s == '\0' || s == '\r');
   return r;
}

function bool
is_numeric(char *s)
{
   bool r =  (s >= "0" && s <= "9") || (s >= "0.0" && s <= "9.9");
   return r;
}

// @Incomplete: Check the string ourselves but im using this C lib function for now
function int
convert_char_to_int(char *str)
{
   int r = 0;
   return r;
}

#pragma pack(push, 1)
#define HEADER_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))
enum
{
 RIFF_CODE  = HEADER_CODE('R', 'I', 'F', 'F'),
 WAVE_CODE  = HEADER_CODE('W', 'A', 'V', 'E'),
 FMT_CODE   = HEADER_CODE('f', 'm', 't', ' '),
 DATA_CODE  = HEADER_CODE('d', 'a', 't', 'a'),
};

typedef struct WaveIdentifier {
   u32 chunk_id;
   u32 chunk_size;
   u32 wave_id;
} Wave_Header;

typedef struct WaveChunk {
   u32 id;
   u32 size;
} WaveChunk;

typedef struct FmtChunk {
   u16   audio_format;
   u16   num_channels;
   u32   sample_rate;
   u32   bytes_per_second;
   u16   bytes_per_block;
   u16   bits_per_sample;
} FmtChunk;
#pragma pack(pop)

struct ChunkIterator {
   u8 *at;
   u8 *end;
};

function ChunkIterator
parse_current_chunk(void *at, void *end)
{
   ChunkIterator res;
   res.at  = (u8*)at;
   res.end = (u8*)end;
   return res;
}

function bool
load_wav_file (const char *filename)
{
   //@Incomplete: Check if the byte order is little endian
   MarshFile wav_file      = debug_read_file(filename);
   const char *strip_name  = strip_file_path(filename);
   if (!wav_file.size) { printf("[ERROR]: Failed to load the WAV file %s", strip_name); return false; }

   WaveIdentifier *identifier = (WaveIdentifier*)wav_file.contents;
   assert(identifier->chunk_id == RIFF_CODE);
   assert(identifier->wave_id == WAVE_CODE);
   u16 channels; s16 *sample_data; u32 bits_per_sample;

   ChunkIterator iter = parse_current_chunk(identifier + 1, (u8*)(identifier + 1) + (identifier->chunk_size - 4));
   while(true)
   {
      if (iter.at >= iter.end) break;
      WaveChunk *current_chunk = (WaveChunk*)iter.at;
      switch(current_chunk->id)
      {
         case FMT_CODE:
         {
            FmtChunk *fmt = (FmtChunk*) (void*)(iter.at + sizeof(WaveChunk));
            u16 channels = fmt->num_channels;
            printf("FormatCode!\n");
         } break;

         case DATA_CODE:
         {
            sample_data = (s16*) (void*)(iter.at + sizeof(WaveChunk));
            bits_per_sample = current_chunk->size;
            printf("DataCode!\n");
         } break;
      }
      u32 size = (current_chunk->size + 1) & ~1;
      iter.at += sizeof(WaveChunk*) + size;
   }

   return true;
}

inline bool
sdl_get_key_state(SDL_KeyboardEvent *key_event, SDL_Keysym *keysym, SDL_Keycode keycode)
{
   bool is_pressed = false;
   if (keysym->sym == keycode) {
      if (key_event->state == SDL_PRESSED)
         is_pressed = true;
      if (key_event->state == SDL_RELEASED)
         is_pressed = false;
   }

   return is_pressed;
}

function void
SDL_process_events (SDL_Context *context, GameInput *input)
{
   while (SDL_PollEvent(context->event))
   {
      u32 event_type = context->event->type;
      switch(event_type)
      {
         case SDL_QUIT:
         {
            context->running = false;
         } break;

         case SDL_WINDOWEVENT:
         {
            SDL_WindowEvent window = context->event->window;
            if(window.event == SDL_WINDOWEVENT_CLOSE &&
               window.windowID == SDL_GetWindowID(context->window)){
               context->running = false;
            }
         } break;

         case SDL_KEYUP:
         case SDL_KEYDOWN:
         {
            SDL_KeyboardEvent key_event   = context->event->key;
            SDL_Keysym key_code           = context->event->key.keysym;

            input->w_pressed              = sdl_get_key_state(&key_event, &key_code, SDLK_w);
            input->a_pressed              = sdl_get_key_state(&key_event, &key_code, SDLK_a);
            input->s_pressed              = sdl_get_key_state(&key_event, &key_code, SDLK_s);
            input->d_pressed              = sdl_get_key_state(&key_event, &key_code, SDLK_d);
            input->spacebar_pressed       = sdl_get_key_state(&key_event, &key_code, SDLK_SPACE);
            input->left_control_pressed   = sdl_get_key_state(&key_event, &key_code, SDLK_LCTRL);

            if (key_code.sym == (SDLK_e)) {
               if(key_event.type == SDL_KEYDOWN) {
                  if (input->editor_button_pressed) {
                     input->editor_button_pressed = false;
                  } else {
                    input->editor_button_pressed = true;
                  }
               }
            }

            if (key_code.sym == SDLK_ESCAPE)
               context->running = false;
         } break;

         case SDL_MOUSEMOTION:
         {
            SDL_MouseMotionEvent mouse_motion_event   = context->event->motion;
            input->mouse_state.pos.x                  = mouse_motion_event.x;
            input->mouse_state.pos.y                  = mouse_motion_event.y;
            input->mouse_state.window_id              = mouse_motion_event.windowID;
            // @Bug: https://github.com/libsdl-org/SDL/issues/3701
            // SDL relative mouse motion sometimes reports negative values even when
            // the mouse is immobile
            input->mouse_state.vel.x                  = mouse_motion_event.xrel;
            input->mouse_state.vel.y                  = mouse_motion_event.yrel;
         } break;

         case SDL_MOUSEWHEEL:
         {
            SDL_MouseWheelEvent mouse_wheel_event  = context->event->wheel;
            input->mouse_state.wheel_data          = mouse_wheel_event.preciseY;
         } break;

         case SDL_MOUSEBUTTONDOWN:
         case SDL_MOUSEBUTTONUP:
         {
            SDL_MouseButtonEvent mouse_button_event = context->event->button;
            if (mouse_button_event.state == SDL_PRESSED) {
               if (mouse_button_event.button == SDL_BUTTON_LEFT) {
                  if (mouse_button_event.clicks == 1) {
                     input->left_button_pressed = true;
                  } else if (mouse_button_event.clicks == 2) {
                     printf("left button double clicked\n");
                     input->left_button_double_pressed = true;
                  }
               }
               if (mouse_button_event.button == SDL_BUTTON_RIGHT) {
                 input->right_button_pressed = true;
               }
            } else if (mouse_button_event.state == SDL_RELEASED) {
               if (mouse_button_event.button == SDL_BUTTON_LEFT) {
                  if (mouse_button_event.clicks == 1) {
                     input->left_button_pressed = false;
                  } else if(mouse_button_event.clicks == 2) {
                     input->left_button_double_pressed = false;
                  }
               }
               if (mouse_button_event.button == SDL_BUTTON_RIGHT) {
                  input->right_button_pressed = false;
               }
            }
         } break;
      }
   }
}

int main (int argc, char **argv)
{
   SDL_Context 	context;
   SDL_Event      event;
   SDL_Window     *window;
   SDL_Surface    *surface;
   SDL_Renderer   *renderer;
   SDL_Texture    *texture;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
     printf("Error: %s\n", SDL_GetError());
     return -1;
   }

   // @TODO(Bug): When you resize the window, the framebuffer does not refresh / swap
   // @TODO: Vulkan crashes on resize, so resizable is disable until sawpchain recreation is implemented
   // @Bug: When the window is not in focus the window crashes
   SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE   |
                                                   SDL_WINDOW_OPENGL  |
                                                  // SDL_WINDOW_VULKAN |
                                                  SDL_WINDOW_ALLOW_HIGHDPI);

   window = SDL_CreateWindow("Marshmallow\n",
                        SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED,
                        screen_w,
                        screen_h,
                        window_flags);

   surface = SDL_GetWindowSurface(window);

   GameMemory game_memory              = {};
   game_memory.permanent_storage_size  = Megabytes(64);
   game_memory.transient_storage_size  = Gigabytes(4);

   context                             = {0};
   context.event 	                     = &event;
   context.window                      = window;
   context.surface                     = surface;
   context.running                     = true;
   context.total_size                  = game_memory.permanent_storage_size + game_memory.transient_storage_size;
   context.game_memory_block           = initialize_memory_block(context.total_size);

   game_memory.permanent_storage       = context.game_memory_block;
   game_memory.transient_storage       = (u8*)game_memory.permanent_storage + game_memory.permanent_storage_size;

   GameInput input = {0};

   u64 start_time             = 0, current_time = 0;
   u64 frame_counter          = 0;
   float fps                  = (float)SDL_get_tick_value();
   // float fps               = SDL_get_performance_count();
   float cap_fps              = 0;
   u64 performace_frequency   = SDL_GetPerformanceFrequency();

   SYSTEM_INFO    sysInfo;
   GetSystemInfo(&sysInfo);

   // bool success = vulkan_init(window);
   bool success = init_sdl_opengl(window, screen_w, screen_h);
   bool init_mouse = true;

   stbtt_fontinfo font_info;
   // stbtt_bakedchar ascii_table[96]; //space to del

   unsigned char *temp_bitmap = (unsigned char *)malloc(sizeof(unsigned char) * (512 * 512));
   unsigned char *ttf_buffer  = (unsigned char *)malloc(sizeof(unsigned char) * (1 << 20));
   stbtt_bakedchar *ascii_table = (stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar) * 96);

   MarshFile file = debug_read_file("C:\\Windows\\Fonts\\arial.ttf");
   // char codepoint = 'S';
   int font_w = 512, font_h = 512, font_xoff = 0, font_yoff = 0;

   stbtt_InitFont(&font_info, (u8*)file.contents, 0);
   stbtt_BakeFontBitmap(font_info.data, font_info.fontstart, 32, temp_bitmap, font_w, font_h, 32, 96, ascii_table);
   gl_init_font_texture(temp_bitmap, font_w, font_h);

   // Audio Driver Stuff
   {
      ALCdevice *device;
      ALboolean enumeration;
      ALCcontext  *audio_context;
      ALCenum error;

      device = alcOpenDevice(NULL);
      if (!device) { printf("Failed to open AL device!\n"); }

      enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
      if (enumeration == AL_FALSE)
         printf("Enumeration is not supported!\n");

      audio_context = alcCreateContext(device, NULL);
      if (!alcMakeContextCurrent(audio_context)) {
         error = alGetError();
         printf("[ERROR]");
      }
   }

   // bool s = load_wav_file("C:\\marshmallow\\data\\searching_the_past.wav");

   while (context.running)
   {
      u64 fps_start_counter = SDL_get_performance_count();

      // @Todo: FPS is broken because it continously increases over time rather than reflecting the average frames rendered in one second. elapsed MS is more accurate
      float cap_fps  = SDL_get_tick_value();
      float avgfps   = frame_counter / ((float)SDL_get_seconds_elapsed(fps, SDL_get_tick_value()) / 1000.0f);
      if (avgfps > 2000000) avgfps = 0.0;

      SDL_process_events(&context, &input);

      SDL_Backbuffer backbuffer   = {0};
      backbuffer.w                = screen_w;
      backbuffer.h                = screen_h;
      backbuffer.pixel_format     = SDL_PIXELFORMAT_ARGB8888;
      backbuffer.pitch            = screen_w * 4;
      backbuffer.pixels           = (u32*)malloc(sizeof(u32) * (screen_w * screen_h));
      backbuffer.bytes_pp         = 4;
      context.backbuffer          = &backbuffer;
      memset(context.backbuffer->pixels, 0, screen_w * sizeof(u32) * screen_h);

      GameBackbuffer global_backbuffer = {0};
      global_backbuffer.w              = backbuffer.w;
      global_backbuffer.h              = backbuffer.h;
      global_backbuffer.pixel_format   = backbuffer.pixel_format;
      global_backbuffer.pitch          = backbuffer.pitch;
      global_backbuffer.pixels         = backbuffer.pixels;
      global_backbuffer.bytes_pp       = backbuffer.bytes_pp;

      gl_begin_frame();

      if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
         SDL_Delay(10);
         continue;
      }

      char text[256];
      _snprintf_s(text, sizeof(text), "Behold! The miracle of ressurection!");

       //marshmallow_print(text, 0, 720, v4(1.0, 1.0, 1.0, 1.0), ascii_table);

      game_update_and_render(&game_memory, &input, &global_backbuffer);
      gl_render_frame(init_mouse, &input);

// @Depreciated: software renderer
#if 0
      SDL_swap_framebuffers(&context);
#endif
      free(context.backbuffer->pixels);
      gl_swap_framebuffers(window);

      u64 fps_end_counter = SDL_get_performance_count();
      ++frame_counter;
      float elapsed_ms = ((fps_end_counter - fps_start_counter) / (float)performace_frequency) * 1000.0f;
      // printf("FPS : %.04f | MS per frame: %.04f\n", avgfps, elapsed_ms);

      cap_fps = (float)SDL_get_seconds_elapsed(cap_fps, SDL_get_tick_value());
      if (cap_fps < SCREEN_TICKS_PER_FRAME) {
         SDL_Delay(SCREEN_TICKS_PER_FRAME - cap_fps);
      }
   }

   gl_cleanup();
   SDL_DestroyWindow(window);
   SDL_Quit();
   stbtt_FreeBitmap(temp_bitmap, font_info.userdata);

	return 0;
}

