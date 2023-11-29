#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "./gui/libgui.h"
#include "config.h"
#include "mega.h"
#include "vdp.h"
#include "proc.h"
#include "bottom.h"

#include "cursors.h"

#include "pic/default_overlay.xpm"
#include "pic/back.xpm"
#include "pic/clear_to_color.xpm"
#include "pic/fill.xpm"
#include "pic/line.xpm"
#include "pic/pic.xpm"
#include "pic/undo.xpm"
#include "pic/zoom_in.xpm"
#include "pic/zoom_out.xpm"
#include "pic/scroll_a.xpm"
#include "pic/scroll_b.xpm"
#include "pic/window.xpm"
#include "pic/sprite.xpm"
#include "pic/scroll_a_edit.xpm"
#include "pic/scroll_b_edit.xpm"
#include "pic/window_edit.xpm"
#include "pic/sprite_edit.xpm"
#include "pic/small_knob.xpm"
#include "pic/pic_pat.xpm"
#include "pic/put_pat.xpm"
#include "pic/scroll_a_high.xpm"
#include "pic/scroll_b_high.xpm"
#include "pic/sprite_high.xpm"
#include "pic/flip.xpm"
#include "pic/stop.xpm"
#include "pic/new.xpm"
#include "pic/clear.xpm"
#include "pic/pal_hi_low.xpm"
#include "pic/select.xpm"
#include "pic/clob.xpm"
#include "pic/plus.xpm"
#include "pic/minus.xpm"
#include "pic/a.xpm"
#include "pic/b.xpm"


#define WIDTH	640	
#define	HEIGHT	480


group_t *main_grp;
group_t *load_grp, *save_grp;
group_t *high_low_grp;
group_t *pal_hi_low_grp;
group_t *hor_ver_grp;
group_t *copy_move_grp;
group_t *pointers;
group_t *select_grp;
group_t *change_message_grp;

SDL_Cursor *arrow, *cross, *vertical, *horizontal, *no;
SDL_Cursor *working_cursor, *cross_scroll_a, *cross_scroll_b, *cross_sprite;
SDL_Cursor *hor_a_flip, *ver_a_flip, *hor_b_flip, *ver_b_flip;

int current_tool;

int load_initial;
char *load_initial_name;

int undo_A, undo_B;

struct select_file_t *overlay_sel;
struct select_file_t *background_sel;

struct object_t *text_edit_object;

struct object_t *knob_message_box, *knob, *knob_icon, *knob_message;

struct object_t *knob_text;
struct object_t *rgb_box_object;
struct object_t *preview_box_object;
struct object_t *preview_scroll_bar;
struct object_t *preview_x_scroll;
struct object_t *preview_y_scroll;
struct object_t *preview_ntsc, *preview_pal;
struct object_t *preview_40c,  *preview_32c;
struct object_t *preview_zoom_in, *preview_zoom_out;
struct object_t *preview_overlay;
struct object_t *stop_object;
struct object_t *clob_object;
struct object_t *info_object;

struct object_t *sprite_radio;
struct object_t *hscroll_radio;
struct object_t *scroll_a_radio;
struct object_t *scroll_b_radio;
struct object_t *window_radio;
struct object_t *select_special_object;
struct object_t *horizontal_scroll_bar, *vertical_scroll_bar;

struct object_t *scroll_size1, *scroll_size2;
struct object_t *sprite_size1, *sprite_size2;

struct object_t *bg_color_box;

struct object_t *background_object;
struct object_t *select_object, *put_object, *fill_object, *line_object, *pic_object;
struct object_t *pic_pat_object, *put_pat_object, *flip_object;
struct object_t *clear_to_color_object, *pal_hi_low_object, *select_object;

struct object_t *plus_object, *minus_object, *a_object, *b_object;
struct object_t *select_a_object, *select_b_object;


int sprite_overlay_pic = -1;
int a_pattern=0, b_pattern=0;

color_t sprite_color, hscroll_color, scroll_a_color, scroll_b_color, window_color;

struct menu_entry_t load_menu[] = {
 {"  MEGA  Text     ", CALL_BUTTON, LOAD_HAPPY, load_save_middle},
 {"  VRAM  Binary   ", CALL_BUTTON, LOAD_VRAM,  load_save_middle},
 {"  VSRAM Binary   ", CALL_BUTTON, LOAD_VSRAM, load_save_middle},
 {"  CRAM  Binary   ", CALL_BUTTON, LOAD_CRAM,  load_save_middle},
 {"  BMP   Binary   ", CALL_BUTTON, LOAD_BMP,  load_save_middle},
 {(char *)NULL,0,0,NULL}
};

struct menu_entry_t save_menu[] = {
 {"  MEGA  Text     ", CALL_BUTTON, SAVE_HAPPY, load_save_middle},
 {"  VRAM  Binary   ", CALL_BUTTON, SAVE_VRAM,  load_save_middle},
 {"  VSRAM Binary   ", CALL_BUTTON, SAVE_VSRAM, load_save_middle},
 {"  CRAM  Binary   ", CALL_BUTTON, SAVE_CRAM,  load_save_middle},
 {"  CRAM  Text     ", CALL_BUTTON, SAVE_CRAM_HEAD, load_save_middle},
/* {"  LOAD MESSAGE   ", CALL_BUTTON, CHANGE_LOAD_MESSAGE, load_save_middle},*/
 {(char *)NULL,0,0,NULL}
};

struct menu_entry_t high_low_menu[] = {
 {" HIGH ", CALL_BUTTON,HIGH,high_low_menu_bot},
 {" LOW  ", CALL_BUTTON,LOW, high_low_menu_bot},
 {(char *)NULL,0,0,NULL}
};

struct menu_entry_t horizontal_vertical_menu[] = {
 {" HORIZONTAL ", CALL_BUTTON, HORIZONTAL, hor_ver_menu_bot},
 {" VERTICAL   ", CALL_BUTTON, VERTICAL, hor_ver_menu_bot},
 {(char *)NULL,0,0,NULL}
};

struct menu_entry_t pal_hi_low_menu[] = {
 {" PALETTE ", CALL_BUTTON, PALETTE, pal_hi_low_menu_bot},
 {"  HIGH   ", CALL_BUTTON, HIGH, pal_hi_low_menu_bot},
 {"  LOW    ", CALL_BUTTON, LOW,  pal_hi_low_menu_bot},
 {(char *)NULL, 0, 0, NULL}
};

struct menu_entry_t copy_move_menu[] = {
 {" COPY ", CALL_BUTTON, COPY, copy_move_menu_bot},
 {" MOVE ", CALL_BUTTON, MOVE, copy_move_menu_bot},
 {(char *)NULL, 0, 0, NULL}
};

int load_ovr(char *filename) {
 int x=0, y=0;
 Uint8 r=0,g=0,b=0, *pix=0;
 int color=0;
 int i=0;
 SDL_Surface *tmp_surface=0;


 if( filename != NULL)
  tmp_surface = IMG_Load(filename);
 else
  tmp_surface = IMG_ReadXPMFromArray((char **)default_overlay_xpm);

 if(!tmp_surface) return -1;

 pix = tmp_surface->pixels;
 i = 0;
 
 for(y=0;y<240;y++) {
  for(x=0;x<320;x++) {
   if(x < tmp_surface->w &&
      y < tmp_surface->h) {
     color = *(int *)pix;
     SDL_GetRGB(color,tmp_surface->format, &r, &g, &b);
    } else {
     /* generate some pretty hash */
     r = x^y;
     g = x^y;
     b = ((x^y)&0xf)*0xf;
   }
   vdp_screen[i].ovr_color.r = r;
   vdp_screen[i].ovr_color.g = g;
   vdp_screen[i++].ovr_color.b = b;
   pix+=tmp_surface->format->BytesPerPixel;
  }
  pix-= (tmp_surface->format->BytesPerPixel * 320);
  pix+= tmp_surface->pitch;
 }
 SDL_FreeSurface(tmp_surface);
 return 1;
}

void save_state(void) {
 undo_A++;
 if((undo_A-undo_B) > MAX_UNDO)
  undo_B++;
 memcpy(undo_vram[undo_A%MAX_UNDO], undo_vram[(undo_A-1)%MAX_UNDO],0xffff * sizeof(Uint8));
 current_vdp->vram = undo_vram[undo_A%MAX_UNDO];
}

void undo(void) {
 undo_A--;
 if(undo_A<=undo_B)
  undo_A++;
 current_vdp->vram = undo_vram[undo_A%MAX_UNDO];
}

/*setup_windows{{{*/
void setup_windows(int flags) {
 obj_param_t tmp_parm;

 globl_drop_depth =25;
 globl_flags = DROP_SHADOW;
 init_gui(WIDTH, HEIGHT, flags);

 SET_COLOR(globl_fg, 		0   ,0   ,0);
 SET_COLOR(globl_bg, 		224 ,224 ,204);
 SET_COLOR(globl_move_color, 	0x1f,0x8f,0x1f);
 SET_COLOR(hscroll_color, 	0   ,0x7f,0xff);
 SET_COLOR(sprite_color, 	0xff,0x7f,0);
 SET_COLOR(window_color, 	0   ,0   ,0xff);
 SET_COLOR(scroll_b_color, 	0xff,0   ,0xff);
 SET_COLOR(scroll_a_color, 	0xff,0   ,0);

 change_message_grp = new_group(40,40, 202, 77, globl_flags, globl_drop_depth);
 simple_window(change_message_grp, 200, 75);
 PARM(50, 45,100,20, &globl_fg, &globl_bg, SHOW_FOCUS | QUIT_BUTTON, proc_button_box);
 tmp_parm.dp1 = (void *)"Okay";
 new_obj(change_message_grp, &tmp_parm);
 PARM(12,16,172,20,&globl_fg, &globl_bg, MAX_CHARS|SHOW_FOCUS, proc_edit_line);
 tmp_parm.d3 = 80;
 text_edit_object = new_obj(change_message_grp, &tmp_parm);

 select_grp = new_group(0,0,WIDTH, HEIGHT,0,0);
 PARM(305,126,320,240,&globl_fg,&globl_bg,0,select_special);
 select_special_object = new_obj(select_grp, &tmp_parm);


 PARM(0,0, 304, 126, 0,0, 0, select_quit_on_click);
 tmp_parm.user_flags = 0;
 new_obj(select_grp, &tmp_parm);

 PARM(0,126,304,240, 0,0, 0, select_quit_on_click);
 tmp_parm.user_flags = HORIZONTAL;
 new_obj(select_grp, &tmp_parm);

 PARM(0, 366, 305, 112, 0,0, 0, select_quit_on_click);
 tmp_parm.user_flags = 0;
 new_obj(select_grp, &tmp_parm);

 PARM(305, 0, 320, 126, 0,0, 0, select_quit_on_click);
 new_obj(select_grp, &tmp_parm);

 PARM(305, 366, 320, 114, 0,0, 0, select_quit_on_click);
 tmp_parm.user_flags = VERTICAL;
 new_obj(select_grp, &tmp_parm);

 PARM(625, 0, 15, 480, 0,0,0, select_quit_on_click);
 tmp_parm.user_flags = 0;
 new_obj(select_grp, &tmp_parm);

 main_grp = new_group(0,0, WIDTH, HEIGHT, 0, 0);

 PARM(0,0,WIDTH,HEIGHT,&globl_fg,&globl_bg,DROP_ACCUM|LOAD_XPM_FROM_ARRAY,proc_bitmap);
 tmp_parm.dp1 = back_xpm;
 background_object = new_obj(main_grp, &tmp_parm);

 PARM(82,22,0,0,&globl_fg,&globl_bg,
   TOGGLE|CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = undo_button;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 1;
 tmp_parm.dp1 = undo_xpm;
 new_obj(main_grp, &tmp_parm);


 /* tools */

 PARM(124, 22, 0, 0, &globl_fg, &globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY, proc_icon_button);
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = select_xpm;
 select_object = new_obj(main_grp, &tmp_parm);

 PARM(82, 53, 0, 0, &globl_fg, &globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY, proc_icon_button);
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = pal_hi_low_xpm;
 pal_hi_low_object = new_obj(main_grp, &tmp_parm);

 PARM(124, 53, 0, 0, &globl_fg, &globl_bg, 
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = flip_xpm;
 flip_object = new_obj(main_grp, &tmp_parm);
 

 PARM(82,88,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = line_xpm;
 line_object = new_obj(main_grp, &tmp_parm);
 last_tool = line_object;

 PARM(124,88,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = put_pat_xpm;
 put_pat_object = new_obj(main_grp, &tmp_parm);

 PARM(82,123,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button); 
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = fill_xpm;
 fill_object = new_obj(main_grp, &tmp_parm);

 PARM(124,123,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = clear_to_color_xpm;
 clear_to_color_object = new_obj(main_grp, &tmp_parm);


 PARM(82,158,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button); 
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = pic_xpm;
 pic_object = new_obj(main_grp, &tmp_parm);

 PARM(124,158,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button); 
 tmp_parm.callback = tool_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = pic_pat_xpm;
 pic_pat_object = new_obj(main_grp, &tmp_parm);



 /* not tools */

#ifndef WINDOWS
/* PARM(580,450,50,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON|DROP_SHADOW,proc_button_box);
 tmp_parm.dp1 = (void *)"F/W";
 tmp_parm.callback = fw_bottom;
 new_obj(main_grp, &tmp_parm);
*/
#endif

 
 PARM(580,1,50,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON|DROP_SHADOW,proc_button_box);
 tmp_parm.dp1 = (void *)"Quit";
 tmp_parm.callback = really_quit;
 new_obj(main_grp, &tmp_parm);

/*
 * FIXME window crashes don't know why 
 PARM(500,1,50,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON|DROP_SHADOW,proc_button_box);
 tmp_parm.dp1 = (void *)"About";
 tmp_parm.callback = about;
 new_obj(main_grp, &tmp_parm);
*/

 PARM(20,1, 50,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON|DROP_SHADOW,proc_button_box);
 tmp_parm.dp1 = (void *)"Load";
 tmp_parm.user_flags = LOAD;
 tmp_parm.callback=load_save_top;
 new_obj(main_grp, &tmp_parm);

 load_grp = new_menu(20,5, load_menu, &globl_fg, &globl_bg);

 PARM(75,1, 50,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON|DROP_SHADOW,proc_button_box);
 tmp_parm.dp1 = (void *)"Save";
 tmp_parm.user_flags = SAVE;
 tmp_parm.callback=load_save_top;
 new_obj(main_grp, &tmp_parm);

 save_grp = new_menu(75,5, save_menu, &globl_fg, &globl_bg);
 high_low_grp = new_menu(124,88, high_low_menu, &globl_fg, &sprite_color);
 hor_ver_grp = new_menu(124,88,horizontal_vertical_menu, &globl_fg, &sprite_color);
 pal_hi_low_grp = new_menu(124,88, pal_hi_low_menu, &globl_fg, &sprite_color);
 copy_move_grp = new_menu(124,88,copy_move_menu, &globl_fg, &sprite_color);

 /*
 PARM(380,1,90,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON|DROP_SHADOW,proc_button_box);
 tmp_parm.dp1 = (void *)"Background";
 tmp_parm.callback = change_background;
 new_obj(main_grp, &tmp_parm);
*/
 PARM(425,81,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = preview_zoom_change;
 tmp_parm.d1 = FALSE;
 tmp_parm	.d2 = 4;
 tmp_parm.dp1 = zoom_out_xpm;
 preview_zoom_out = new_obj(main_grp,&tmp_parm);

 PARM(461,81,0,0,&globl_fg,&globl_bg,
   CALL_BUTTON|SHOW_FOCUS|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = preview_zoom_change;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 5;
 tmp_parm.dp1 = zoom_in_xpm;
 preview_zoom_in = new_obj(main_grp,&tmp_parm);

 PARM(499,83, 31,27, &globl_fg,&globl_bg,CALL_BUTTON,proc_button_box);
 tmp_parm.callback = change_bg_color;
 tmp_parm.dp1 = (char *)" ";
 bg_color_box = new_obj(main_grp, &tmp_parm);
 UPDATE_BG_COLOR;

 PARM(305,114,320,11,&globl_move_color, &globl_bg,SHOW_FOCUS|CALL_BUTTON,proc_scroll_bar);
 tmp_parm.d2 = 256;
 tmp_parm.d1 = 0;
 tmp_parm.callback = preview_scroll_change;
 preview_scroll_bar = new_obj(main_grp, &tmp_parm);

 PARM(292,126,11,240,&globl_move_color,&globl_bg,SHOW_FOCUS|CALL_BUTTON,proc_scroll_bar);
 tmp_parm.d2 = 0;
 tmp_parm.d1 = 0;
 preview_y_scroll = new_obj(main_grp,&tmp_parm);

 PARM(305,367,320,11,&globl_move_color,&globl_bg,SHOW_FOCUS|CALL_BUTTON,proc_scroll_bar);
 tmp_parm.d2 = 0;
 tmp_parm.d1 = 0;
 preview_x_scroll = new_obj(main_grp,&tmp_parm);


 tmp_parm.callback = preview_size_change;
 PARM(293,82,0,0,&globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON, proc_radio_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 1;
 tmp_parm.dp1 = (void *)"NTSC";
 preview_ntsc = new_obj(main_grp, &tmp_parm);

 PARM(293,96,0,0,&globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 1;
 tmp_parm.dp1 = (void *)"PAL";
 preview_pal = new_obj(main_grp, &tmp_parm);

 PARM(348,82,0,0,&globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON, proc_radio_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = (void *)"40 cell";
 preview_40c = new_obj(main_grp, &tmp_parm);

 PARM(348,96,0,0,&globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 2;
 tmp_parm.dp1 = (void *)"32 cell";
 preview_32c = new_obj(main_grp, &tmp_parm);




 PARM(190,297,11,176,&globl_move_color,&globl_bg,CALL_BUTTON|SHOW_FOCUS,proc_scroll_bar);
 tmp_parm.d2 = 0xf6;
 tmp_parm.d1 = 0;
 tmp_parm.callback = pattern_select_bot;
 pattern_select_scroll_bar = new_obj(main_grp, &tmp_parm);

 
 PARM(10,301,179,0xa*16,&globl_fg,&globl_bg,0,proc_pattern_select);
 pattern_select_object = new_obj(main_grp, &tmp_parm);

 PARM(10,22,70,260,&globl_fg,&globl_bg,0,proc_palette_change_object);
 palette_change_object = new_obj(main_grp, &tmp_parm);

 /* Pattern edit */
 PARM(88,200,67,83,&globl_fg,&globl_bg,0,proc_pattern_edit);
 pattern_edit_object = new_obj(main_grp, &tmp_parm);

 PARM(305,126,320,240,&globl_fg,&globl_bg,0,proc_preview_object);
 preview_object = new_obj(main_grp,&tmp_parm);

 PARM(540,88,65,17,&globl_fg,&globl_bg,SHOW_FOCUS|CALL_BUTTON,proc_button_box);
 tmp_parm.dp1 = (void *)"overlay";
 tmp_parm.callback = load_ovr_window;
 preview_overlay = new_obj(main_grp, &tmp_parm);


/* View and edit what? */

 tmp_parm.callback = edit_change;
 PARM(254,96,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY, proc_icon_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 3;
 tmp_parm.user_flags = EDIT_SCROLL_A;
 tmp_parm.dp1 = scroll_a_edit_xpm;
 new_obj(main_grp, &tmp_parm);
 currently_editing = EDIT_SCROLL_A;

 PARM(254,120,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY, proc_icon_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 3;
 tmp_parm.user_flags = EDIT_SCROLL_B;
 tmp_parm.dp1 = scroll_b_edit_xpm;
 new_obj(main_grp, &tmp_parm);

 PARM(254,144,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY, proc_icon_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 3;
 tmp_parm.user_flags = EDIT_SPRITE;
 tmp_parm.dp1 = sprite_edit_xpm;
 new_obj(main_grp, &tmp_parm);


 tmp_parm.callback = edit_change;
 PARM(230,96,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY|SINGLE_RADIO, proc_icon_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 4;
 tmp_parm.user_flags = VIEW_SCROLL_B;
 tmp_parm.dp1 = scroll_b_xpm;
 new_obj(main_grp, &tmp_parm);

 PARM(230,120,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY|SINGLE_RADIO, proc_icon_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 4;
 tmp_parm.user_flags = VIEW_SCROLL_A;
 tmp_parm.dp1 = scroll_a_xpm;
 new_obj(main_grp, &tmp_parm);


 PARM(230,120+24,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY|SINGLE_RADIO, proc_icon_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 4;
 tmp_parm.user_flags = VIEW_SCROLL_B_HIGH;
 tmp_parm.dp1 = scroll_b_high_xpm;
 new_obj(main_grp, &tmp_parm);

 PARM(230,120+24+24,0,0,&globl_fg,&globl_bg, 
   SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY|SINGLE_RADIO, proc_icon_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 4;
 tmp_parm.user_flags = VIEW_SCROLL_A_HIGH;
 tmp_parm.dp1 = scroll_a_high_xpm;
 new_obj(main_grp, &tmp_parm);





 PARM(353,25,31,31, &hscroll_color, &globl_bg, 0, proc_scroll_size);
 new_obj(main_grp, &tmp_parm); 

 PARM(415,25,41,41, &globl_fg, &globl_bg, 0, proc_sprite_size);
 new_obj(main_grp, &tmp_parm); 

 PARM(369, 64,0,0,&globl_fg, &globl_bg, 0, proc_ctext);
 tmp_parm.dp1 = (void *)malloc( 8 );
 snprintf(tmp_parm.dp1, 8, "64x64");
 scroll_width = 1;
 scroll_height = 1;

 scroll_size2 =new_obj(main_grp, &tmp_parm);

 sprite_width = 0;
 sprite_height = 0;

 PARM(410, 393,41,51, &globl_fg, &globl_bg, CALL_BUTTON|MODULAR|HEX|INACTIVE, proc_knob);
 tmp_parm.d1 = 40;
 tmp_parm.d2 = 0x400;
 tmp_parm.user_flags = KNOB;
 tmp_parm.callback = knob_tick;
 knob = new_obj(main_grp, &tmp_parm);


 PARM(244,410, 23,24,&globl_fg, &globl_bg,
   LOAD_XPM_FROM_ARRAY|SHOW_FOCUS|CALL_BUTTON|SINGLE_RADIO,proc_icon_button);
 tmp_parm.dp1 = small_knob_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 200;
 tmp_parm.callback = knob_select;
 knob_icon = new_obj(main_grp,&tmp_parm);

 PARM(315, 422, 0,0, &globl_fg, &globl_bg, 0, proc_ctext);
 tmp_parm.dp1 = (void *)malloc(64);
 snprintf( tmp_parm.dp1, 64, ":<");
 knob_text = new_obj(main_grp,&tmp_parm);


 PARM(272, 256, 11, 82, &globl_fg, &globl_bg, 0, proc_scroll_bar_special)
 tmp_parm.d1 = 2;
 tmp_parm.d2 = 2;
 tmp_parm.user_flags = HORIZONTAL;
 horizontal_scroll_bar = new_obj(main_grp, &tmp_parm);

 PARM(493,385, 82,11, &globl_fg, &globl_bg, 0, proc_scroll_bar_special);
 tmp_parm.d1 = 0;
 tmp_parm.d2 = 1;
 tmp_parm.user_flags = VERTICAL;
 vertical_scroll_bar = new_obj(main_grp, &tmp_parm);

 PARM(462, 401, 0,0, &globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY,
   proc_icon_button);
 tmp_parm.d1 = TRUE;
 tmp_parm.d2 = 5;
 tmp_parm.dp1 = (void *)scroll_a_edit_xpm;
 tmp_parm.callback = knob_tick;
 tmp_parm.user_flags = SELECT_A;
 select_a_object = new_obj(main_grp, &tmp_parm);

 PARM(462, 423, 0,0, &globl_fg, &globl_bg, SHOW_FOCUS|CALL_BUTTON|LOAD_XPM_FROM_ARRAY,
   proc_icon_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 5;
 tmp_parm.dp1 = (void *)scroll_b_edit_xpm;
 tmp_parm.callback = knob_tick;
 tmp_parm.user_flags = SELECT_B;
 new_obj(main_grp, &tmp_parm);

 tmp_parm.proc = load_default_mega;
 select_b_object = new_obj(main_grp, &tmp_parm);

 PARM(172, 177, 0,0, &globl_fg, &globl_bg, SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,
   proc_icon_button);
// tmp_parm.callback = stop_bot;
 tmp_parm.dp1 = (void *)clob_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 clob_object = new_obj(main_grp, &tmp_parm);

 PARM(172, 202, 0,0, &globl_fg, &globl_bg, CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,
   proc_icon_button);
 tmp_parm.callback = stop_bot;
 tmp_parm.dp1 = (void *)stop_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 stop_object = new_obj(main_grp, &tmp_parm);

 PARM(172, 227, 0,0, &globl_fg, &globl_bg, CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,
   proc_icon_button);
 tmp_parm.callback = go_bot;
 tmp_parm.dp1 = (void *)clear_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 new_obj(main_grp, &tmp_parm);

 PARM(172, 252, 0,0, &globl_fg, &globl_bg, CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,
   proc_icon_button);
 tmp_parm.callback = new_bot;
 tmp_parm.dp1 = (void *)new_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 new_obj(main_grp, &tmp_parm);

 PARM(490, 400, 140, 45, &globl_fg, &globl_bg, DROP_SHADOW, proc_info);
 info_object = new_obj(main_grp, &tmp_parm);

/* SHADOW BOX */

 PARM(170, 175, 25, 100, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 PARM(460, 400, 25, 45, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp, &tmp_parm);


 PARM(270, 254, 14, 86, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 PARM(491,383, 86, 14, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 PARM(240, 407, 150, 30, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 knob_message_box = new_obj(main_grp, &tmp_parm);

 PARM(405, 400, 50, 45, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 PARM(340, 21, 58, 50, &globl_fg,&globl_bg, DROP_SHADOW, proc_shadow_box);
 scroll_size1 = new_obj(main_grp, &tmp_parm);

 PARM(340+67, 21, 58, 50, &globl_fg,&globl_bg, DROP_SHADOW, proc_shadow_box);
 sprite_size1 = new_obj(main_grp, &tmp_parm);


 PARM(340+67+67+((58/4)-6), 20+((50/4)-6)-1, 20, 20, &globl_fg, &globl_bg, 
   CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.callback = sprite_overlay;
 tmp_parm.dp1 = (void *)minus_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 minus_object = new_obj(main_grp, &tmp_parm);

 PARM(340+67+67+((58/4)-6), 20+((50/2)+2)-1, 20, 20, &globl_fg, &globl_bg, 
   CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.dp1 = (void *)plus_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 plus_object =  new_obj(main_grp, &tmp_parm);


 PARM(340+67+67+((58/4)+15), 20+((50/4)-6)-1, 20, 20, &globl_fg, &globl_bg, 
   CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.dp1 = (void *)a_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 a_object = new_obj(main_grp, &tmp_parm);

 PARM(340+67+67+((58/4)+15), 20+((50/2)+2)-1, 20, 20, &globl_fg, &globl_bg, 
   CALL_BUTTON|SHOW_FOCUS|SINGLE_RADIO|LOAD_XPM_FROM_ARRAY,proc_icon_button);
 tmp_parm.dp1 = (void *)b_xpm;
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 401;
 b_object = new_obj(main_grp, &tmp_parm);





 PARM(340+67+67,21,58,50, &globl_fg, &globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp,&tmp_parm);
 
 
 PARM(228, 94, 50, 148-24-24, &globl_fg,&globl_bg, DROP_SHADOW, proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 PARM(8,19,152,266,&globl_fg,&globl_bg,DROP_SHADOW,proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 PARM(8,295,198,179,&globl_fg,&globl_bg,DROP_SHADOW,proc_shadow_box);
 new_obj(main_grp, &tmp_parm);

 

 PARM(288,80,340,300,&globl_fg,&globl_bg,DROP_SHADOW,proc_shadow_box);
 preview_box_object =  new_obj(main_grp, &tmp_parm);

 red.r = 0x7f;
 red.g = 0;
 red.b = 0;

 green.r = 0;
 green.g = 0x7f;
 green.b = 0;
  
 blue.r = 0;
 blue.g = 0;
 blue.b = 0x7f;

 PARM(168,22,100,11,&red,&globl_bg,SHOW_FOCUS|CALL_BUTTON,proc_scroll_bar);
 tmp_parm.d2 = 7;
 tmp_parm.d1 = 
  current_vdp->palette[current_palette][current_palette_index].r / (0xff/7);
 tmp_parm.callback = color_change_bot;
 rgb_red_object = new_obj(main_grp, &tmp_parm);

 tmp_parm.d1 = 
  current_vdp->palette[current_palette][current_palette_index].g / (0xff/7);
 tmp_parm.y+=18;
 tmp_parm.fg = &green;
 rgb_green_object = new_obj(main_grp, &tmp_parm);


 tmp_parm.d1 = 
  current_vdp->palette[current_palette][current_palette_index].b / (0xff/7);
 tmp_parm.y+=18;
 tmp_parm.fg = &blue;
 rgb_blue_object = new_obj(main_grp, &tmp_parm);

 PARM(301,38,24,8,&current_color_text_color,&globl_bg,0,proc_ctext);
 tmp_parm.dp1 = (void *)malloc(4);
 current_color_text_object = new_obj(main_grp,&tmp_parm);

 PARM(301,52,24,8, &current_color_text_color, &globl_bg, 0,proc_ctext);
 tmp_parm.dp1 = (void *)malloc(4);
 current_color_text2_object = new_obj(main_grp, &tmp_parm);
  
 update_color_text();

 PARM(276,30,50,30,&globl_fg,&current_vdp->palette[0][0],0,proc_shadow_box);
 current_color_object = new_obj(main_grp, &tmp_parm);

 PARM(165,19,170,55,&globl_fg,&globl_bg,DROP_SHADOW,proc_shadow_box);
 rgb_box_object = new_obj(main_grp, &tmp_parm);



 overlay_sel = setup_overlay_window(400,300, LOAD, "an image", load_ovr_callback);
 background_sel = setup_overlay_window(400,300, LOAD, "an image", load_background_callback);


 pointers = new_group(200, 300, 151,151,globl_flags,globl_drop_depth);
 simple_window(pointers, 150, 150);

 PARM( 25, 100, 100, 30, &globl_fg, &globl_bg, QUIT_BUTTON|SHOW_FOCUS, proc_button_box);
 tmp_parm.dp1 = (char *)"OK";
 new_obj(pointers, &tmp_parm);

 PARM(30, 20, 0,0, &hscroll_color, &globl_bg, SHOW_FOCUS|SINGLE_RADIO, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 50;
 tmp_parm.dp1 = (char *)"hscroll";
 hscroll_radio = new_obj(pointers, &tmp_parm);

 PARM(30, 31, 0,0, &sprite_color, &globl_bg, SHOW_FOCUS|SINGLE_RADIO, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 50;
 tmp_parm.dp1 = (char *)"sprite";
 sprite_radio = new_obj(pointers, &tmp_parm);

 PARM(30, 42, 0,0, &scroll_a_color, &globl_bg, SHOW_FOCUS|SINGLE_RADIO, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 50;
 tmp_parm.dp1 = (char *)"scroll_a";
 scroll_a_radio = new_obj(pointers, &tmp_parm);

 PARM(30, 53, 0,0, &scroll_b_color, &globl_bg, SHOW_FOCUS|SINGLE_RADIO, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 50;
 tmp_parm.dp1 = (char *)"scroll_b";
 scroll_b_radio = new_obj(pointers, &tmp_parm);

 PARM(30, 64, 0,0, &window_color, &globl_bg, SHOW_FOCUS|SINGLE_RADIO, proc_radio_button);
 tmp_parm.d1 = FALSE;
 tmp_parm.d2 = 50;
 tmp_parm.dp1 = (char *)"window";
 window_radio = new_obj(pointers, &tmp_parm);

 load_sel = setup_overlay_window(400,300, LOAD, "123456", load_save_bottom);
 save_sel = setup_overlay_window(400,300, SAVE, "123456", load_save_bottom);
}
/*}}}*/

static SDL_Cursor *cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for ( row=0; row<32; ++row ) {
    for ( col=0; col<32; ++col ) {
      if ( col % 8 ) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

int caption_thread(void *a) {
 char *store, *buf;
 int size, i, j;
 
 store = strdup(CAPTION);
 buf = strdup(CAPTION);
 size = strlen(store);
 
 for(i=0;;i++) {
  for(j=0;j<size;j++) {
   buf[j] = store[(j+i)%size];
  }
  SDL_SetWindowTitle(win, buf);
  SDL_Delay(100);
 }
}

int main(int argc, char **argv) {
 Uint8 cram_buffer[128];
 int flags;
 int i;

#ifndef WINDOWS
 printf(
"_  _ ____ ____ ____    _  _ ____ ___  ___  _   _    ____ ___  ____ _ ___ ____\n"
"|\\/| |___ | __ |__|    |__| |__| |__] |__]  \\_/     [__  |__] |__/ |  |  |___\n"
"|  | |___ |__] |  |    |  | |  | |    |      |      ___] |    |  \\ |  |  |___\n" 
 );                                                                      

#endif


 gui_screen = (SDL_Surface *)0;
 vdp_init();
 flags = 0;
 load_initial = FALSE;
 for(i=1;i<argc;i++){
  if(strncmp("-f", argv[i], 3)==0) {
  // flags = FULLSCREEN;
   printf("FIX THIS\n");
  } else
   if(argv[i][0]!='-'){
    load_initial = TRUE;
    load_initial_name = strdup(argv[i]);
   }
 }

 /* FIX this is a hack. Some how the default rgb scroll bars are not 0 when
  * loading fresh */

 current_vdp->palette[0][0].r = 0;
 current_vdp->palette[0][0].g = 0;
 current_vdp->palette[0][0].b = 0;
 

 setup_windows(flags);

 /* now that we have gui_screen->format we can map colors */
 memset(cram_buffer,0,128);
 load_palette(current_vdp, cram_buffer);
 

 arrow = cursor(arrow_data);
 cross = cursor(cross_data);
 horizontal = cursor(horizontal_data);
 vertical = cursor(vertical_data);
 no = cursor(no_data);
 cross_scroll_a = cursor(cross_scroll_a_data);
 cross_scroll_b = cursor(cross_scroll_b_data);
 cross_sprite = cursor(cross_sprite_data);
 hor_a_flip = cursor(hor_a_flip_data);
 ver_a_flip = cursor(ver_a_flip_data);
 hor_b_flip = cursor(hor_b_flip_data);
 ver_b_flip = cursor(ver_b_flip_data);

 working_cursor = cross;
 SDL_SetCursor(arrow);
 undo_A = 
  undo_B = 0;
 hor_ver = HORIZONTAL;
 selection_v1.x = NO_SELECTION;
 save_state();
 load_ovr(NULL);
 render_vdp(0,vdp_h);
 SDL_CreateThread(caption_thread, "caption", (void *)0);

 group_loop(main_grp);
 SDL_Quit();

#ifndef WINDOWS
 printf("\nThanks for using this program!!\nIf you want to contact the author email pantsbutt@gmail.com\n");
#endif
 return 0;
}
