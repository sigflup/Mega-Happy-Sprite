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


void do_overlay_window(struct select_file_t *selector);
struct select_file_t *setup_overlay_window(int w, int h, int type, char *file_type_name, 
  int (*load_proc)(struct select_file_t *selector, char *filename));
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj5MACgkQMNO4A6bnBrPN/QCePYzuvB5q/nAqZhHsasOVrTr6
0m4An1d2Kqa3ITJD4Ld48y2Z2HNk3dRJ
=N2aR
-----END PGP SIGNATURE-----
*/
