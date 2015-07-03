/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <assert.h>

#include <jansson.h>

#include "list.h"
#include "frame.h"

Frame* frame_create(void)
{
    Frame *frame = malloc(sizeof *frame);

    strcpy(frame->filename, "undefined");
    frame->need_redraw = true;
    frame->duration = 0;

    frame->image = NULL;
    frame->image_size = 0;

    frame->texture = NULL;

    return frame;
}

void frame_destroy(Frame *frame)
{
    if(frame->image) {
        free(frame->image);
        frame->image = NULL;
    }

    if(frame->texture) {
        SDL_DestroyTexture(frame->texture);
        frame->texture = NULL;
    }

    free(frame);
    frame = NULL;
}

Node* get_frame_with_filename(Node *node, const char *filename)
{
    node = list_head(node);

    //printf("get %s\n", filename);
    Frame *frame;

    while(node != NULL) {
        frame = (Frame *)node->data;

        //printf("%s\tcmp\t%s\n", frame->filename, filename);

        if(strcmp(frame->filename, filename) == 0) {
            return node;
        }

        node = node->next;
    }

    return NULL;
}

void get_frame_durations(Node *node, const char *filename)
{
    json_t       *root;
    json_error_t  error;

    json_t     *frame_array; // array of frame objects
    json_t     *frame_obj;   // frame object
    json_t     *file_key;
    json_t     *delay_key;
    const char *file_value;
    uint16_t    delay_value;

    Frame *frame;

    assert(node->prev == NULL);

    root = json_load_file(filename, 0, &error);

    if(!root) {
        fprintf(stderr, "decoding %s failed at line %d: %s\n",
                filename, error.line, error.text);
        json_decref(root);
        return;
    }

    if(!json_is_object(root)) {
        fprintf(stderr, "root JSON value is not an object\n");
        json_decref(root);
        return;
    }

#if 0
    const char *name_value;
    frame_array = NULL;

    if(json_unpack_ex(root, &error, 0, "{s:o}", &name_value, frame_array)) {
        fprintf(stderr, "unpacking %s failed at line %d: %s\n",
                filename, error.line, error.text);
        json_decref(root);
        return;
    }

    assert(strcmp(name_value, "frames") == 0);
    free(name_value);
    name_value = NULL;
#endif

    frame_array = json_object_get(root, "frames");

    if(!json_is_array(frame_array)) {
        fprintf(stderr, "the value of \"frames\" is not an array\n");
        json_decref(root);
        return;
    }

    for(size_t i = 0; i < json_array_size(frame_array); i++) {
        frame_obj = json_array_get(frame_array, i);
        if(!json_is_object(frame_obj)) {
            fprintf(stderr, "frame value is not an object\n");
            json_decref(root);
            return;
        }

        file_key = json_object_get(frame_obj, "file");
        if(!json_is_string(file_key)) {
            fprintf(stderr, "the value of \"file\" is not a string\n");
            json_decref(root);
            return;
        }

        delay_key = json_object_get(frame_obj, "delay");
        if(!json_is_integer(delay_key)) {
            fprintf(stderr, "the value of \"delay\" is not an integer\n");
            json_decref(root);
            return;
        }

        file_value  = json_string_value(file_key);
        delay_value = (uint16_t)json_integer_value(delay_key);

        node = get_frame_with_filename(node, file_value);

        if(node == NULL) {
            fprintf(stderr, "couldn't find frame '%s'\n", file_value);
            continue;
        }

        frame = (Frame *)node->data;
        frame->duration = delay_value;
    }

    json_decref(root);
}
