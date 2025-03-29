#ifndef MARSHMALLOW_PLATFORM_H
#define MARSHMALLOW_PLATFORM_H

//
// This file is just a base for a lot of logic and data that are transient between platforms.
// For a particular platform layer, it uses the logic and data provided as hooks into its own
// platform specific implementation. They may be separated out into their own distinct files
// later on but they are kept here for now.
//

// typedef unsigned char       u8;
// typedef unsigned short      u16;
// typedef unsigned int        u32;
// typedef unsigned long long  u64;

// typedef signed char         s8;
// typedef signed short        s16;
// typedef signed int          s32;
// typedef signed long long    s64;

// typedef int                 b32;
// typedef int                 b32x;
// typedef float               f32;
// typedef double              f64;

#define Kilobytes(Value)  ((Value)*1024)
#define Megabytes(Value)  (Kilobytes(Value)*1024)
#define Gigabytes(Value)  (Megabytes(Value)*1024)
#define Terabytes(Value)  (Gigabytes(Value)*1024)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Pi32 3.14159265359f


typedef struct GameMemory {
   bool        is_initialized;
   u64         permanent_storage_size;
   void        *permanent_storage;    //Required to be cleared to zero at startup

   u64         transient_storage_size;
   void        *transient_storage;    //Required to be cleared to zero at startup
} GameMemory;

typedef struct ButtonState {
   const char *name;
   int         ends_down;
   int         half_transition_count;
   bool        button_pressed;
   bool        button_double_pressed;
} ButtonState;

typedef struct GameInput {
   V2 mouse_pos;
   float wheel_data;
   /*union {
      struct {
         //@Incomplete: This only has the mouse button keys
         ButtonState left;
         ButtonState right;
         ButtonState middle;
         ButtonState xbutton1;
         ButtonState xbutton2;
      };
      ButtonState mouse_button[5];
   };*/
   bool left_button_pressed;
   bool q_button_pressed;
   bool left_button_double_pressed;
   bool editor_button_pressed;
   bool right_button_pressed;
   bool right_bracket_pressed;

} GameInput;

typedef struct GameBackbuffer {
   u32 w;
   u32 h;
   u32 pixel_format;
   u32 pitch;
   u32 *pixels;
   u32 bytes_pp;
} GameBackbuffer;

#endif