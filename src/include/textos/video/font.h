#pragma once

struct _FONT_INFO
{
  u8  *base;   // 位图地址
  u8  w;   // 宽
  u8  h;  // 高
};

typedef struct _FONT_INFO font_t;

/*
   Display a character `Code` on the screen,
   using the specific `Font`
*/
int font_show (u8 code, font_t *f, u64 x, u64 y, u32 fg, u32 bg);

/* Get a font info from the built-in array */
font_t *font_get (int idx);
