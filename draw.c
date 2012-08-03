#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "./gui/libgui.h"
#include "config.h"
#include "mega.h"
#include "vdp.h"
#include "draw.h"
#include "bottom.h"

Uint8 draw_buffer[320][240];

void put_pix_draw_buffer(Uint8 *ram, int pat, int x, int y){
 draw_buffer[x][y] = 1;
}

int get_pix(Uint8 *ram, int pat, int x, int y) {
 int base = 0;
 Uint8 dat = 0;
 base = (pat * 0x20)+(((y*8)+x)>>1);
 dat = ram[base];
 if( (x&1)==0) 
  dat>>=4;
 return dat & 0xf;
}

int get_pix_preview_special(Uint8 *ram, int pat, int x, int y) {
 if(draw_buffer[x][y] == 1)
  return current_palette_index;

 switch(currently_editing) {
  case EDIT_SCROLL_A:
   return vdp_screen[x + (y*320)].A.index;
  case EDIT_SCROLL_B:
   return vdp_screen[x + (y*320)].B.index;
  case EDIT_SPRITE:
   return sprite_screen[x+(y*32)].index;
 } 
 return 0;
}

int get_pix_special(Uint8 *ram, int pat, int x, int y) {
 printf("whaa\n");
 if(draw_buffer[x][y] == 1) 
  return current_palette_index;
 else
  return get_pix(ram, pat, x,y);
}


void put_pix(Uint8 *ram, int pat, int x, int y) {
 int base =0;
 Uint8 dat=0,ldat=0;
 base = (pat * 0x20) +(((y*8)+x)>>1);
 dat = ram[base];
 ldat = dat;
 if( (x&1)==0) {
  dat &=0x0f;
  dat += (current_palette_index<<4);
 } else {
  dat &=0xf0;
  dat+=current_palette_index;
 }
 ram[base] = dat;
}

#define PUSH(Y,XL,XR,DY) \
 if(sp<stack+MAX_FLOOD && Y+(DY)>=wy1 && Y+(DY)<=wy2) \
 {sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy=DY; sp++;}

#define POP(Y,XL,XR,DY) \
 {sp--; Y=sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr; }

void do_flood(Uint8 *ram, int pat, int x, int y, int wx1, int wy1, int wx2, int wy2, 
  void (*put)(Uint8 *,int,int,int), int (*get)(Uint8 *,int,int,int)) {
 int l=0, x1=0, x2=0, dy=0; 
 int ov=0;
 struct seg stack[MAX_FLOOD], *sp = stack;
 ov = get(ram,pat,x,y);
 if(ov==current_palette_index || x<wx1 || x>wx2 || y<wy1 || y>wy2) return;
 PUSH(y,x,x,1);
 PUSH(y+1,x,x,-1);
 while(sp>stack) {
  POP(y,x1,x2,dy);
  for(x=x1;x>=wx1 && get(ram,pat,x,y) == ov; x--)
   put(ram,pat,x,y);
  if(x>=x1) goto s;
  l = x+1;
  if(l<x1) PUSH(y,l,x1-1, -dy);
  x = x1+1;
  do {
   for(; x<=wx2 && get(ram,pat,x,y)==ov;x++)
    put(ram,pat,x,y);
   PUSH(y,l,x-1,dy);
   if(x>x2+1) PUSH(y,x2+1,x-1,-dy);
s: for(x++;x<=x2 && get(ram,pat,x,y)!=ov;x++);
   l = x;
  } while (x<=x2);
 }
}

void do_line(Uint8 *ram, int pat, int x1, int y1, int x2, int y2, void (*proc)(Uint8 *,int,int,int)) {
 int dx = x2-x1;
 int dy = y2-y1;
 int i1=0, i2=0;
 int x=0, y=0;
 int dd=0;

#define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond) \
{ \
 if (d##pri_c == 0) { \
  proc(ram,pat,x1, y1); \
  return; \
 } \
 i1 = 2 * d##sec_c; \
 dd = i1 - (sec_sign (pri_sign d##pri_c)); \
 i2 = dd - (sec_sign (pri_sign d##pri_c)); \
 x = x1; \
 y = y1; \
 while (pri_c pri_cond pri_c##2) { \
  proc(ram,pat,x,y); \
  if (dd sec_cond 0) { \
   sec_c = sec_c sec_sign 1; \
   dd += i2; \
  } else \
   dd += i1; \
  pri_c = pri_c pri_sign 1; \
  } \
 }

   if (dx >= 0) {
      if (dy >= 0) {
	 if (dx >= dy) {
	    /* (x1 <= x2) && (y1 <= y2) && (dx >= dy) */
	    DO_LINE(+, x, <=, +, y, >=);
	 }
	 else {
	    /* (x1 <= x2) && (y1 <= y2) && (dx < dy) */
	    DO_LINE(+, y, <=, +, x, >=);
	 }
      }
      else {
	 if (dx >= -dy) {
	    /* (x1 <= x2) && (y1 > y2) && (dx >= dy) */
	    DO_LINE(+, x, <=, -, y, <=);
	 }
	 else {
	    /* (x1 <= x2) && (y1 > y2) && (dx < dy) */
	    DO_LINE(-, y, >=, +, x, >=);
	 }
      }
   }
   else {
      if (dy >= 0) {
	 if (-dx >= dy) {
	    /* (x1 > x2) && (y1 <= y2) && (dx >= dy) */
	    DO_LINE(-, x, >=, +, y, >=);
	 }
	 else {
	    /* (x1 > x2) && (y1 <= y2) && (dx < dy) */
	    DO_LINE(+, y, <=, -, x, <=);
	 }
      }
      else {
	 if (-dx >= -dy) {
	    /* (x1 > x2) && (y1 > y2) && (dx >= dy) */
	    DO_LINE(-, x, >=, -, y, <=);
	 }
	 else {
	    /* (x1 > x2) && (y1 > y2) && (dx < dy) */
	    DO_LINE(-, y, >=, -, x, <=);
	 }
      }
   }

   #undef DO_LINE

}
