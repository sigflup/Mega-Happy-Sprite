#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "gui_types.h"
#include "link.h"
#include "drop.h"
#include "gui.h"
#include "draw.h"
#include "std_dialog.h"
#include "font.h"
#include "timer.h"
#include "load_save.h"
#include "menu.h"

int menu_text(int msg, struct object_t *obj, int data) {
 switch(msg) {
  case MSG_DRAW:
   if(obj->in_focus == 1) {
    fill_box(obj->param.x, obj->param.y, 
             obj->param.x+obj->param.w, obj->param.y + obj->param.h, 
	     obj->param.fg, obj->param.bg, NO_HASH);
    draw_text(obj->param.x, obj->param.y, obj->param.dp1, obj->param.bg, obj->param.bg, NO_HASH,0);
   } else {
    fill_box(obj->param.x, obj->param.y, 
             obj->param.x+obj->param.w, obj->param.y + obj->param.h, 
	     obj->param.bg, obj->param.fg, NO_HASH);
    draw_text(obj->param.x, obj->param.y, obj->param.dp1, obj->param.fg, obj->param.bg, NO_HASH,0);
   }
   break;
  case MSG_INFOCUS:
  case MSG_OUTFOCUS:
   menu_text(MSG_DRAW, obj, data);
   break;
  case MSG_PRESS:
   if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE)
    return obj->param.callback(obj, 0);
   break;
 }
 return RET_OK;
}

int quit_proc(int msg, struct object_t *obj, int data) {
 if(msg == MSG_INFOCUS || msg==MSG_MOUSEMOVE)
  return RET_QUIT;
 else
  return RET_OK;
}

group_t *new_menu(int x, int y, struct menu_entry_t *root, color_t *fg, color_t *bg) {
 group_t *new;
 obj_param_t tmp_parm;
 int w, h;
 int i,j;
 w = 10;
 for(i=0;;i++) {
  if(root[i].name!=NULL) {
   j = strlen(root[i].name);
   if(((j * 8)+10)>w)
    w = (j*8)+10;
  } else {
   break; 
  }
 }
 h = (i*8)+10;
 new = new_group(x, y, w+2, h+2, globl_flags, globl_drop_depth);
 PARM(0,0,w,h, fg, bg, 0, proc_shadow_box);
 new_obj(new, &tmp_parm); 

 PARM(0-x,0-y, x, gui_screen->h, 0,0,0, quit_proc);
 new_obj(new, &tmp_parm);

 PARM(0, 0-y, w, y, 0,0,0, quit_proc);
 new_obj(new, &tmp_parm);

 PARM(0, h, w, gui_screen->h -  h, 0,0,0, quit_proc);
 new_obj(new, &tmp_parm);

 PARM(w, 0-y, gui_screen->w - w, gui_screen->h, 0,0,0, quit_proc);
 new_obj(new, &tmp_parm);

 for(j=0;j<i;j++) {
  PARM(5,5+(j*8), w-10, 8, fg, bg, root[j].flags,menu_text);
  tmp_parm.dp1 = root[j].name; 
  tmp_parm.callback = root[j].callback;
  tmp_parm.user_flags = root[j].user_flags;
  new_obj(new, &tmp_parm);
 }

 return new;
}
