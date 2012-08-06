#define B0000	0x0
#define B0001	0x1
#define B0010	0x2
#define B0011	0x3
#define B0100	0x4
#define B0101	0x5
#define B0110	0x6
#define B0111	0x7
#define B1000	0x8
#define B1001	0x9
#define B1010	0xA
#define	B1011	0xB
#define B1100	0xC
#define B1101	0xD
#define B1110	0xE
#define B1111	0xF

#define MAX_UNDO	0xf	

#define	LINE		0
#define FILL		1
#define PIC		2
#define CLRCOLOR 	3
#define PIC_PAT		4
#define PUT_PAT		5
#define FLIP		6
#define HI_LOW		7
#define PUT_PAL		8
#define SELECT		9

#define LOAD_CRAM	0
#define LOAD_VRAM	1
#define LOAD_HAPPY	4
#define LOAD_VSRAM	5
#define SAVE_CRAM	2
#define SAVE_VRAM	3
#define SAVE_HAPPY	6
#define SAVE_VSRAM	7
#define SAVE_CRAM_HEAD	8
#define CHANGE_LOAD_MESSAGE	9

extern group_t *main_grp;
extern group_t *load_grp, *save_grp;
extern group_t *high_low_grp;
extern group_t *pal_hi_low_grp;
extern group_t *hor_ver_grp;
extern group_t *copy_move_grp;
extern group_t *pointers;
extern group_t *select_grp;
extern group_t *change_message_grp;

extern SDL_Cursor *arrow, *cross, *vertical, *horizontal, *no;
extern SDL_Cursor *working_cursor;
extern SDL_Cursor *cross_scroll_a, *cross_scroll_b, *cross_sprite;
extern SDL_Cursor *hor_a_flip, *ver_a_flip, *hor_b_flip, *ver_b_flip;

extern struct select_file_t *overlay_sel;
extern struct select_file_t *background_sel;

extern int load_initial;
extern char *load_initial_name;

extern int current_tool;
extern int undo_A, undo_B, undo_D;

extern color_t sprite_color, hscroll_color, scroll_a_color, scroll_b_color, window_color;

extern struct object_t *text_edit_object;
extern struct object_t *rgb_box_object;
extern struct object_t *bg_color_box;
extern struct object_t *sprite_radio;
extern struct object_t *hscroll_radio;
extern struct object_t *scroll_a_radio;
extern struct object_t *scroll_b_radio;
extern struct object_t *window_radio;
extern struct object_t *knob_text;
extern struct object_t *select_special_object;
extern struct object_t *horizontal_scroll_bar, *vertical_scroll_bar;
extern struct object_t *info_object;

extern struct object_t *stop_object;
extern struct object_t *clob_object;

void save_state(void);
void undo(void);

extern struct object_t *knob_message_box, *knob, *knob_icon, *knob_message;

extern struct object_t *preview_box_object;
extern struct object_t *preview_scroll_bar;
extern struct object_t *preview_x_scroll, *preview_y_scroll;
extern struct object_t *preview_ntsc, *preview_pal;
extern struct object_t *preview_40c,  *preview_32c;
extern struct object_t *preview_zoom_in, *preview_zoom_out;
extern struct object_t *preview_overlay;

extern struct object_t *scroll_size1, *scroll_size2;
extern struct object_t *sprite_size1, *sprite_size2;

extern struct object_t *background_object;
extern struct object_t *select_object, *put_object, *fill_object, *line_object, *pic_object;
extern struct object_t *pic_pat_object, *put_pat_object, *flip_object;
extern struct object_t *clear_to_color_object, *pal_hi_low_object, *select_object;

extern struct object_t *plus_object, *minus_object, *a_object, *b_object;
extern struct object_t *select_a_object, *select_b_object;

extern int sprite_overlay_pic;
extern int a_pattern, b_pattern;

int load_ovr(char *filename);
