
#define PARM(X,Y,W,H,FG,BG,FLAGS,PROC)	tmp_parm.x = X;\
				        tmp_parm.y = Y; \
					tmp_parm.w = W; \
					tmp_parm.h = H; \
					tmp_parm.fg = FG; \
					tmp_parm.bg = BG; \
					tmp_parm.flags = FLAGS; \
					tmp_parm.proc = PROC;

#define MAX_GAUSS_DEPTH	70

#define	MOUSE_MOVE	0
#define MOUSE_UP	1
#define MOUSE_DOWN	2
#define KEY_DOWN	3
#define KEY_UP		4

#define MSG_DRAW	0
#define MSG_START	1
#define MSG_INFOCUS	2
#define MSG_OUTFOCUS	3
#define MSG_CLICK	4  // button down
#define MSG_UNCLICK	5  // button up
#define MSG_PRESS	6  // button actually pressed
#define MSG_RADIO	7 
#define MSG_CLEAR_INTERNAL1	8
#define MSG_RADIO2	9
#define MSG_CLEAR_INTERNAL2	10
#define MSG_KEYDOWN	11
#define MSG_KEYUP	12
#define MSG_RELOAD	13
#define MSG_TICK	14
#define MSG_DESTROY	15
#define MSG_MOUSEMOVE	16

#define FULLSCREEN	1

#define RET_QUIT	0
#define RET_OK		1

#define MESSAGE_OBJECT(q,x) q->param.proc(x, q, 0)

#define MAP_COLOR(W) \
 W.map = SDL_MapRGB(gc->format,W.r, W.g, W.b);

#define SET_COLOR(W,R,G,B)\
{ \
 W.r = R;\
 W.g = G;\
 W.b = B;\
 MAP_COLOR(W); \
}

#ifdef WINDOWS
extern int gui_flags, gui_w, gui_h;
#endif

extern int globl_flags;
extern int lock_update;
extern int globl_dirt, globl_quit_value;
extern int globl_drop_depth;

extern int floating_key;
extern SDL_Surface *gui_screen;
extern int gui_mouse_x, gui_mouse_y;
extern void (*globl_tick)(void);
extern void (*globl_wait_tick)(void);

typedef struct {
 unsigned char r,g,b;
 Uint32 map; 
} color_t;

extern color_t globl_fg, globl_bg;
extern color_t globl_move_color;

struct object_t declair;

typedef struct {
 int (*proc)(int MSG, struct object_t *obj, int data);
 int x,y;
 int w,h;
 color_t *fg, *bg;
 int d1, d2, d3;
 int flags;
 void *dp1, *dp2, *dp3;
 int (*callback)(struct object_t *obj, int data);
 int quit_value;
 int pad2[128];
 int user_flags;
 int pad[128];
} obj_param_t;

struct object_t {
 struct list_head node;
 obj_param_t param;
 int in_focus, clicked, flags;
};

extern struct object_t *focus_obj;

typedef struct {
 int flags;
 int pos_x, pos_y;
 int w,h;
 int ready;
 struct object_t *objs;
 drop_t *drop_buffer;
// Uint8 *drop_buffer;
 int drop_w, drop_h, drop_d;

} group_t;

extern group_t *current_grp;

void default_tick(void);
void null_tick(void);
int wait_on_mouse(void);
struct object_t *new_obj(group_t *grp, obj_param_t *param);
group_t *new_group(int x,int y, int w, int h, int flags, int drop_d);
int destroy_group(group_t *grp);
int group_loop(group_t *grp);
int broadcast_group(group_t *grp, int msg, int data);
int init_gui(int x, int y, int flags);
