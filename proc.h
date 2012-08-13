#define	HORIZONTAL	1	
#define VERTICAL	2


#define	HOR_LINES	0
#define HOR_8LINES	1
#define HOR_SCREEN	2

#define VER_SCREEN	0
#define VER_2CELLS	1

#define BACKGROUND	0
#define SCROLL_A	1
#define SCROLL_B	2
#define SPRITE		3

#define NO_SELECTION	-1
#define SEL_W	(selection_v2.x - selection_v1.x)
#define SEL_H	(selection_v2.y - selection_v1.y)


#define DRAW_PREVIEW	MESSAGE_OBJECT(preview_box_object,MSG_DRAW); \
 			MESSAGE_OBJECT(preview_ntsc, MSG_DRAW); \
			MESSAGE_OBJECT(preview_pal, MSG_DRAW); \
			MESSAGE_OBJECT(preview_40c, MSG_DRAW); \
			MESSAGE_OBJECT(preview_32c, MSG_DRAW); \
 			MESSAGE_OBJECT(preview_scroll_bar, MSG_DRAW); \
			MESSAGE_OBJECT(preview_x_scroll, MSG_DRAW); \
			MESSAGE_OBJECT(preview_y_scroll, MSG_DRAW); \
			MESSAGE_OBJECT(preview_zoom_in, MSG_DRAW); \
			MESSAGE_OBJECT(preview_zoom_out,MSG_DRAW); \
			MESSAGE_OBJECT(preview_overlay, MSG_DRAW); \
			MESSAGE_OBJECT(bg_color_box, MSG_DRAW);\
                        MESSAGE_OBJECT(preview_object, MSG_DRAW)

#define DRAW_RGB_CHOOSER	current_color_object->param.proc(MSG_DRAW,current_color_object,0);\
 				current_color_text_object->param.proc(MSG_DRAW,\
				  current_color_text_object,0); \
				current_color_text2_object->param.proc(MSG_DRAW, \
				  current_color_text2_object,0); \
                                rgb_red_object->param.proc(MSG_DRAW,rgb_red_object,0); \
				rgb_green_object->param.proc(MSG_DRAW,rgb_green_object,0); \
                                rgb_blue_object->param.proc(MSG_DRAW,rgb_blue_object,0)

typedef struct {
 int x, y;
} coord_t;

extern int vdp_w, vdp_h;

extern Uint8 selection_buffer[ 320 * 240];
extern coord_t selection_v1, selection_v2;


extern struct object_t *pattern_select_scroll_bar,
                       *pattern_select_object;

extern struct object_t *pattern_edit_object,
       		       *palette_change_object;

extern struct object_t *preview_object;

extern struct object_t *current_color_object;
extern struct object_t *current_color_text_object, *current_color_text2_object;

extern struct object_t *rgb_red_object,
           	       *rgb_green_object,
		       *rgb_blue_object;

extern color_t red,green,blue;
extern color_t current_color_text_color;

extern int sprite_zoom, scroll_plane_zoom;

void update_color_text(void);
void change_color(int palette, int index);

int preview_scroll_change(struct object_t *obj, int data);
int preview_zoom_change(struct object_t *obj, int data);

int preview_size_change(struct object_t *obj, int data);

int proc_info(int msg, struct object_t *obj, int data);
int select_quit_on_click(int msg, struct object_t *obj, int data);
int load_default_mega(int msg, struct object_t *obj, int data);
int select_special(int msg, struct object_t *obj, int data);
int proc_sprite_size(int msg, struct object_t *obj, int data);
int proc_scroll_size(int msg, struct object_t *obj, int data);
int proc_preview_object(int msg, struct object_t *obj, int data);
int pattern_select_bot(struct object_t *obj, int data);
int color_change_bot(struct object_t *obj, int data);
int proc_palette_change_object(int msg, struct object_t *obj, int data);
int proc_pattern_edit(int msg, struct object_t *obj, int data);
int proc_pattern_select(int msg, struct object_t *obj, int data);
int proc_scroll_bar_special(int msg, struct object_t *obj, int data);
int line_edit_wonked(int msg, struct object_t *obj, int data);
void update_zoom(int in); 
