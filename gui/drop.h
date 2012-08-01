/*
-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

  http://pgp.mit.edu:11371/pks/lookup?op=get&search=0xA6E70B3
  m m mm mmm .----------.  .---------------------. mmm mm m m
  8 8 88 888 | .--------`  |  .------------------` 888 88 8 8
  8 8 88 888 | ```````|`V```````|   |``||``|`````| 888 88 8 8
  8 8 88 888 `------  | |  [] | |``````||  |  [] | 888 88 8 8
  8 8 88 888 |``````  | |     | ````|````  |     | 888 88 8 8
  ` ` `` ``` ``````````````>  |````````````|   |`` ``` `` ` `
                ==============`            `---`
                                 L A B O R A T O R I E S
   
    Good hello, this is official SigFLUP-Labs sourcecode :-()

 This is GNU software, please read ../LICENSE for details
*/
/* at 35 we start to overflow into negative numbers */
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
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj44ACgkQMNO4A6bnBrMQ/wCfa9K4DeyV3Q3wQSWjjygc1Duw
LzUAn3L4aFFFX3J7R6AePjcI/9F4qaBI
=TOXN
-----END PGP SIGNATURE-----
*/
