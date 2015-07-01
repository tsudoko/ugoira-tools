/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <SDL.h>

typedef struct frame_t {
    char     filename[11]; // "file"  in ugokuIllustData
    uint16_t duration;     // "delay" in ugokuIllustData

    char   *image;
    size_t  image_size;

    SDL_Texture *texture;
} Frame;

Node* get_frame_with_filename(Node *node, const char *filename);
void get_frame_durations(Node *node, const char *filename);
