#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

Node* list_create(void *data)
{
    Node *head = NULL;
    head = malloc(sizeof(Node));
    head->data = data;

    return head;
}

Node* list_insert_after(Node* node, void *data)
{
    Node *new_node = list_create(data);

    node->next = new_node;
    new_node->prev = node;

    return new_node;
}

void list_print(Node* node)
{
    Node *i = NULL;

    assert(node->prev == NULL); // FIXME
    printf("[");

    for(i = node;;i = i->next) {
        printf("%p", i->data);
        if(i->next != NULL) {
            printf(", ");
        } else {
            printf("]\n");
            break;
        }
    }
}
