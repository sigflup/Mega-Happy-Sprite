struct menu_entry_t {
 char *name;
 int flags;
 int user_flags;
 int (*callback)(struct object_t *obj, int data);
};

group_t *new_menu(int x, int y, struct menu_entry_t *root, color_t *fg, color_t *bg);
