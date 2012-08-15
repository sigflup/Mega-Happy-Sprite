#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "./gui/libgui.h"
#include "config.h"
#include "mega.h"
#include "vdp.h"
#include "draw.h"
#include "proc.h"
#include "bottom.h"
#include "mega_file.h"

struct object_t *pattern_select_scroll_bar,
		*pattern_select_object;

struct object_t *pattern_edit_object,
		*palette_change_object,
		*current_color_object,
		*current_color_text_object,
		*current_color_text2_object;

struct object_t *preview_object;

struct object_t *rgb_red_object,
		*rgb_green_object,
		*rgb_blue_object;

int vdp_w, vdp_h;

color_t red,green,blue;
color_t current_color_text_color;

coord_t selection_v1, selection_v2;
Uint8 selection_buffer[320*240];

int sprite_zoom = 0, scroll_plane_zoom=0;

/*update_color_text{{{*/
void update_color_text(void) {
 float luma;
 snprintf(current_color_text_object->param.dp1, 4,"%d%d%d", 
   current_vdp->palette[current_palette][current_palette_index].r / (0xff/7),
   current_vdp->palette[current_palette][current_palette_index].g / (0xff/7),
   current_vdp->palette[current_palette][current_palette_index].b / (0xff/7));
 snprintf(current_color_text2_object->param.dp1,4,"%X.%X", 
   current_palette, current_palette_index);

 luma =(((float)current_vdp->palette[current_palette][current_palette_index].r * 0.2f) +
        ((float)current_vdp->palette[current_palette][current_palette_index].g * 0.6f) +
        ((float)current_vdp->palette[current_palette][current_palette_index].b * 0.2f));

 if(luma>86.0f) {
  current_color_text_color.r = 0;
  current_color_text_color.g = 0;
  current_color_text_color.b = 0; 
 } else {
  current_color_text_color.r = 0xff;
  current_color_text_color.g = 0xff;
  current_color_text_color.b = 0xff; 
 }
 MAP_COLOR(current_color_text_color);
}
/*}}}*/
/*change_color{{{*/
void change_color(int palette, int index) {
 int diff;
 if(current_palette != palette) 
  diff = 1;
 else 
  diff = 0;
 current_palette = palette;
 current_palette_index = index;
 current_color_object->param.bg = &current_vdp->palette[palette][index];
 rgb_red_object->param.d1 = 
  current_vdp->palette[current_palette][current_palette_index].r / (0xff/7);
 rgb_green_object->param.d1 = 
  current_vdp->palette[current_palette][current_palette_index].g / (0xff/7);
 rgb_blue_object->param.d1 = 
  current_vdp->palette[current_palette][current_palette_index].b / (0xff/7);
 update_color_text();
 DRAW_RGB_CHOOSER;
 MESSAGE_OBJECT(palette_change_object, MSG_DRAW);
 if(diff == 1) {
  MESSAGE_OBJECT(pattern_edit_object,MSG_DRAW);
  MESSAGE_OBJECT(pattern_select_object,MSG_DRAW);
  //MESSAGE_OBJECT(preview_object, MSG_DRAW);
 }
}
/*}}}*/
/*change_color_bot{{{*/
int color_change_bot(struct object_t *obj, int data) {
 if(obj == rgb_red_object) 
  current_vdp->palette[current_palette][current_palette_index].r = 
   obj->param.d1 * (0xff/7);
 if(obj == rgb_green_object) 
  current_vdp->palette[current_palette][current_palette_index].g = 
   obj->param.d1 * (0xff/7);
 if(obj == rgb_blue_object) 
  current_vdp->palette[current_palette][current_palette_index].b = 
   obj->param.d1 * (0xff/7);
 MAP_COLOR(current_vdp->palette[current_palette][current_palette_index]);
 MESSAGE_OBJECT(current_color_object,MSG_DRAW);
 update_color_text();
 DRAW_RGB_CHOOSER;
 MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
 MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
 MESSAGE_OBJECT(palette_change_object, MSG_DRAW);
 render_vdp(0,vdp_h);
 MESSAGE_OBJECT(preview_object, MSG_DRAW);
 MESSAGE_OBJECT(bg_color_box, MSG_DRAW);


 UPDATE_OBJECT(current_color_object);
 UPDATE_OBJECT(pattern_edit_object);
 UPDATE_OBJECT(pattern_select_object);
 UPDATE_OBJECT(palette_change_object);
 UPDATE_OBJECT(preview_object);
 UPDATE_OBJECT(bg_color_box);
 return RET_OK;
}
/*}}}*/
/*select_quit_on_click{{{*/
int select_quit_on_click(int msg, struct object_t *obj, int data) {

 switch(msg) {
  case MSG_CLICK:
   knob->param.flags |= INACTIVE;
   snprintf(knob_text->param.dp1, 64, ":<");
   return RET_QUIT;
  case MSG_INFOCUS:
 // XXX doesn't get sent every focus event, don't know why
   if(obj->param.user_flags == HORIZONTAL) {
    select_special_object->param.dp1 = (void *)horizontal;
    select_special_object->param.user_flags= HORIZONTAL;
   }
   if(obj->param.user_flags == VERTICAL) {
    select_special_object->param.dp1 = (void *)vertical;
    select_special_object->param.user_flags= VERTICAL;
   }
   break; 
 }
 return RET_OK;
}
/*}}}*/
/*load_default_mega{{{*/
int load_default_mega(int msg, struct object_t *obj, int data) {
 struct select_file_t test;
 if(load_initial != TRUE) return RET_OK;
 if(msg == MSG_START) {
  test.usr_flags = LOAD;
  load_save_mega(&test, load_initial_name);
  snprintf(scroll_size2->param.dp1, 8, "%dx%d", 1<<(5+scroll_width), 1<<(5+scroll_height));

  change_color(0, 0);
 }
 return RET_OK;
}
/*}}}*/
/*select_special{{{*/
int select_special(int msg, struct object_t *obj, int data) {
 int actual_x, actual_y;
 int ix, iy;
 int x,y;
 int lx, ly;
 int off_x, off_y;
 int mode=0, offset=0;
 int x_div, y_div=0;
// printf("user_flags: %d\n", obj->param.user_flags);
 switch(msg) {
  case MSG_START:
   obj->param.dp1 = (void *)horizontal;
   obj->param.user_flags = HORIZONTAL;
//   printf("we start\n");
  case MSG_INFOCUS:
   if(current_vdp->tv_type == NTSC)
    off_y = 8;
   else
    off_y = 0;
   if(current_vdp->cell_w == 32)
    off_x = 32;
   else
    off_x = 0;
   SDL_SetCursor((SDL_Cursor *)obj->param.dp1);
   switch(horizontal_scroll_bar->param.d1) {
    case HOR_LINES:
     y_div = 1;
     mode = 3;
     break;
    case HOR_8LINES:
     y_div = 8;
     mode = 2;
     break;
    case HOR_SCREEN:
     y_div = vdp_h*2;
     mode = 0;
     break;
   }
   if(vertical_scroll_bar->param.d1 == VER_SCREEN) 
    x_div = vdp_w*2;
   else
    x_div = 16;

   x = -1;
   y = -1;
   for(;;) {
    if( gui_mouse_x >= obj->param.x &&
        gui_mouse_x <= obj->param.x + obj->param.w &&
	gui_mouse_y >= obj->param.y &&
	gui_mouse_y <= obj->param.y + obj->param.h) {
     lx =x;
     ly =y;
     x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x)/x_div;
     y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y)/y_div;
     if((lx != x || ly != y )&&
        (x>=0 && y>=0 && x <= ((vdp_w-1)/x_div) && y<=((vdp_h-1)/y_div)) ) { 
     if(obj->param.user_flags == HORIZONTAL) {  
#define REPEAT\
      for(iy = (y * y_div);iy< (y*y_div) + y_div;iy++)\
       for(ix=0;ix<vdp_w;ix++) {\
      actual_x = ((ix-vdp_x)*vdp_zoom)+off_x;\
      actual_y = ((iy-vdp_y)*vdp_zoom)+off_y;\
      if(actual_x>=off_x &&\
        actual_y>=off_y &&\
        actual_x<=((vdp_w+off_x)-1) &&\
        actual_y<=((vdp_h+off_y)-1))\
       fill_box(preview_object->param.x+actual_x,\
  	       preview_object->param.y+actual_y,\
 	       preview_object->param.x+actual_x+vdp_zoom,\
 	       preview_object->param.y+actual_y+vdp_zoom,\
  	       obj->param.fg, obj->param.bg, XOR);\
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
#undef REPEAT
       snprintf(knob_text->param.dp1, 64, "Horiz: %d", y);

       switch(mode) {
	case 0x00:
	 offset = current_vdp->hscroll;
	 break;
	case 0x02:
	 offset = current_vdp->hscroll + (y*0x20);
	 break;
	case 0x03:
	 offset = current_vdp->hscroll + (y * 4);
	 break;
       }	
       knob->param.d1 = current_vdp->vram[offset+1+(2*knob_ab)] | 
	              ((current_vdp->vram[offset+(2*knob_ab)]&7)<<8);

       knob->param.flags &= ~INACTIVE;
       MESSAGE_OBJECT(knob, MSG_DRAW);
       MESSAGE_OBJECT(knob_message_box, MSG_DRAW);
       MESSAGE_OBJECT(knob_icon, MSG_DRAW);
       MESSAGE_OBJECT(knob_text, MSG_DRAW);
       UPDATE_OBJECT(knob_message_box);
       UPDATE_OBJECT(knob);
      } else {
#define REPEAT\
      for(iy = 0;iy<vdp_h;iy++) \
       for(ix = (x* x_div);ix< (x*x_div) + x_div;ix++) {\
      actual_x = ((ix-vdp_x)*vdp_zoom)+off_x;\
      actual_y = ((iy-vdp_y)*vdp_zoom)+off_y;\
      if(actual_x>=off_x &&\
        actual_y>=off_y &&\
        actual_x<=((vdp_w+off_x)-1) &&\
        actual_y<=((vdp_h+off_y)-1))\
       fill_box(preview_object->param.x+actual_x,\
  	       preview_object->param.y+actual_y,\
 	       preview_object->param.x+actual_x+vdp_zoom,\
 	       preview_object->param.y+actual_y+vdp_zoom,\
  	       obj->param.fg, obj->param.bg, XOR);\
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
#undef REPEAT
       if(vertical_scroll_bar->param.d1 == VER_SCREEN) 
	offset = 0;
       else
	offset = (x*4);
       knob->param.d1 = current_vdp->vsram[offset+1+(2*knob_ab)] | 
	              ((current_vdp->vsram[offset+(2*knob_ab)]&7)<<8);
       snprintf(knob_text->param.dp1, 64, "Vert: %d", x);
       knob->param.flags &= ~INACTIVE;
       MESSAGE_OBJECT(knob, MSG_DRAW);
       MESSAGE_OBJECT(knob_message_box, MSG_DRAW);
       MESSAGE_OBJECT(knob_icon, MSG_DRAW);
       MESSAGE_OBJECT(knob_text, MSG_DRAW);
       UPDATE_OBJECT(knob_message_box);
       UPDATE_OBJECT(knob);
      } 
     }
    } else 
     break;
    if(wait_on_mouse() == MOUSE_DOWN) {
     knob->param.flags &= (INACTIVE ^ ~(int)0);
     if(obj->param.user_flags == HORIZONTAL)  {
      knob_data_offset = offset; 
      knob_or = HORIZONTAL;
     }else {
      knob_data_offset = offset;
      knob_or = VERTICAL;
     }
     return RET_QUIT;
    }
   }
 
  case MSG_OUTFOCUS:
   SDL_SetCursor(no);
   break;
 }
 return RET_OK;
}
/*}}}*/
/* update_zoom {{{ */
void update_zoom(int in) {
 float dx, dy;
 int scene_w, scene_h; 
 int off;
 vdp_zoom = in; 
 if(vdp_zoom < 1)
  vdp_zoom=1;

 if(currently_editing != EDIT_SPRITE) {
  scroll_plane_zoom = in;
  scene_w = vdp_w;
  scene_h = vdp_h;
  if(vdp_zoom !=1)  {
   if((preview_x_scroll->param.d1 == 0 )&&
      (preview_x_scroll->param.d2 == 0 )&&
      (preview_y_scroll->param.d1 == 0 )&&
      (preview_y_scroll->param.d2 == 0 )) {
    dx = 0.5f;
    dy = 0.5f;
   } else {
    dx = (float)preview_x_scroll->param.d1 / (float)preview_x_scroll->param.d2;
    dy = (float)preview_y_scroll->param.d1 / (float)preview_y_scroll->param.d2;
   }
   if(dx > 1.0f)
    dx = 1.0f;
   if(dy > 1.0f)
    dy = 1.0f;
   preview_x_scroll->param.d2 = (scene_w - (scene_w/vdp_zoom))-1;
   preview_y_scroll->param.d2 = (scene_h - (scene_h/vdp_zoom))-1;
   if(currently_editing != EDIT_SPRITE) {
    preview_x_scroll->param.d1 = (int)(dx*(float)(preview_x_scroll->param.d2));
    preview_y_scroll->param.d1 = (int)(dy*(float)(preview_y_scroll->param.d2));
   }
   vdp_x = preview_x_scroll->param.d1;
   vdp_y = preview_y_scroll->param.d1;
  } else {
   preview_x_scroll->param.d1 =
    preview_x_scroll->param.d2 =
     preview_y_scroll->param.d1 =
      preview_y_scroll->param.d2 = 0;
   vdp_x = 0;
   vdp_y = 0;
   }
 } else {
  sprite_zoom = in;
  scene_w = (sprite_width+1) *8;
  scene_h = (sprite_height+1)*8;
  

  if((scene_w * vdp_zoom) > SPRITE_WIDTH) {
   off = abs((SPRITE_WIDTH - (scene_w * vdp_zoom)) / vdp_zoom);
   preview_x_scroll->param.d2 = off+1;
   if(preview_x_scroll->param.d1>preview_x_scroll->param.d2)
    preview_x_scroll->param.d1 = preview_x_scroll->param.d2; 
   vdp_x = preview_x_scroll->param.d1;
  } else {
   preview_x_scroll->param.d1 = 0;
   preview_x_scroll->param.d2 = 0;
   vdp_x = 0;
  } 

  if((scene_h * vdp_zoom) > SPRITE_HEIGHT) {
   off = abs((SPRITE_HEIGHT - (scene_h * vdp_zoom)) / vdp_zoom);
   preview_y_scroll->param.d2 = off+1;
   if(preview_y_scroll->param.d1>preview_y_scroll->param.d2)
    preview_y_scroll->param.d1 = preview_y_scroll->param.d2; 
   vdp_y = preview_y_scroll->param.d1;
  } else {
   preview_y_scroll->param.d1 = 0;
   preview_y_scroll->param.d2 = 0;
   vdp_y = 0;
  }
 }
 DRAW_PREVIEW;

 UPDATE_OBJECT( preview_x_scroll);
 UPDATE_OBJECT( preview_y_scroll); 
 return;
}
/* }}} */
/*preview_zoom_change{{{*/
int preview_zoom_change(struct object_t *obj, int data) {
 UPDATE_OBJECT(obj);
 if(obj == preview_zoom_in) {
  preview_zoom_in->param.d1 = FALSE;
  update_zoom(vdp_zoom + 1);
 } else {
  preview_zoom_out->param.d1 = FALSE;
  update_zoom(vdp_zoom -1);
 }
 return RET_OK;
}
/*}}}*/
/*preview_scroll_change{{{*/
int preview_scroll_change(struct object_t *obj, int data) {
 if( obj != preview_scroll_bar) {
  vdp_x = preview_x_scroll->param.d1;
  vdp_y = preview_y_scroll->param.d1;
 } 
 MESSAGE_OBJECT(preview_object, MSG_DRAW);
 return RET_OK;
}
/*}}}*/
/*preview_size_change{{{*/
int preview_size_change(struct object_t *obj, int data) {
 if(preview_ntsc->param.d1 == TRUE)  
  current_vdp->tv_type = NTSC;
 else
  current_vdp->tv_type = PAL;
 
 if(preview_40c->param.d1 == TRUE)
  current_vdp->cell_w = 40;
 else
  current_vdp->cell_w = 32;
 if(currently_editing != EDIT_SPRITE) {
  render_vdp(0,vdp_h);
  vdp_zoom = 1;
  preview_x_scroll->param.d1 =
   preview_x_scroll->param.d2 =
    preview_y_scroll->param.d1 =
     preview_y_scroll->param.d2 = 0;

  vdp_w = (current_vdp->cell_w == 40 ? 320 : 256);
  vdp_h = (current_vdp->tv_type == NTSC ? 224 : 240);
  DRAW_PREVIEW;

  MESSAGE_OBJECT( pattern_select_object, MSG_DRAW);
 }
 return RET_OK;
}
/*}}}*/
/*proc_sprite_size {{{ */
int proc_sprite_size(int msg, struct object_t *obj, int data) {
 int i;
 int x=0,y=0;
 int lx,ly;
 int ox, oy;
 switch(msg) {
  case MSG_DRAW:

   fill_box(obj->param.x,obj->param.y,
     obj->param.x+((sprite_width+1)*10),obj->param.y+((sprite_height+1)*10),
     obj->param.fg,obj->param.bg,HASH);
   for(i=0;i<5;i++) {
    vline(obj->param.x+(i*10),obj->param.y,obj->param.y+41,&globl_fg,obj->param.bg,NO_HASH);
    hline(obj->param.x,obj->param.y+(i*10),obj->param.x+40,&globl_fg,obj->param.bg,NO_HASH);
   }
   break;
  case MSG_CLICK:
   ox = sprite_width;
   oy = sprite_height;
   lx = ((gui_mouse_x-obj->param.x) / 10);
   ly = ((gui_mouse_y-obj->param.y) / 10);
   for(;;) {
    lx = x;
    ly = y;
    x = ((gui_mouse_x-obj->param.x) / 10);
    y = ((gui_mouse_y-obj->param.y) / 10);
    if(lx!=x || ly!=y)
     if(x>=0 && x<=3 && y>=0 && y<=3) {
      sprite_width = x;
      sprite_height = y;
      
      MESSAGE_OBJECT(sprite_size1, MSG_DRAW);
      proc_sprite_size(MSG_DRAW, obj, 0);
      UPDATE_OBJECT(sprite_size1);
      if(currently_editing == EDIT_SPRITE) {
       vdp_w = 8 * (sprite_width + 1);
       vdp_h = 8 * (sprite_height +1);
       update_zoom(vdp_zoom);
       DRAW_PREVIEW;
      }
     }
    if(wait_on_mouse() == MOUSE_UP) break;
   } 
   break;
 }
 return RET_OK;

}
/* }}} */
/*proc_scroll_size{{{*/
int proc_scroll_size(int msg, struct object_t *obj, int data) {
 int i;
 int x=0,y=0;
 int lx,ly;
 int ox, oy;
 int flags;
 switch(msg) {
  case MSG_DRAW:
   if((scroll_width+scroll_height) <=2) 
    flags = NO_HASH;
   else 
    flags = HASH;

   fill_box(obj->param.x,obj->param.y,
     obj->param.x+((scroll_width+1)*10),obj->param.y+((scroll_height+1)*10),
     obj->param.fg,obj->param.bg,flags);
   for(i=0;i<4;i++) {
    vline(obj->param.x+(i*10),obj->param.y,obj->param.y+31,&globl_fg,obj->param.bg,NO_HASH);
    hline(obj->param.x,obj->param.y+(i*10),obj->param.x+30,&globl_fg,obj->param.bg,NO_HASH);
   }
   break;
  case MSG_CLICK:
   ox = scroll_width;
   oy = scroll_height;
   lx = ((gui_mouse_x-obj->param.x) / 10);
   ly = ((gui_mouse_y-obj->param.y) / 10);
   for(;;) {
    lx = x;
    ly = y;
    x = ((gui_mouse_x-obj->param.x) / 10);
    y = ((gui_mouse_y-obj->param.y) / 10);
    if(lx!=x || ly!=y)
     if(x>=0 && x<=2 && y>=0 && y<=2) {
      snprintf(scroll_size2->param.dp1, 8, "%dx%d", 1<<(5+x), 1<<(5+y));
      scroll_width = x;
      scroll_height = y;
      MESSAGE_OBJECT(scroll_size1, MSG_DRAW);
      proc_scroll_size(MSG_DRAW, obj, 0);
      MESSAGE_OBJECT(scroll_size2, MSG_DRAW);
      UPDATE_OBJECT(scroll_size1);
      if((scroll_width+scroll_height)<=2) {
       render_vdp(0,vdp_h);
       MESSAGE_OBJECT(preview_object, MSG_DRAW);
       UPDATE_OBJECT(preview_object);
      }
     }
    if(wait_on_mouse() == MOUSE_UP) break;
   } 
   if((scroll_width+scroll_height)>2) {
    scroll_width = ox;
    scroll_height = oy;
    MESSAGE_OBJECT(scroll_size1, MSG_DRAW);
    proc_scroll_size(MSG_DRAW, obj, 0);
    MESSAGE_OBJECT(scroll_size2, MSG_DRAW);
    UPDATE_OBJECT(scroll_size1);
   }
   break;
 }
 return RET_OK;
}
/*}}}*/
/*proc_info{{{*/
int proc_info(int msg, struct object_t *obj, int data) {
 switch(msg) {
  case MSG_START:
   obj->param.dp1 = (void *)malloc(300);
   obj->param.dp2 = (void *)malloc(300);

   snprintf(obj->param.dp1, 30, ":/");
   snprintf(obj->param.dp2, 30, ":-|");

   break;
  case MSG_DRAW:
   proc_shadow_box(MSG_DRAW, obj, 0);
   hline(obj->param.x+5, obj->param.y+(obj->param.h/2),obj->param.x+obj->param.w-5, 
     obj->param.fg, obj->param.bg, NO_HASH);
   draw_text(
     obj->param.x+(obj->param.w/2)-CENTER_OF_STRING(obj->param.dp1)-2, 
     obj->param.y+(obj->param.h/4)-4,
     obj->param.dp1, obj->param.fg, obj->param.bg, NO_HASH, 0);
   draw_text(
     obj->param.x+(obj->param.w/2)-CENTER_OF_STRING(obj->param.dp2)-2, 
     obj->param.y+((obj->param.h/4)*3)-4,
     obj->param.dp2, obj->param.fg, obj->param.bg, NO_HASH, 0);

   break;
 }
 return RET_OK;
}
/*}}}*/
/*proc_preview_object{{{*/
int proc_preview_object(int msg, struct object_t *obj, int data) {
 int x=0,y=0;
 int w=0,h=0;
 int ix=0,iy=0;
 int info_x=0, info_y=0;
 int pal=0;
 int actual_x=0, actual_y=0;
 int i=0,j=0;
 pixel_dump_t *pat_pix=0;
 int pat_num=0;
 int x2=0, y2=0;
 int x3=0, y3=0;

 Uint8 *pix=0;
 color_t A;
 int off_x=0, off_y=0;
 int mask=0, *intp=0;
 int tmp_color=0;
 int ret=0;
 SDL_Rect src;
 vdp_pixel *current_pixels=0;
 int current_w=0, current_h=0;
 int tmp_h=0, tmp_w=0;

 if(currently_editing == EDIT_SPRITE) {
  tmp_w = (vdp_w -preview_x_scroll->param.d1)* vdp_zoom;
  if(tmp_w > 320) tmp_w = 320; 
  tmp_h = (vdp_h -preview_y_scroll->param.d1) * vdp_zoom;
  if(tmp_h > 240) tmp_h = 240; 
 } else {
  tmp_h = vdp_h;
  tmp_w = vdp_w;
 }

 if(currently_editing != EDIT_SPRITE) {
  if(current_vdp->tv_type == NTSC) 
   off_y = 8;
  else
   off_y = 0;
  if(current_vdp->cell_w == 32)
   off_x = 32;
  else
   off_x = 0;
 } else {
  off_x = 0;
  off_y = 0;
 }
 
 if(currently_editing == EDIT_SPRITE) {
  current_pixels = sprite_screen;
  current_w = 32;
  current_h = 32;
  render_sprite();
 } else {
  current_pixels = vdp_screen;
  current_w = 320;
  current_h = 240;
 }


 switch(msg) {
  case MSG_START:
   break;
  case MSG_MOUSEMOVE:
   x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
   y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);

   if( x >= 0 &&
       x < tmp_w &&
       y >= 0 &&
       y < tmp_h) {
    snprintf(info_object->param.dp1, 30, "(%d,%d)", x, y);
    snprintf(info_object->param.dp2, 30, ":-|");
    MESSAGE_OBJECT(info_object, MSG_DRAW);
    UPDATE_OBJECT(info_object);
   }
   break;
  case MSG_DRAW:
    fill_box(obj->param.x, obj->param.y,
             obj->param.x+obj->param.w,obj->param.y+obj->param.h,obj->param.bg,obj->param.bg,NO_HASH);
    mask = 0;
    for(x=0;x<gui_screen->format->BytesPerPixel;x++)
     mask |= 0xff << (8*x);
#ifdef WORDS_BIGENDIAN
    mask<<=8;
#endif
    mask^=~(int)0;


   if(vdp_zoom == 1) {

    /* XXX pointer math */
    pix = (Uint8 *)(gui_screen->pixels + (gui_screen->pitch * obj->param.y) +
                    gui_screen->format->BytesPerPixel * obj->param.x);
    if(currently_editing != EDIT_SPRITE) {
     if(current_vdp->tv_type == NTSC) 
      pix += (gui_screen->pitch * 8); 
     if(current_vdp->cell_w == 32) 
      pix += (gui_screen->format->BytesPerPixel * 32);
    }

#define PUT_COLOR \
        intp = (int *)pix;\
        *intp&=mask;\
        *intp+=tmp_color;\
        pix+=gui_screen->format->BytesPerPixel 
#define GET_COLOR(Q)\
        tmp_color = SDL_MapRGB(gui_screen->format,Q.r,Q.g,Q.b)

    if(preview_scroll_bar->param.d1 < 256) {
     if(preview_scroll_bar->param.d1 != 0) {
      for(y=0;y<tmp_h;y++){
       for(x=0;x<tmp_w;x++){

#define ALPHA_BLEND(Q)\
	A.r = Q.color.r + (((vdp_screen[(x+vdp_x)+((y+vdp_y)*320)].ovr_color.r - Q.color.r)\
	      *preview_scroll_bar->param.d1)>>8);\
	A.g = Q.color.g + (((vdp_screen[(x+vdp_x)+((y+vdp_y)*320)].ovr_color.g - Q.color.g)\
	      *preview_scroll_bar->param.d1)>>8);\
	A.b = Q.color.b + (((vdp_screen[(x+vdp_x)+((y+vdp_y)*320)].ovr_color.b - Q.color.b)\
	       *preview_scroll_bar->param.d1)>>8)

	ALPHA_BLEND(current_pixels[x+(y*current_w)]);


	GET_COLOR(A);
#ifdef WORDS_BIGENDIAN
        tmp_color <<= 8;
#endif
        PUT_COLOR;
       }
       pix+= (gui_screen->pitch - (gui_screen->format->BytesPerPixel * tmp_w ));
      }
     } else {
      for(y=0;y<tmp_h;y++) {
       for(x=0;x<tmp_w;x++) {
	GET_COLOR(current_pixels[x+(y*current_w)].color);
#ifdef WORDS_BIGENDIAN
        tmp_color <<= 8;
#endif

        PUT_COLOR;
       }
       pix+= (gui_screen->pitch - (gui_screen->format->BytesPerPixel * tmp_w));
      }
     }
    } else {
     for(y=0;y<tmp_h;y++) {
      for(x=0;x<tmp_w;x++) {
       GET_COLOR(vdp_screen[x+(y*320)].ovr_color);
#ifdef WORDS_BIGENDIAN
       tmp_color <<= 8;
#endif
       PUT_COLOR;
      }
      pix+= (gui_screen->pitch - (gui_screen->format->BytesPerPixel * tmp_w));
     }
    }
#undef PUT_COLOR
  
   } else {

    if(preview_scroll_bar->param.d1 < 256) {
     if(preview_scroll_bar->param.d1 != 0) {
      /* MIX */
      src.w = vdp_zoom;
      src.h = vdp_zoom;
      for(y = 0;y< (tmp_h / vdp_zoom);y++)
       for(x = 0;x< (tmp_w / vdp_zoom);x++) {
	ALPHA_BLEND(current_pixels[(x+vdp_x)+((y+vdp_y)*current_w)]);
        GET_COLOR(A);
        src.x = (x * vdp_zoom) + obj->param.x+off_x;
        src.y = (y * vdp_zoom) + obj->param.y+off_y;
        SDL_FillRect(gui_screen, &src, tmp_color);
       }
      src.w = tmp_w%vdp_zoom;
      if(src.w != 0) {
       x = tmp_w / vdp_zoom;
       for(y = 0;y<(tmp_h/ vdp_zoom);y++) {
	ALPHA_BLEND(current_pixels[(x+vdp_x)+((y+vdp_y)*current_w)]);
        GET_COLOR(A);
        src.x = ( x * vdp_zoom) + obj->param.x+off_x;
        src.y = ( y * vdp_zoom) + obj->param.y+off_y;

        SDL_FillRect(gui_screen, &src, tmp_color); 
       }
      }
      src.w = vdp_zoom;
      src.h = tmp_h%vdp_zoom;
      if(src.h != 0) {
       y = tmp_h / vdp_zoom;
       for(x = 0;x<(tmp_w/vdp_zoom);x++) {
	ALPHA_BLEND(current_pixels[(x+vdp_x)+((y+vdp_y)*current_w)]);
        GET_COLOR(A);
        src.x = ( x * vdp_zoom) + obj->param.x+off_x;
        src.y = ( y * vdp_zoom) + obj->param.y+off_y;

        SDL_FillRect(gui_screen, &src, tmp_color); 
       }
      }
      src.w = tmp_w%vdp_zoom;
      src.h = tmp_h %vdp_zoom;
      if(src.h!=0 &&src.w!=0) {
       y = tmp_h/vdp_zoom;
       x = tmp_w/vdp_zoom;
       ALPHA_BLEND(current_pixels[(x+vdp_x)+((y+vdp_y)*current_w)]);
       GET_COLOR(A);
       src.x = (x * vdp_zoom) + obj->param.x+off_x;
       src.y = (y * vdp_zoom) + obj->param.y+off_y;

       SDL_FillRect(gui_screen, &src, tmp_color); 
      }
#undef ALPHA_BLEND
     } else {
      /* VDP */
      src.w = vdp_zoom;
      src.h = vdp_zoom;
      for(y = 0;y< (tmp_h / vdp_zoom);y++)
       for(x = 0;x< (tmp_w / vdp_zoom);x++) {
        GET_COLOR(current_pixels[(x+vdp_x)+((y+vdp_y)*current_w)].color);
        src.x = (x * vdp_zoom) + obj->param.x+off_x;
        src.y = (y * vdp_zoom) + obj->param.y+off_y;

        SDL_FillRect(gui_screen, &src, tmp_color);
       }
      src.w = tmp_w%vdp_zoom;
      if(src.w != 0) {
       x = tmp_w / vdp_zoom;
       for(y = 0;y<(tmp_h/ vdp_zoom);y++) {
        GET_COLOR(current_pixels[(x+vdp_x)+ ((y+vdp_y)*current_w)].color);
        src.x = ( x * vdp_zoom) + obj->param.x+off_x;
        src.y = ( y * vdp_zoom) + obj->param.y+off_y;

        SDL_FillRect(gui_screen, &src, tmp_color); 
       }
      }
      src.w = vdp_zoom;
      src.h = tmp_h%vdp_zoom;
      if(src.h != 0) {
       y = tmp_h / vdp_zoom;
       for(x = 0;x<(tmp_w/vdp_zoom);x++) {
        GET_COLOR(current_pixels[(x+vdp_x)+ ((y+vdp_y)*current_w)].color);
        src.x = ( x * vdp_zoom) + obj->param.x+off_x;
        src.y = ( y * vdp_zoom) + obj->param.y+off_y;

        SDL_FillRect(gui_screen, &src, tmp_color); 
       }
      }
      src.w = tmp_w %vdp_zoom;
      src.h = tmp_h %vdp_zoom;
      if(src.h!=0 &&src.w!=0) {
       y = tmp_h/vdp_zoom;
       x = tmp_w/vdp_zoom;
       GET_COLOR(current_pixels[(x+vdp_x)+ ((y+vdp_y)*current_w)].color);
       src.x = (x * vdp_zoom) + obj->param.x+off_x;
       src.y = (y * vdp_zoom) + obj->param.y+off_y;

       SDL_FillRect(gui_screen, &src, tmp_color); 
      }

     }
    } else {
     /*OVR */
      src.w = vdp_zoom;
      src.h = vdp_zoom;
      for(y = 0;y< (tmp_h / vdp_zoom);y++)
       for(x = 0;x< (tmp_w / vdp_zoom);x++) {
        GET_COLOR(vdp_screen[(x+vdp_x)+((y+vdp_y)*320)].ovr_color);
        src.x = (x * vdp_zoom) + obj->param.x+off_x;
        src.y = (y * vdp_zoom) + obj->param.y+off_y;
        SDL_FillRect(gui_screen, &src, tmp_color);
       }
      src.w = tmp_w%vdp_zoom;
      if(src.w != 0) {
       x = tmp_w / vdp_zoom;
       for(y = 0;y<(tmp_h/ vdp_zoom);y++) {
        GET_COLOR(vdp_screen[(x+vdp_x)+ ((y+vdp_y)*320)].ovr_color);
        src.x = ( x * vdp_zoom) + obj->param.x+off_x;
        src.y = ( y * vdp_zoom) + obj->param.y+off_y;
        SDL_FillRect(gui_screen, &src, tmp_color); 
       }
      }
      src.w = vdp_zoom;
      src.h = tmp_h%vdp_zoom;
      if(src.h != 0) {
       y = tmp_h / vdp_zoom;
       for(x = 0;x<(tmp_w/vdp_zoom);x++) {
        GET_COLOR(vdp_screen[(x+vdp_x)+ ((y+vdp_y)*320)].ovr_color);
        src.x = ( x * vdp_zoom) + obj->param.x+off_x;
        src.y = ( y * vdp_zoom) + obj->param.y+off_y;
        SDL_FillRect(gui_screen, &src, tmp_color); 
       }
      }
      src.w = tmp_w %vdp_zoom;
      src.h = tmp_h %vdp_zoom;
      if(src.h!=0 &&src.w!=0) {
       y = tmp_h/vdp_zoom;
       x = tmp_w/vdp_zoom;
       GET_COLOR(vdp_screen[(x+vdp_x)+ ((y+vdp_y)*320)].ovr_color);
       src.x = (x * vdp_zoom) + obj->param.x+off_x;
       src.y = (y * vdp_zoom) + obj->param.y+off_y;
       SDL_FillRect(gui_screen, &src, tmp_color); 
      }

    }

#undef	GET_COLOR
   }


   if(selection_v1.x != NO_SELECTION) {

    num_xor_pix = 0;
    do_line(NULL, 0, 
      selection_v1.x, selection_v1.y, selection_v2.x, selection_v1.y, put_xor_hash_pix);
    do_line(NULL, 0, 
      selection_v2.x, selection_v1.y, selection_v2.x, selection_v2.y, put_xor_hash_pix);
    do_line(NULL, 0, 
      selection_v2.x, selection_v2.y, selection_v1.x, selection_v2.y, put_xor_hash_pix);
    do_line(NULL, 0, 
      selection_v1.x, selection_v2.y, selection_v1.x, selection_v1.y, put_xor_hash_pix);
    for(i=0;i<num_xor_pix;i++) {
     actual_x = ((xor_pixels[i].x-vdp_x)*vdp_zoom) + off_x; 
     actual_y = ((xor_pixels[i].y-vdp_y)*vdp_zoom) + off_y; 
     if(actual_x >= off_x && 
       actual_y >= off_y && 
       actual_x <= (tmp_w+off_x-1) && 
       actual_y <= (tmp_h+off_y-1)) 
       fill_box(obj->param.x+actual_x,
 	        obj->param.y+actual_y,
                obj->param.x+actual_x+vdp_zoom,
		obj->param.y+actual_y+vdp_zoom, 
		&globl_fg, &globl_bg, XOR); 
     } 

   }
   break;
  case MSG_CLICK:
   x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
   y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
   if(x < 0 || y < 0) break;
   /* HERE */
   switch(current_tool) {
    case SELECT:
     for(;;) {
      x2 = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
      y2 = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      snprintf(info_object->param.dp1, 30, "A(%d,%d)", x, y);
      snprintf(info_object->param.dp2, 30, "B(%d,%d)", x2, y2);
      MESSAGE_OBJECT(info_object, MSG_DRAW);
      UPDATE_OBJECT(info_object);
      num_xor_pix = 0;
      do_line(NULL, 0, x, y, x2, y, put_xor_hash_pix);
      do_line(NULL, 0, x2, y, x2, y2, put_xor_hash_pix);
      do_line(NULL, 0, x2, y2, x, y2, put_xor_hash_pix);
      do_line(NULL, 0, x, y2, x, y, put_xor_hash_pix);  
#define REPEAT\
      for(i=0;i<num_xor_pix;i++) {\
       actual_x = ((xor_pixels[i].x-vdp_x)*vdp_zoom) + off_x; \
       actual_y = ((xor_pixels[i].y-vdp_y)*vdp_zoom) + off_y; \
       if(actual_x >= off_x && \
	  actual_y >= off_y && \
	  actual_x <= (obj->param.w-1) && \
	  actual_y <= (obj->param.h-1)) \
        fill_box(obj->param.x+actual_x,\
 	         obj->param.y+actual_y,\
	         obj->param.x+actual_x+vdp_zoom,\
		 obj->param.y+actual_y+vdp_zoom, \
		 &globl_fg, &globl_bg, XOR); \
      }
      REPEAT
      UPDATE_OBJECT(preview_object);
      if(wait_on_mouse() == MOUSE_UP) break;
      REPEAT
     }
     if(x != x2 && y != y2) {
      if(x<0) x = 0;
      if(y<0) y = 0;
      if(x>=tmp_w) x = tmp_w-1;
      if(y>=tmp_h) y = tmp_h-1;
      if(x2<0) x2 = 0;
      if(y2<0) y2 = 0;
      if(x2>=tmp_w) x2 = tmp_w-1;
      if(y2>=tmp_h) y2 = tmp_h-1;

      if(x2 < x) {
       i = x;
       x = x2;
       x2 = i;
      }

      if(y2 < y) {
       i = y;
       y = y2;
       y2 = i;
      }
       
      selection_v1.x = x;
      selection_v1.y = y;
      selection_v2.x = x2;
      selection_v2.y = y2;
      info_x = x;
      info_y = y;
      for(y=0;y<SEL_H;y++) 
       for(x=0;x<SEL_W;x++) {
        selection_buffer[x + (320*y)] = 
	 vdp_screen[ (x+selection_v1.x) + ((y+selection_v1.y)*320)].index;
       }
      copy_move_grp->pos_x = gui_mouse_x - 6;
      copy_move_grp->pos_y = gui_mouse_y - 6;
      copy_move = NOT_SELECTED;
      group_loop(copy_move_grp);

      if(copy_move != NOT_SELECTED){
       snprintf(info_object->param.dp2, 30, ":-|");
       save_state();
       for(i=0;i<320*240;i++){
	vdp_backbuf[i].color.r = vdp_screen[i].color.r;
	vdp_backbuf[i].color.g = vdp_screen[i].color.g;
	vdp_backbuf[i].color.b = vdp_screen[i].color.b;
       }
       for(i=0;i<320*240;i++)
        draw_buffer[i%320][i/320] = 0xfe;
       if(copy_move == MOVE) {
	for(y=selection_v1.y;y<selection_v2.y;y++)
	 for(x=selection_v1.x;x<selection_v2.x;x++) {
          vdp_backbuf[x+(320*y)].color.r = 
	   current_vdp->palette[current_palette][current_vdp->bg_index].r;
          vdp_backbuf[x+(320*y)].color.g = 
	   current_vdp->palette[current_palette][current_vdp->bg_index].g;
          vdp_backbuf[x+(320*y)].color.b = 
	   current_vdp->palette[current_palette][current_vdp->bg_index].b;
          draw_buffer[x][y] = current_vdp->bg_index;
	 }
       }	
       x2 = -1;
       y2 = -1;
       w = SEL_W;
       h = SEL_H;
       for(;;) {
        x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
        y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
        if(x != x2 || y != y2) {
	 selection_v1.x = x - w;
	 selection_v2.x = x;
	 selection_v1.y = y - h;
	 selection_v2.y = y; 
   
   	 snprintf(info_object->param.dp1, 30, "DIF(%d,%d)", selection_v1.x-info_x, 
	                                                    selection_v1.y-info_y);
         MESSAGE_OBJECT(info_object, MSG_DRAW);
         UPDATE_OBJECT(info_object);


	 for(iy=0;iy<tmp_h;iy++)
	  for(ix=0;ix<tmp_w;ix++) {
	   if((ix < selection_v1.x) || 
	      (ix > selection_v2.x) ||
	      (iy < selection_v1.y) ||
	      (iy > selection_v2.y)) {
	    vdp_screen[ix+(320*iy)].color.r = vdp_backbuf[ix+(320*iy)].color.r;
	    vdp_screen[ix+(320*iy)].color.g = vdp_backbuf[ix+(320*iy)].color.g;
	    vdp_screen[ix+(320*iy)].color.b = vdp_backbuf[ix+(320*iy)].color.b;
	   } else {
	    switch(currently_editing) {
	     case EDIT_SCROLL_A:
	      pal = vdp_screen[ix+(iy*320)].A.pal;
	      break;
	     case EDIT_SCROLL_B:
	      pal = vdp_screen[ix+(iy*320)].B.pal;
	      break;
	     case EDIT_SPRITE:
	     case EDIT_WINDOW:
	      printf("finish me\n");
	      break;
	    }
            vdp_screen[ix+(320*iy)].color.r = 
	     current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].r;
            vdp_screen[ix+(320*iy)].color.g = 
	     current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].g;
             vdp_screen[ix+(320*iy)].color.b = 
	     current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].b;

	   }
	  } 
 	 MESSAGE_OBJECT(obj, MSG_DRAW);
	 UPDATE_OBJECT(obj);
	}	 
	x2 = x;
	y2 = y;
        ret = wait_on_mouse();
	if(ret == KEY_DOWN) {
	 for(iy=selection_v1.y;iy<selection_v2.y;iy++)
	  for(ix=selection_v1.x;ix<selection_v2.x;ix++)
	   if(ix>=0 && ix <tmp_w &&
	      iy>=0 && iy <tmp_h) {
	    switch(currently_editing) {
	     case EDIT_SCROLL_A:
	      pal = vdp_screen[ix+(iy*320)].A.pal;
	      break;
	     case EDIT_SCROLL_B:
	      pal = vdp_screen[ix+(iy*320)].B.pal;
	      break;
	     case EDIT_SPRITE:
	     case EDIT_WINDOW:
	      printf("finish me\n");
	      break;
	    }
	    vdp_backbuf[ ix + ( iy * 320)].color.r = 
             current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].r;
	    vdp_backbuf[ ix + ( iy * 320)].color.g = 
             current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].g;
	    vdp_backbuf[ ix + ( iy * 320)].color.b = 
             current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].b;
            draw_buffer[ix][iy] = 
	     selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)];
	   }
	}
	if(ret == MOUSE_DOWN) {
         for(iy=selection_v1.y;iy<selection_v2.y;iy++)
	  for(ix=selection_v1.x;ix<selection_v2.x;ix++)
	   if(ix>=0 && ix <tmp_w &&
	      iy>=0 && iy <tmp_h) {
	    switch(currently_editing) {
	     case EDIT_SCROLL_A:
	      pal = vdp_screen[ix+(iy*320)].A.pal;
	      break;
	     case EDIT_SCROLL_B:
	      pal = vdp_screen[ix+(iy*320)].B.pal;
	      break;
	     case EDIT_SPRITE:
	     case EDIT_WINDOW:
	      printf("finish me\n");
	      break;
	    }

	    vdp_backbuf[ ix + ( iy * 320)].color.r = 
             current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].r;
	    vdp_backbuf[ ix + ( iy * 320)].color.g = 
             current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].g;
	    vdp_backbuf[ ix + ( iy * 320)].color.b = 
             current_vdp->palette[pal][
	      selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)]
	     ].b;
            draw_buffer[ix][iy] = 
	     selection_buffer[(ix-selection_v1.x) + ((iy-selection_v1.y)*320)];
	   }
	 break;
	}
       }
      num_xor_pix = 0;
      for(iy = 0;iy<tmp_h;iy++)
       for(ix = 0;ix<tmp_w;ix++)
	if(draw_buffer[ix][iy]!=0xfe) {
	 xor_pixels[num_xor_pix].x = ix;
	 xor_pixels[num_xor_pix].y = iy;
	 xor_pixels[num_xor_pix].index = draw_buffer[ix][iy];
	 num_xor_pix++;
	}

      } else 
       num_xor_pix = 0;
      
     } else { 
      num_xor_pix = 0;
      snprintf(info_object->param.dp1, 12, "(%d,%d)", x, y);
      snprintf(info_object->param.dp2, 12, ":-|");
      MESSAGE_OBJECT(info_object, MSG_DRAW);
      UPDATE_OBJECT(info_object);
      selection_v1.x = NO_SELECTION;
     }
     break;
    case LINE:
    save_state();
    for(;;) {
     x2 = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
     y2 = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      num_xor_pix = 0;
      do_line(NULL, 0, x, y, x2, y2, put_xor_pix); 

      snprintf(info_object->param.dp1, 30, "A(%d,%d)",x,y);
      snprintf(info_object->param.dp2, 30, "B(%d,%d),%d", x2, y2, num_xor_pix);
      MESSAGE_OBJECT(info_object, MSG_DRAW);
      UPDATE_OBJECT(info_object);

   
      REPEAT
      UPDATE_OBJECT(preview_object);
      REPEAT

     if(wait_on_mouse()==MOUSE_UP) break;

    }
    snprintf(info_object->param.dp1, 30, "(%d,%d)",x2,y2);
    snprintf(info_object->param.dp2, 30, ":-|");
    MESSAGE_OBJECT(info_object, MSG_DRAW);
    UPDATE_OBJECT(info_object);

    break;
   case FILL:
    save_state();
    for(;;) {
     x2 = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
     y2 = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
     if(x2 >=0 &&
        y2 >=0 &&
        x2 < tmp_w &&
        y2 < tmp_h) {	 

       num_xor_pix = 0;
       memset(draw_buffer, 0, 320*240 * sizeof(Uint8));
     
       do_flood(0, 0, x2, y2, 0,0, tmp_w-1, tmp_h-1, put_pix_draw_buffer, get_pix_preview_special);

       num_xor_pix  = 0;
       for(j=0;j<(currently_editing==EDIT_SPRITE ? (sprite_height+1)*8:240);j++)
        for(i=0;i<(currently_editing==EDIT_SPRITE ? (sprite_width+1)*8:320);i++)
 	 if(draw_buffer[i][j] == 1) {
	  xor_pixels[num_xor_pix].x = i;
	  xor_pixels[num_xor_pix].y = j;
	  xor_pixels[num_xor_pix].index = current_palette_index;
	  num_xor_pix++;
	 }
	 
#define REPEAT\
       for(i=0;i<num_xor_pix;i++) {\
        actual_x = ((xor_pixels[i].x-vdp_x)*vdp_zoom) + off_x; \
        actual_y = ((xor_pixels[i].y-vdp_y)*vdp_zoom) + off_y; \
        if(actual_x >= off_x && \
	   actual_y >= off_y && \
	   actual_x <= (obj->param.w-1) && \
	   actual_y <= (obj->param.h-1)) \
         fill_box(obj->param.x+actual_x,\
 	          obj->param.y+actual_y,\
	          obj->param.x+actual_x+vdp_zoom,\
		  obj->param.y+actual_y+vdp_zoom, \
		  &globl_fg, &globl_bg, XOR); \
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
#undef REPEAT
     }
     if(wait_on_mouse()==MOUSE_UP) break;

    }
    break;

    case PIC:
    x2 = -1;
    y2 = -1;
    for(;;) {
     x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
     y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
     if(x!= x2 || y!= y2) {
#define REPEAT\
      actual_x = (((x-vdp_x) * vdp_zoom)+off_x);\
      actual_y = (((y-vdp_y) * vdp_zoom)+off_y);\
      if(actual_x >= off_x &&\
         actual_y >= off_y &&\
         actual_x <= (obj->param.w-1) &&\
         actual_y <= (obj->param.h-1))\
      fill_box(obj->param.x+actual_x,\
               obj->param.y+actual_y,\
               obj->param.x+actual_x+vdp_zoom,\
               obj->param.y+actual_y+vdp_zoom,\
               &globl_fg, &globl_bg, XOR);
      REPEAT
      UPDATE_OBJECT(preview_object);
      REPEAT
#undef REPEAT
      if(x>=0 &&
	 y>=0 &&
	 x<=tmp_w &&
	 y<=tmp_h)
 	 if(current_palette!=current_pixels[x+(current_w*y)].palette ||
	    current_palette_index!=current_pixels[x+(current_w*y)].index) {
          change_color(current_pixels[x+(current_w*y)].palette,current_pixels[x+(current_w*y)].index);


          UPDATE_OBJECT(pattern_edit_object);
	  UPDATE_OBJECT(palette_change_object);
	  UPDATE_OBJECT(pattern_select_object);
	  UPDATE_OBJECT(rgb_box_object);
         }
     }
     x2 = x;
     y2 = y;
     if(wait_on_mouse()==MOUSE_UP) break;
    }
    break;
    case PIC_PAT:
     x2 = -1;
     y2 = -1;
     for(;;) {
      x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
      y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      if((x!=x2 || y!=y2 )&&
	  x>=0 && y>= 0 && x<=tmp_w && y<=tmp_h) {
       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
         j = vdp_screen[x+(320*y)].A.pat_index;
	 pat_num = vdp_screen[x+(320*y)].A.pat;
	 ix = vdp_screen[x+(320*y)].A.sx;
	 iy = vdp_screen[x+(320*y)].A.sy;
	 pat_pix = pat_pix_a;
	 SDL_SetCursor(cross_scroll_a);
	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 j = vdp_screen[x+(320*y)].B.pat_index;
	 pat_num = vdp_screen[x+(320*y)].B.pat;
	 ix = vdp_screen[x+(320*y)].B.sx;
	 iy = vdp_screen[x+(320*y)].B.sy;
	 pat_pix = pat_pix_b;
	 SDL_SetCursor(cross_scroll_b);
	 break;
	default:
//	 printf("un imp\n");
	 goto un_imp_done;
       }

#define REPEAT\
       for(i=0;i<pat_pix[j].num;i++) {\
        actual_x = ((pat_pix[j].px[i]-vdp_x)*vdp_zoom)+off_x;\
        actual_y = ((pat_pix[j].py[i]-vdp_y)*vdp_zoom)+off_y;\
        if(actual_x >= off_x &&\
           actual_y >= off_y &&\
           actual_x <= (obj->param.w-1) &&\
           actual_y <= (obj->param.h-1))\
        fill_box(obj->param.x+actual_x,\
                 obj->param.y+actual_y,\
                 obj->param.x+actual_x+vdp_zoom,\
                 obj->param.y+actual_y+vdp_zoom,\
                 &globl_fg, &globl_bg, XOR);\
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
       snprintf(info_object->param.dp1, 12, "(%d,%d)",ix,iy);
       snprintf(info_object->param.dp2, 12, "%03x",pat_num);
       MESSAGE_OBJECT(info_object, MSG_DRAW);
       UPDATE_OBJECT(info_object);

       if(current_pattern != pat_num) {
	current_pattern = pat_num;
	MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
	UPDATE_OBJECT(pattern_edit_object);
       }
      }
      x2 = x;
      y2 = y;
      if(wait_on_mouse()==MOUSE_UP) break;
     }
un_imp_done:
    break;
    case PUT_PAT:
     save_state();
     x2 = -1;
     y2 = -1;
     for(;;) {
      x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
      y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      if((x!=x2 || y!=y2 )&&
	  x>=0 && y>= 0 && x<=tmp_w && y<=tmp_h) {
       switch(currently_editing) {
	case EDIT_SCROLL_A:
	 pat_num = vdp_screen[x+(320*y)].A.pat;
	 pat_pix = pat_pix_a;
	 SDL_SetCursor(cross_scroll_a);

	 for(y3=0;y3<sprite_height+1;y3++)
	  for(x3=0;x3<sprite_width+1;x3++) {
	   j = vdp_screen[(x+(8*x3)) + (320*(y+(8*y3)))].A.pat_index;
	   REPEAT
	  }
         UPDATE_OBJECT(preview_object);
	 for(y3=0;y3<sprite_height+1;y3++)
	  for(x3=0;x3<sprite_width+1;x3++) {
	   j = vdp_screen[(x+(8*x3)) + (320*(y+(8*y3)))].A.pat_index;
	   REPEAT
	  }
         j = vdp_screen[x+(320*y)].A.pat_index;

	 break;
	case EDIT_SCROLL_B:
	 pat_num = vdp_screen[x+(320*y)].B.pat;
	 pat_pix = pat_pix_b;
	 SDL_SetCursor(cross_scroll_b);

	 for(y3=0;y3<sprite_height+1;y3++)
	  for(x3=0;x3<sprite_width+1;x3++) {
	   j = vdp_screen[(x+(8*x3)) + (320*(y+(8*y3)))].B.pat_index;
	   REPEAT
	  }
         UPDATE_OBJECT(preview_object);
	 for(y3=0;y3<sprite_height+1;y3++)
	  for(x3=0;x3<sprite_width+1;x3++) {
	   j = vdp_screen[(x+(8*x3)) + (320*(y+(8*y3)))].B.pat_index;
	   REPEAT
	  }

 
	 break;
       }
      }
      x2 = x;
      y2 = y;
      if(wait_on_mouse()==MOUSE_UP) break;
     }
     if(gui_mouse_x >= obj->param.x && 
      gui_mouse_x <= obj->param.x+obj->param.w && 
      gui_mouse_y >= obj->param.y && 
      gui_mouse_y <= obj->param.y+obj->param.h) {
      switch(currently_editing) {
       case EDIT_SCROLL_A:
	i = 0;
	for(x3=0;x3<sprite_width+1;x3++) {
         for(y3=0;y3<sprite_height+1;y3++) {
          j = vdp_screen[(x+(8*x3)) + (320*(y+(8*y3)))].A.pat_index;
   	  current_vdp->vram[current_vdp->scroll_a + (j<<1) + 1] = (current_pattern+i)&0xff;
	  current_vdp->vram[current_vdp->scroll_a + (j<<1)    ] = 
	  (((current_pattern+i)>>8)&3) | ((current_palette&3)<<5) |
	  (high_low == HIGH ? (1<<7) : 0);
          i++;
	 }
	}
	break;
       case EDIT_SCROLL_B:
	i = 0;
	for(x3=0;x3<sprite_width+1;x3++) {
         for(y3=0;y3<sprite_height+1;y3++) {
          j = vdp_screen[(x+(8*x3)) + (320*(y+(8*y3)))].B.pat_index;
   	  current_vdp->vram[current_vdp->scroll_b + (j<<1) + 1] = (current_pattern + i)&0xff;
	  current_vdp->vram[current_vdp->scroll_b + (j<<1)    ] = 
	  (((current_pattern+i)>>8)&3) | ((current_palette&3)<<5) |
	  (high_low == HIGH ? (1<<7) : 0);
          i++;
	 }
	}
	break;
      } 
     }
     render_vdp(0,tmp_h);
     break;
    case FLIP:
     save_state();
     x2 = -1;
     y2 = -1;
     for(;;) {
      x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
      y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      if((x!=x2 || y!=y2 )&&
	  x>=0 && y>= 0 && x<=tmp_w && y<=tmp_h) {
       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
         j = vdp_screen[x+(320*y)].A.pat_index;
	 pat_num = vdp_screen[x+(320*y)].A.pat;
	 pat_pix = pat_pix_a;
         if(hor_ver == HORIZONTAL) 
          SDL_SetCursor(hor_a_flip);
         else
  	  SDL_SetCursor(ver_a_flip);

	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 j = vdp_screen[x+(320*y)].B.pat_index;
	 pat_num = vdp_screen[x+(320*y)].B.pat;
	 pat_pix = pat_pix_b;
         if(hor_ver == HORIZONTAL) 
          SDL_SetCursor(hor_b_flip);
         else
  	  SDL_SetCursor(ver_b_flip);

	 break;
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
      }
      x2 = x;
      y2 = y;
      if(wait_on_mouse()==MOUSE_UP) break;
     }
     if(gui_mouse_x >= obj->param.x && 
      gui_mouse_x <= obj->param.x+obj->param.w && 
      gui_mouse_y >= obj->param.y && 
      gui_mouse_y <= obj->param.y+obj->param.h) {

       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
	 if(hor_ver == HORIZONTAL) 
 	  current_vdp->vram[current_vdp->scroll_a + (j<<1) ] ^= 8;
	 else
	  current_vdp->vram[current_vdp->scroll_a + (j<<1) ] ^= 16;
	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 if(hor_ver == HORIZONTAL) 
 	  current_vdp->vram[current_vdp->scroll_b + (j<<1) ] ^= 8;
	 else
	  current_vdp->vram[current_vdp->scroll_b + (j<<1) ] ^= 16;

	 break;
       }

     }
     render_vdp(0,tmp_h);
     break;

    case PUT_PAL:
     save_state();
     x2 = -1;
     y2 = -1;
     for(;;) {
      x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
      y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      if((x!=x2 || y!=y2 )&&
	  x>=0 && y>= 0 && x<=tmp_w && y<=tmp_h) {
       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
         j = vdp_screen[x+(320*y)].A.pat_index;
	 pat_num = vdp_screen[x+(320*y)].A.pat;
	 pat_pix = pat_pix_a;
         SDL_SetCursor(cross_scroll_a);
	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 j = vdp_screen[x+(320*y)].B.pat_index;
	 pat_num = vdp_screen[x+(320*y)].B.pat;
	 pat_pix = pat_pix_b;
         SDL_SetCursor(cross_scroll_b);
	 break;
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
      }
      x2 = x;
      y2 = y;
      if(wait_on_mouse()==MOUSE_UP) break;
     }
     if(gui_mouse_x >= obj->param.x && 
      gui_mouse_x <= obj->param.x+obj->param.w && 
      gui_mouse_y >= obj->param.y && 
      gui_mouse_y <= obj->param.y+obj->param.h) {

       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
	 current_vdp->vram[current_vdp->scroll_a + (j<<1) ] &= ~(3<<5);
	 current_vdp->vram[current_vdp->scroll_a + (j<<1) ] |= ((current_palette&3)<<5);
	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 current_vdp->vram[current_vdp->scroll_b + (j<<1) ] &= ~(3<<5);
	 current_vdp->vram[current_vdp->scroll_b + (j<<1) ] |= ((current_palette&3)<<5);

	 break;
       }

     }
     render_vdp(0,tmp_h);
     break;

    case HI_LOW:
     save_state();
     x2 = -1;
     y2 = -1;
     for(;;) {
      x = ((((gui_mouse_x - obj->param.x) - off_x) / vdp_zoom) + vdp_x);
      y = ((((gui_mouse_y - obj->param.y) - off_y) / vdp_zoom) + vdp_y);
      if((x!=x2 || y!=y2 )&&
	  x>=0 && y>= 0 && x<=tmp_w && y<=tmp_h) {
       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
         j = vdp_screen[x+(320*y)].A.pat_index;
	 pat_num = vdp_screen[x+(320*y)].A.pat;
	 pat_pix = pat_pix_a;
         SDL_SetCursor(cross_scroll_a);
	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 j = vdp_screen[x+(320*y)].B.pat_index;
	 pat_num = vdp_screen[x+(320*y)].B.pat;
	 pat_pix = pat_pix_b;
         SDL_SetCursor(cross_scroll_b);
	 break;
       }
       REPEAT
       UPDATE_OBJECT(preview_object);
       REPEAT
#undef REPEAT
      }
      x2 = x;
      y2 = y;
      if(wait_on_mouse()==MOUSE_UP) break;
     }
     if(gui_mouse_x >= obj->param.x && 
      gui_mouse_x <= obj->param.x+obj->param.w && 
      gui_mouse_y >= obj->param.y && 
      gui_mouse_y <= obj->param.y+obj->param.h) {

       switch(vdp_screen[x+(320*y)].pic_top) {
	case VIS_SCROLL_A:
	case VIS_SCROLL_A_HIGH:
	 current_vdp->vram[current_vdp->scroll_a + (j<<1) ] &= ~(1<<7);
	 current_vdp->vram[current_vdp->scroll_a + (j<<1) ] |= 
	  (high_low == HIGH ? (1<<7) : 0);
	 break;
	case VIS_SCROLL_B:
	case VIS_SCROLL_B_HIGH:
	 current_vdp->vram[current_vdp->scroll_b + (j<<1) ] &= ~(1<<7);
	 current_vdp->vram[current_vdp->scroll_b + (j<<1) ] |= 
	  (high_low == HIGH ? (1<<7) : 0);

	 break;
       }

     }
     render_vdp(0,tmp_h);
     break;




   }

   if(gui_mouse_x < obj->param.x ||
      gui_mouse_x > obj->param.x+obj->param.w ||
      gui_mouse_y < obj->param.y ||
      gui_mouse_y > obj->param.y+obj->param.h)
    SDL_SetCursor(arrow);
   else
    SDL_SetCursor(working_cursor);
   if(current_tool == LINE ||
      current_tool == FILL ||
      current_tool == SELECT)  {
    if(currently_editing == EDIT_SPRITE) {
     for(i=0;i<num_xor_pix;i++) {
      put_pix(current_vdp->vram, 
	sprite_screen[xor_pixels[i].x + (xor_pixels[i].y*32)].pattern,
	xor_pixels[i].x%8,
	xor_pixels[i].y%8);
     }
    } else
     collapse();
    MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
    MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
    if(currently_editing == EDIT_SPRITE)
     render_sprite();
    else
     render_vdp(0,tmp_h);
   }
   MESSAGE_OBJECT(obj, MSG_DRAW);
   break;
  case MSG_INFOCUS:
   SDL_SetCursor(working_cursor);
   break;
  case MSG_OUTFOCUS:
   snprintf(info_object->param.dp1, 12, ":/");
   snprintf(info_object->param.dp2, 12, ":-|");
   MESSAGE_OBJECT(info_object, MSG_DRAW);
   UPDATE_OBJECT(info_object);
   SDL_SetCursor(arrow);
   break;

  case MSG_KEYDOWN:
   switch(data) {
    case SDLK_UP:
     sprite_overlay(minus_object, 1);
     break;
    case SDLK_DOWN:
     sprite_overlay(plus_object, 1);
     break;
   }
   break;
 }
 UPDATE_OBJECT(obj);
// proc_shadow_box(msg,obj,data);
 return RET_OK;
}
/*}}}*/
/*proc_palette_change_object{{{*/
int proc_palette_change_object(int msg, struct object_t *obj, int data) {
 int j,i;
 int p,q;
 int lp,lq;
 switch(msg) {
  case MSG_DRAW:
   proc_shadow_box(MSG_DRAW,obj, data);
   for(j = 0;j<4;j++) 
    for(i=0;i<16;i++) {
     fill_box(obj->param.x+(j*16)+6 ,obj->param.y+(i*16)+4,
	      obj->param.x+(j*16)+16,obj->param.y+(i*16)+18, 
	      &current_vdp->palette[j][i],&globl_bg,NO_HASH);
     box( obj->param.x+(j*16)+5,obj->param.y+(i*16)+3,
          obj->param.x+(j*16)+17,obj->param.y+(i*16)+19,&globl_fg,&globl_bg,NO_HASH);

     vline(obj->param.x+(current_palette*16)+9,obj->param.y,obj->param.y+obj->param.h,
       &globl_fg,&globl_bg, NO_HASH);
     vline(obj->param.x+(current_palette*16)+10,obj->param.y,obj->param.y+obj->param.h,
       &globl_bg,&globl_fg, NO_HASH);
     vline(obj->param.x+(current_palette*16)+11,obj->param.y,obj->param.y+obj->param.h,
       &globl_fg,&globl_bg, NO_HASH);

     hline(obj->param.x,
       obj->param.y+(current_palette_index*16)+10,obj->param.x+obj->param.w,
       &globl_fg,&globl_bg,NO_HASH);
     hline(obj->param.x,
       obj->param.y+(current_palette_index*16)+11,obj->param.x+obj->param.w,
       &globl_bg,&globl_fg,NO_HASH);
     hline(obj->param.x,
       obj->param.y+(current_palette_index*16)+12,obj->param.x+obj->param.w,
       &globl_fg,&globl_bg,NO_HASH);
    }
   break;
  case MSG_CLICK:
   lp = 400;
   lq = 400;
   p = current_palette;
   q = current_palette_index;
   for(;;) {
    i = ((gui_mouse_x - obj->param.x)-3) / 16;
    if((i>=0) && (i<4))
     p = i;
    i = ((gui_mouse_y - obj->param.y)-3) / 16;
    if((i>=0) && (i<16))
     q = i;
    if(lp!=p || lq!=q) { 
     change_color(p,q);
     UPDATE_OBJECT(current_color_object);
     UPDATE_OBJECT(palette_change_object);
     UPDATE_OBJECT(pattern_select_object);
     UPDATE_OBJECT(pattern_edit_object);
     UPDATE_OBJECT(rgb_red_object);
     UPDATE_OBJECT(rgb_green_object);
     UPDATE_OBJECT(rgb_blue_object);
     if(currently_editing == EDIT_SPRITE) {
      DRAW_PREVIEW;
     }
    }
    lp = p;
    lq = q;
    if(wait_on_mouse() == MOUSE_UP) break;
   }
   break;
 }
 return RET_OK;
}
/*}}}*/
/*proc_pattern_edit{{{*/
int proc_pattern_edit(int msg,struct object_t *obj, int data) {
 int x,y;
 int lx, ly;
 int j;
 int x1=0, y1=0;
 int sub_x, sub_y;
 color_t *save_bg;
 char buf[4];
 switch(msg) {
  case MSG_DRAW:
   save_bg = obj->param.bg;
   if(pat_ref[current_pattern] !=0)
    obj->param.bg = &sprite_color;
   if(pat_stop[current_pattern] == TRUE)
    obj->param.bg = &scroll_a_color;
   proc_shadow_box(MSG_DRAW,obj,data);
   obj->param.bg = save_bg;

   if(pat_stop[current_pattern] == TRUE) 
    stop_object->param.d1 = TRUE;
   else
    stop_object->param.d1 = FALSE;
   MESSAGE_OBJECT(stop_object, MSG_DRAW);

   snprintf(buf, 4, "%03X", current_pattern);
   draw_text(obj->param.x+21,obj->param.y+2, buf, obj->param.fg, obj->param.bg, NO_HASH,0);
   draw_pattern(current_vdp->vram,current_pattern,obj->param.x+2 ,obj->param.y+10, 64, 64);
   UPDATE_OBJECT(stop_object);
   break;
  case MSG_CLICK:
   switch(current_tool) {
    case PIC:
#define TOOL_PROLOG \
     lx = 400; \
     ly = 400; \
     for(;;) { \
      if( gui_mouse_x > (obj->param.x+2 ) && gui_mouse_x < (obj->param.x+66) && \
	  gui_mouse_y > (obj->param.y+10) && gui_mouse_y < (obj->param.y+74)) { \
       x = (gui_mouse_x - obj->param.x -2 ) / 8; \
       y = (gui_mouse_y - obj->param.y -10) / 8;
      TOOL_PROLOG
       if(x!=lx || y!=ly) {
        j = get_pix(current_vdp->vram, current_pattern, x,y);
	if(j!=current_palette_index) {
         change_color(current_palette, j);
         UPDATE_OBJECT(current_color_object);
         UPDATE_OBJECT(palette_change_object);
         UPDATE_OBJECT(pattern_select_object);
         UPDATE_OBJECT(pattern_edit_object);
         UPDATE_OBJECT(rgb_red_object);
         UPDATE_OBJECT(rgb_green_object);
         UPDATE_OBJECT(rgb_blue_object);
	}
#define REPEAT \
	fill_box(obj->param.x+2+ (8*x), obj->param.y+10+(8*y),\
	         obj->param.x+10+(8*x), obj->param.y+18+(8*y), &globl_fg,&globl_bg,XOR);
        REPEAT
        UPDATE_OBJECT(obj);
        REPEAT
#undef	REPEAT	
       } 
#define TOOL_EPLOG \
       lx = x; \
       ly = y; \
      } \
      if(wait_on_mouse() == MOUSE_UP) break; \
     }
     TOOL_EPLOG
     break;
    case LINE:
     j =0;
     save_state();
     TOOL_PROLOG
      if(lx != x || ly != y) {
       memset(draw_buffer,0, 320 * 8);
       if(j == 0) {
	x1 = x;
	y1 = y;
	j++;
       }
       do_line(NULL, 0, x1, y1, x, y, put_pix_draw_buffer);
#define REPEAT \
       for(sub_y=0;sub_y<8;sub_y++) \
	for(sub_x=0;sub_x<8;sub_x++) \
	  if(draw_buffer[sub_x][sub_y] == 1) \
  	   fill_box(obj->param.x+2+ (8*sub_x), obj->param.y+10+(8*sub_y),\
	            obj->param.x+10+(8*sub_x), obj->param.y+18+(8*sub_y), &globl_fg,&globl_bg,XOR);
       REPEAT
       UPDATE_OBJECT(obj);
       REPEAT
#undef REPEAT

      } 
     TOOL_EPLOG
     for(sub_y=0;sub_y<8;sub_y++)
      for(sub_x=0;sub_x<8;sub_x++)
       if(draw_buffer[sub_x][sub_y] == 1)
	put_pix(current_vdp->vram, current_pattern, sub_x, sub_y);
     MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
     MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
     break;
    case FILL:
     save_state();
     TOOL_PROLOG
      if(lx!=x || ly != y) {
       memset(draw_buffer,0, 320 * 8);
       do_flood(current_vdp->vram,current_pattern,x,y,0,0,7,7,put_pix_draw_buffer,get_pix_special);
#define REPEAT \
       for(sub_y=0;sub_y<8;sub_y++) \
	for(sub_x=0;sub_x<8;sub_x++) \
	  if(draw_buffer[sub_x][sub_y] == 1) \
  	   fill_box(obj->param.x+2+ (8*sub_x), obj->param.y+10+(8*sub_y),\
	            obj->param.x+10+(8*sub_x), obj->param.y+18+(8*sub_y), &globl_fg,&globl_bg,XOR);
       REPEAT
       UPDATE_OBJECT(obj);
       REPEAT
#undef REPEAT


      }
     TOOL_EPLOG
     for(sub_y=0;sub_y<8;sub_y++)
      for(sub_x=0;sub_x<8;sub_x++)
       if(draw_buffer[sub_x][sub_y] == 1)
	put_pix(current_vdp->vram, current_pattern, sub_x, sub_y);
     MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
     MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
     break;
    case CLRCOLOR:
     save_state();
#define REPEAT \
     fill_box(obj->param.x+2, obj->param.y+10,obj->param.x+66,obj->param.y+74, \
       &globl_fg,&globl_bg,XOR);
     REPEAT
     UPDATE_OBJECT(obj);
     REPEAT
     for(;;)
      if(wait_on_mouse() == MOUSE_UP) break;
     for(sub_y=0;sub_y<8;sub_y++)
      for(sub_x=0;sub_x<8;sub_x++)
       put_pix(current_vdp->vram, current_pattern, sub_x, sub_y);
     MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
     MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
     break;
#undef TOOL_PROLOG
#undef TOOL_EPLOG
   }
   render_vdp(0,vdp_h);
   MESSAGE_OBJECT(preview_object, MSG_DRAW);
   break;
  case MSG_INFOCUS:
   SDL_SetCursor(working_cursor);
   break;
  case MSG_OUTFOCUS:
   SDL_SetCursor(arrow);
   break;
  case MSG_KEYDOWN:
   if(data == SDLK_DOWN) {
    current_pattern+=(sprite_width +1) * (sprite_height +1);
    if(current_pattern>0x7ff)
     current_pattern = 0x7ff;
    if(currently_editing == EDIT_SPRITE) {
     DRAW_PREVIEW;
    }
   } 
   if(data == SDLK_UP) {
    current_pattern-=(sprite_width +1) * (sprite_height +1);
    if(current_pattern<0)
     current_pattern = 0;
    if(currently_editing == EDIT_SPRITE) {
     DRAW_PREVIEW;
    }

   }
   if(data == SDLK_RIGHT)
    pat_stop[current_pattern] = TRUE;
   if(data == SDLK_LEFT)
    pat_stop[current_pattern] = FALSE;

   MESSAGE_OBJECT(obj, MSG_DRAW);

   break;

 }
 return RET_OK;
}

int pattern_select_bot(struct object_t *obj, int data) {
 MESSAGE_OBJECT(pattern_select_object, MSG_DRAW);
/* MESSAGE_OBJECT(current_color_object, MSG_DRAW);
 MESSAGE_OBJECT(current_color_text_object,MSG_DRAW); */
 UPDATE_OBJECT(pattern_select_object);
 return RET_OK;
}
/*}}}*/
/*proc_pattern_select{{{*/
int proc_pattern_select(int msg, struct object_t *obj, int data) {
 int i, j,q, top;
 int li;
 int mult_pointers;
 int x,y;
 int  k, kolor;
 int pat_num;
 int poffset;
 union {
  Uint8  *b;
  Uint32 *l;
 } pix, next;
 int mask;
 color_t *text_color[5];

 char buf[6];
 switch(msg) {
  case MSG_DRAW:
   mask = (gc->format->Rmask |
           gc->format->Gmask |
	   gc->format->Bmask |
	   gc->format->Amask)^~0;

   fill_box( obj->param.x, obj->param.y, 
             obj->param.x+(5*8)+2,  obj->param.y+(0xa * 16)+8, obj->param.bg, obj->param.bg, NO_HASH);
   /* XXX pointer math */
   pix.b=(Uint8 *)(gc->pixels + (gc->pitch*obj->param.y));
   for(i=0;i<0xa;i++) {
    top = ((pattern_select_scroll_bar->param.d1 +i) << 8);
    snprintf(buf, 6, "%04X", top);

    for(q=0;q<5;q++)
     text_color[q] = obj->param.bg; 
    if( (top&0xe000) == top  )  
     text_color[0] = &scroll_a_color;
    if( (top&0xe000) == top)  
     text_color[1] = &scroll_b_color;
    if( (top&(current_vdp->cell_w == 40 ? 0xf000 : 0xf800))== top )  
     text_color[2] = &window_color;
    if( (top&(current_vdp->cell_w == 40 ? 0xfc00 : 0xfe00)) == top  ) 
     text_color[3] = &sprite_color;
    if( (top&0xfc00) == top) 
     text_color[4] = &hscroll_color;
    for(q = 0; q< 5;q++) 
     fill_box(obj->param.x+11+(q*6), (i*16)+4+obj->param.y,
              obj->param.x+16+(q*6), (i*16)+4+obj->param.y+16,
 	      text_color[q], obj->param.bg, NO_HASH);

    draw_text(obj->param.x+9,(i*16)+8+obj->param.y, 
      buf, obj->param.fg, obj->param.bg, NO_HASH,0);
 
    mult_pointers=0;
    text_color[0] = obj->param.bg;
    if(top == current_vdp->scroll_a) {
     text_color[0] = &scroll_a_color;
     mult_pointers++; 
    }
    if(top == current_vdp->scroll_b) {
     text_color[0] = &scroll_b_color;
     mult_pointers++; 
    }
    if(top == current_vdp->window) {
     text_color[0] = &window_color;
     mult_pointers++; 
    }
    if(top == current_vdp->hscroll) {
     text_color[0] = &hscroll_color;
     mult_pointers++; 
    }
    if(top == current_vdp->sprite_table) {
     text_color[0] = &sprite_color;
     mult_pointers++; 
    }
    if(mult_pointers>1)
     text_color[0] = &globl_move_color;

    fill_box(obj->param.x+1,(i*16)+4+obj->param.y, 
             obj->param.x+1+8,(i*16)+4+16+obj->param.y, 

      text_color[0], obj->param.bg, NO_HASH);

    if(((a_pattern*32) >= top) && ((a_pattern*32)<(top+0x100)) ) 
     fill_box(obj->param.x+3,(i*16)+4+obj->param.y, 
              obj->param.x+3+2,(i*16)+4+16+obj->param.y, 
      obj->param.fg, obj->param.bg, NO_HASH);

    if(((b_pattern*32) >= top) && ((b_pattern*32)<(top+0x100)) ) 
     fill_box(obj->param.x+3,(i*16)+4+obj->param.y, 
              obj->param.x+3+2,(i*16)+4+16+obj->param.y, 
      obj->param.fg, obj->param.bg, NO_HASH);




    for(y=0;y<8;y++) {
     /* XXX pointer math */
     pix.b = (Uint8 *)(gui_screen->pixels+(gui_screen->pitch*(obj->param.y+4+(i*16)+(y*2)))+
                       (gui_screen->format->BytesPerPixel * (46+obj->param.x)));


     next.b =(Uint8 *)(pix.b + gui_screen->pitch);
     for(j=0;j<8;j++) { 
      pat_num = ((pattern_select_scroll_bar->param.d1 + i) *8) +  j;
      poffset = pat_num * 0x20 + (y*4); 
      for(k=0;k<8;k+=2) { 
       kolor = (current_vdp->vram[poffset]>>4) &0xf;

#define PIXEL \
       *pix.l&=mask; \
       *pix.l|=current_vdp->palette[current_palette][kolor].map; \
       pix.b+=gc->format->BytesPerPixel;

       /* we're drawing 2x2 pixels at 4-bit nibbles so we draw 4-pixels
	* per iteration */

       PIXEL; 
       PIXEL;

       kolor = current_vdp->vram[poffset]&0xf;

       PIXEL;
       PIXEL;


#undef PIXEL
       poffset++;
      }
     }
     /* we're drawing 2x2 pixels, so we double the scan-line */
     /* XXX pointer math */
     memcpy(next.b, (void *)(next.b - gui_screen->pitch), 
            gc->format->BytesPerPixel*128);
    }
   }
  break;
  case MSG_CLICK:
   x = gui_mouse_x;
   y = gui_mouse_y;
   if(x < (obj->param.x + 128+46) && x > (obj->param.x +46) &&
      y < (obj->param.y + 164) &&    y > (obj->param.y +4)) { 
    i = current_pattern;
    li = i;
    for(;;) {
     
     if(gui_mouse_x < (obj->param.x + 128+46) && gui_mouse_x > (obj->param.x +46) &&
        gui_mouse_y < (obj->param.y + 164) &&    gui_mouse_y > (obj->param.y +4)) {
      x = ((gui_mouse_x - obj->param.x - 46) / 16) % 8;
      y = ((gui_mouse_y - obj->param.y - 4)  / 16);
      i = ((y<<3)+x) + (pattern_select_scroll_bar->param.d1 << 3);
      if(li != i) {
       if(gui_mouse_x < (obj->param.x + 128+46) && gui_mouse_x > (obj->param.x +46) &&
         gui_mouse_y < (obj->param.y + 164) &&    gui_mouse_y > (obj->param.y +4)) {
        current_pattern = i; 
        MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
	UPDATE_OBJECT(pattern_edit_object);
       }
      }
      li = i;
#undef REPEAT
#define REPEAT \
      fill_box(obj->param.x + 46 + (16*x),  obj->param.y + 4 + (16*y),  \
               obj->param.x + 46 + (16*x)+16,obj->param.y +20+ (16*y), &globl_fg, &globl_bg, XOR);  
      REPEAT
      UPDATE_OBJECT(obj);
      REPEAT
#undef REPEAT
     }
     if(currently_editing == EDIT_SPRITE) {
      DRAW_PREVIEW;
     }
     if(wait_on_mouse() == MOUSE_UP) break;
    }
   }
   if( (gui_mouse_x > obj->param.x) && (gui_mouse_x < (obj->param.x + 42)) &&
       (gui_mouse_y > (obj->param.y+8) && (gui_mouse_y < (obj->param.y + 160)))) {
    i = (gui_mouse_y - (obj->param.y+4)) / 16;
    top = (i + pattern_select_scroll_bar->param.d1) <<8;
    fill_box(obj->param.x+10, 4+obj->param.y+(i*16),
             obj->param.x+42, 20+obj->param.y+(i*16), &globl_fg, &globl_bg, XOR);
    UPDATE_OBJECT(obj);
  

    if( ((top&0xe000)==top)) {  
     scroll_a_radio->param.flags &= (INACTIVE ^ ~(int)0);
     if(top == current_vdp->scroll_a)
      scroll_a_radio->param.d1 = TRUE;
     else
      scroll_a_radio->param.d1 = FALSE;
    } else 
     scroll_a_radio->param.flags |= INACTIVE;

    if( ((top&0xe000) == top)) {
     scroll_b_radio->param.flags &= (INACTIVE ^ ~(int)0);
     if(top == current_vdp->scroll_b)
      scroll_b_radio->param.d1 = TRUE;
     else
      scroll_b_radio->param.d1 = FALSE;
    } else
     scroll_b_radio->param.flags |= INACTIVE;

    if((top&(current_vdp->cell_w == 40 ? 0xfc00 : 0xfe00)) == top) { 
     sprite_radio->param.flags &= (INACTIVE ^ ~(0));
     if(top == current_vdp->sprite_table)
      sprite_radio->param.d1 = TRUE;
     else
      sprite_radio->param.d1 = FALSE;
    } else
     sprite_radio->param.flags |= INACTIVE;

    if((top& (current_vdp->cell_w == 40 ? 0xf000 : 0xf800))==top) {
     window_radio->param.flags &= (INACTIVE ^ ~(int)0);
     if(top == current_vdp->window) 
      window_radio->param.d1 = TRUE;
     else
      window_radio->param.d1 = FALSE;
    } else
     window_radio->param.flags |= INACTIVE;

    if((top&0xfc00) == top) {
     hscroll_radio->param.flags &= (INACTIVE ^ ~(int)0);
     if(top == current_vdp->hscroll)
      hscroll_radio->param.d1 = TRUE;
     else
      hscroll_radio->param.d1 = FALSE;
    } else
     hscroll_radio->param.flags |= INACTIVE;

    group_loop(pointers);

    if((CHECK_FLAG(hscroll_radio->param.flags, INACTIVE) == FALSE) &&
     hscroll_radio->param.d1 == TRUE)
     current_vdp->hscroll = top;

   if((CHECK_FLAG(scroll_a_radio->param.flags, INACTIVE) == FALSE) &&
     scroll_a_radio->param.d1 == TRUE)
     current_vdp->scroll_a = top;

   if((CHECK_FLAG(scroll_b_radio->param.flags, INACTIVE) == FALSE) &&
     scroll_b_radio->param.d1 == TRUE)
     current_vdp->scroll_b = top;

   if((CHECK_FLAG(window_radio->param.flags, INACTIVE) == FALSE) &&
     window_radio->param.d1 == TRUE)
     current_vdp->window = top;

   if((CHECK_FLAG(sprite_radio->param.flags, INACTIVE) == FALSE) &&
     sprite_radio->param.d1 == TRUE)
     current_vdp->sprite_table = top;

    
    MESSAGE_OBJECT(obj, MSG_DRAW);
    render_vdp(0,vdp_h);
    MESSAGE_OBJECT( preview_object, MSG_DRAW);
    UPDATE_OBJECT( preview_object);
   }
  break;
  case MSG_KEYDOWN:
   if(data == SDLK_DOWN) {
    current_pattern+=(sprite_width + 1) * (sprite_height+1);
    if(a_pattern == b_pattern) {
     if(current_pattern>0x7ff)
      current_pattern = 0x7ff;
    } else {
     if(current_pattern > b_pattern)
      current_pattern = a_pattern; 
    }
    if(currently_editing == EDIT_SPRITE) {
     DRAW_PREVIEW;
    }
   } 
   if(data == SDLK_UP) {
    current_pattern-=(sprite_width +1) * (sprite_height+1);
    if(a_pattern == b_pattern) {
     if(current_pattern<0)
      current_pattern = 0;
    } else {
     if(current_pattern < a_pattern)
      current_pattern = b_pattern;
    }
    if(currently_editing == EDIT_SPRITE) {
     DRAW_PREVIEW;
    }
   }
   if(data == SDLK_RIGHT)
    pat_stop[current_pattern] = TRUE;
   if(data == SDLK_LEFT)
    pat_stop[current_pattern] = FALSE;

   MESSAGE_OBJECT(pattern_edit_object, MSG_DRAW);
   UPDATE_OBJECT( pattern_edit_object); 

   break;
 }
 return RET_OK;
}
/*}}}*/
#define BOX_SIZE	11	
/*proc_scroll_bar_special{{{*/
int proc_scroll_bar_special(int msg, struct object_t *obj, int data) {
 color_t half_color;
 float real_x, real_y; 
 int ret;
 int dec_key1, dec_key2;
 int inc_key1, inc_key2;
 int px, py, x, y, d, ld;
 int ix, iy, actual_x, actual_y;

 int start;
 int off_x, off_y;
 color_t *color;
 int flip;

 if(CHECK_FLAG(obj->param.flags, INACTIVE) == TRUE) return RET_OK;

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
    start = 0;
    ld = -1;
    for(;;) {
     if(start != 0)
      ld = obj->param.d1;
     else
      start++;
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

      if(current_vdp->tv_type == NTSC)
       off_y = 8;
      else
       off_y = 0;
      if(current_vdp->cell_w == 32)
       off_x = 32;
      else
       off_x = 0;

      if(obj->param.user_flags == HORIZONTAL) {
       flip = 0;
       for(iy=0;iy<vdp_h;iy++) {
        if(flip == 0)
         color = &sprite_color;
        else
         color = &hscroll_color;
        switch(obj->param.d1) {
 	case HOR_8LINES:
         if( (iy%8) == 7)
          flip^=1;
 	 break;
        case HOR_LINES:
 	 flip^=1;
 	 break;
        }
        for(ix=0;ix<vdp_w;ix++) {
#define REPEAT \
 	actual_x = ((ix-vdp_x)*vdp_zoom)+off_x;\
 	actual_y = ((iy-vdp_y)*vdp_zoom)+off_y;\
 	if(actual_x>=off_x &&\
 	   actual_y>=off_y &&\
 	   actual_x<=((vdp_w+off_x)-1) &&\
 	   actual_y<=((vdp_h+off_y)-1))\
 	 fill_box(preview_object->param.x+actual_x,\
 	          preview_object->param.y+actual_y,\
		  preview_object->param.x+actual_x+vdp_zoom,\
		  preview_object->param.y+actual_y+vdp_zoom,\
		  color, obj->param.bg, NO_HASH);
	 REPEAT
        }
       }
      } else {
       for(iy=0;iy<vdp_h;iy++) {
	flip = 0;
	for(ix=0;ix<vdp_w;ix++) {
	 if(flip == 0)
	  color = &sprite_color;
	 else
	  color = &hscroll_color;
	 if(obj->param.d1 == VER_2CELLS)
	  if( (ix%16)==15)
	   flip^=1;
         REPEAT 
#undef REPEAT
	}
       }
        
      }
 
      if(currently_editing != EDIT_SPRITE) { 
       UPDATE_OBJECT(preview_object);
      }
/*      if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
       ret =  obj->param.callback( obj, NULL);
       if(ret != RET_OK) return ret;
      }
      */

      proc_scroll_bar_special(MSG_DRAW,obj, 0);
      UPDATE_OBJECT(obj);
     }
    }
    
   }
   render_vdp(0,vdp_h);
   DRAW_PREVIEW;
   UPDATE_OBJECT(preview_object);

   break;
  case MSG_INFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE)
    proc_scroll_bar_special(MSG_DRAW,obj, 0);
   break;
  case MSG_OUTFOCUS:
   if(CHECK_FLAG(obj->param.flags, SHOW_FOCUS) == TRUE) 
    proc_scroll_bar_special(MSG_DRAW,obj, 0);
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

    proc_scroll_bar_special(MSG_DRAW,obj, 0);
    UPDATE_OBJECT(obj);
   }
   if(data == dec_key1|| data == dec_key2) {
    obj->param.d1--;
    if(obj->param.d1 < 0) obj->param.d1++;
    if(CHECK_FLAG(obj->param.flags, CALL_BUTTON) == TRUE) {
     ret =  obj->param.callback( obj, 0);
     if(ret != RET_OK) return ret;
    }
    proc_scroll_bar_special(MSG_DRAW,obj, 0);
    UPDATE_OBJECT(obj);

   }
   break;
 }
 return RET_OK;
}
/*}}}*/
/* line_edit_wonked {{{ */
int line_edit_wonked(int msg, struct object_t *obj, int data) {
 char *cp;
 color_t tmp_color;
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
    return RET_QUIT;
   } else { 
    if( data == SDLK_BACKSPACE) {
     obj->param.d2--;
     if(obj->param.d2 < 0)
      obj->param.d2 = 0;
     else
      cp[obj->param.d2] = 0;
      
    } else {
     obj->param.d2++;
     if(obj->param.d2 > obj->param.d3) 
      obj->param.d2 = obj->param.d3;
     cp[obj->param.d2-1] = data;
     cp[obj->param.d2] = 0;
    }
   }
   MESSAGE_OBJECT(obj, MSG_DRAW);
   break;
 
 }
 return RET_OK;
}

/* }}} */
