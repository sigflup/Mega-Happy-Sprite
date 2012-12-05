#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <SDL.h>
#include <libgui.h>
#include <math.h>
#include "config.h"
#include "mega.h"
#include "vdp.h"
#include "draw.h"
#include "proc.h"
#include "bottom.h"
#include "uu.h"

#include "bmp.h"


int load_bmp(struct select_file_t *selector, char *filename) {
 SDL_Surface *bmp;
 int i,x,y;
 int r,g,b;
 unsigned char *pix;
 if((bmp = SDL_LoadBMP(filename))<=0) 
  return NOPE_TRY_AGAIN;

 if(bmp->format->BytesPerPixel != 1)  
  return NOPE_TRY_AGAIN;

 if(bmp->format->palette->ncolors > 15)
  return NOPE_TRY_AGAIN;

 if(bmp->w > 320)
  return NOPE_TRY_AGAIN;
 
 if(bmp->h > 224)
  return NOPE_TRY_AGAIN;

 for(i=0;i<15;i++) {
  r = bmp->format->palette->colors[i].r / (0xff/7);
  r*= (0xff/7);

  g = bmp->format->palette->colors[i].g / (0xff/7);
  g*= (0xff/7);

  b = bmp->format->palette->colors[i].b / (0xff/7);
  b*= (0xff/7);

  current_vdp->palette[current_palette][i+1].r = r;
  current_vdp->palette[current_palette][i+1].g = g;
  current_vdp->palette[current_palette][i+1].b = b;
  MAP_COLOR(current_vdp->palette[current_palette][i]);
 } 


 pix = (unsigned char *)bmp->pixels;
 num_xor_pix = bmp->w * bmp->h;
 i =0;
 for(y=0;y<bmp->h;y++)
  for(x=0;x<bmp->w;x++) {
   xor_pixels[i].x = x;
   xor_pixels[i].y = y;
   xor_pixels[i].index = pix[x+(y*bmp->pitch)]+1;
   i++;
  }

 collapse();
  
 
 return LOAD_OK_QUIT;
}
