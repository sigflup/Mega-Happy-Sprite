#define CENTER_OF_STRING(a)	(((strlen(a)*8)/2))

#define CR_TERMINAL	8

extern unsigned char font[];

void draw_char(int x, int y, char c, int fg_color, int bg_color, int flags);
void draw_text(int x, int y, char *in, color_t *fg, color_t *bg, int hash, int max);
