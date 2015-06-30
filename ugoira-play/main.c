/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

#include "list.h"
#include "archive.h"
#include "frame.h"

#include "main.h"

Node* generate_textures(Node *node, SDL_Renderer *r)
{
    assert(node->prev == NULL);

    SDL_RWops   *current_rwop;
    SDL_Texture *current_texture;

    Frame *frame;

    for(;;) {
        frame = (Frame*)node->data;

        current_rwop = SDL_RWFromMem(frame->image, frame->image_size);

        if(!current_rwop) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "couldn't create RWops: %s", SDL_GetError());
        }

        if(IMG_isJPG(current_rwop)) {
            SDL_Log("(%p) loaded image is a JPG", node);
        } else {
            SDL_Log("(%p) loaded image is not a JPG", node);
        }

        current_texture = IMG_LoadTexture_RW(r, current_rwop, true);

        if(!current_texture) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "couldn't create texture: %s", IMG_GetError());
        }

        frame->texture = current_texture;

        if(node->next == NULL) {
            break;
        }

        node = node->next;
    }

    return list_head(node);
}

uint64_t get_time_ms(void)
{
    uint64_t        ms;
    time_t          s;
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);
    s = spec.tv_sec;
    ms = (uint64_t)(s * 1000) + (uint64_t)(spec.tv_nsec / 1.0e6);

    return ms;
}

void render_frame(Node *node, SDL_Renderer *r)
{
    SDL_RenderClear(r);
    SDL_RenderCopy(r, ((Frame*)node->data)->texture, NULL, NULL);
    SDL_RenderPresent(r);

}

void switch_filtering_mode(void)
{
    SDL_bool ret;

    if(strcmp(SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY), "1") == 0) {
        ret = SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    } else {
        ret = SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    }

    if(!ret) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "couldn't switch filtering mode");
    }
}

int main(int argc, char **argv)
{
    SDL_Window   *w;
    SDL_Renderer *r;

    Node *current_node;
    char *filename;
    bool  paused = false;

    if(argc <= 1) {
        fprintf(stderr, "usage: %s file.zip\n", basename(argv[0]));
        return 1;
    }

    filename = argv[1];
    current_node = read_whole_archive(filename);

    if(!current_node) {
        fprintf(stderr, "current_node is NULL\n");

        return 1;
    }

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "sdl initialization failed: %s\n", SDL_GetError());

        return 1;
    }

    if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "couldn't enable linear filtering");
    }

    w = SDL_CreateWindow(filename,
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         640,
                         420,
                         SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE);

    if(!w) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "window creation failed: %s", SDL_GetError());
        return 1;
    }

    r = SDL_CreateRenderer(w, -1, 0);

    if(!r) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "renderer creation failed: %s", SDL_GetError());
    }

    current_node = generate_textures(current_node, r);

    int width, height;
    SDL_QueryTexture(((Frame*)current_node->data)->texture,
                     NULL, NULL, &width, &height);

    SDL_SetWindowSize(w, width, height);
    SDL_ShowWindow(w);

    uint64_t frame_time = get_time_ms();
    //assert((int64_t)(frame_time - 1000) > 0);
    //frame_time -= 1000;

    SDL_Event e;

    for(;;) {
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_WINDOWEVENT: {
                    switch(e.window.event) {
                        case SDL_WINDOWEVENT_EXPOSED: {
                            //SDL_Log("window %d expose", e.window.windowID);
                            render_frame(current_node, r);
                            break;
                        }
                    }
                    break;
                }

                case SDL_KEYUP: {
                    if(e.key.keysym.mod == KMOD_NONE) {
                        switch(e.key.keysym.sym) {
                            case SDLK_q: {
                                SDL_Event quit_event;
                                quit_event.type = SDL_QUIT;

                                SDL_PushEvent(&quit_event);
                                break;
                            }

                            case SDLK_a: {
                                switch_filtering_mode();
                                generate_textures(list_head(current_node), r);
                                break;
                            }

                            case SDLK_SPACE: {
                                if(paused) {
                                    paused = false;
                                } else {
                                    paused = true;
                                }
                                break;
                            }
                        }
                    }
                    break;
                }

                case SDL_QUIT: {
                    SDL_Log("got quit event");

                    Node *temp;

                    current_node = list_head(current_node);
                    do {
                        Frame *frame = (Frame*)current_node->data;

                        free(frame->image);
                        frame->image = NULL;

                        SDL_DestroyTexture(frame->texture);
                        frame->texture = NULL;

                        free(frame);
                        frame = NULL;

                        temp = current_node->next;

                        free(current_node);
                        current_node = NULL;

                        current_node = temp;
                    } while(current_node != NULL);

                    SDL_DestroyRenderer(r);
                    SDL_DestroyWindow(w);
                    exit(0);
                }
            }
        }

        // FIXME: pausing breaks timing
        if(!paused && (get_time_ms() / 1000 > frame_time / 1000)) {
            if(current_node->prev) {
            assert(current_node->prev->data != NULL);
            }

            assert(current_node->data != NULL);

            SDL_Log("current node: %p (%s)",
                    current_node, ((Frame*)current_node->data)->filename);
            render_frame(current_node, r);

            if(current_node->next != NULL) {
                current_node = current_node->next;
            } else {
                SDL_Log("head");
                current_node = list_head(current_node);
            }

            frame_time = get_time_ms();
        }

        SDL_Delay(16);
    }
}
