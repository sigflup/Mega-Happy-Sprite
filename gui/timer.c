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

int timer_lock;

gui_timer_t *globl_timer;

Uint32 timer_callback(Uint32 interval) {
 gui_timer_t *walker;

/* if(globl_dirt == 1 || lock_update == 0) {
  clipped_update(0,0,0,0);
  globl_dirt = 0;
 } */ 

 if(globl_timer == (void *)-1) return interval;
 if(timer_lock == 1) return interval;
 walker = globl_timer;
 for(;;) {
  if( CHECK_FLAG(walker->flags, STOPPED) == FALSE) {
   walker->timer++;
   if(walker->timer == walker->reset) {
    walker->timer = 0;
    if(CHECK_FLAG(walker->flags, ACTIVE_ONLY_WITH_PARENT) == FALSE) 
     walker->obj->param.proc(walker->msg, walker->obj, walker->data);
    else
     if(current_grp == walker->parent_grp)
      walker->obj->param.proc(walker->msg, walker->obj, walker->data);
   }
  }
  walker = (gui_timer_t *)walker->node.next;
  if(walker == globl_timer) break;
 }
 return interval;
}

void init_timers(void) {
 globl_timer = (void *)-1;
 timer_lock = 0;
 SDL_SetTimer( 20, timer_callback);
}

gui_timer_t *add_timer(struct object_t *obj, int reset, int msg, int data,group_t *parent,int flags) {
 gui_timer_t *new;
 timer_lock = 1;
 if(globl_timer == (void *)-1) {
  globl_timer = (gui_timer_t *)malloc(sizeof(gui_timer_t) );
  INIT_LIST_HEAD(&globl_timer->node);
  new = globl_timer;
 } else {
  new = (gui_timer_t *)malloc(sizeof(gui_timer_t) );
  list_add(&new->node, &globl_timer->node);
 }
 new->timer = 0;
 new->reset = reset;
 new->msg = msg;
 new->data = data;
 new->obj = obj;
 new->parent_grp = parent;
 new->flags = flags;
 timer_lock = 0;
 return new; 
}

void del_timer(gui_timer_t *in) {
 timer_lock = 1;
 list_del(&in->node);
 if(in == globl_timer) {
  free(globl_timer);
  globl_timer = (void *)-1;
 }
 timer_lock = 0;
}
