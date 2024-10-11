/*
 * Mega Happy Sprite is released under the BSD 3-Clause license.
 * read ../LICENSE for more info
 */

struct menu_entry_t {
 char *name;
 int flags;
 int user_flags;
 int (*callback)(struct object_t *obj, int data);
};

group_t *new_menu(int x, int y, struct menu_entry_t *root, color_t *fg, color_t *bg);
