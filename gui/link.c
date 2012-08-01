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
#include "../config.h"
#include "link.h"

void __list_add(struct list_head *new,
                struct list_head *prev,
		struct list_head *next) {
 next->prev = new;
 new->next = next;
 new->prev = prev;
 prev->next = new;
}

void list_add(struct list_head *new, struct list_head *head) {
 __list_add(new,head,head->next);
}

void __list_del(struct list_head *prev, struct list_head *next) {
 next->prev = prev;
 prev->next = next;
}

void list_del(struct list_head *entry) {
 __list_del(entry->prev, entry->next);
}
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.9 (OpenBSD)

iEYEARECAAYFAkogj4sACgkQMNO4A6bnBrOzLgCfdCfK1H7Yeq8UgLzE5Setw0mz
MV4AnRXU9OknLYJymPEUwBviuIwReiJJ
=4vCn
-----END PGP SIGNATURE-----
*/
