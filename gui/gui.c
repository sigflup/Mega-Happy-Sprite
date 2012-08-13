#ifdef WINDOWS
# include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include <SDL.h>
#include "gui_types.h"
#include "link.h"
#include "drop.h"
#include "gui.h"
#include "draw.h"
#include "std_dialog.h"
#include "timer.h"

#ifdef WINDOWS
int gui_flags, gui_w, gui_h;
#endif
SDL_Surface *gui_screen;

void (*globl_tick)(void);
void (*globl_wait_tick)(void);

int gui_mouse_x, gui_mouse_y;
struct object_t *focus_obj;

int globl_flags;
int lock_update;
int globl_dirt, globl_quit_value;
int globl_drop_depth;
int floating_key;
color_t globl_fg, globl_bg;
color_t globl_move_color;
group_t *current_grp;

/* XXX this is really ugly, make a better solution */
char shift_map[19][2] = {
 "1!",
 "2@",
 "3#",
 "4$",
 "5%",
 "6^",
 "7&",
 "8*",
 "9(",
 "0)",
 "-_",
 "=+",
 "[{",
 "]}",
 ";:",
 "'\"",
 ",<",
 ".>",
 "/?"
};

group_t *new_group(int x, int y, int w, int h, int flags, int drop_d) {
 group_t *new;
 new = (group_t *)malloc(sizeof(group_t));
 new->pos_x = x;
 new->pos_y = y;
 new->w = w;
 new->h = h;
 new->flags = flags;
 new->drop_d = drop_d;


 new->drop_w = w + (w/4);
 new->drop_h = h + (h/4);
 
 new->ready = 0;
 new->objs = (struct object_t *)NULL;
 return new;
}

struct object_t *new_obj(group_t *grp, obj_param_t *param) {
 struct object_t *new;
 new = (struct object_t *)malloc(sizeof(struct object_t));
 new->in_focus = FALSE;
 new->clicked = FALSE;
 memcpy(&new->param, param, sizeof(obj_param_t));
 if(grp->objs == NULL) {
  grp->objs = new;
  INIT_LIST_HEAD(&grp->objs->node);
 } else
  list_add(&new->node,&grp->objs->node);
 return new;
}


int broadcast_group(group_t *grp, int msg, int data) {
 struct object_t *walker; 
 walker = grp->objs;
 globl_dirt = 0;
 for(;;) {
  walker->param.proc(msg, walker, data);
  walker = (struct object_t *)walker->node.next;
  if(walker == grp->objs) break;
 }
 if(globl_dirt == 1) 
  clipped_update(grp->pos_x, grp->pos_y, grp->w, grp->h);
 return 0;
}

int wait_on_mouse(void) {
 int ret;
 ret = MOUSE_MOVE;
 SDL_Event event;
 while(SDL_PollEvent(&event) ) {
  switch(event.type) {
   case SDL_QUIT:
    SDL_Quit();
    exit(-1);
    break;
   case SDL_MOUSEMOTION:
    gui_mouse_x = event.motion.x;
    gui_mouse_y = event.motion.y;
    break;
   case SDL_MOUSEBUTTONUP:
    ret = MOUSE_UP;
    break;
   case SDL_MOUSEBUTTONDOWN:
    ret = MOUSE_DOWN;
    break;
   case SDL_KEYDOWN:
    floating_key = event.key.keysym.sym;
    ret = KEY_DOWN;
    break;
   case SDL_KEYUP:
    ret = KEY_UP;
    break;
   break;
  }
 }
 if(globl_wait_tick!=(void *)-1)
  globl_wait_tick();
 return ret;
}

void null_tick(void) {
}

void default_tick(void) {
 SDL_Delay(10);
}

int group_loop(group_t *grp) {
 SDL_Rect drop_clip;
 unsigned char *save_buf, *pix;
 int i;
 int key, modstate;
 int clip_x, clip_y, clip_w, clip_h;
 SDL_Event event;
 struct object_t *walker;
 group_t *old_grp;

 old_grp = current_grp;
 current_grp = grp;


 if((grp->pos_x > gui_screen->w) ||
    (grp->pos_y > gui_screen->h) ) return 0;

 if(CHECK_FLAG(grp->flags, DROP_SHADOW) == TRUE &&
    CHECK_FLAG(grp->flags, DROP_SHADOW_READY) == FALSE) {
  grp->drop_buffer = new_drop(grp->drop_d);
  grp->flags |= DROP_SHADOW_READY;
 }

 if(CHECK_FLAG(grp->flags, DROP_SHADOW) == TRUE) {
  clip_w = grp->drop_w;
  clip_h = grp->drop_h;
 } else {
  clip_w = grp->w;
  clip_h = grp->h;
 }
 clip_x = grp->pos_x;
 clip_y = grp->pos_y;

 if(grp->pos_x < 0) {
  clip_w += grp->pos_x;
  clip_x = 0;
 } else
  if((clip_x + clip_w) >= gui_screen->w ) 
   clip_w = gui_screen->w - clip_x - 1;
//   clip_w -= (clip_x + clip_w) - gui_screen->w + 1;

 if(grp->pos_y < 0) {
  clip_h += grp->pos_y;
  clip_y = 0;
 } else
  if((clip_y + clip_h) > gui_screen->h ) 
   clip_h -= (clip_y + clip_h) - gui_screen->h;

 save_buf=(unsigned char *)malloc((clip_w*clip_h)*gui_screen->format->BytesPerPixel);

/* XXX pointer math */
 pix = (unsigned char *)(gui_screen->pixels +
                         (clip_y * gui_screen->pitch) +
			 (clip_x * gui_screen->format->BytesPerPixel) );

 LOCK;

 /* XXX we crash here sometimes 
  * because the entire clipping area is bigger then the screen :( */


 if((clip_h - clip_y)>gui_screen->h)
  clip_h = gui_screen->h - clip_y;
 for(i=0;i<clip_h;i++) 
  /* XXX pointer math */
  memcpy(
    (unsigned char *)(save_buf+(i*(clip_w*gui_screen->format->BytesPerPixel))),
    (unsigned char *)(pix +    (i*gui_screen->pitch)),
                                        clip_w*gui_screen->format->BytesPerPixel);
 UNLOCK;

 if(CHECK_FLAG(grp->flags, DROP_SHADOW) == TRUE) {
  drop_clip.w = grp->w;
  drop_clip.h = grp->h; 
  draw_drop(gui_screen, grp->pos_x, grp->pos_y,grp->drop_buffer,grp->drop_w, grp->drop_h,&drop_clip);
 }

 
 if(grp->ready == 0) {
  broadcast_group(grp, MSG_START, 0);
  grp->ready = 1;
 }

  walker = grp->objs;
   if(walker != NULL){
    for(;;) {
     if(gui_mouse_x > (grp->pos_x + walker->param.x)                   &&
        gui_mouse_x < (grp->pos_x + walker->param.x + walker->param.w) &&
	gui_mouse_y > (grp->pos_y + walker->param.y)                   &&
	gui_mouse_y < (grp->pos_y + walker->param.y + walker->param.h)) {
      if(walker->in_focus == FALSE) 
        walker->in_focus = TRUE;
     } else
      if(walker->in_focus == TRUE) 
       walker->in_focus = FALSE;

     walker = (struct object_t *)walker->node.next;
     if(walker == grp->objs) break;
    }
   }

 broadcast_group(grp, MSG_DRAW, 0);

 clipped_update(0,0,0,0);

 for(;;) {
  globl_dirt = 0;
//  while(SDL_WaitEvent(&event) ) {
  while(SDL_PollEvent(&event) ) {
   switch(event.type) {
    case SDL_QUIT:
     SDL_Quit();
     exit(-1);
    break;
    case SDL_MOUSEMOTION:
     gui_mouse_x = event.motion.x;
     gui_mouse_y = event.motion.y;
     walker = grp->objs;
     if(walker != NULL){
      for(;;) {
       if(gui_mouse_x > (grp->pos_x + walker->param.x)                   &&
	  gui_mouse_x < (grp->pos_x + walker->param.x + walker->param.w) &&
	  gui_mouse_y > (grp->pos_y + walker->param.y)                   &&
	  gui_mouse_y < (grp->pos_y + walker->param.y + walker->param.h)) {
	if(walker->in_focus == FALSE) {
	 walker->in_focus = TRUE;
	 if(walker->param.proc(MSG_INFOCUS, walker, 0) == RET_QUIT) 
	  goto done1;
	} else
	 if(walker->param.proc(MSG_MOUSEMOVE,walker,0) == RET_QUIT)
	  goto done1;
       } else
	if(walker->in_focus == TRUE) {
	 walker->in_focus = FALSE;
	 if(walker->param.proc(MSG_OUTFOCUS, walker, 0) == RET_QUIT)
	  goto done1;
	}

       walker = (struct object_t *)walker->node.next;
       if(walker == grp->objs) break;
      }
     }

    break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
     walker= grp->objs;
     for(;;) {
      if(walker->in_focus == TRUE) {
       if(event.type == SDL_MOUSEBUTTONDOWN) {
	walker->clicked = TRUE;
	if(walker->param.proc(MSG_CLICK, walker, 0) == RET_QUIT) 
	 goto done1;
       } else {
	if(walker->clicked == TRUE) { 
	 walker->clicked = FALSE;
	 if(walker->param.proc(MSG_PRESS, walker, 0) == RET_QUIT)
	  goto done1;
	}
	if(walker->param.proc(MSG_UNCLICK, walker, 0) == RET_QUIT)
	 goto done1;
       }
      }
      walker = (struct object_t *)walker->node.next;
      if(walker == (struct object_t *)grp->objs) break;
     }
    break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
     walker = grp->objs;
     for(;;) {
      if(walker->in_focus == TRUE) {
       if(event.type == SDL_KEYUP) {
	key = event.key.keysym.sym;
        if( key<SDLK_NUMLOCK || key > SDLK_COMPOSE) {
         modstate = SDL_GetModState();
	 if( CHECK_FLAG(modstate, KMOD_LSHIFT) == TRUE ||
	     CHECK_FLAG(modstate, KMOD_RSHIFT) == TRUE ||
	     CHECK_FLAG(modstate, KMOD_CAPS) == TRUE) {
	  key = toupper(key);
	  for(i=0;i<19;i++) 
	   if(key == shift_map[i][0]) {
	    key = shift_map[i][1];
	    break;
	   }
	 }
	 if(walker->param.proc(MSG_KEYUP, walker, key) == RET_QUIT)
	  goto done1;
	}
       } else {
	key = event.key.keysym.sym;
        if( key<SDLK_NUMLOCK || key > SDLK_COMPOSE) {
	 modstate = SDL_GetModState();
         if( CHECK_FLAG(modstate, KMOD_LSHIFT) == TRUE ||
	     CHECK_FLAG(modstate, KMOD_RSHIFT) == TRUE ||
	     CHECK_FLAG(modstate, KMOD_CAPS) == TRUE) {
	  key = toupper(key);
	  for(i=0;i<19;i++) 
	   if(key == shift_map[i][0]) {
	    key = shift_map[i][1];
	    break;
	   }
	 }

	 if(walker->param.proc(MSG_KEYDOWN,walker,key) == RET_QUIT)
	 goto done1;
	}
       }
      }
      walker = (struct object_t *)walker->node.next;
      if(walker == (struct object_t *)grp->objs) break;
     }
    break;
   }
  }
  if(globl_dirt == 1)
   clipped_update(current_grp->pos_x, current_grp->pos_y,
                  current_grp->pos_x+current_grp->w, current_grp->pos_y+current_grp->h);
  if(globl_tick!=(void *)-1)
   globl_tick();
 }
done1:
 LOCK;
 for(i=0;i<clip_h;i++) 
  /* XXX pointer math */
  memcpy( (unsigned char *)(pix + (i*gui_screen->pitch)),
          (unsigned char *)(save_buf+(i*(clip_w*gui_screen->format->BytesPerPixel))),
	                                      clip_w*gui_screen->format->BytesPerPixel);
 UNLOCK;
 if(globl_quit_value == MOVE_GROUP) 
  clipped_update(walker->param.d1, walker->param.d2, clip_w, clip_h);
 else
  clipped_update(grp->pos_x, grp->pos_y, clip_w, clip_h);

 free(save_buf);

 broadcast_group(current_grp, MSG_MOUSEMOVE,0);

 current_grp = old_grp;
 if(globl_quit_value == MOVE_GROUP)  {
  globl_quit_value = 0;
  group_loop(grp);
 }

 return 0;
}

int destroy_group(group_t *grp) {
 struct object_t *walker;
 struct object_t *prev;

 broadcast_group(grp, MSG_DESTROY, 0);

 walker = grp->objs;
 if(walker != (struct object_t *)NULL) {
  for(;;) {
   prev = walker;
   walker = (struct object_t *)walker->node.next;
   if(walker==grp->objs) break;
//   free(prev);
  }
 }

 free(grp);
 return 0;
}

int init_gui(int x,int y, int flags) {
 int sdl_flags;
#ifdef WINDOWS
 char buf[256];
#endif
 SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO);
 init_timers();
 globl_tick = default_tick;
 globl_wait_tick = (void *)-1;
 sdl_flags = 0;
 if(CHECK_FLAG(flags,FULLSCREEN)== TRUE) sdl_flags |= SDL_FULLSCREEN;

 if(!(gui_screen = SDL_SetVideoMode(x, y, 24, sdl_flags))) {
#ifndef WINDOWS
  printf("could not open screen: %s\n", SDL_GetError());
#else
  snprintf(buf, 256, "could not open screen: %s", SDL_GetError());
  MessageBox(0, buf, "SDL_SetVideoMode", MB_OK);
#endif
  exit(0);
 }
#ifdef WINDOWS
 gui_flags = sdl_flags;
 gui_w = x;
 gui_h = y;
#endif

 SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
 gc = gui_screen;
 drop_init();
 lock_update = 0;
 return 1;
}
