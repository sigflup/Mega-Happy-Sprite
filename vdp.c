/*
 * Mega Happy Sprite is released under the BSD 3-Clause license.
 * read LICENSE for more info
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "./gui/libgui.h"
#include "mega.h"
#include "vdp.h"
#include "draw.h"
#include "proc.h"
#include "bottom.h"

vdp_t *current_vdp;

int visible;

int pat_ref[0x400];
int pat_stop[0x400];

pat_bin_t pat_bin[0x500];
int pat_bin_num;

Uint8 *undo_vram[MAX_UNDO];

vdp_pixel vdp_screen[320*240];
vdp_pixel vdp_backbuf[320*240];
vdp_pixel sprite_screen[32*32];
vdp_pixel sprite_backbuf[32*32];

int scroll_width, scroll_height;
int sprite_width, sprite_height;

int visable;


int current_palette;
int current_palette_index;
int current_pattern;

int vdp_zoom, vdp_x, vdp_y;
int ovr_zoom, ovr_x, ovr_y;

pixel_dump_t pat_pix_a[4096];
pixel_dump_t pat_pix_b[4096];



void draw_pattern(Uint8 *ram, int num, int x, int y, int w, int h) {
 int poffset;
 int W, H;
 int k, k2;
 int i,j;
 color_t tmp_color;
 poffset = num *0x20;
 W = w/ 8;
 H = h/ 8;
 
 for(j = 0;j<8; j++) 
  for(i=0;i<4;i++) {
   k = (ram[poffset]>>4)&0xf;
   k2=  ram[poffset]&0xf;

   tmp_color.r = current_vdp->palette[current_palette][k].r;
   tmp_color.g = current_vdp->palette[current_palette][k].g;
   tmp_color.b = current_vdp->palette[current_palette][k].b;
   MAP_COLOR(tmp_color);
   fill_box( x+((i*2)*W),  y+(j*H),
             x+((i*2)*W)+W,y+(j*H)+H, &tmp_color, &tmp_color, NO_HASH); 
   tmp_color.r = current_vdp->palette[current_palette][k2].r;
   tmp_color.g = current_vdp->palette[current_palette][k2].g;
   tmp_color.b = current_vdp->palette[current_palette][k2].b;
   MAP_COLOR(tmp_color);
   fill_box( x+((i*2)*W)+W,  y+(j*H),
             x+((i*2)*W)+W+W,y+(j*H)+H, &tmp_color, &tmp_color, NO_HASH); 
 
   poffset++;
  }

}

void store_palette(vdp_t *vdp, Uint8 *ram) {
 int q,i;
 Uint8 a,b;
 i = 0;
 for(q=0;q<64;q++) {
  a = 0|((vdp->palette[q/16][q%16].r/(0xff/7))&7)<<1|
        ((vdp->palette[q/16][q%16].g/(0xff/7))&7)<<5;
  b = 0|((vdp->palette[q/16][q%16].b/(0xff/7))&7)<<1;
  ram[i++] = a;
  ram[i++] = b;
 }
}

void load_palette(vdp_t *vdp, Uint8 *ram) {
 int q, i;
 Uint8 a,b;
 i = 0;
 for(q=0;q<64;q++) {
  a = ram[i++];
  b = ram[i++];
  
  vdp->palette[q/16][q%16].r = ((a&0xe)>>1) * 	(0xff/7);
  vdp->palette[q/16][q%16].g = (((a>>4)&0xe)>>1) * 	(0xff/7);
  vdp->palette[q/16][q%16].b = ((b&0xe)>>1) * 	(0xff/7); 
  MAP_COLOR(vdp->palette[q/16][q%16]);
 }
}

void copy_vdp(vdp_t *dst, vdp_t *src) {
 memcpy(dst->vram, src->vram, 0xffff);
 memcpy(dst->palette,src->palette, sizeof(color_t) * 64); 
}

int new_pattern(Uint8 *pixels, int index, int *pat) {
 int i,j;
 int ret;
 int color;
 int found;

 ret = index;

 for(i = 0;i<0x400;i++) {
  found = TRUE;
  for(j = 0;j<64;j++) {
   if( pixels[j] != get_pix(current_vdp->vram, i, j%8, j/8)) {
    found = FALSE;
    break;
   }
  }
  if(found == TRUE) break;
 }
  
 if(found == TRUE) {
  *pat = i;
  pat_ref[i]++;
  return ret;
 } 

 color = current_palette_index;
 for(i=0;i<0x400;i++) 
  if( pat_stop[i] == FALSE &&
      pat_ref[i] == 0) 
   break;

 if(i == 0x400) {
  alert(gui_screen->w/2, gui_screen->h/2, "Out of pattern space!", "Oh no :O"); 
  return -1;
 }

 for(j=0;j<64;j++) {
  current_palette_index = pixels[j]&0xf;
  put_pix(current_vdp->vram, i, j%8, j/8);
 }


 current_palette_index = color;
 *pat = i;
 pat_ref[i]++;
 return ret;
}

void collapse(void) {
 scroll_data_t *scroll_data = (scroll_data_t *)0;
 int table_offset=0;
 int i,j;
 int index=0;
 int num;
 int pat;
 int found;
 int x=0,y=0;
 int store;
 pat_bin_num = 0;
 if(clob_object->param.d1 == TRUE) {
  for(i=0;i<num_xor_pix;i++) {
   switch(currently_editing) {
    case EDIT_SCROLL_A:
     index = vdp_screen[ xor_pixels[i].x + ( xor_pixels[i].y * vdp_w )].A.pat;
     x     = vdp_screen[ xor_pixels[i].x + ( xor_pixels[i].y * vdp_w )].A.ix;
     y     = vdp_screen[ xor_pixels[i].x + ( xor_pixels[i].y * vdp_w )].A.iy;
     break;
    case EDIT_SCROLL_B:
     index = vdp_screen[ xor_pixels[i].x + ( xor_pixels[i].y * vdp_w )].B.pat;
     x     = vdp_screen[ xor_pixels[i].x + ( xor_pixels[i].y * vdp_w )].B.ix;
     y     = vdp_screen[ xor_pixels[i].x + ( xor_pixels[i].y * vdp_w )].B.iy;
     break;
   }
   if(index != 0) {
    store = current_palette_index;
    current_palette_index = xor_pixels[i].index;
    put_pix(current_vdp->vram,  index, x, y); 
    current_palette_index = store;
   }
  }
  return;
 }
 for(i=0;i<num_xor_pix;i++) {
  found = 0;
  switch(currently_editing) {
   case EDIT_SCROLL_A:
    table_offset = current_vdp->scroll_a;
    scroll_data = &vdp_screen[xor_pixels[i].x + (xor_pixels[i].y * 320)].A;
    break;
   case EDIT_SCROLL_B:
    table_offset = current_vdp->scroll_b;
    scroll_data = &vdp_screen[xor_pixels[i].x + (xor_pixels[i].y * 320)].B;
    break;
  }

  for(j=0;j<pat_bin_num;j++) {
   index = scroll_data->pat_index;
   if(index == pat_bin[j].index) {
    found = 1;
    pat_bin[j].pixels[ 
     scroll_data->ix + (scroll_data->iy *8)] = xor_pixels[i].index;
    break;
   }
  }
  if(found == 0) {
   pat_bin[pat_bin_num].index = scroll_data->pat_index;
   num = scroll_data->pat;
   for(j=0;j<64;j++) {

    pat_bin[pat_bin_num].pixels[j] =  
     get_pix(current_vdp->vram, num, j%8, j/8);
   } 
   pat_bin[pat_bin_num].pixels[ 
    scroll_data->ix + (scroll_data->iy * 8)] = xor_pixels[i].index;

   pat_bin_num++; 
  }
  
 }
 for(j=0;j<pat_bin_num;j++) {
  index = new_pattern(pat_bin[j].pixels, pat_bin[j].index, &pat);
  if(index == -1) return;
  
  current_vdp->vram[table_offset + (index<<1) + 1] = pat &0xff;
  current_vdp->vram[table_offset + (index<<1)    ] &= ~3; 
  current_vdp->vram[table_offset + (index<<1)    ] |= ((pat>>8)&3);
 }
}

void render_sprite(void) {
 int x,y, i;
 int sx, sy;
 int dst_x, dst_y, dst_i;
 int src_x, src_y;
 int pal, data;
 vdp_pixel over_screen[32*32];
 unsigned char r,g,b;
 unsigned char r2,g2, b2;

 if(sprite_overlay_pic != -1) {
  for(src_x = 0, dst_i = 0;src_x<sprite_width+1;src_x++)
   for(src_y = 0;src_y<sprite_height+1;src_y++, dst_i++) {
    dst_x = dst_i % (sprite_width+1);
    dst_y = dst_i / (sprite_width+1);
    for(sy=0;sy<8;sy++)
     for(sx=0;sx<8;sx++) {
      data = get_pix(current_vdp->vram,
        sprite_overlay_pic+src_x+(src_y*(sprite_width+1)),sx,sy);
      over_screen[(sx+(dst_x*8))+((sy+(dst_y*8))*32)].index = data;
      over_screen[(sx+(dst_x*8))+((sy+(dst_y*8))*32)].pattern =
       sprite_overlay_pic+src_x+(src_y*(sprite_width+1)); 

      over_screen[(sx+(dst_x*8))+((sy+(dst_y*8))*32)].palette=current_palette;
      }
   }

 }

 for(src_x = 0, dst_i = 0;src_x<sprite_width+1;src_x++)
  for(src_y = 0;src_y<sprite_height+1;src_y++, dst_i++) {
   dst_x = dst_i % (sprite_width+1);
   dst_y = dst_i / (sprite_width+1);
   for(sy=0;sy<8;sy++)
    for(sx=0;sx<8;sx++) {
     data = get_pix(current_vdp->vram,
       current_pattern+src_x+(src_y*(sprite_width+1)),sx,sy);
     sprite_screen[(sx+(dst_x*8))+((sy+(dst_y*8))*32)].index = data;
     sprite_screen[(sx+(dst_x*8))+((sy+(dst_y*8))*32)].pattern =
      current_pattern+src_x+(src_y*(sprite_width+1)); 

     sprite_screen[(sx+(dst_x*8))+((sy+(dst_y*8))*32)].palette=current_palette;
     }
  }

 if(sprite_overlay_pic == -1) {
  for(y=0, i=0;y<32;y++)
   for(x=0;x<32;x++, i++) {
    data = sprite_screen[i].index;
    pal  = sprite_screen[i].palette;
    sprite_screen[i].color.r = current_vdp->palette[pal&3][data&0xf].r;
    sprite_screen[i].color.g = current_vdp->palette[pal&3][data&0xf].g;
    sprite_screen[i].color.b = current_vdp->palette[pal&3][data&0xf].b;
   } 
 } else {
  for(y=0, i=0;y<32;y++)
   for(x=0;x<32;x++, i++) {
    data = sprite_screen[i].index;
    pal  = sprite_screen[i].palette;
    r = current_vdp->palette[pal&3][data&0xf].r/2;
    g = current_vdp->palette[pal&3][data&0xf].g/2;
    b = current_vdp->palette[pal&3][data&0xf].b/2;

    data = over_screen[i].index;
    pal =  over_screen[i].palette;
    r2= current_vdp->palette[pal&3][data&0xf].r/2;
    g2= current_vdp->palette[pal&3][data&0xf].g/2;
    b2= current_vdp->palette[pal&3][data&0xf].b/2;
    
    sprite_screen[i].color.r = r+r2;
    sprite_screen[i].color.g = g+g2;
    sprite_screen[i].color.b = b+b2;

   } 

 }
}

void render_vdp(int start, int end) {
 Uint8 high;
 int x,y, i;
 int x_offset=0, y_offset=0;
 unsigned int scroll_data;
 int scroll_x, scroll_y;
 int data=0,pal=0;
 int off, top, pic_top=0;

 int a_index;
 int a_pattern_data;
 int a_pattern_num;
 int a_palette_num;
 int a_ix, a_iy;
 int a_sx, a_sy;
 int a_priority;

 int b_index;
 int b_pattern_data;
 int b_pattern_num;
 int b_palette_num;
 int b_ix, b_iy;
 int b_sx, b_sy;
 int b_priority;

 int scrl_w, scrl_h;

 scrl_w = 1<<(5+scroll_width);
 scrl_h = 1<<(5+scroll_height);

 for(i = 0; i<4096;i++) {
  pat_pix_a[i].num = 0;
  pat_pix_b[i].num = 0;
 }

 for(i=0;i<0x400;i++)
  pat_ref[i] = 0;

 for(y = 0;y<scrl_h;y++)
  for(x = 0;x<scrl_w;x++) {
   high = current_vdp->vram[current_vdp->scroll_a+ ((x+(scrl_w*y))<<1) ];
   pat_ref[
    current_vdp->vram[current_vdp->scroll_a +((x+(scrl_w*y))<<1) +1] | ((high&7)<<8)
   ]++;
   high = current_vdp->vram[current_vdp->scroll_b+ ((x+(scrl_w*y))<<1) ];
   pat_ref[
    current_vdp->vram[current_vdp->scroll_b +((x+(scrl_w*y))<<1) +1] | ((high&7)<<8)
   ]++;
  }

 for(y = start;y<end;y++)
  for(x=0;x<vdp_w;x++) {

   off = x+(320*y);
    
   if(vertical_scroll_bar->param.d1 == VER_SCREEN)
    y_offset = 0;
   else
    y_offset = (x / 16)<<2;

   scroll_data = current_vdp->vsram[y_offset+1] | 
               ((current_vdp->vsram[y_offset]&7)<<8);

   scroll_y = (y+(scroll_data))%(scrl_h*8);


   switch(horizontal_scroll_bar->param.d1) {
    case HOR_LINES:
     x_offset = current_vdp->hscroll + (y<<2);
     break;
    case HOR_8LINES:
     x_offset = current_vdp->hscroll + ((y&~7)<<2);
     break;
    case HOR_SCREEN:
     x_offset = current_vdp->hscroll;
     break;
   }


   scroll_data = current_vdp->vram[x_offset+1] |
               ((current_vdp->vram[x_offset]&7)<<8);
   scroll_x = (x+(0x400-scroll_data))%(scrl_w*8);


   a_ix = scroll_x%8;
   a_iy = scroll_y%8;
   a_index = (((scroll_x/8 % scrl_w)) + ((((scroll_y/8)%scrl_h)) * scrl_w)) * 2;
   a_sx  = (scroll_x/8) %scrl_w;
   a_sy  = (scroll_y/8) %scrl_h;

   if(pat_pix_a[a_index>>1].num<64) {
    pat_pix_a[a_index>>1].px[pat_pix_a[a_index>>1].num] = x;
    pat_pix_a[a_index>>1].py[pat_pix_a[a_index>>1].num] = y;
    pat_pix_a[a_index>>1].num++;
   }

   high = current_vdp->vram[current_vdp->scroll_a+a_index];
   a_pattern_num = current_vdp->vram[current_vdp->scroll_a + a_index+1] | ((high&7)<<8); 
//   pat_ref[a_pattern_num]++;
   a_palette_num = (high>>5)&3;
   if(FLIP_Y(high) == 1)
    a_iy = 7-a_iy;
   if(FLIP_X(high) == 1)
    a_ix = 7-a_ix;
   a_priority = (high>>7)&1;
   a_pattern_data = get_pix(current_vdp->vram,a_pattern_num, a_ix, a_iy); 

   scroll_data = current_vdp->vram[x_offset+3] |
               ((current_vdp->vram[x_offset+2]&7)<<8);
   scroll_x = (x+(0x400-scroll_data))%(scrl_w*8);

   scroll_data = current_vdp->vsram[y_offset+3] | 
               ((current_vdp->vsram[y_offset+2]&7)<<8);
   scroll_y = (y+(0x400-scroll_data))%(scrl_h*8);


   b_ix = scroll_x%8;
   b_iy = scroll_y%8;
   b_index = (((scroll_x/8 % scrl_w)) + ((((scroll_y/8)%scrl_h)) * scrl_w)) * 2;
   b_sx  = (scroll_x/8) %scrl_w;
   b_sy  = (scroll_y/8) %scrl_h;


   if(pat_pix_b[b_index>>1].num<64) {
    pat_pix_b[b_index>>1].px[pat_pix_b[b_index>>1].num] = x;
    pat_pix_b[b_index>>1].py[pat_pix_b[b_index>>1].num] = y;
    pat_pix_b[b_index>>1].num++;
   }

   high = current_vdp->vram[current_vdp->scroll_b+b_index];
   b_pattern_num = current_vdp->vram[current_vdp->scroll_b + b_index+1] | ((high&7)<<8); 
//   pat_ref[b_pattern_num]++;
   b_palette_num = (high>>5)&3;
   if(FLIP_Y(high) == 1)
    b_iy = 7-b_iy;
   if(FLIP_X(high) == 1)
    b_ix = 7-b_ix;
   b_priority = (high>>7)&1;
   b_pattern_data = get_pix(current_vdp->vram,b_pattern_num, b_ix, b_iy); 

   vdp_screen[off].A.pat = a_pattern_num;
   vdp_screen[off].A.ix = a_ix;
   vdp_screen[off].A.iy = a_iy;
   vdp_screen[off].A.sx = a_sx;
   vdp_screen[off].A.sy = a_sy;
   vdp_screen[off].A.pal = a_palette_num;
   vdp_screen[off].A.index = a_pattern_data;
   vdp_screen[off].A.priority = a_priority;
   vdp_screen[off].A.pat_index = a_index>>1;


   vdp_screen[off].B.pat = b_pattern_num;
   vdp_screen[off].B.ix = b_ix;
   vdp_screen[off].B.iy = b_iy;
   vdp_screen[off].B.sx = b_sx;
   vdp_screen[off].B.sy = b_sy;
   vdp_screen[off].B.pal = b_palette_num;
   vdp_screen[off].B.index = b_pattern_data;
   vdp_screen[off].B.priority = b_priority;
   vdp_screen[off].B.pat_index = b_index>>1;

   if(CHECK_FLAG(visible, VIS_SCROLL_B) == TRUE &&
      b_priority == 0) {
    pic_top = VIS_SCROLL_B;
    if(b_pattern_data != 0) 
     top = VIS_SCROLL_B;
    else 
     top = VIS_BACKGROUND;
   } else
    top = VIS_BACKGROUND; 

   if(CHECK_FLAG(visible, VIS_SCROLL_A) == TRUE &&
      a_priority == 0 &&
      a_pattern_data != 0)
    top = pic_top = VIS_SCROLL_A;

//   if(CHECK_FLAG(visible, VIS_SPRITE) == TRUE &&
//     sprite_priority == 0 &&
//     sprite_pattern_data != 0)
//    top = VIS_SPRITE;

   if(CHECK_FLAG(visible, VIS_SCROLL_B_HIGH) == TRUE &&
      b_priority == 1 &&
      b_pattern_data != 0)
    top = pic_top = VIS_SCROLL_B_HIGH;

   if(CHECK_FLAG(visible, VIS_SCROLL_A_HIGH) == TRUE &&
      a_priority == 1 &&
      a_pattern_data != 0)
    top = pic_top = VIS_SCROLL_A_HIGH;

//   if(CHECK_FLAG(visible, VIS_SPRITE_HIGH) == TRUE &&
//      sprite_priority == 0 &&
//      sprite_index != 0)
//    top = VIS_SPRITE_HIGH;
   
  switch(top) {
    case VIS_BACKGROUND:
     pal = current_vdp->bg_pal;
     vdp_screen[ off ].pattern = 0;
     vdp_screen[ off ].pat_index = 0;
     data = current_vdp->bg_index;
     break;
    case VIS_SCROLL_A: 
    case VIS_SCROLL_A_HIGH:
     pal = a_palette_num;
     vdp_screen[ off ].pattern = a_pattern_num;
     vdp_screen[ off ].pat_index = a_index>>1;
     data = a_pattern_data;
     break;
    case VIS_SCROLL_B:
    case VIS_SCROLL_B_HIGH:
     pal = b_palette_num;
     vdp_screen[ off ].pattern = b_pattern_num;
     vdp_screen[ off ].pat_index = b_index>>1;
     data = b_pattern_data;
     break;
   } 


   vdp_screen[off].palette = pal;
   vdp_screen[off].index = data;
   vdp_screen[off].top = top;
   vdp_screen[off].pic_top = pic_top;

   vdp_screen[off].color.r = current_vdp->palette[pal&3][data&0xf].r;
   vdp_screen[off].color.g = current_vdp->palette[pal&3][data&0xf].g;
   vdp_screen[off].color.b = current_vdp->palette[pal&3][data&0xf].b;

  }
}


int vdp_init(void) {
 int i;

 current_vdp = (vdp_t *)malloc(sizeof(vdp_t));

 current_vdp->vsram = (Uint8 *)malloc(80);
 current_vdp->bg_pal = 0;
 current_vdp->bg_index = 0;

 for(i = 0;i<MAX_UNDO;i++) 
  undo_vram[i] = (Uint8 *)malloc(0xffff);

 current_vdp->vram = undo_vram[0];

 current_palette = 0;
 current_palette_index = 0;
 current_vdp->tv_type = NTSC;
 current_vdp->cell_w = 40;

 current_vdp->scroll_a =     0xc000;
 current_vdp->scroll_b =     0xe000;
 current_vdp->window =       0xa000;
 current_vdp->sprite_table = 0xf800;
 current_vdp->hscroll =      0xfc00; 
/*
 current_vdp->scroll_a =     0x4000;
 current_vdp->scroll_b =     0x2000;
 current_vdp->window =       0xa000;
 current_vdp->sprite_table = 0xf800;
 current_vdp->hscroll =      0x7000; 
*/

 visible = VIS_SCROLL_B      | VIS_SCROLL_A      | VIS_SPRITE |
           VIS_SCROLL_B_HIGH | VIS_SCROLL_A_HIGH | VIS_SPRITE_HIGH; 


 vdp_x = 
  vdp_y = 0;

 vdp_zoom = 1;
 
 memset(current_vdp->vram,0,0xffff);
 memset(current_vdp->vsram,0, 80);
 for(i=1;i<0x400;i++)
  pat_stop[i] = FALSE;
 pat_stop[0] = TRUE;

 vdp_w = (current_vdp->cell_w == 40 ? 320 : 256);
 vdp_h = (current_vdp->tv_type == NTSC ? 224 : 240);
 return 1;
}

