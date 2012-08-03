struct list_head {
 struct list_head *next, *prev;
};

#define INIT_LIST_HEAD(ptr) do { \
 (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)


void list_add(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);
