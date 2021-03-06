/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

typedef struct frame_t {
    char     filename[11]; // "file"  in ugokuIllustData
    bool     need_redraw;
    uint16_t duration;     // "delay" in ugokuIllustData

    char   *image;
    size_t  image_size;

    SDL_Texture *texture;
} Frame;

Frame *frame_create(void);
void frame_destroy(Frame *frame);
Node *get_frame_with_filename(Node *node, const char *filename);
void get_frame_durations(Node *node, const char *filename);
