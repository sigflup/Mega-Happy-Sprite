/*
-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

  http://pgp.mit.edu:11371/pks/lookup?op=get&search=0xA6E70B3
  m m mm mmm .----------.  .---------------------. mmm mm m m
  8 8 88 888 | .--------`  |  .------------------` 888 88 8 8
  8 8 88 888 | ```````|`V```````|   |``||``|`````| 888 88 8 8
  8 8 88 888 `------  | |  [] | |``````||  |  [] | 888 88 8 8
  8 8 88 888 |``````  | |     | ````|````  |     | 888 88 8 8
  ` ` `` ``` ``````````````>  |````````````|   |`` ``` `` ` `
                ==============`            `---`
                                 L A B O R A T O R I E S
   
    Good hello, this is official SigFLUP-Labs sourcecode :-()

 This is GNU software, please read ../LICENSE for details
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include <math.h>
#include <SDL.h>
#include "gui_types.h"
#include "link.h"
#include "drop.h"
#include "gui.h"
#include "draw.h"
#include "std_dialog.h"

SDL_Surface *gc;

void clipped_update(int x, int y, int w, int h) {
 int pw=0,ph=0;

 if((x+y+w+h) == 0) 
  SDL_UpdateRect(gc, 0,0,0,0);
 else {

  pw = x+w;
  ph = y+h;

  if(x < 0 && pw > gc->w) {
   pw = gc->w;
   x = 0;
   SDL_UpdateRect(gc, 0,0,0,0);
   return;
  } else {
   if(pw > gc->w) {
    if( x > gc->w) return;
    else
     pw = gc->w - x;
   } else
    pw = w;
   if(x < 0) x = 0;
  }
  if(y<0 && ph > gc->h) {
   ph = gc->h;
   y = 0;
   SDL_UpdateRect(gc, 0,0,0,0);
   return;
  } else {
   if(ph > gc->h) {
    if( y > gc->h) return;
    else
     ph = gc->h - y;
   } else
    ph = h;
   if(y < 0) y = 0;
   SDL_UpdateRect(gc,x,y, pw,ph);
  }
 }
}

int vline(int rx, int ry, int ry2, color_t *fg, color_t *bg, int type) {
 int x=0,y=0, y2=0, i=0;
 int hash_start=0;
 Uint32 mask=0;
 union {
  Uint8  *b;
  Uint32 *l;
 } pix;

 if( (rx+ current_grp->pos_x) < 0) 
  return; 
 if( (rx + current_grp->pos_x) >= gc->w)
  return;
 else
  x = rx;
 if( (ry+ current_grp->pos_y) < 0) 
  y = current_grp->pos_y * -1;
 else
  y = ry;
 if((ry2+ current_grp->pos_y) > gc->h) 
  y2 = (gc->h - current_grp->pos_y)-1;
 else
  y2 = ry2;

 pix.b = (Uint8 *)((int)gc->pixels + 
                   (gc->pitch * (y+current_grp->pos_y)) + 
		   (gc->format->BytesPerPixel * (x+current_grp->pos_x)));

 mask = gc->format->Rmask | 
        gc->format->Gmask |
	gc->format->Bmask |
	gc->format->Amask;
 LOCK;

 switch(type) {
  case XOR:
   for(i = y;i<y2;i++)  {
    *pix.l^=mask;
    pix.b+=gc->pitch;
   }
   break;
  case HASH:
   hash_start = (x^y)&1;
   mask^=~0;
 
   for(i = y;i<y2;i++) {
    *pix.l&=mask;
    *pix.l|= (hash_start == 1 ? fg->map : bg->map);
    hash_start^=1;
    pix.b+=gc->pitch;  
   }
   break;
  case NO_HASH:
   mask^=~0;

   for(i=y;i<y2;i++){
    *pix.l&=mask;
    *pix.l|=fg->map;
    pix.b+=gc->pitch;
   }
   break;
 }

 UNLOCK;
 globl_dirt = 1;
}

int hline(int rx, int ry, int rx2, color_t *fg, color_t *bg, int type) {
 int x,y,x2, i;
 int hash_start;
 Uint32 mask;
 union {
  Uint8 *b;
  Uint32 *l;
 } pix;

 if( (rx+ current_grp->pos_x) < 0 ) 
  x = current_grp->pos_x * -1;
 else
  x = rx;

 if( (ry+ current_grp->pos_y) < 0 ) 
  return;
 
 if( (ry+current_grp->pos_y) >= gc->h) 
   return;
 else
  y = ry;

 if((rx2+current_grp->pos_x) > gc->w) 
  x2 = (gc->w - current_grp->pos_x)-1;
 else 
  x2 = rx2;

 pix.b = (Uint8 *)((int)gc->pixels+
                   (gc->pitch * (y+current_grp->pos_y)) +
		   (gc->format->BytesPerPixel * (x+current_grp->pos_x)));

 mask = gc->format->Rmask |
        gc->format->Gmask |
	gc->format->Bmask |
	gc->format->Amask;

 LOCK;
 switch(type) {
  case XOR:
   if((y == (gc->h-1)) && (x2 > (gc->w-1))) 
    x2--; 
   for(i = x;i<x2;i++)  {
    *pix.l^=mask;
    pix.b+=gc->format->BytesPerPixel;
   }
   break;
  case HASH:
   hash_start = (x^y)&1;
   mask^=~0;
   if((y == (gc->h-1)) && (x2 > (gc->w-1))) 
    x2--;
   for(i = x;i<x2;i++) {
    hash_start^=1;
    *pix.l&=mask; 
    *pix.l|=(hash_start == 1 ? fg->map : bg->map);
    pix.b+=gc->format->BytesPerPixel;  
   }
   break;
  case NO_HASH:
   mask^=~0;
   if((y == (gc->h-1)) && (x2 > (gc->w-1))) 
    x2--; 
   for(i=x;i<x2;i++){
    *pix.l&=mask;
    *pix.l|=fg->map;
    pix.b+=gc->format->BytesPerPixel;
   }
   break;
 }
 UNLOCK;
 globl_dirt = 1;
}


int fill_box(int x, int y, int x2, int y2,color_t *fg,color_t *bg,int type){
 int rx, ry, rx2, ry2, i;
 int s,t,j, hash_start;
 Uint32 mask;
 union {
  Uint8  *b;
  Uint32 *l;
 } pix;

 if((x+ current_grp->pos_x) >= gc->w)
  return;

 if((y+ current_grp->pos_y) >= gc->h)
  return;

 if((x+ current_grp->pos_x) < 0)  
  rx = current_grp->pos_x * -1;
 else
  rx = x;

 if((y+ current_grp->pos_y) < 0) 
  ry = current_grp->pos_y * -1;
 else
  ry = y;

 if((x2+current_grp->pos_x) > gc->w) 
  rx2 = (gc->w - current_grp->pos_x) -1;
 else 
  rx2 = x2;

 if((y2+ current_grp->pos_y) > gc->h) 
  ry2 = (gc->h - current_grp->pos_y)-1;
 else
  ry2 = y2;

 pix.b = (Uint8 *)((int)gc->pixels +
                   (gc->pitch * (ry+current_grp->pos_y)) +
		   (gc->format->BytesPerPixel * (rx+current_grp->pos_x)));

 mask = gc->format->Rmask |
        gc->format->Gmask |
	gc->format->Bmask |
	gc->format->Amask;
 LOCK;
 switch(type) {
  case XOR:
   if((ry2>(gc->h-1)) && (rx2 > (gc->w-1))) {
    rx2--;
    ry2--;
   }
   for(t=ry;t<ry2;t++) {
    for(s=rx;s<rx2;s++) {
     *pix.l^=mask;
     pix.b+=gc->format->BytesPerPixel;
    }
    pix.b-=gc->format->BytesPerPixel * (rx2-rx);
    pix.b+=gc->pitch;
   }
   break;
  case HASH:
   mask^=~0;
   if((ry2 > (gc->h-1)) && (rx2>(gc->w-1))) {
    rx2--;
    ry2--;
   }

   for(t=ry;t<ry2;t++) {
    for(s=rx;s<rx2;s++) {
     hash_start= ((rx+s)^(ry+t))&1;
     *pix.l&=mask;
     *pix.l|=(hash_start == 1 ? fg->map : bg->map);
     pix.b+=gc->format->BytesPerPixel; 
    }
    pix.b-=gc->format->BytesPerPixel * (rx2-rx);
    pix.b+=gc->pitch;
   }
   break; 
  case NO_HASH:
   mask^=~0;
   if((ry2 > (gc->h-1)) && (rx2 > (gc->w-1))) {
    rx2--;
    ry2--;
   } 

   for(t=ry;t<ry2;t++) {
    for(s=rx;s<rx2;s++) {
     *pix.l&=mask;
     *pix.l|=fg->map;
     pix.b+=gc->format->BytesPerPixel;
    }
    pix.b-=gc->format->BytesPerPixel * (rx2-rx);
    pix.b+=gc->pitch;
   }
   break;
 }
 UNLOCK;
 globl_dirt = 1;
}

int box(int x, int y, int x2, int y2, color_t *fg, color_t *bg, int type) {
 vline(x,y, y2, fg, bg,   type);
 vline(x2,y,y2, fg, bg,   type);
 hline(x,y, x2, fg, bg,   type);
 hline(x,y2,x2+1, fg, bg, type);
}


/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj4cACgkQMNO4A6bnBrMXPACgl32KVFEiMmz6KJzaW14kMv2H
a1QAn2B/h4WxlVR8Bgtn8esDT+qzjeNl
=snKc
-----END PGP SIGNATURE-----
*/
