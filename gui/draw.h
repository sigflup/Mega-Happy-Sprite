#define NO_HASH		1	
#define HASH		2
/* ok so the above two are constants for non-text draw stuff */
#define XOR		3

#define LOCK	if(SDL_MUSTLOCK(gui_screen) != 0) SDL_LockSurface(gui_screen)
#define UNLOCK	if(SDL_MUSTLOCK(gui_screen) != 0) SDL_LockSurface(gui_screen)

#define UPDATE_OBJECT(Q) clipped_update(current_grp->pos_x+Q->param.x, \
                         current_grp->pos_y + Q->param.y, \
                         current_grp->pos_x + Q->param.x + Q->param.w, \
                         current_grp->pos_y + Q->param.y + Q->param.h)

extern SDL_Surface *gc;

void clipped_update(int x, int y, int w, int h);
int vline(int rx, int ry, int ry2, color_t *fg, color_t *bg, int type);
int hline(int rx, int ry, int rx2, color_t *fg, color_t *bg, int type);
int other_hline(int rx, int ry, int rx2, color_t *fg, color_t *bg, int type);
int fill_box(int x, int y, int x2, int y2, color_t *fg, color_t *bg, int type);
int box(int x, int y, int x2, int y2, color_t *fg, color_t *bg, int type);

void clear_screen(void);
