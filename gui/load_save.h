#define BIGBUF		2048
#define MEDBUF		256
#define SMALLBUF 	64

#define LOAD_OK_QUIT	1
#define NOPE_TRY_AGAIN	2

#define LOAD	1
#define SAVE	2
#define CLICKED	1
#define NO_TEXT	2


struct select_file_t {
 group_t *grp;
 int type;
 int usr_flags;
 char file_type_name[SMALLBUF]; 
 gui_timer_t *blinky;
 int (*load_proc)(struct select_file_t *selector, char *filename);
 char path[BIGBUF];
 char name[BIGBUF];
 char return_buf[BIGBUF];
 char text_lines[BIGBUF][MEDBUF];
 struct object_t **lines;
 int offset;
 int selected_line, nlines;
 int end;
 struct object_t *scroll_bar;
 struct object_t *name_object;
};
void read_dir(struct select_file_t *selector); 
int line_edit(int msg, struct object_t *obj, int data);
void do_overlay_window(struct select_file_t *selector);
struct select_file_t *setup_overlay_window(int w, int h, int type, char *file_type_name, 
  int (*load_proc)(struct select_file_t *selector, char *filename));
