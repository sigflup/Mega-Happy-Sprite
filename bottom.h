/*
 * Mega Happy Sprite is released under the BSD 3-Clause license.
 * read LICENSE for more info
 */

#define VIEW_SCROLL_A	0
#define VIEW_SCROLL_B	1
#define VIEW_WINDOW	2
#define VIEW_SPRITE	4
#define VIEW_SCROLL_B_HIGH	10
#define VIEW_SCROLL_A_HIGH	11
#define VIEW_SPRITE_HIGH	12

#define EDIT_SCROLL_A	5
#define EDIT_SCROLL_B	6
#define EDIT_WINDOW	7
#define EDIT_SPRITE	9

#define KNOB		2	
#define SELECT_A	0	
#define SELECT_B	1	

#define NOT_SELECTED	0
#define HIGH		1
#define LOW		2
#define PALETTE		3
#define SELECTED	4
#define COPY		5
#define MOVE		6

extern struct object_t *last_tool;
extern int high_low;
extern int hor_ver;
extern int copy_move;

extern struct select_file_t *load_sel, *save_sel;
extern int currently_editing;

extern int num_xor_pix;
extern int knob_data_offset;
extern int knob_or;
extern int knob_ab;

typedef struct {
 int x,y;
 int index;
} xor_pix;

extern xor_pix xor_pixels[320*240];

int go_bot(struct object_t *obj, int data);
int new_bot(struct object_t *obj, int data);
int stop_bot(struct object_t *obj, int data);
void put_xor_pix(Uint8 *ram, int pat, int x, int y);
void put_xor_hash_pix(Uint8 *ram, int pat, int x, int y);
int high_low_menu_bot(struct object_t *obj, int data);
int pal_hi_low_menu_bot(struct object_t *obj, int data);
int hor_ver_menu_bot(struct object_t *obj, int data);
int copy_move_menu_bot(struct object_t *obj, int data);
int change_bg_color(struct object_t *obj, int data);
int knob_tick(struct object_t *obj, int data);
int knob_select(struct object_t *obj, int data);
int edit_change(struct object_t *obj, int data);
int tool_change(struct object_t *obj, int data);
int change_background(struct object_t *obj, int data);
int load_background_callback(struct select_file_t *selector, char *filename);
int load_ovr_callback(struct select_file_t *selector, char *filename);
int load_ovr_window(struct object_t *obj, int data);
int undo_button(struct object_t *obj, int data);
int about(struct object_t *obj, int data);
int really_quit(struct object_t *obj, int data);
int fw_bottom(struct object_t *obj, int data);
int load_save_top(struct object_t *obj, int data);
int load_save_middle(struct object_t *obj, int data);
int load_save_bottom(struct select_file_t *selector, char *filename);
int sprite_overlay(struct object_t *obj, int data);

