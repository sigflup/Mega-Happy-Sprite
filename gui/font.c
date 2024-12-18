/*
 * Mega Happy Sprite is released under the BSD 3-Clause license.
 * read ../LICENSE for more info
 */


#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "gui_types.h"
#include "link.h"
#include "drop.h"
#include "gui.h"
#include "draw.h"
#include "std_dialog.h"
#include "font.h"

void draw_char(int x, int y, char c, int fg_color, int bg_color, int flags) {
 int t,s;
 Uint32 color, mask;
 union {
  Uint8  *b;
  Uint32 *l;
 } pix, pix2;

 if((x+current_grp->pos_x) < 0) return;
 if((x+current_grp->pos_x) > gc->w-8) return;
 if((y+current_grp->pos_y) < 0) return;
 if((y+current_grp->pos_y) > gc->h-8) return;

 /* XXX pointer math */
 pix.b = (Uint8 *)(gc->pixels+
                   ((x+current_grp->pos_x)*gc->format->BytesPerPixel) +
		   ((y+current_grp->pos_y)*gc->pitch));

 mask = (gc->format->Rmask | 
         gc->format->Gmask |
 	 gc->format->Bmask |
	 gc->format->Amask)^~0;

 for(t=0;t<8;t++)
  for(s=0;s<8;s++) {
   if(((font[ ((c-0x20) * 8)+t]>>(8-s)) &1) == 1) {
    if(CHECK_FLAG(flags,NO_HASH) == TRUE) 
     color = fg_color;
    else 
     color = ((t&1)==1 ? fg_color : bg_color);
    pix2.b = &pix.b[(t * gc->pitch)+(s*gc->format->BytesPerPixel)];
    *pix2.l&=mask;
    *pix2.l|=color;
   }
  }

}

void draw_text(int x,int y, char *in, color_t *fg, color_t *bg, int flags, int max) {
 int i;
 char terminal;

 if(CHECK_FLAG(flags, CR_TERMINAL) == TRUE) 
  terminal = '\n';
 else
  terminal = 0;

 LOCK;
 for(i=0;i<strlen(in);i++) {
  if(in[i] == terminal) break;
  if(CHECK_FLAG(flags, MAX_CHARS) == TRUE)
   if(i > max) break;
  draw_char( x+ (i*8), y, in[i],fg->map, bg->map, flags);
 }
 UNLOCK;
 globl_dirt = 1;
}

/* fonts 0x20 - 0x7f */

unsigned char font[] = { 
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00,
 0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00,
 0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00,
 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00,
 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00,
 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00,
 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,
 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00,
 0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00,
 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00,
 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00,
 0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00,
 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00,
 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00,
 0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00,
 0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00,
 0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00,
 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30,
 0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00,
 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00,
 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
 0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00,
 0x7C, 0xC6, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00,
 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00,
 0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,
 0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00,
 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00,
 0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00,
 0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00,
 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00,
 0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00,
 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00,
 0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00,
 0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00,
 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00,
 0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00,
 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00,
 0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00,
 0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00,
 0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00,
 0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00,
 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
 0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
 0xC6, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0xC6, 0x00,
 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00,
 0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00,
 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,
 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,
 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00,
 0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00,
 0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00,
 0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
 0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
 0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00,
 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
 0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00,
 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78,
 0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,
 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
 0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xD6, 0x00,
 0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
 0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00,
 0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0,
 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E,
 0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00,
 0x00, 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00,
 0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00,
 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
 0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
 0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00,
 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
 0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,
 0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00,
 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
 0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00,
 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00
};
