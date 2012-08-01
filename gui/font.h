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
#define CENTER_OF_STRING(a)	(((strlen(a)*8)/2))

#define CR_TERMINAL	8

extern unsigned char font[];

void draw_char(int x, int y, char c, int fg_color, int bg_color, int flags);
void draw_text(int x, int y, char *in, color_t *fg, color_t *bg, int hash, int max);
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj48ACgkQMNO4A6bnBrMYyACeI7/lfob5BeagH5+13NHLTbTG
HFwAoIkYod7tDdyj7dCP4DoyCVw2XTJF
=qUfH
-----END PGP SIGNATURE-----
*/
