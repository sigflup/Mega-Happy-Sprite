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

struct menu_entry_t {
 char *name;
 int flags;
 int user_flags;
 int (*callback)(struct object_t *obj, int data);
};

group_t *new_menu(int x, int y, struct menu_entry_t *root, color_t *fg, color_t *bg);
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj5MACgkQMNO4A6bnBrMbfwCeOzMVyiwZuYYY/5/KR7Oz96/M
ksoAoInr+Fmla3b0h/TbMAqKQVRTDxSv
=xCnm
-----END PGP SIGNATURE-----
*/
