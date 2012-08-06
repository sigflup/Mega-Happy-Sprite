#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "gui_types.h"
#include "link.h"
#include "drop.h"
#include "gui.h"
#include "draw.h"
#include "std_dialog.h"
#include "font.h"
#include "knob.xpm"
#include "knob_point.xpm"
#include "bknob.xpm"
#include "bknob_point.xpm"

int proc_knob(int msg, struct object_t *obj, int data) {
 SDL_Rect dst;
 float theta;
 char buf[10];
 float scale;
 int x,j;
 int y;
 int px, py;
 int rx, ry;
 int ret;
 int lx;
 ret = RET_OK;
 switch(msg) {
  case MSG_START:
   obj->param.w = 41;
   obj->param.h = 51;
   if(CHECK_FLAG(obj->param.flags, BLUE) == TRUE) {
    obj->param.dp1 = (void *)IMG_ReadXPMFromArray(bknob_xpm);
    obj->param.dp2 = (void *)IMG_ReadXPMFromArray(bknob_point_xpm);
   } else {
    obj->param.dp1 = (void *)IMG_ReadXPMFromArray(knob_xpm);
    obj->param.dp2 = (void *)IMG_ReadXPMFromArray(knob_point_xpm);

   }
   break;
  case MSG_DRAW:
   dst.x = obj->param.x + current_grp->pos_x;
   dst.y = obj->param.y + current_grp->pos_y;
   fill_box(obj->param.x, obj->param.y+42, 
            obj->param.x+obj->param.w, obj->param.y+obj->param.h, 
	    obj->param.bg, obj->param.bg, NO_HASH);
   if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) {
    snprintf(buf, 10, ":C");
   } else {
    if(CHECK_FLAG(obj->param.flags, HEX) == TRUE)
     snprintf(buf, 10, "%x", obj->param.d1);
    else
     snprintf(buf, 10, "%d", obj->param.d1);
   }
   draw_text(obj->param.x + (obj->param.w/2) - ((strlen(buf)*8)/2), obj->param.y+42,
             buf,obj->param.fg, obj->param.bg, NO_HASH,0);


   SDL_BlitSurface( (SDL_Surface *)obj->param.dp1,NULL, gui_screen, &dst);
   if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) break;
   theta = ((float)(obj->param.d2 - obj->param.d1) / (float)obj->param.d2)*(2*M_PI);
   px = 0;
   py = 13;
   rx = (cos(theta)*px+ sin(theta)*py )+20+obj->param.x+current_grp->pos_x;
   ry = (cos(theta)*py- sin(theta)*px )+20+obj->param.y+current_grp->pos_y;


   dst.x = rx - 5;
   dst.y = ry - 5;
   SDL_BlitSurface( (SDL_Surface *)obj->param.dp2, NULL, gui_screen, &dst);

  
   break;
  case MSG_CLICK:
   if((CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) &&
      (msg != MSG_START)) break;
   j = obj->param.d1;
   if(obj->param.d2 > gui_screen->w)
    scale = 1.0f; 
   else
    scale = (float)obj->param.d2 / (float)gui_screen->w;

   x = gui_mouse_x;
   y = gui_mouse_y;

   for(;;) {
    lx = obj->param.d1;
   
    obj->param.d1 = j - (int)((float)((x -gui_mouse_x) + ((y-gui_mouse_y)*3))*scale);


    if(CHECK_FLAG(obj->param.flags, MODULAR) == TRUE) {
     while(obj->param.d1<0)
      obj->param.d1+=obj->param.d2;
     obj->param.d1 %= obj->param.d2;
    } else {
     if(obj->param.d1 < 0) 
      obj->param.d1 = 0;
     if(obj->param.d1 > obj->param.d2) 
      obj->param.d1 = obj->param.d2;
    }
    if(lx != obj->param.d1) {
     MESSAGE_OBJECT(obj, MSG_DRAW);
     UPDATE_OBJECT(obj);
     if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
      ret =  obj->param.callback( obj, 0);
      if(ret != RET_OK) return ret;
     }
    }
    if(wait_on_mouse() == MOUSE_UP) break;
   }

   break;
  case MSG_KEYDOWN:
   if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) break;
   if(data == SDLK_UP)
    if(obj->param.d1 != obj->param.d2) obj->param.d1++;
   if(data == SDLK_DOWN)
    if(obj->param.d1 != 0) obj->param.d1--;
   MESSAGE_OBJECT(obj, MSG_DRAW); 
   if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
    ret =  obj->param.callback( obj, 0);
    if(ret != RET_OK) return ret;
   }

   break;
  case MSG_DESTROY:
   SDL_FreeSurface((SDL_Surface *)obj->param.dp1);
   SDL_FreeSurface((SDL_Surface *)obj->param.dp2);
   break;
 }
 return ret;
}

int proc_bitmap(int msg, struct object_t *obj, int data) {
 SDL_Rect des;
 SDL_Surface *bmp;
 struct object_t *walker;
 drop_t *tmp_drop;
 if((CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) &&
    (msg != MSG_START)) return RET_OK;
 switch(msg) {
  case MSG_RELOAD:
   obj->param.flags &= (LOAD_XPM_FROM_ARRAY ^ ~0);
   if(obj->param.dp2)
    SDL_FreeSurface(obj->param.dp2);
  case MSG_START:
   if(CHECK_FLAG(obj->param.flags,LOAD_XPM_FROM_ARRAY) == TRUE)
    bmp = IMG_ReadXPMFromArray(obj->param.dp1);
   else
    bmp = IMG_Load((char *)obj->param.dp1);

   if(!bmp) {
    obj->param.flags |= INVALID;
    obj->param.dp2 = NULL;
    return RET_OK;
   }
   obj->param.flags &= (INVALID ^ ~0);
   obj->param.dp2 = (void *)bmp;
   obj->param.w = bmp->w;
   obj->param.h = bmp->h;
   if(CHECK_FLAG(obj->param.flags,DROP_ACCUM) == TRUE)  {
    walker = current_grp->objs;
    for(;;) {
     if(CHECK_FLAG(walker->param.flags, DROP_SHADOW)== TRUE) { 
      tmp_drop = new_drop(globl_drop_depth);
      draw_drop(bmp,walker->param.x, walker->param.y,tmp_drop, 
       walker->param.w+(walker->param.w/4),walker->param.h+(walker->param.h/4), NULL);	
     }
     walker = (struct object_t *)walker->node.next;
     if(walker == (struct object_t *)current_grp->objs) break;
    } 
   }
  break;
  case MSG_DRAW:
   if(obj->param.dp2 == NULL) return RET_OK;
   bmp = (SDL_Surface *)obj->param.dp2;

   des.x = obj->param.x + current_grp->pos_x;
   des.y = obj->param.y + current_grp->pos_y;

   SDL_BlitSurface( (SDL_Surface *)obj->param.dp2,NULL, gui_screen, &des);
  globl_dirt = 1;
  break;

 }
 return RET_OK;
 
}

int proc_icon_button(int msg, struct object_t *obj, int data) {
 SDL_Rect *src, des;
 SDL_Surface *bmp;
 int active_before;
 if((CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) &&
    (msg != MSG_START)) return RET_OK;

 switch(msg) {
  case MSG_START:
   if(CHECK_FLAG(obj->param.flags,LOAD_XPM_FROM_ARRAY) == TRUE) 
     obj->param.dp2 = (void *)IMG_ReadXPMFromArray(obj->param.dp1);
    else
     obj->param.dp2 = (void *)IMG_Load((char *)obj->param.dp1);


   if( obj->param.dp2  == NULL ) {
    obj->param.flags |= INVALID;
    printf("can't open %s\n", (char *)obj->param.dp1);
    exit(-1);
   }
   bmp = (SDL_Surface *)obj->param.dp2;
   obj->param.w = bmp->w+1;
   obj->param.h = bmp->h+1;
   obj->param.dp3 = (void *)malloc( sizeof(SDL_Rect) );
   src = (SDL_Rect *)obj->param.dp3;
   src->w = bmp->w;
   src->h = bmp->h;
   src->x = 0;
   src->y = 0; 
  break;
  case MSG_DRAW:
   if( CHECK_FLAG(obj->param.flags, INVALID)== TRUE) return RET_OK;
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE && obj->in_focus == TRUE) 
    box(obj->param.x, obj->param.y, 
        obj->param.x+obj->param.w, 
	obj->param.y+obj->param.h, obj->param.fg, obj->param.bg,HASH);
   else
    box(obj->param.x, obj->param.y, 
        obj->param.x+obj->param.w, 
	obj->param.y+obj->param.h, obj->param.bg, obj->param.bg,NO_HASH);

   des.x = obj->param.x + current_grp->pos_x+1;
   des.y = obj->param.y + current_grp->pos_y+1;
   src = (SDL_Rect *)obj->param.dp3;
   bmp = (SDL_Surface *)obj->param.dp2;

   if(obj->param.d1 == 1) {
    des.x+=2;
    des.y+=2;
    src->w=bmp->w-2;
    src->h=bmp->h-2;
    hline(obj->param.x+1,obj->param.y+1, obj->param.x+obj->param.w+1,
      obj->param.bg, obj->param.bg, NO_HASH);
    hline(obj->param.x+1,obj->param.y+2, obj->param.x+obj->param.w+1,
      obj->param.bg, obj->param.bg, NO_HASH);
    vline(obj->param.x+1,obj->param.y+2, obj->param.y+obj->param.h,
      obj->param.bg, obj->param.bg, NO_HASH);
    vline(obj->param.x+2,obj->param.y+2, obj->param.y+obj->param.h,
      obj->param.bg, obj->param.bg, NO_HASH);
   } else {
    src->w = bmp->w;
    src->h = bmp->h;
   }

   SDL_BlitSurface( (SDL_Surface *)obj->param.dp2,src, gui_screen, &des);
  globl_dirt = 1;
  break;

  case MSG_CLEAR_INTERNAL2:
   if(CHECK_FLAG(obj->param.flags, TOGGLE) == TRUE) break;
   if(CHECK_FLAG(obj->param.flags, SINGLE_RADIO) == TRUE) break;
   if( data == obj->param.d2)
    obj->param.flags &= (~0^INTERNAL2);
   break;
  case MSG_RADIO2: 
   if(CHECK_FLAG(obj->param.flags, TOGGLE) == TRUE) break;
   if(CHECK_FLAG(obj->param.flags, SINGLE_RADIO) == TRUE) break;
   if( data == obj->param.d2) {
    if( CHECK_FLAG(obj->param.flags, INTERNAL2) == FALSE) 
     obj->param.d1 = FALSE;
    else 
     obj->param.d1 = TRUE;
    proc_icon_button(MSG_DRAW, obj, 0);
   }
   break;
  case MSG_CLICK:
   if(CHECK_FLAG(obj->param.flags, SINGLE_RADIO) == TRUE) {
    if(obj->param.d1 == TRUE) 
     obj->param.d1 = FALSE;
    else
     obj->param.d1 = TRUE;
    proc_icon_button(MSG_DRAW,obj, 0);
    if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE)
     return obj->param.callback(obj, 0);
    break;
   }

   if(CHECK_FLAG(obj->param.flags, TOGGLE) == TRUE) { 
    obj->param.d1 ^= 1;
    proc_icon_button(MSG_DRAW, obj, 0);
    if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE)
    return obj->param.callback(obj,0);
    break;
   }
   active_before = obj->param.d1;
   broadcast_group(current_grp, MSG_CLEAR_INTERNAL2, obj->param.d2);
   obj->param.flags |= INTERNAL2;
   broadcast_group(current_grp, MSG_RADIO2, obj->param.d2);
   if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE &&
     active_before == 0) 
   return obj->param.callback(obj, 0);
   break;
  case MSG_INFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_icon_button(MSG_DRAW, obj, 0);
   break;
  case MSG_OUTFOCUS:
   obj->clicked = FALSE;
   proc_icon_button(MSG_DRAW, obj, 0);
   break;

 }
 return RET_OK;
}

int proc_radio_button(int msg, struct object_t *obj, int data) {
 int active_before;

 if((CHECK_FLAG(obj->param.flags, INACTIVE)) == TRUE &&
    (msg != MSG_START)) return RET_OK; 
 
 switch(msg) {
  case MSG_START:
   obj->param.w = (strlen((char *)obj->param.dp1) * 8) + 20;
   obj->param.h = 12;
   break;
  case MSG_DRAW:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE && obj->in_focus == TRUE) 
    box(obj->param.x, obj->param.y, 
        obj->param.x+obj->param.w, 
	obj->param.y+obj->param.h, obj->param.fg, obj->param.bg,HASH);
   else
    box(obj->param.x, obj->param.y, 
        obj->param.x+obj->param.w, 
	obj->param.y+obj->param.h, obj->param.bg, obj->param.bg,NO_HASH);

   draw_text(obj->param.x+1, obj->param.y+3, "X", obj->param.fg, obj->param.bg, 
     (obj->param.d1 == TRUE ? NO_HASH : HASH),0 );
   vline(obj->param.x+12, obj->param.y+3, obj->param.y+11, obj->param.fg, obj->param.bg, NO_HASH); 
   draw_text(obj->param.x+15, obj->param.y+3,
     (char *)obj->param.dp1,obj->param.fg,obj->param.bg,NO_HASH,0);
   break;
  case MSG_CLEAR_INTERNAL1:
   if(CHECK_FLAG(obj->param.flags, SINGLE_RADIO)== TRUE) break;
   if( data == obj->param.d2)
    obj->param.flags &= (~0^INTERNAL1);
   break;
  case MSG_RADIO: 
   if(CHECK_FLAG(obj->param.flags, SINGLE_RADIO)== TRUE) break;

   if( data == obj->param.d2) {
    if( CHECK_FLAG(obj->param.flags, INTERNAL1) == FALSE) 
     obj->param.d1 = FALSE;
    else 
     obj->param.d1 = TRUE;
    proc_radio_button(MSG_DRAW, obj, 0);
   }
   break;
  case MSG_CLICK:
   if(CHECK_FLAG(obj->param.flags, SINGLE_RADIO) == TRUE) {
    if(obj->param.d1 == TRUE) 
     obj->param.d1 = FALSE;
    else
     obj->param.d1 = TRUE;
    proc_radio_button(MSG_DRAW,obj, 0);
    if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE)
     return obj->param.callback(obj, 0);
    break;
   }
   active_before = obj->param.d1;
   broadcast_group(current_grp, MSG_CLEAR_INTERNAL1, obj->param.d2);
   obj->param.flags |= INTERNAL1;
   broadcast_group(current_grp, MSG_RADIO, obj->param.d2);
   if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE &&
     active_before == 0) 
   return obj->param.callback(obj, 0);
   break;
  case MSG_INFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_radio_button(MSG_DRAW, obj, 0);
   break;
  case MSG_OUTFOCUS:
   obj->clicked = FALSE;
   proc_radio_button(MSG_DRAW, obj, 0);
   break;

 }

 return RET_OK;
}

int proc_move_button(int msg, struct object_t *obj, int data) {
 int x,y;
 int px, py;
 int clip_x1, clip_y1, clip_x2, clip_y2;
 int CLIP_x1, CLIP_y1, CLIP_x2, CLIP_y2;


 if((CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) &&
    (msg != MSG_START)) return RET_OK;

 switch(msg) {
  case MSG_DRAW:
   box(obj->param.x, obj->param.y, obj->param.x+ obj->param.w, obj->param.y+obj->param.h,
    obj->param.fg, obj->param.bg, NO_HASH);
   fill_box(obj->param.x + 1, obj->param.y+1,
            obj->param.x+obj->param.w -1, obj->param.y+obj->param.h-1,
	    obj->param.fg, obj->param.bg, HASH); 
   break;
  case MSG_CLICK:
   x = gui_mouse_x;
   y = gui_mouse_y;
   px = 0;
   py = 0;
   lock_update = 1;
   for(;;) {
    clip_x1 = px;
    clip_y1 = py;
    fill_box(clip_x1, clip_y1, 
             clip_x1+current_grp->w, clip_y1+current_grp->h,obj->param.fg, obj->param.bg, XOR);
    if(wait_on_mouse() == MOUSE_UP) break;
    px = gui_mouse_x-x;
    py = gui_mouse_y-y;
    clip_x2 = px;
    clip_y2 = py;
    fill_box(px,py, current_grp->w+px, current_grp->h+py,obj->param.fg, obj->param.bg, XOR);

 // XXX make this cleaner, redundant ops
   if(clip_x1 < clip_x2) {
     CLIP_x1 = clip_x1 + current_grp->pos_x;
     CLIP_x2 = clip_x2 + current_grp->w +current_grp->pos_x;
     CLIP_x2 -= CLIP_x1;
    } else {
     CLIP_x1 = clip_x2 + current_grp->pos_x;
     CLIP_x2 = clip_x1 + current_grp->w +current_grp->pos_x;
     CLIP_x2 -= CLIP_x1;
    }
    if(clip_y1 < clip_y2) {
     CLIP_y1 = clip_y1 + current_grp->pos_y;
     CLIP_y2 = clip_y2 + current_grp->h + current_grp->pos_y;
     CLIP_y2 -= CLIP_y1;
    } else {
     CLIP_y1 = clip_y2 + current_grp->pos_y;
     CLIP_y2 = clip_y1 + current_grp->h + current_grp->pos_y;
     CLIP_y2 -= CLIP_y1;
    }
    clipped_update(CLIP_x1, CLIP_y1, CLIP_x2, CLIP_y2);
   }
   lock_update = 0;
   obj->param.d1 = current_grp->pos_x;
   obj->param.d2 = current_grp->pos_y;
   current_grp->pos_x += px;
   current_grp->pos_y += py;
   /*XXX see BUGS 3 */
   if(current_grp->pos_y < 0)
    current_grp->pos_y = 0;
   globl_quit_value = MOVE_GROUP;
   return RET_QUIT;
   break;
 }
 return RET_OK;
}

int proc_text(int msg, struct object_t *obj, int data) {
 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

 if(msg == MSG_DRAW) 
  draw_text(obj->param.x,obj->param.y,
    (char *)obj->param.dp1,obj->param.fg,obj->param.bg,NO_HASH|CR_TERMINAL, 0 );
 return RET_OK;
}

int proc_ctext(int msg, struct object_t *obj, int data) {
 int flag;
 int len;
 flag = NO_HASH;
 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

 if(msg == MSG_DRAW) {
  if( CHECK_FLAG(obj->param.flags, MAX_CHARS) == TRUE) {
   flag|=MAX_CHARS; 
   len = strlen((char *)obj->param.dp1);
   if(len > obj->param.w)
    len = obj->param.w;
  } else
   len = strlen((char *)obj->param.dp1);
  draw_text(obj->param.x - ((len*8)/2), 
            obj->param.y - 4, (char *)obj->param.dp1, obj->param.fg, obj->param.bg, 
	    flag,obj->param.w);
 }
 return RET_OK;
}

int proc_button_box(int msg, struct object_t *obj, int data) {
 color_t *fg, *bg;
 
 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

 switch(msg) {
  case MSG_DRAW:
   box(obj->param.x, obj->param.y, obj->param.x+obj->param.w,obj->param.y+obj->param.h,
     obj->param.fg, obj->param.bg, NO_HASH);
   vline(obj->param.x + obj->param.w+1, obj->param.y+1, 
         obj->param.y + obj->param.h+1, obj->param.fg, obj->param.bg, NO_HASH);
   hline(obj->param.x +1, obj->param.y + obj->param.h+1, 
         obj->param.x + obj->param.w+2, obj->param.fg, obj->param.bg, NO_HASH);
  
   if(obj->clicked == TRUE) {
    fg = obj->param.bg;
    bg = obj->param.fg;
   } else {
    fg = obj->param.fg;
    bg = obj->param.bg;
   }

   fill_box( obj->param.x + 1, obj->param.y+1,
   	      obj->param.x + obj->param.w,
   	      obj->param.y + obj->param.h, bg, fg, NO_HASH); 
   draw_text(obj->param.x+(obj->param.w/2)-CENTER_OF_STRING((char *)obj->param.dp1),
      obj->param.y+(obj->param.h/2)-4,
      (char *)obj->param.dp1, fg, bg, NO_HASH,0);

  if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE &&
      obj->in_focus == TRUE) 
    box(obj->param.x+2, obj->param.y+2, obj->param.x+obj->param.w -2, obj->param.y+obj->param.h-2,
        obj->param.fg, obj->param.bg, HASH);
  break; 
  case MSG_INFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_button_box(MSG_DRAW, obj, 0);
   break;
  case MSG_OUTFOCUS:
   obj->clicked = FALSE;
   proc_button_box(MSG_DRAW, obj, 0);
   break;
  case MSG_CLICK:
   proc_button_box(MSG_DRAW,obj, 0);
   break;
  case MSG_UNCLICK:
   proc_button_box(MSG_DRAW,obj, 0);
   break;
  case MSG_PRESS:
   if(CHECK_FLAG(obj->param.flags, QUIT_BUTTON) == TRUE) {
    globl_quit_value = obj->param.quit_value;
    return RET_QUIT;
   }
   if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
    return obj->param.callback(obj, 0);
   }
   break;
 }
 return RET_OK;
}


int proc_box(int msg, struct object_t *obj, int data) {
 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

 if(msg == MSG_DRAW)
  box(obj->param.x, obj->param.y, obj->param.x+obj->param.w,obj->param.y+obj->param.h,
    obj->param.fg, obj->param.bg, NO_HASH);
 return RET_OK;
}

int proc_shadow_box(int msg, struct object_t *obj, int data) {
 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

 switch(msg) {
  case MSG_DRAW:
   box(obj->param.x, obj->param.y, obj->param.x+obj->param.w,obj->param.y+obj->param.h,
     obj->param.fg, obj->param.bg, NO_HASH);
   vline(obj->param.x + obj->param.w+1, obj->param.y+1, 
         obj->param.y + obj->param.h+1, obj->param.fg, obj->param.bg, NO_HASH);
   hline(obj->param.x +1, obj->param.y + obj->param.h+1, 
         obj->param.x + obj->param.w+2, obj->param.fg, obj->param.bg, NO_HASH);
   fill_box( obj->param.x + 1, obj->param.y+1,
  	     obj->param.x + obj->param.w,
  	     obj->param.y + obj->param.h, obj->param.bg, obj->param.bg, NO_HASH);
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE &&
      obj->in_focus == TRUE) 
    box(obj->param.x+2, obj->param.y+2, obj->param.x+obj->param.w -2, obj->param.y+obj->param.h-2,
        obj->param.fg, obj->param.bg, HASH);
  break; 
  case MSG_INFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_shadow_box(MSG_DRAW, obj, 0);
   break;
  case MSG_OUTFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_shadow_box(MSG_DRAW, obj, 0);
   break;
 }
 return RET_OK;
}

/* Scroll bar, parameters:
 * d1 = current position
 * d2 = limit
 */
#define BOX_SIZE	11	

int proc_scroll_bar(int msg, struct object_t *obj, int data) {
 color_t half_color;
 float real_x, real_y; 
 int ret;
 int dec_key1, dec_key2;
 int inc_key1, inc_key2;
 int px, py, x, y, d, ld;
 if((CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) && 
    (msg != MSG_START)) return RET_OK;

 switch(msg) {
  case MSG_START:
   if( obj->param.w > obj->param.h) 
    obj->param.h = 11; // horizontal scroll bar, height always 11
   else
    obj->param.w = 11;
   break;
  case MSG_DRAW:
   SET_COLOR(half_color, obj->param.fg->r,
                         obj->param.fg->g,
			 obj->param.fg->b);

   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    if(obj->in_focus == FALSE) {
     SET_COLOR(half_color, half_color.r/2,
                           half_color.g/2,
			   half_color.b/2);
   }

   fill_box(obj->param.x, obj->param.y, 
     obj->param.w+obj->param.x, obj->param.h + obj->param.y, &half_color, obj->param.bg, HASH);

   if( obj->param.h == 11) {
    /* horizontal 
     * Why floats, incase d2 > w */
    real_x = (float)
     (((float)((obj->param.w-BOX_SIZE)/(float)obj->param.d2)*obj->param.d1)+obj->param.x);
    real_y = (float)obj->param.y;
   } else {
    /* vertical */
    real_x = (float)obj->param.x;
    real_y = (float)
     (((float)((obj->param.h-BOX_SIZE)/(float)obj->param.d2)*obj->param.d1)+obj->param.y);

   }

   /* Kinda looks like a burger, doesn't it. That's what we'll call it */
   fill_box((int)real_x,(int)real_y,(int)real_x+11,(int)real_y+11,
     obj->param.fg, obj->param.bg, NO_HASH);
   vline( (int)real_x + 2, (int)real_y + 2, (int)real_y + 9, obj->param.bg, obj->param.bg, NO_HASH);
   vline( (int)real_x + 8, (int)real_y + 2, (int)real_y + 9, obj->param.bg, obj->param.bg, NO_HASH);
   vline( (int)real_x + 4, (int)real_y + 2, (int)real_y + 9, obj->param.bg, obj->param.bg, NO_HASH);
   vline( (int)real_x + 6, (int)real_y + 2, (int)real_y + 9, obj->param.bg, obj->param.bg, NO_HASH);


   break;
  case MSG_CLICK:
   if( obj->param.h == 11) {
    /* horizontal 
     * Why floats, incase d2 > w */
    real_x = (float)
     (((float)((obj->param.w-BOX_SIZE)/(float)obj->param.d2)*obj->param.d1)+obj->param.x);
    real_y = (float)obj->param.y;
   } else {
    /* vertical */
    real_x = (float)obj->param.x;
    real_y = (float)
     (((float)((obj->param.h-BOX_SIZE)/(float)obj->param.d2)*obj->param.d1)+obj->param.y);
   }
   if(gui_mouse_x > (int)real_x+ current_grp->pos_x      &&
      gui_mouse_x < ((int)real_x+ current_grp->pos_x+11) &&
      gui_mouse_y > (int)real_y+ current_grp->pos_y      &&
      gui_mouse_y < ((int)real_y+ current_grp->pos_y+11))  {
    /* inside scrollbar */
    x = gui_mouse_x;
    y = gui_mouse_y;
    d = obj->param.d1;
    for(;;) {
     ld = obj->param.d1;
     if(wait_on_mouse() == MOUSE_UP) break;
     px = gui_mouse_x - x;
     py = gui_mouse_y - y;
     if(obj->param.h == 11) 
      obj->param.d1 = d + ((float)px / ((float)obj->param.w / (float)obj->param.d2));
     else
      obj->param.d1 = d + ((float)py / ((float)obj->param.h / (float)obj->param.d2));

     if(obj->param.d1 < 0) 
      obj->param.d1 = 0;
     if(obj->param.d1 > obj->param.d2) 
      obj->param.d1 = obj->param.d2;
     if(ld!=obj->param.d1) {
      if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
       ret =  obj->param.callback( obj, 0);
       if(ret != RET_OK) return ret;
      }
      proc_scroll_bar(MSG_DRAW,obj, 0);
      UPDATE_OBJECT(obj);
     }
    }

   }


   break;
  case MSG_INFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_scroll_bar(MSG_DRAW,obj, 0);
   break;
  case MSG_OUTFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE) 
    proc_scroll_bar(MSG_DRAW,obj, 0);
   break;
  case MSG_KEYDOWN:
   if(obj->param.h == 11 ) {
    inc_key1 = SDLK_RIGHT;
    inc_key2 = SDLK_UP;
    dec_key1 = SDLK_LEFT;
    dec_key2 = SDLK_DOWN;
   } else {
    inc_key1 = SDLK_RIGHT;
    inc_key2 = SDLK_DOWN;
    dec_key1 = SDLK_LEFT;
    dec_key2 = SDLK_UP;
   }
   if(data == inc_key1 || data == inc_key2) {
    obj->param.d1++;
    if(obj->param.d1 > obj->param.d2) obj->param.d1--;
    if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
     ret =  obj->param.callback( obj, 0);
     if(ret != RET_OK) return ret;
    }

    proc_scroll_bar(MSG_DRAW,obj, 0);
    UPDATE_OBJECT(obj);
   }
   if(data == dec_key1|| data == dec_key2) {
    obj->param.d1--;
    if(obj->param.d1 < 0) obj->param.d1++;
    if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
     ret =  obj->param.callback( obj, 0);
     if(ret != RET_OK) return ret;
    }
    proc_scroll_bar(MSG_DRAW,obj, 0);
    UPDATE_OBJECT(obj);

   }
   break;
 }
 return RET_OK;
}

int proc_hash_box(int msg, struct object_t *obj, int data) {
 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

 if(msg == MSG_DRAW) 
  fill_box(obj->param.x,obj->param.y,
    obj->param.w+obj->param.x, obj->param.h+obj->param.y, obj->param.fg, obj->param.bg, HASH);
 
 return RET_OK;
}

void simple_window(group_t *grp, int w, int h) {
 obj_param_t tmp_parm;
 
 tmp_parm.x = 0;
 tmp_parm.y = 0;
 tmp_parm.w = w;
 tmp_parm.h = h;
 tmp_parm.fg = &globl_fg;
 tmp_parm.bg = &globl_bg;
 
 tmp_parm.proc = proc_shadow_box;
 tmp_parm.flags =0;
 new_obj(grp, &tmp_parm);
 
 tmp_parm.x = 3;
 tmp_parm.y = 3;
 tmp_parm.w-=6;
 tmp_parm.h= 10;
 tmp_parm.fg = &globl_move_color;
 tmp_parm.proc = proc_move_button;
 new_obj(grp, &tmp_parm); 
}

struct object_t **scroll_text_text;
struct object_t *scroll_text_bar;
int scroll_text_num_lines;
int *scroll_text_lines;
char *scroll_text_data;

int scroll_text_bot(int msg, struct object_t *obj, int data) {
 int i;
 for(i = 0;i< scroll_text_num_lines;i++) 
  scroll_text_text[i]->param.dp1 = (void *)&scroll_text_data[scroll_text_lines[i+
    scroll_text_bar->param.d1]];
 broadcast_group(current_grp, MSG_DRAW, 0);
 return RET_OK;
}

void scroll_text_window(int cent_x, int cent_y, int w, int h, char *text, int len) {
 group_t *scroll_text_group;
 obj_param_t tmp_parm;

 int char_h;
 int width, max_width;
 int i,j;

 max_width = width = 0;
 j = 1;
 scroll_text_lines = (int *)malloc(sizeof(int) * len); 
 scroll_text_lines[0] = 0;

 for(i=0;i<len;i++) { 
  width++;
  if(text[i] == '\n')  {
   i++;
   scroll_text_lines[j++] = i;
   if(width > max_width)
    max_width = width;
   width = 0;
  }
 }

 scroll_text_group= 
  new_group(cent_x-(w/2),cent_y-(h/2),w+3,h+3,globl_flags,globl_drop_depth);

 simple_window(scroll_text_group, w,h);

 tmp_parm.x = 3;
 tmp_parm.y = 15;
 tmp_parm.w = w-6;
 tmp_parm.h = h-43;
 tmp_parm.fg = &globl_fg;
 tmp_parm.bg = &globl_bg;
 tmp_parm.flags = 0;
 tmp_parm.proc = proc_box;
 new_obj(scroll_text_group, &tmp_parm); 

 tmp_parm.x = (w/2) - 20;
 tmp_parm.y = h - 25;
 tmp_parm.h = 20;
 tmp_parm.w = 40;
 tmp_parm.flags = QUIT_BUTTON | SHOW_FOCUS;
 tmp_parm.proc = proc_button_box;
 tmp_parm.dp1 = (void *)"Okay"; 
 new_obj(scroll_text_group, &tmp_parm);

 char_h = ((h-64)/8)-1;

 tmp_parm.x = (w - 16);
 tmp_parm.y = 17;
 tmp_parm.h = h-46;
 tmp_parm.w = 11;
 tmp_parm.d1 = 0;
 tmp_parm.d2 = j - char_h -1;
 tmp_parm.fg = &globl_move_color;
 tmp_parm.bg = &globl_bg;
 tmp_parm.flags = SHOW_FOCUS | CALL_BUTTON;
 tmp_parm.proc = proc_scroll_bar;
 tmp_parm.callback = (void *)scroll_text_bot;
 scroll_text_bar = new_obj(scroll_text_group, &tmp_parm);

 tmp_parm.x = 4;
 tmp_parm.y= 17;
 tmp_parm.w = w-6;
 tmp_parm.fg = &globl_fg;
 tmp_parm.bg = &globl_bg;
 tmp_parm.flags = 0;
 tmp_parm.proc = proc_text;
 tmp_parm.dp1 = (void *)text;

 scroll_text_num_lines = char_h+1;
 scroll_text_text = (struct object_t **)malloc(sizeof(struct object_t *)*char_h);

// scroll_text_text[0] = new_obj(scroll_text_group, &tmp_parm);

 scroll_text_data = text;
 
 for(i = 0;i< scroll_text_num_lines;i++) {
  tmp_parm.dp1 = (void *)&scroll_text_data[scroll_text_lines[i+
    scroll_text_bar->param.d1]];
  scroll_text_text[i] = new_obj(scroll_text_group, &tmp_parm);
  tmp_parm.y+=9;
 }


 group_loop(scroll_text_group);

 destroy_group(scroll_text_group);
 free(scroll_text_lines);
 free(scroll_text_text);

}

void alert(int cent_x, int cent_y, char *message, char *ok_text) {
 group_t *prompt_group;
 obj_param_t tmp_parm;
 int w,h;
 int lns, width,i,j;
 int k;
 char *lines[MAX_PROMPT_LINES];
 width = j = 0;
 lns = 0;
 for(i=0;;i++) {
  if(message[i] == 0) {
   lines[lns] = (char *)malloc( j + 1 );
   for(k = 0;k<j;k++) 
    lines[lns][k] = message[(i-j)+k];
   lines[lns][k] = 0;
   lns++;
   break;
  };
  if(message[i] == '\n') {
   lines[lns++] = (char *)malloc( j + 1 );
   for(k = 0;k<j;k++) 
    lines[lns-1][k] = message[(i-j)+k];
   lines[lns-1][k] = 0;
   j = 0;
  } else {
   j++;
   if(j>width) width = j;
  }
 }

 w = width * 8 + 10;
 h = lns * 8 + 60;

 prompt_group = new_group(cent_x-(w/2), cent_y-(h/2),w+3,h+3, globl_flags,globl_drop_depth);

 simple_window(prompt_group, w, h);

 tmp_parm.w = w;
 tmp_parm.fg = &globl_fg;
 tmp_parm.bg = &globl_bg;
 tmp_parm.flags = 0;

 tmp_parm.x = w/2;
 for(i = 0;i<lns;i++) { 
  tmp_parm.y = 25+(i*9);
  tmp_parm.proc=proc_ctext;
  tmp_parm.dp1 = lines[i]; 
  new_obj(prompt_group, &tmp_parm);
 }


 tmp_parm.x = (w/2) -( CENTER_OF_STRING(ok_text)+4);
 tmp_parm.y = h- 25;
 tmp_parm.w = (strlen(ok_text)*9)+10;
 tmp_parm.h = 20;
 tmp_parm.proc = proc_button_box;
 tmp_parm.flags = QUIT_BUTTON | SHOW_FOCUS;
 tmp_parm.dp1 = (void *)ok_text;
 tmp_parm.quit_value = TRUE;
 new_obj(prompt_group, &tmp_parm);


 group_loop(prompt_group);

 destroy_group(prompt_group);
 for(i=0;i<lns;i++) 
  free(lines[i]);
 return;
}


int prompt(int cent_x, int cent_y, char *message, char *yes_text, char *no_text) {
 group_t *prompt_group;
 obj_param_t tmp_parm;
 int w,h;
 int lns, width,i,j;
 int k;
 char *lines[MAX_PROMPT_LINES];
 width = j = 0;
 lns = 0;
 for(i=0;;i++) {
  if(message[i] == 0) {
   lines[lns] = (char *)malloc( j + 1 );
   for(k = 0;k<j;k++) 
    lines[lns][k] = message[(i-j)+k];
   lines[lns][k] = 0;
   lns++;
   break;
  };
  if(message[i] == '\n') {
   lines[lns++] = (char *)malloc( j + 1 );
   for(k = 0;k<j;k++) 
    lines[lns-1][k] = message[(i-j)+k];
   lines[lns-1][k] = 0;
   j = 0;
  } else {
   j++;
   if(j>width) width = j;
  }
 }

 w = width * 8 + 10;
 h = lns * 8 + 60;

 prompt_group = new_group(cent_x-(w/2), cent_y-(h/2),w+3,h+3, globl_flags,globl_drop_depth);

 simple_window(prompt_group, w, h);

 tmp_parm.w = w;
 tmp_parm.fg = &globl_fg;
 tmp_parm.bg = &globl_bg;
 tmp_parm.flags = 0;

 tmp_parm.x = w/2;
 for(i = 0;i<lns;i++) { 
  tmp_parm.y = 25+(i*9);
  tmp_parm.proc=proc_ctext;
  tmp_parm.dp1 = lines[i]; 
  new_obj(prompt_group, &tmp_parm);
 }


 tmp_parm.x = 10;
 tmp_parm.y = h- 25;
 tmp_parm.w = (strlen(yes_text)*9)+10;
 tmp_parm.h = 20;
 tmp_parm.proc = proc_button_box;
 tmp_parm.flags = QUIT_BUTTON | SHOW_FOCUS;
 tmp_parm.dp1 = (void *)yes_text;
 tmp_parm.quit_value = TRUE;
 new_obj(prompt_group, &tmp_parm);

 tmp_parm.x = w - ((strlen(no_text)*9)+20);
 tmp_parm.y = h- 25;
 tmp_parm.w = (strlen(no_text)*9)+10;
 tmp_parm.h = 20;
 tmp_parm.proc = proc_button_box;
 tmp_parm.flags = QUIT_BUTTON | SHOW_FOCUS;
 tmp_parm.dp1 = (void *)no_text;
 tmp_parm.quit_value = FALSE;
 new_obj(prompt_group, &tmp_parm);

 group_loop(prompt_group);

 destroy_group(prompt_group);
 for(i=0;i<lns;i++) 
  free(lines[i]);
 return globl_quit_value;
}

int proc_edit_line(int msg, struct object_t *obj, int data) {
#define CWIDTH	(obj->param.w/8)
 color_t color;
 int i;
 char *byte;
 switch(msg) {
  case MSG_START:
   obj->param.d1 = 0;
   obj->param.dp1 = (void *)"hello there";
   obj->param.d3 = strlen(obj->param.dp1);

   break;
  case MSG_TICK:
   obj->param.d1^=1;
   break;
  case MSG_DRAW:
   if(obj->in_focus == 0) {
    fill_box(obj->param.x, obj->param.y,
             obj->param.x+obj->param.w, obj->param.y+obj->param.h,
	     obj->param.bg, obj->param.bg, NO_HASH);
    SET_COLOR(color, obj->param.fg->r, 
                     obj->param.fg->g,
		     obj->param.fg->b);
   } else {
    SET_COLOR(color, obj->param.bg->r/4,
                     obj->param.bg->g/4,
		     obj->param.bg->b);
    fill_box(obj->param.x,obj->param.y,
             obj->param.x+obj->param.w,obj->param.y+obj->param.h,
	     &color, &color, NO_HASH);
    color.r = obj->param.bg->r;
    color.g = obj->param.bg->g;
    MAP_COLOR(color);
   }
   box(obj->param.x, obj->param.y,
       obj->param.x+obj->param.w,obj->param.y+obj->param.h,
       obj->param.fg, obj->param.bg, NO_HASH);
   byte = (char *)obj->param.dp1;
   for(i = 0;i<obj->param.d3;i++) {
    if(byte[i] == 0) break;
    draw_char(obj->param.x+2 + (i*8), obj->param.y + ((obj->param.h/2)-3), 
              byte[i], color.map, color.map, NO_HASH);   
   }
   break;
  case MSG_INFOCUS:
  case MSG_OUTFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS)==TRUE)
    MESSAGE_OBJECT(obj, MSG_DRAW);
   break;
 }
 return RET_OK;
#undef CWIDTH
}

