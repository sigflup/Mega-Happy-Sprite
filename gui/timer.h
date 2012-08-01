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
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj5UACgkQMNO4A6bnBrPnsQCfco0mmfqEsV9yFwmjquwYQh0F
k2UAoIRyd+THRWNWYxlxeR7q88wov/Oi
=vU8s
-----END PGP SIGNATURE-----
*/
