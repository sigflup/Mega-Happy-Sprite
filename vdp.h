#define	NTSC	0
#define PAL	1

#define SPRITE_WIDTH	320
#define SPRITE_HEIGHT	240

#define FLIP_X(q)	((q&8)>>3)
#define FLIP_Y(q)	((q&16)>>4)

#define VIS_BACKGROUND		0
#define VIS_SCROLL_B		1
#define VIS_SCROLL_A		2
#define VIS_SPRITE		4
#define VIS_SCROLL_B_HIGH	8
#define VIS_SCROLL_A_HIGH	16
#define VIS_SPRITE_HIGH		32

#define UPDATE_BG_COLOR\
 bg_color_box->param.bg=&current_vdp->palette[current_vdp->bg_pal][current_vdp->bg_index];

typedef struct {
 int bg_pal, bg_index;
 int tv_type;
 int cell_w;
 int scroll_a;
 int scroll_b;
 int window;
 int sprite_table;
 int hscroll;
 Uint8 *vram;
 Uint8 *vsram;
 color_t palette[4][16];
} vdp_t;

typedef struct {
 int pat;
 int ix, iy;
 int sx, sy;
 int pal, index;
 int priority;
 int pat_index;
} scroll_data_t;

typedef struct {
 int num;
 int px[64], py[64];
} pixel_dump_t;

extern pixel_dump_t pat_pix_a[4096];
extern pixel_dump_t pat_pix_b[4096];

typedef struct {
 int index;
 Uint8 pixels[64];
} pat_bin_t;

extern pat_bin_t pat_bin[0x500];
extern int pat_bin_num; 

typedef struct {
 int palette, index;
 int pattern;
 color_t color; 
 int pre_index, pre_palette;
 int pre_parent;
 color_t ovr_color;
 int top, pic_top;
 int pat_index;

 scroll_data_t A,B;

} vdp_pixel;


extern int sprite_width, sprite_height;
extern int scroll_width, scroll_height;

extern int pat_ref[0x400];
extern int pat_stop[0x400];

extern int visible;

extern vdp_pixel vdp_screen[320*240];
extern vdp_pixel vdp_backbuf[320*240];

extern vdp_pixel sprite_screen[32*32];
extern vdp_pixel sprite_backbuf[32*32];

extern vdp_t *current_vdp;
extern Uint8 *undo_vram[MAX_UNDO];

extern int current_palette;
extern int current_palette_index;
extern int current_pattern;

extern int vdp_zoom, vdp_x, vdp_y;
extern int ovr_zoom, ovr_x, ovr_y;

int priority_pixels(int *flags, int a_pri,int a_data,
                                int b_pri,int b_data,
			        int g_data);

void collapse(void);
void render_sprite(void); 
void render_vdp(int start, int end);
void copy_vdp(vdp_t *dst, vdp_t *src);
void draw_pattern(Uint8 *ram, int num, int x, int y, int w, int h);
void store_palette(vdp_t *vdp, Uint8 *ram);
void load_palette(vdp_t *vdp, Uint8 *ram);
int vdp_init(void);
