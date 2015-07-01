/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

Node* list_create(void *data)
{
    Node *head = malloc(sizeof(Node));
    head->prev = NULL;
    head->next = NULL;

    head->data = data;

    return head;
}

Node* list_insert_after(Node *node, void *data)
{
    Node *new_node = list_create(data);

    node->next = new_node;
    new_node->prev = node;

    return new_node;
}

Node* list_head(Node *node)
{
    if(node == NULL) {
        return node;
    }

    while(node->prev != NULL) {
        node = node->prev;
    }

    return node;
}

Node* list_last(Node *node)
{
    if(node == NULL) {
        return node;
    }

    while(node->next != NULL) {
        node = node->next;
    }

    return node;
}

void list_print(Node *node)
{
    Node *i = NULL;

    if(node == NULL) {
        printf("[]\n");
        return;
    }

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
