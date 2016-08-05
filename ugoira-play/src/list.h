/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

typedef struct node_t {
    struct node_t *prev;
    void *data;
    struct node_t *next;
} Node;

Node *list_create(void *data);
Node *list_insert_after(Node *node, void *data);
Node *list_head(Node *node);
Node *list_last(Node *node);
void list_print(Node *node);
