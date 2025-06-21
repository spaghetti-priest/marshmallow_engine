#ifndef MARSHMALLOW_H
#define MARSHMALLOW_H

// typedef struct Player {
//    V2 pos;
//    Tile tile;
//    Tile *next;
// } Player;

typedef struct Camera {
   V2 pos;
} Camera;

typedef struct Arena
{
   size_t  size;
   u8      *base_pointer;
   size_t  used;
} Arena;

// typedef struct World {
//    Tilemap *tilemap;
//    Player *player;
// } World;

enum GameModes {
   EDITOR_MODE,
   RUNTIME_MODE,
};

// typedef struct GameState {
//    MemoryArena arena;
//    World *world;
//    void *thing1;
//    GameModes game_modes;
// } GameState;

#define push_struct(arena, type)        (type *)arena_push(arena, sizeof(type))
#define push_array(arena, count, type)  (type *)arena_push(arena, (count * sizeof(type)))

static inline void
arena_alloc (Arena *arena, size_t size, u8 *base_pointer)
{
   arena->size         = size;
   arena->base_pointer = base_pointer;
   arena->used         = 0;
}

static inline void* 
arena_push (Arena *arena, size_t size)
{
   assert((arena->used + size) <= arena->size);
   void *result = arena->base_pointer + arena->used;
   arena->used += size;
   return (result);
}

void game_update_and_render(GameMemory *memory, GameInput *input, GameBackbuffer *backbuffer);

#endif