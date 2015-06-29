#include <SDL.h>

typedef struct frame_t {
    char     filename[7]; // "file"  in ugokuIllustData
    uint16_t duration;    // "delay" in ugokuIllustData

    char   *image;
    size_t  image_size;

    SDL_Texture *texture;
} Frame;
