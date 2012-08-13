#include <stdio.h>
#include <stdlib.h>
#ifndef WINDOWS 
 #include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SDL.h>
#include "./gui/libgui.h"
#include "config.h"
#include "mega.h"
#include "vdp.h"
#include "proc.h"
#include "bottom.h"
#include "mega_file.h"
#include "help_text.h"

struct select_file_t *load_sel, *save_sel;
struct object_t *last_tool;

int currently_editing;

int need_update;
int num_xor_pix;

int high_low;
int copy_move;
int hor_ver;

int knob_data_offset;
int knob_or;
int knob_ab;
xor_pix xor_pixels[320*240];

/* XXX replace this load save jonk with menu code */

void put_xor_hash_pix(Uint8 *ram, int pat, int x, int y) {
 if( x >= 0 && y >= 0 &&
     x < vdp_w &&
     y < vdp_h &&
     ((x^y)&2)==2) {
  xor_pixels[num_xor_pix].x = x;
  xor_pixels[num_xor_pix].y = y;
  xor_pixels[num_xor_pix].index = current_palette_index;
  num_xor_pix++; 
 }
}


void put_xor_pix(Uint8 *ram, int pat, int x, int y) {
 if( x >= 0 && y >= 0 &&
     x < vdp_w &&
     y < vdp_h) {
  xor_pixels[num_xor_pix].x = x;
  xor_pixels[num_xor_pix].y = y;
  xor_pixels[num_xor_pix].index = current_palette_index;
  num_xor_pix++; 
 }
}

int go_bot(struct object_t *obj, int data) {
 int i=0;
 obj->param.d1 = FALSE;
 for(i=1;i<0x400;i++) 
  pat_stop[i]  = FALSE;
 pat_stop[0] = TRUE;
 MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
 UPDATE_OBJECT(pattern_edit_object);
 MESSAGE_OBJECT(obj, MSG_DRAW);
 UPDATE_OBJECT(obj);
 return RET_OK;
}

int new_bot(struct object_t *obj, int data) {
 int i=0;
 obj->param.d1 = FALSE;

 save_state();
 for(i=0;i<0x400;i++)
  if( pat_stop[i] == FALSE &&
      pat_ref[i] == 0) {
   current_pattern = i;
   memset(&current_vdp->vram[i*0x20],0, 0x20);
   break;
  }
 
 MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
 MESSAGE_OBJECT(pattern_select_object, MSG_DRAW); 

 MESSAGE_OBJECT(obj, MSG_DRAW);
 UPDATE_OBJECT(obj);
 return RET_OK;
}

int stop_bot(struct object_t *obj, int data) {
 if(pat_stop[current_pattern] == TRUE)
  pat_stop[current_pattern] = FALSE;
 else
  pat_stop[current_pattern] = TRUE;
 MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
 return RET_OK;
}

int load_save_bottom(struct select_file_t *selector, char *filename) {
 int fp = 0;
 FILE *FP = 0;
 int q = 0;
 Uint8 cram_buffer[128];
 struct stat qstat;

 switch(selector->usr_flags) {
  case LOAD_VRAM:
   fp = open(filename, O_RDONLY);
   fstat(fp, &qstat);
   if(qstat.st_size < 0xffff) {
    close(fp);
    return NOPE_TRY_AGAIN;
   } else {
    read(fp,current_vdp->vram, 0xffff);
    close(fp);
    return LOAD_OK_QUIT;
   }
   break;
  case LOAD_CRAM:
   fp = open(filename, O_RDONLY);
   fstat(fp, &qstat);
   if(qstat.st_size != 128) {
    close(fp);
    return NOPE_TRY_AGAIN;
   } else {
    read(fp,cram_buffer,128);
    close(fp);
    load_palette(current_vdp, cram_buffer);
    return LOAD_OK_QUIT;
   }
   break;
  case LOAD_VSRAM:
   fp = open(filename, O_RDONLY);
   fstat(fp, &qstat);
   if(qstat.st_size != 80) {
    close(fp);
    return NOPE_TRY_AGAIN;
   } else {
    read(fp,current_vdp->vsram,128);
    close(fp);
    return LOAD_OK_QUIT;
   }
   break;

  case SAVE_VRAM:
   fp = open(filename, O_CREAT|O_WRONLY, 0750);
   if(fp>0) {
    write(fp, current_vdp->vram,0xffff);
    close(fp);
   }
   return LOAD_OK_QUIT;
  case SAVE_CRAM_HEAD:
   if((FP = fopen(filename, "wb"))) {
    fprintf(FP, "const color_t color[]={\n"); 
    for(q=0;q<64;q++) {
     fprintf(FP," {%d, %d, %d},\n", current_vdp->palette[q/16][q%16].r/ (0xff/7), 
                                current_vdp->palette[q/16][q%16].g/ (0xff/7),
				current_vdp->palette[q/16][q%16].b/ (0xff/7) );
    } 
    fprintf(FP, "};\n");
    fclose(FP);
   }
   return LOAD_OK_QUIT;
  case SAVE_CRAM:
   fp = open(filename, O_CREAT|O_WRONLY, 0750);
   if(fp>0) {
    store_palette(current_vdp, cram_buffer);
    write(fp,cram_buffer, 128);
    close(fp);
   }
   return LOAD_OK_QUIT;
 }
 return LOAD_OK_QUIT;
}

int load_save_middle(struct object_t *obj, int data) {
 void *old_tick = 0;
 gui_timer_t *blinky = 0;

 switch(obj->param.user_flags) {
  case LOAD_VRAM:
   load_sel->usr_flags = LOAD_VRAM;
   load_sel->load_proc = load_save_bottom;
   snprintf(load_sel->file_type_name, 6, "VRAM");
   do_overlay_window(load_sel);
   need_update = 1;
   break;
  case LOAD_CRAM:
   load_sel->usr_flags = LOAD_CRAM;
   load_sel->load_proc = load_save_bottom;
   snprintf(load_sel->file_type_name, 6, "CRAM");
   do_overlay_window(load_sel);
   need_update = 1;
   break;
  case LOAD_VSRAM:
   load_sel->usr_flags = LOAD_VSRAM;  
   load_sel->load_proc = load_save_bottom; 
   snprintf(load_sel->file_type_name, 6, "VSRAM");
   do_overlay_window(load_sel);
   need_update = 1;
   break;
  case LOAD_HAPPY:
   load_sel->usr_flags = LOAD;  
   load_sel->load_proc = load_save_mega; 
   snprintf(load_sel->file_type_name, 6, "mega");
   do_overlay_window(load_sel);
   need_update = 1;
   break;
  case SAVE_VRAM:
   save_sel->usr_flags = SAVE_VRAM;
   save_sel->load_proc = load_save_bottom;
   snprintf(save_sel->file_type_name, 6, "VRAM");
   do_overlay_window(save_sel);
   break;
  case SAVE_CRAM:
   save_sel->usr_flags = SAVE_CRAM;
   save_sel->load_proc = load_save_bottom;
   snprintf(save_sel->file_type_name, 6, "CRAM");
   do_overlay_window(save_sel);
   break;
  case SAVE_CRAM_HEAD:
   save_sel->usr_flags = SAVE_CRAM_HEAD;
   save_sel->load_proc = load_save_bottom;
   snprintf(save_sel->file_type_name, 6, "CRAM");
   do_overlay_window(save_sel);
   break;
  case SAVE_VSRAM:
   save_sel->usr_flags = SAVE_VSRAM;
   save_sel->load_proc = load_save_bottom;
   snprintf(save_sel->file_type_name, 6, "VSRAM");
   do_overlay_window(save_sel);
   break;
  case SAVE_HAPPY:
   save_sel->usr_flags = SAVE;
   save_sel->load_proc = load_save_mega;
   snprintf(save_sel->file_type_name, 6, "mega");
   do_overlay_window(save_sel);
   break;
  case CHANGE_LOAD_MESSAGE:
   text_edit_object->param.dp1 = load_message;
   text_edit_object->param.d2 = strlen(load_message);
   old_tick = (void *)globl_tick;
   globl_tick = null_tick;
   blinky = add_timer(text_edit_object, 12, MSG_TICK, 0, change_message_grp,
                      ACTIVE_ONLY_WITH_PARENT);
   group_loop(change_message_grp);
   del_timer(blinky);
   globl_tick = old_tick;
   break;

 }
 return RET_QUIT;
}

int load_save_top(struct object_t *obj, int data) {
 need_update = 0;
 group_loop((obj->param.user_flags == LOAD ? load_grp : save_grp));
 if(need_update == 1) {
  change_color(current_vdp->bg_pal, current_vdp->bg_index);
  render_vdp(0,vdp_h);
  broadcast_group(main_grp,MSG_DRAW,0);
 }
 return RET_OK;
}


int change_bg_color(struct object_t *obj, int data) {
 current_vdp->bg_pal = current_palette;
 current_vdp->bg_index = current_palette_index;
 render_vdp(0,vdp_h);
 MESSAGE_OBJECT(preview_object, MSG_DRAW);
 UPDATE_BG_COLOR;
 MESSAGE_OBJECT(bg_color_box, MSG_DRAW);
 return RET_OK;
}

int knob_tick(struct object_t *obj, int data) {
 if(obj->param.user_flags == SELECT_A) {
  knob_ab = SELECT_A;
  if(knob_or == HORIZONTAL) 
   knob->param.d1 = current_vdp->vram[knob_data_offset+1+(2*knob_ab)] |
                 ((current_vdp->vram[knob_data_offset+  (2*knob_ab)]&0x7)<<8);
  else 
   knob->param.d1 = current_vdp->vsram[knob_data_offset+1+(2*knob_ab)] |
                 ((current_vdp->vsram[knob_data_offset+  (2*knob_ab)]&0x7)<<8);
  MESSAGE_OBJECT(knob, MSG_DRAW);
  UPDATE_OBJECT(knob);
 }
 if(obj->param.user_flags == SELECT_B) {
  knob_ab = SELECT_B;
  if(knob_or == HORIZONTAL) 
   knob->param.d1 = current_vdp->vram[knob_data_offset+1+(2*knob_ab)] |
                 ((current_vdp->vram[knob_data_offset+  (2*knob_ab)]&0x7)<<8);
  else 
   knob->param.d1 = current_vdp->vsram[knob_data_offset+1+(2*knob_ab)] |
                 ((current_vdp->vsram[knob_data_offset+  (2*knob_ab)]&0x7)<<8);
  
  MESSAGE_OBJECT(knob, MSG_DRAW);
  UPDATE_OBJECT(knob);
 }

 if(obj->param.user_flags == KNOB) {
  if(knob_or == HORIZONTAL) {
   current_vdp->vram[knob_data_offset+1+(2*knob_ab)] = obj->param.d1 &0xff;
   current_vdp->vram[knob_data_offset+(2*knob_ab)]=(obj->param.d1&0x700)>>8;
  } else {
   current_vdp->vsram[knob_data_offset+1+(2*knob_ab)] = obj->param.d1 &0xff;
   current_vdp->vsram[knob_data_offset+(2*knob_ab)]=(obj->param.d1&0x700)>>8;
  }
 }
 render_vdp(0,vdp_h);
 MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
 MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
 MESSAGE_OBJECT(preview_object, MSG_DRAW);
 UPDATE_OBJECT(preview_object);
 UPDATE_OBJECT(pattern_select_object);
 UPDATE_OBJECT(pattern_edit_object);


 return RET_OK;
}

int knob_select(struct object_t *obj, int data) {
 snprintf(knob_text->param.dp1, 64, "Select!");
 MESSAGE_OBJECT(knob_message_box, MSG_DRAW);
 MESSAGE_OBJECT(knob_text, MSG_DRAW);
 MESSAGE_OBJECT(obj,MSG_DRAW);
 UPDATE_OBJECT(knob_message_box);

 SDL_SetCursor(no);
 
 group_loop(select_grp);

 SDL_SetCursor(arrow);

 knob_icon->param.d1 = FALSE;
 MESSAGE_OBJECT(knob, MSG_DRAW);
 MESSAGE_OBJECT(knob_message_box, MSG_DRAW);
 MESSAGE_OBJECT(knob_text, MSG_DRAW);
 MESSAGE_OBJECT(obj,MSG_DRAW);

 return RET_OK;
}

int edit_change(struct object_t *obj, int data) {
 switch(obj->param.user_flags) {
  case EDIT_SCROLL_A:
  case EDIT_SCROLL_B:
   vdp_w = (current_vdp->cell_w == 40 ? 320 : 256);
   vdp_h = (current_vdp->tv_type == NTSC ? 224 : 240);
   currently_editing = obj->param.user_flags;
   update_zoom(scroll_plane_zoom);
   DRAW_PREVIEW;
   return RET_OK;
  case EDIT_SPRITE:
   vdp_w = 8 * (sprite_width+1); 
   vdp_h = 8 * (sprite_height+1);
   currently_editing = obj->param.user_flags;
   update_zoom(sprite_zoom);
   switch(current_tool) {
    case SELECT:
     select_object->param.d1= FALSE;
     MESSAGE_OBJECT(select_object, MSG_DRAW);
     UPDATE_OBJECT(select_object);
     last_tool = (struct object_t *)-1;
     break;
    case PIC_PAT:
     pic_pat_object->param.d1= FALSE;
     MESSAGE_OBJECT(pic_pat_object, MSG_DRAW);
     UPDATE_OBJECT(pic_pat_object);
     last_tool = (struct object_t *)-1;
     break;
    case PUT_PAT:
     put_pat_object->param.d1= FALSE;
     MESSAGE_OBJECT(put_pat_object, MSG_DRAW);
     UPDATE_OBJECT(put_pat_object);
     last_tool = (struct object_t *)-1;
     break;
    case FLIP:
     flip_object->param.d1= FALSE;
     MESSAGE_OBJECT(flip_object, MSG_DRAW);
     UPDATE_OBJECT(flip_object);
     last_tool = (struct object_t *)-1;
     break;
    case HI_LOW:
    case PUT_PAL:
     pal_hi_low_object->param.d1= FALSE;
     MESSAGE_OBJECT(pal_hi_low_object, MSG_DRAW);
     UPDATE_OBJECT(pal_hi_low_object);
     last_tool = (struct object_t *)-1;
     break;


   }
   DRAW_PREVIEW;
   break;
  case VIEW_SCROLL_A:
   if(obj->param.d1 == 1) 
    visible |= VIS_SCROLL_A;
   else
    visible &= ~VIS_SCROLL_A;
   break;
  case VIEW_SCROLL_B:
   if(obj->param.d1 == 1)
    visible |= VIS_SCROLL_B;
   else
    visible &= ~VIS_SCROLL_B;
   break;
  case VIEW_SPRITE:
   if(obj->param.d1 == 1)
    visible |= VIS_SPRITE;
   else
    visible &= ~VIS_SPRITE;
   break;
  case VIEW_SCROLL_A_HIGH:
   if(obj->param.d1 == 1)
    visible |= VIS_SCROLL_A_HIGH;
   else
    visible &= ~VIS_SCROLL_A_HIGH;
   break;
  case VIEW_SCROLL_B_HIGH:
   if(obj->param.d1 == 1)
    visible |= VIS_SCROLL_B_HIGH;
   else
    visible &= ~VIS_SCROLL_B_HIGH;
   break;
  case VIEW_SPRITE_HIGH:
   if(obj->param.d1 == 1)
    visible |= VIS_SPRITE_HIGH;
   else
    visible &= ~VIS_SPRITE_HIGH;
   break;
 }
 render_vdp(0,vdp_h);
 MESSAGE_OBJECT(preview_object, MSG_DRAW);
 return RET_OK;
}

int hor_ver_menu_bot(struct object_t *obj, int data) {
 hor_ver = obj->param.user_flags;
 return RET_QUIT;
}

int high_low_menu_bot(struct object_t *obj, int data) {
 high_low = obj->param.user_flags;
 return RET_QUIT;
}

int pal_hi_low_menu_bot(struct object_t *obj, int data) {
 high_low = obj->param.user_flags;
 return RET_QUIT;
}

int copy_move_menu_bot(struct object_t *obj, int data) {
 copy_move = obj->param.user_flags;
 return RET_QUIT;
}


int tool_change(struct object_t *obj, int data) {
 if( obj == select_object) {
#define REPEAT \
  if(currently_editing == EDIT_SPRITE) { \
   obj->param.d1 = FALSE; \
   MESSAGE_OBJECT(obj, MSG_DRAW); \
   UPDATE_OBJECT(obj); \
   if(last_tool != (struct object_t *)-1) { \
    last_tool->param.d1 = TRUE; \
    MESSAGE_OBJECT(last_tool, MSG_DRAW); \
    UPDATE_OBJECT(last_tool); \
   } \
   return RET_OK; \
  }
  REPEAT;
  current_tool = SELECT;
  last_tool = obj;
  return RET_OK;
 }
 if( obj == fill_object) {
  current_tool = FILL;
  last_tool = obj;
  return RET_OK;
 }
 if(obj == line_object) {
  current_tool = LINE;
  last_tool = obj;
  return RET_OK;
 }
 if(obj == pic_object) {
  current_tool = PIC;
  last_tool = obj;
  return RET_OK;
 }
 if( obj == clear_to_color_object) {
  current_tool = CLRCOLOR;
  last_tool = obj;
  return RET_OK;
 }
 if( obj == pic_pat_object) {
  REPEAT;
  current_tool = PIC_PAT;
  last_tool = obj;
  return RET_OK;
 }
 if( obj == put_pat_object) {
  REPEAT;
  high_low_grp->pos_x = gui_mouse_x-6;
  high_low_grp->pos_y = gui_mouse_y-6;
  high_low = NOT_SELECTED;
  group_loop(high_low_grp);
  if(high_low == NOT_SELECTED) {
   MESSAGE_OBJECT(last_tool, MSG_CLICK);
   return RET_OK;
  }
  current_tool = PUT_PAT;
  return RET_OK;
 }
 if( obj == flip_object) {
  REPEAT;
  hor_ver_grp->pos_x = gui_mouse_x - 6;
  hor_ver_grp->pos_y = gui_mouse_y - 6;
  hor_ver = NOT_SELECTED;
  group_loop(hor_ver_grp); 
  if(hor_ver == NOT_SELECTED) {
   MESSAGE_OBJECT(last_tool, MSG_CLICK);
   return RET_OK;
  } 
  current_tool = FLIP;
  return RET_OK;
 }
 if( obj == pal_hi_low_object) {
  REPEAT;
  pal_hi_low_grp->pos_x = gui_mouse_x - 6;
  pal_hi_low_grp->pos_y = gui_mouse_y - 6;
  high_low = NOT_SELECTED;
  group_loop(pal_hi_low_grp);
  if(high_low == NOT_SELECTED) {
   MESSAGE_OBJECT(last_tool, MSG_CLICK);
   return RET_OK;
  }
  if(high_low == PALETTE) 
   current_tool = PUT_PAL;
  else
   current_tool = HI_LOW; 
  return RET_OK;
 }
 return RET_OK;
}

int change_background(struct object_t *obj, int data) {
 do_overlay_window(background_sel);

// background_object->param.dp1 = (char *)"b2.jpg";
// MESSAGE_OBJECT(background_object,MSG_RELOAD);
 broadcast_group(main_grp,MSG_DRAW,0);
 return RET_OK;
}

int load_background_callback(struct select_file_t *selector, char *filename) {
 group_t *old = 0;
 background_object->param.dp1 = strdup(filename);
 old = current_grp;
 current_grp = main_grp;
 MESSAGE_OBJECT(background_object,MSG_RELOAD);
 current_grp = old;
 if(CHECK_FLAG(background_object->param.flags, INVALID) == TRUE) 
  return NOPE_TRY_AGAIN;
 else
  return LOAD_OK_QUIT; 
}

int load_ovr_callback(struct select_file_t *selector, char *filename) {
 if(load_ovr(filename) == -1) {
  return NOPE_TRY_AGAIN;
 } else { 
  return LOAD_OK_QUIT;
 }
}

int load_ovr_window(struct object_t *obj, int data) {
 do_overlay_window(overlay_sel);
 MESSAGE_OBJECT(preview_object, MSG_DRAW);
 return RET_OK;
}

int undo_button(struct object_t *obj, int data) {
 UPDATE_OBJECT(obj);
 obj->param.d1 = FALSE;

 undo();

 render_vdp(0,vdp_h); 
 if(select_a_object->param.d1 == TRUE)
  knob_tick(select_a_object, 0);
 else
  knob_tick(select_b_object, 0);
 broadcast_group(current_grp,MSG_DRAW,0);
 return RET_OK;
}


int about(struct object_t *obj, int data) {
 scroll_text_window(gui_screen->w/2,gui_screen->h/2,600, 200, help_text, help_text_len);
 return RET_OK;
}

int really_quit(struct object_t *obj, int data) {
 if(prompt(gui_screen->w/2, gui_screen->h/2,
    "  Nah, really quit?  ",
    "Quit :O", "Nope :)") == TRUE) return RET_QUIT;
 return RET_OK;
}

int fw_bottom(struct object_t *obj, int data) {
#ifdef WINDOWS
 gui_flags^=SDL_FULLSCREEN;
 gui_screen = SDL_SetVideoMode(gui_w, gui_h, 24, gui_flags);
 broadcast_group(main_grp, MSG_DRAW, 0);
#else
 SDL_WM_ToggleFullScreen(gui_screen); 
#endif
 return RET_OK;
}

int sprite_overlay(struct object_t *obj, int data) {
 int store;
 obj->param.d1 = TRUE;
 MESSAGE_OBJECT(obj, MSG_DRAW);
 UPDATE_OBJECT(obj);

 if(obj == a_object) { 
  a_pattern = current_pattern;
  MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
  UPDATE_OBJECT(pattern_select_object);
 }

 if(obj == b_object) {
  b_pattern = current_pattern;
  if(b_pattern < a_pattern) {
   store = a_pattern;
   a_pattern = b_pattern;
   b_pattern = store;
  }
  MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
  UPDATE_OBJECT(pattern_select_object);

 }


 if(obj == plus_object) {
  sprite_overlay_pic = current_pattern + ((sprite_width+1)*(sprite_height+1));
  if(sprite_overlay_pic > b_pattern)
   sprite_overlay_pic = a_pattern;
  render_sprite();
  DRAW_PREVIEW;
 }


 if(obj == minus_object) {
  sprite_overlay_pic = current_pattern - ((sprite_width+1)*(sprite_height+1));
  if(sprite_overlay_pic < a_pattern)
   sprite_overlay_pic = b_pattern;
  render_sprite();
  DRAW_PREVIEW;
 } 

 if(data == 0)
  while(wait_on_mouse()!=MOUSE_UP);
 else
  while(wait_on_mouse()!=KEY_UP);

 sprite_overlay_pic = -1;
 render_sprite();
 DRAW_PREVIEW;
 obj->param.d1 = FALSE;
 MESSAGE_OBJECT(obj, MSG_DRAW);
 UPDATE_OBJECT(obj);

 return RET_OK;
}
