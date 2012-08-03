#define MAX_DEPTH	80	

typedef struct {
 struct list_head node;
 int w,h;
 int depth;
 Uint8 *buffer;
} drop_t;

extern drop_t *drops;

void build_coef(void);
void draw_drop(SDL_Surface *dst, int x, int y, drop_t *drop, int w, int h, SDL_Rect *clip);
drop_t *new_drop(int depth);
void drop_init(void);
