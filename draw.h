#define MAX_FLOOD	100000

struct seg {
 short y,xl,xr,dy;
};

extern Uint8 draw_buffer[320][240];

void put_pix_draw_buffer(Uint8 *ram, int pat, int x, int y);
int get_pix(Uint8 *ram, int pat, int x, int y);
int get_pix_special(Uint8 *ram, int pat, int x, int y);
int get_pix_preview_special(Uint8 *ram, int pat, int x, int y);
void put_pix(Uint8 *ram, int pat, int x, int y);
void do_flood(Uint8 *ram, int pat, int x, int y, int wx1, int wy1, int wx2, int wy2, 
  void (*put)(Uint8 *,int,int,int), int (*get)(Uint8 *,int,int,int));
void do_line(Uint8 *ram, int pat, int x1, int y1, int x2, int y2, void (*proc)(Uint8 *,int,int,int));
