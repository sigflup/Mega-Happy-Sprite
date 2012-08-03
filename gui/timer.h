#define ACTIVE_ONLY_WITH_PARENT	1
#define STOPPED			2

typedef struct {
 struct list_head node;
 int flags;
 struct object_t *obj;
 group_t *parent_grp;
 int timer; 
 int reset;
 int msg, data;
} gui_timer_t;

extern gui_timer_t *globl_timer;
extern Uint32 timer_callback(Uint32 interval);

void init_timers(void);
gui_timer_t *add_timer(struct object_t *obj, int reset, int msg, int data, group_t *parent,int flags);

void del_timer(gui_timer_t *in);
