typedef struct node_t {
    struct node_t *prev;
    void *data;
    struct node_t *next;
} Node;

Node* list_create(void *data);
Node* list_insert_after(Node *node, void *data);
void list_print(Node *node);
