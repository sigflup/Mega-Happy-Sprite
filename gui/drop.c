#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include <SDL.h>
#include "gui_types.h"
#include "link.h"
#include "drop.h"
#include "gui.h"
#include "draw.h"
#include "std_dialog.h"

int *gauss_fact;
int gauss_sum;
int gauss_width;

int *gauss_coef;

drop_t *drops;

int drop_start;

void tack_line(int offset, int depth) {
 int a=0, b=0;
 int k=0;
 for(k = 0;k< depth; k++) {
   if(  (offset - (depth -1 ) + k) == offset) 
   a = 0;
  else
   a = gauss_coef[offset - (depth-1) + k];
  if( (offset - (depth-1) +k -1) < (offset - (depth-1)))
   b = 0;
  else
   b = gauss_coef[offset - (depth-1)+k-1];
  gauss_coef[k+offset] = a+b;
 } 
}

void build_coef(void) {
 int i=0,j;
 j = 0;
 for(i = 0;i<MAX_DEPTH;i++)
  j+=i;
 gauss_coef = (int *)malloc(j * sizeof(int));
 gauss_coef[0] = 1;

 j = 1;
 for(i = 2; i< MAX_DEPTH;i++) {
  tack_line(j, i);
  j+=i;
 }
}

void drop_init(void) {
 build_coef();
 drops = (drop_t *)malloc(sizeof(drop_t) );
 INIT_LIST_HEAD(&drops->node);
 drop_start = 0;
}

void draw_drop(SDL_Surface *dst, int x, int y, drop_t *drop, int w, int h, SDL_Rect *clip) {
 float dx,dy;
 int ix,iy;
 Uint16 alpha;
 Uint8 r,g,b;
 int sx, sy;
 int j;
 Uint8 *pix;
 int color;
 int mask;
 int *intp;
 int real_w, real_h;
 int offset_x, offset_y;

 /*XXX if drop is bigger then dst this doesn't work right */

 if((x> dst->w) || y > dst->h) return;

 if( (y+h) > dst->h) 
  real_h = dst->h - y;
 else
  real_h = h;

 if( (x+w) >= dst->w)
  real_w = dst->w - x - 1;
 else
  real_w = w;

 if(x < 0) {
  real_w = w + x;
  offset_x = x * -1; 
  x = 0;
 } else
  offset_x = 0;

 if(y < 0) {
  real_h = h + y;
  offset_y = y * -1;
  y = 0;
 } else
  offset_y = 0;

 /* XXX pointer math */
 pix = (Uint8 *)(dst->pixels + 
                 (dst->pitch * y) + (dst->format->BytesPerPixel * x));

 mask = 0;
 for(j=0;j<dst->format->BytesPerPixel;j++)
  mask |= (0xff << (8*j));
#ifdef WORDS_BIGENDIAN
 mask<<=8;
#endif

 mask^=~0;

 LOCK;
 dx = (float)drop->w / (float)w;
 dy = (float)drop->h / (float)h;
 for(iy = 0;iy<real_h;iy++) {
  for(ix =0;ix<real_w;ix++) {
   sx = (int)((float)(ix+offset_x) * dx);
   sy = (int)((float)(iy+offset_y) * dy);
   intp = (int *)pix;
  
   alpha = drop->buffer[sx+(sy*drop->w)];
   color = *intp & (mask^~0);
#ifdef WORDS_BIGENDIAN
   color>>=8;
#endif
   r = g = b= 0;
   SDL_GetRGB((Uint32)color, dst->format, &r, &g, &b);


   color = SDL_MapRGB(dst->format, (r*(alpha))>>8, 
                                   (g*(alpha))>>8,
                                   (b*(alpha))>>8);
#ifdef WORDS_BIGENDIAN
   color<<=8;
#endif
//   color = SDL_MapRGB(dst->format, alpha, 0, 0);
   *intp &= mask;
   *intp += color;
   pix+=dst->format->BytesPerPixel;
  }
  pix-=(dst->format->BytesPerPixel * real_w);
  pix+=dst->pitch;
 }
}

void new_gauss_width(int width) {
 int i,k,w;
 if(width == 0)
  w = 1;
 else 
  w = width;
 gauss_width = w;
 k = 0;
 gauss_sum = 1;
 for(i=0;i<w;i++) {
  gauss_sum*=2;
  k+=i;
 } 
 gauss_sum/=2;
 gauss_fact = (int *)&gauss_coef[k];
}

int blur(Uint8 *src, int w, int h) {
 Uint8 *tmp;
 int k,x,y;
 int sum;
 int index;


 tmp = (Uint8 *)malloc( w * h );
 for(y=0;y<h;y++) 
  for(x=0;x<w;x++) {
   sum = 0;
   for(k=0;k<gauss_width;k++) { 
    index = x - k + (gauss_width/2); //(((gauss_width-1)>>1)+k);


    if(index < w && index > 0) 
     sum+= src[ index + (y*w) ] * gauss_fact[k];
    else 
     sum+=0xff * gauss_fact[k];
   }
   tmp[ x+(y*w)] = sum/gauss_sum; 
  } 

 for(y=0;y<h;y++) 
  for(x=0;x<w;x++) {
   sum = 0;
   for(k=0;k<gauss_width;k++) {
    index = y - k + (gauss_width/2); //  (((gauss_width-1)>>1)+k);
    if(index < h && index > 0) 
     sum+= tmp[ x+ (index*w) ] * gauss_fact[k];
    else
     sum+=0xff * gauss_fact[k];
   }
   src[x+(y*w)]=sum/gauss_sum;
  }
 free(tmp);
 return 0;
}


drop_t *new_drop(int depth) {
 Uint8 *buffer;
 int x,y;
 int actual_w, actual_h;
 int border;
 drop_t *new_walker;

 /* maybe we made this drop before */

 if(drop_start != 0) { 
  new_walker = drops;
  for(;;) {
   if(new_walker->depth == depth) 
    return new_walker;
   new_walker = (drop_t *)new_walker->node.next;
   if(new_walker == (drop_t *)drops)  break;
  }
 }

 new_gauss_width(depth);
 border = gauss_width;
 actual_w = ((gui_screen->w/4)+border);
 actual_h = ((gui_screen->h/4)+border);
 buffer = (Uint8 *)malloc( actual_w * actual_h );

 memset(buffer,0xff, actual_w * (border/2) ); 

 memset(&buffer[actual_w * (border/2)], 0x4f,  actual_w * (actual_h - border));
 

 memset(&buffer[actual_w *(actual_h - (border/2))], 0xff, actual_w*(border/2)); 

 for(y = (border/2); y< actual_h - (border/2);y++ ) 
  for(x = 0;x<(border/2);x++)
   buffer[ y*(actual_w) + x] = 0xff;

 for(y = (border/2); y< actual_h - (border/2);y++ ) 
  for(x = actual_w - (border/2);x<actual_w;x++)
   buffer[ y*(actual_w) + x] = 0xff;


 blur(buffer, actual_w, actual_h);

 /* add to list of drops already made */
 
 if(drop_start == 0)  
  new_walker = drops;
 else
  new_walker = (drop_t *)malloc(sizeof(drop_t));

 new_walker->buffer = buffer;
 new_walker->w = actual_w;
 new_walker->h = actual_h;

 if(drop_start == 0) 
  drop_start++;
 else
  list_add(&new_walker->node, &drops->node);
 return new_walker;
}
