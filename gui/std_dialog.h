/* flags */
#define SHOW_FOCUS		1
#define QUIT_BUTTON		2
#define CALL_BUTTON		4
#define INTERNAL1		8	
#define INVALID			16
#define INTERNAL2		32
#define TOGGLE			64
#define DROP_SHADOW		128
#define DROP_SHADOW_READY	256
#define DROP_ACCUM		512
#define CLICKED_DOWN		1024
#define LOAD_XPM_FROM_ARRAY	2048
#define MAX_CHARS		4096
#define SINGLE_RADIO		8192
#define INACTIVE		16384
#define MODULAR			32768
#define HEX			65536
#define BLUE			131072

#define MAX_PROMPT_LINES	30

#define	MOVE_GROUP	3	


void simple_window(group_t *grp, int w, int h);
int prompt(int cent_x, int cent_y, char *message, char *yes_text, char *no_text);
void alert(int cent_x, int cent_y, char *message, char *ok_text);
void scroll_text_window(int cent_x, int cent_y, int w, int h, char *text, int len); 
int proc_bitmap(int msg, struct object_t *obj, int data);
int proc_scroll_bar(int msg, struct object_t *obj, int data);
int proc_icon_button(int msg, struct object_t *obj, int data);
int proc_radio_button(int msg, struct object_t *obj, int data);
int proc_move_button(int msg, struct object_t *obj, int data);
int proc_text(int msg, struct object_t *obj, int data);
int proc_ctext(int msg, struct object_t *obj, int data);
int proc_box(int msg, struct object_t *obj, int data);
int proc_shadow_box(int msg, struct object_t *obj, int data);
int proc_button_box(int msg, struct object_t *obj, int data);
int proc_hash_box(int msg, struct object_t *obj, int data);
int proc_knob(int msg, struct object_t *obj, int data);
int proc_edit_line(int msg, struct object_t *obj, int data);

