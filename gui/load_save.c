#include <stdio.h>
#include <stdlib.h>
#ifndef WINDOWS
 #include <unistd.h>
#endif
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

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../config.h"

#ifdef WINDOWS
# include "realpath.h"
#endif

#ifndef WINDOWS
# define SLASH_CHAR '/'
#else
# define SLASH_CHAR '\\'
#endif

int do_load(struct select_file_t *selector, char *filename);
int do_save(struct select_file_t *selector, char *filename);

#define BREAKER 0
#define QUITER	4

int fix_file(struct select_file_t *parent, char *filename) {
 char *cp;
 int i;
 char new_path[BIGBUF];
 char tmp_buf[BIGBUF];
 int ret;
 struct stat qstat;


#ifdef WINDOWS
 if((filename[1] == ':')  &&
   ((filename[2] == 0) || (filename[2] == '\\')))  {
  memcpy(new_path, filename, strlen(filename)+1); 
 } else {
#endif

  if(parent->path[1] != 0) { 
   if( parent->path[strlen(parent->path) - 1] != SLASH_CHAR) 
    snprintf(new_path, BIGBUF, "%s%c%s", parent->path, SLASH_CHAR, filename);
   else
    snprintf(new_path, BIGBUF, "%s%s", parent->path, filename);
  } else { 
   if(strncmp("..", filename, 3) != 0)
   snprintf(new_path, BIGBUF, "%s%s", parent->path, filename);
   else {
    parent->selected_line = -1;
    cp = (char *)parent->name_object->param.dp1;
    *cp = 0;
    MESSAGE_OBJECT(parent->name_object, MSG_START);
    MESSAGE_OBJECT(parent->name_object, MSG_DRAW);
    return BREAKER;
   }
  }

#ifdef WINDOWS
 }
#endif


 realpath(new_path, tmp_buf);
 memcpy(new_path, tmp_buf, strlen(tmp_buf)+1);
 
 ret = stat(new_path, &qstat);

 if(parent->type == LOAD){
  if( ret < 0 )
   return BREAKER;
 } else {
  if( ret < 0) {
   if(do_save(parent, new_path) == RET_QUIT)
    return QUITER;
   else
    return BREAKER;
  }
 }

 if(S_ISREG(qstat.st_mode) != 0) {
  if(parent->type == LOAD) {
   if( do_load(parent, new_path) == RET_QUIT)
    return QUITER;
  } else {
   if( prompt(gui_screen->w/2, gui_screen->h/2,
              "So you want to overwrite this file?",
                        "Yes", "No") == TRUE) {
    if( do_save(parent, new_path) == RET_QUIT)
     return QUITER;
   }
  }
 }

 if(S_ISDIR(qstat.st_mode) != 0) {
  if(strncmp(".", filename,2) == 0) {
   parent->selected_line = -1;
   cp = (char *)parent->name_object->param.dp1;
   *cp = 0;
   MESSAGE_OBJECT(parent->name_object, MSG_START);
   MESSAGE_OBJECT(parent->name_object, MSG_DRAW);
   return BREAKER;
  } 
 /* if(strncmp("..", filename,3) == 0) {
   printf("here\n");
   for(i = strlen(new_path) - 4;;i--)
    if(new_path[i] == SLASH_CHAR) {
     if(i != 0)
       new_path[i] = 0;
     else
       new_path[i+1] = 0;
     break;
   }
  } */
  
  for(i=0;i<BIGBUF;i++) {
   parent->path[i] = new_path[i];
   if(parent->path[i] == 0) break;  }
  read_dir(parent);
  broadcast_group(parent->grp, MSG_DRAW, 0);
 }
 return BREAKER; 
}

int line_edit(int msg, struct object_t *obj, int data) {
 char *cp;
 int fp, i;
 struct stat qstat;
 struct select_file_t *parent;
 color_t tmp_color;
 parent = (struct select_file_t *)obj->param.dp2;
 switch(msg) {
  case MSG_START:
   obj->param.d1 = 0;
   obj->param.d2 = strlen((char *)obj->param.dp1);
   break;
  case MSG_TICK:
   obj->param.d1^=1;
  case MSG_DRAW: 
  if(obj->in_focus == 0) {
   fill_box(obj->param.x, obj->param.y,
     obj->param.x + obj->param.w, obj->param.y+obj->param.h, obj->param.bg, obj->param.bg, NO_HASH);
   SET_COLOR(tmp_color, obj->param.fg->r,
                        obj->param.fg->g,
			obj->param.fg->b);
  } else {
   SET_COLOR(tmp_color, obj->param.bg->r/4,
                        obj->param.bg->g/4,
			obj->param.bg->b);
   fill_box(obj->param.x, obj->param.y,
     obj->param.x+obj->param.w, obj->param.y+obj->param.h, &tmp_color, &tmp_color, NO_HASH);
   tmp_color.r = obj->param.bg->r;
   tmp_color.g = obj->param.bg->g;
   MAP_COLOR(tmp_color);
  }
  box(obj->param.x, obj->param.y,
    obj->param.x + obj->param.w, obj->param.y+obj->param.h, obj->param.fg, obj->param.bg, NO_HASH);
  if(CHECK_FLAG(obj->param.flags, MAX_CHARS) == TRUE)  
   draw_text(obj->param.x+2, obj->param.y+(obj->param.h/2)-4,
     (char *)obj->param.dp1, &tmp_color, &tmp_color,
     NO_HASH|MAX_CHARS,(obj->param.w/8)-1);
  else
   draw_text(obj->param.x+2, obj->param.y+(obj->param.h/2)-4,
     (char *)obj->param.dp1, &tmp_color, &tmp_color, NO_HASH,0);
   if(obj->in_focus == 1 && obj->param.d1 == 1) 
    fill_box(obj->param.x+2 +(8*obj->param.d2), obj->param.y+(obj->param.h/2)-4,
             obj->param.x+10+(8*obj->param.d2), obj->param.y+(obj->param.h/2)+5,
	     &globl_bg, &globl_bg, NO_HASH);
  break;
  case MSG_INFOCUS:
  case MSG_OUTFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    line_edit(MSG_DRAW, obj, data);
  break;
  case MSG_KEYDOWN:
  /* XXX scroll the text area for more text please */
   cp = obj->param.dp1;
   if( data == SDLK_RETURN) {
    if(parent->name[0] == 0) break;
    if(parent->name[0] == '/') {
     fp = open(parent->name, O_RDONLY);
     if(parent->type == LOAD) {
      if(fp<0) {
       cp = (char *)parent->name_object->param.dp1;
       *cp = 0;
       MESSAGE_OBJECT(parent->name_object, MSG_START);
       MESSAGE_OBJECT(parent->name_object, MSG_DRAW);
       break;
      }
     } else {
      if(fp<0) {
       if( do_save(parent, parent->name) == RET_QUIT)
	return RET_QUIT;
       break;
      }
     }
     fstat(fp, &qstat);
     close(fp);
     if(S_ISREG(qstat.st_mode) != 0) {
      if(parent->type == LOAD) {
       if( do_load(parent, parent->name) == RET_QUIT)
        return RET_QUIT;
      } else {

       if( prompt(gui_screen->w/2, gui_screen->h/2,
               "So you want to overwrite this file?",
                        "Yes", "No") == TRUE) {
        if( do_save(parent, parent->name) == RET_QUIT)
         return RET_QUIT;
       } else
	return RET_QUIT;
 

      }
     } 
     if(S_ISDIR(qstat.st_mode) != 0){
       for(i=0;i<BIGBUF;i++) {
        parent->path[i] = parent->name[i];
        if(parent->path[i] == 0) break;
       } 
       read_dir(parent);
       cp = (char *)parent->name_object->param.dp1;
       *cp = 0;
       MESSAGE_OBJECT(parent->name_object, MSG_START);
       broadcast_group(parent->grp, MSG_DRAW,0);
       break;
     }
    }
    i = strlen(parent->name);
    if( parent->name[i-1] == '/') parent->name[i-1] = 0; 
    if(fix_file(parent, parent->name) == QUITER)
     return RET_QUIT;
    break;


   } else { 
    if( data == SDLK_BACKSPACE) {
     obj->param.d2--;
     if(obj->param.d2 < 0)
      obj->param.d2 = 0;
     else
      cp[obj->param.d2] = 0;
      
    } else {
     cp[obj->param.d2++] = data;
     cp[obj->param.d2] = 0;
    }
   }
   MESSAGE_OBJECT(obj, MSG_DRAW);
   break;
 
 }
 return RET_OK;
}

int text_hilight(int msg, struct object_t *obj, int data) {
 int q;
 int i;
 int w;
 char *cp;
 struct select_file_t *parent;

 color_t blue;
 color_t *bg_color;

// char new_path[BIGBUF];

 if(CHECK_FLAG(obj->param.d2, NO_TEXT) == TRUE) return RET_OK;

 w = (obj->param.w / 8) -1;

 parent = obj->param.dp1;
 switch(msg) {
  case MSG_DRAW:

   if((parent->selected_line - parent->scroll_bar->param.d1 ) == obj->param.d1) {

    fill_box(obj->param.x, obj->param.y,
              obj->param.x+ obj->param.w, obj->param.y + obj->param.h, 
              obj->param.fg, obj->param.bg , NO_HASH);

    draw_text(obj->param.x, obj->param.y, obj->param.dp2, obj->param.bg, obj->param.bg, 
      NO_HASH|MAX_CHARS, w);
 
   } else {
    if(CHECK_FLAG(obj->param.d2,CLICKED)==TRUE) {
     fill_box(obj->param.x, obj->param.y,
              obj->param.x+ obj->param.w, obj->param.y + obj->param.h, 
              &globl_move_color, obj->param.bg , NO_HASH);
     draw_text(obj->param.x, obj->param.y, obj->param.dp2, obj->param.bg, obj->param.bg, 
       NO_HASH|MAX_CHARS, w);
    } else {
     blue.r =0x7f;
     blue.g =0x9f;
     blue.b =0xff;
     if(((obj->param.d1+parent->scroll_bar->param.d1)&2)==2)
      bg_color = &blue;
     else
      bg_color = obj->param.bg;
     MAP_COLOR((*bg_color));
     fill_box(obj->param.x, obj->param.y,
              obj->param.x+ obj->param.w, obj->param.y + obj->param.h, 
	      bg_color, obj->param.bg , NO_HASH);

     if(obj->in_focus == TRUE) 
      fill_box(obj->param.x, obj->param.y,
               obj->param.x+ obj->param.w, obj->param.y + obj->param.h, 
   	       &globl_move_color, obj->param.bg, HASH);
     draw_text(obj->param.x, obj->param.y, obj->param.dp2, obj->param.fg, obj->param.bg, 
       NO_HASH|MAX_CHARS, w);
    }
   }
   break;
  case MSG_CLICK:
   obj->param.d2 |= CLICKED;
   text_hilight(MSG_DRAW,obj,0);
   break;
  case MSG_OUTFOCUS:
  case MSG_UNCLICK:
   obj->param.d2 &= CLICKED^~0;
   text_hilight(MSG_DRAW,obj,0);
   break;
  case MSG_INFOCUS:
   text_hilight(MSG_DRAW,obj,0);
   break;
  case MSG_PRESS:
   if(parent->selected_line == obj->param.d1 + parent->scroll_bar->param.d1) {
    if(fix_file( parent, obj->param.dp2) == QUITER)
     return RET_QUIT;
    break;
   }
   if(parent->selected_line != -1) {
    q = parent->selected_line - parent->scroll_bar->param.d1;
     if(q < parent->nlines && q>=0) {
     parent->selected_line = -1;
     cp = (char *)parent->name_object->param.dp1;
     *cp = 0;
     MESSAGE_OBJECT(parent->name_object,MSG_START);
     MESSAGE_OBJECT(parent->lines[q], MSG_DRAW);
     UPDATE_OBJECT(parent->lines[q]);
    }
   } 
   parent->selected_line = obj->param.d1 + parent->scroll_bar->param.d1;
   cp = (char *)obj->param.dp2;
   for(i=0;;i++) {
    parent->name[i] = cp[i];
    if(parent->name[i] == 0) break;
   }
   fill_box(2,35,parent->grp->w-2,9,&globl_bg, &globl_bg,NO_HASH);
   MESSAGE_OBJECT( parent->name_object, MSG_START);
   MESSAGE_OBJECT( parent->name_object, MSG_DRAW);
   text_hilight(MSG_DRAW,obj,0);
   UPDATE_OBJECT(obj);
   break;
 }
 return RET_OK;
}

int selector_bot(struct object_t *obj, int data) {
 int i;
 struct select_file_t *parent;
 parent = (struct select_file_t *)obj->param.dp1;
 for(i = 0; i<parent->nlines;i++) {
  parent->lines[i]->param.dp2 = (void *)parent->text_lines[i+ obj->param.d1];
  MESSAGE_OBJECT(parent->lines[i], MSG_DRAW);
 }
 clipped_update(parent->grp->pos_x, parent->grp->pos_y, 
                parent->grp->pos_x+ parent->grp->w, parent->grp->pos_y+parent->grp->h);
 return RET_OK;
}


int qsort_cmp(void *A, void *B) {
 return strncmp(A,B,MEDBUF);
}

void read_dir(struct select_file_t *selector) {
 int i,len;

 char *cp;
 DIR *fd;
 struct dirent *dp;


 len = 0;
 if((fd = opendir(selector->path))<=0) return;

 len = 0;
 while((dp = readdir(fd))>0) {
   for(i=0;;i++) {
    selector->text_lines[len][i] = dp->d_name[i];
    if(dp->d_name[i] == 0) break;
   }  
  len++;
 }

 closedir(fd);

 qsort(selector->text_lines, len-1, sizeof(char)*MEDBUF, (void *)qsort_cmp);

 selector->selected_line = -1;
 cp = (char *)selector->name_object->param.dp1;
 *cp = 0;
 MESSAGE_OBJECT(selector->name_object, MSG_START);
 selector->end = len;


 selector->scroll_bar->param.d2 = len - selector->nlines;
 if(selector->scroll_bar->param.d2 < 0)
  selector->scroll_bar->param.d2 = 0;
 for(i=0;i<selector->nlines;i++) {
  if(i<len) { 
   selector->lines[i]->param.d2 = 0;
   selector->lines[i]->param.dp2 = (void *)selector->text_lines[i]; 
  }
  else
   selector->lines[i]->param.d2 = NO_TEXT;
 }
 selector->scroll_bar->param.d1 = 0;

}


int do_save(struct select_file_t *selector, char *filename) {
 char sorry_text[SMALLBUF];

 if(selector->load_proc(selector, filename) == LOAD_OK_QUIT) {
  selector->blinky->flags |= STOPPED;
  return RET_QUIT;
 } else {
  snprintf(sorry_text, SMALLBUF,"--== Sorry ==--\n*puts down pen* I cannot save this as %s",
    selector->file_type_name);
  alert(gui_screen->w/2, gui_screen->h/2, sorry_text, "OK :C"); 
  return RET_OK;
 }

}

int do_load(struct select_file_t *selector, char *filename) {
 char sorry_text[SMALLBUF];
 int fp;

 if((fp = open(filename, O_RDONLY))<0) {
  snprintf(sorry_text, SMALLBUF, "--== Sorry ==--\n"
                                  "*shuffles papers* %s", strerror(errno) );
  alert(gui_screen->w/2, gui_screen->h/2, sorry_text, "OK :.<");
  return RET_OK;
 }
 close(fp); 


 if(selector->load_proc(selector, filename) == LOAD_OK_QUIT) {
  selector->blinky->flags |= STOPPED;
  return RET_QUIT;
 } else {
  snprintf(sorry_text,SMALLBUF, "--== Sorry ==--\n*puts down pen* I cannot load this as %s",
    selector->file_type_name);
  alert(gui_screen->w/2, gui_screen->h/2, sorry_text, "OK :.<");
  return RET_OK;
 }
}

int ok_cancel(struct object_t *obj, int data) {
 struct select_file_t *parent;
 parent = obj->param.dp2;
 parent->blinky->flags |= STOPPED;
 return RET_QUIT;
}

int ok_do(struct object_t *obj, int data) {
 char name[BIGBUF];
 struct select_file_t *selector;
 selector = (struct select_file_t *)obj->param.dp2;
 if(selector->name[0] == '/') {
  if(selector->type == LOAD)
   return do_load(selector, selector->name);
  else
   return do_save(selector,selector->name);
 }
 snprintf(name, BIGBUF, "%s/%s", selector->path, selector->name);
 if(selector->type == LOAD)
  return do_load(selector, name);
 else
  return do_save(selector,name);
}

struct select_file_t *setup_overlay_window(int w, int h, int type, char *file_type_name, 
  int (*load_proc)(struct select_file_t *selector, char *filename)) {
 int i;
 int nlines;
 struct select_file_t *new;

 new = (struct select_file_t *)malloc(sizeof(struct select_file_t));
 obj_param_t tmp_parm;
 new->grp = new_group( (gui_screen->w/2) - (w/2),
                       (gui_screen->h/2) - (h/2), w+1, h+1, globl_flags, globl_drop_depth);
 nlines = (h - 85) / 9;
 new->nlines = nlines;
 new->lines = (struct object_t **)malloc(sizeof(struct object_t) * nlines);

 simple_window(new->grp, w,h);
 PARM(8,48, w - 16, (nlines*9)+3 ,&globl_fg, &globl_bg, 0, proc_box);
 
 new_obj(new->grp, &tmp_parm);

 PARM(w-21,50, 11, (nlines*9), &globl_move_color, &globl_bg, SHOW_FOCUS|CALL_BUTTON, proc_scroll_bar);
 tmp_parm.d1 = 0;
 tmp_parm.d2 = 0;
 tmp_parm.dp1 = (void *)new;
 tmp_parm.callback = selector_bot;
 new->scroll_bar = new_obj(new->grp, &tmp_parm);


 for(i=0;i<nlines;i++) { 
  PARM(10,50+(i*9), w-32,9, &globl_fg, &globl_bg, 0, text_hilight);
  tmp_parm.d1 = i;
  tmp_parm.dp1 = new;
  new->lines[i] = new_obj(new->grp,&tmp_parm);
 }


 PARM(25, h-30,100,20,&globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON,proc_button_box);
 new->type = type;
 if(type == LOAD) 
  tmp_parm.dp1 = (void *)"Load :)";
 else
  tmp_parm.dp1 = (void *)"Save ;)";
 tmp_parm.dp2 = (void *)new;
 tmp_parm.callback = ok_do;
 tmp_parm.quit_value = TRUE;
 new_obj(new->grp, &tmp_parm);

 PARM(w-125,h-30,100,20,&globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON,proc_button_box);
 tmp_parm.dp1 = (void *)"Cancel :(";
 tmp_parm.dp2 = (void *)new;
 tmp_parm.callback = ok_cancel;
 tmp_parm.quit_value = FALSE;
 new_obj(new->grp, &tmp_parm);



 new->name[0] = 0;
 PARM(2, 34, w-4,12, &globl_fg, &globl_bg, MAX_CHARS|SHOW_FOCUS, line_edit);
 tmp_parm.dp1 = (void *)new->name;
 new->name_object = new_obj(new->grp, &tmp_parm);

 PARM((w/2), 25, ((w-32)/8),0, &globl_fg, &globl_bg, MAX_CHARS, proc_ctext);
 tmp_parm.dp1 = new->path;
 new_obj(new->grp, &tmp_parm);

 for(i=0;;i++) {
  new->file_type_name[i] = file_type_name[i];
  if(new->file_type_name[i] == 0) break;
 }
 new->load_proc = load_proc;
 getcwd( new->path, BIGBUF);
 return new;
}

void do_overlay_window(struct select_file_t *selector) {
 void *old_tick;
 gui_timer_t *blinky;
 old_tick = (void *)globl_tick;
 globl_tick = null_tick;
 read_dir(selector);
 blinky = add_timer(selector->name_object, 12, MSG_TICK, 0, selector->grp, ACTIVE_ONLY_WITH_PARENT);
 selector->blinky = blinky;
 group_loop(selector->grp);
 del_timer(blinky);
 globl_tick = old_tick;
}
