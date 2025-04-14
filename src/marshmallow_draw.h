#ifndef MARSHMALLOW_DRAW_H

#include "marshmallow_base.h"

function void draw_pixel(GameBackbuffer *buffer, s32 x, s32 y, u32 color);
function void test_gradient(GameBackbuffer *buffer, s32 x_offset, s32 y_offset);
function void debug_draw_rect(GameBackbuffer *buffer, V2S p0, V2S p1, f32 r, f32 g, f32 b, f32 a);
function void debug_draw_rectf32(GameBackbuffer *buffer, V2 p0, V2 p1, f32 r, f32 g, f32 b, f32 a);

#define MARSHMALLOW_DRAW_H
#endif
