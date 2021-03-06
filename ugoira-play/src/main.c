/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "list.h"
#include "archive.h"
#include "frame.h"

#define NAMELEN 1024

void
generate_texture(Frame *frame, SDL_Renderer *r)
{
    SDL_RWops   *current_rwops;
    SDL_Texture *current_texture;

    current_rwops = SDL_RWFromMem(frame->image, frame->image_size);

    if(!current_rwops) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "couldn't create RWops: %s", SDL_GetError());
        return;
    }

    if(frame->texture != NULL) {
        SDL_DestroyTexture(frame->texture);
    }

    current_texture = IMG_LoadTexture_RW(r, current_rwops, true);

    if(!current_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "couldn't create texture: %s", IMG_GetError());
        return;
    }

    frame->texture = current_texture;
    frame->need_redraw = false;
}

void
render_frame(Node *node, SDL_Renderer *r)
{
    SDL_RenderClear(r);
    SDL_RenderCopy(r, ((Frame*)node->data)->texture, NULL, NULL);
    SDL_RenderPresent(r);
}

void
switch_filtering_mode(void)
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

int
main(int argc, char **argv)
{
    SDL_Window   *w;
    SDL_Renderer *r;

    Node *current_node;
    char *filename;
    bool  paused = false;

    char   json_filename[NAMELEN];
    size_t json_filename_len;

    if(argc <= 1) {
        fprintf(stderr, "usage: %s file.zip\n", argv[0]);
        return 1;
    }

    filename = argv[1];
    current_node = read_whole_archive(filename);

    if(!current_node) {
        fprintf(stderr, "no frames\n");
        return 1;
    }

    json_filename_len = strlen(filename) + 1;
    assert(json_filename_len < NAMELEN);

    strncpy(json_filename, filename, NAMELEN);
    strncpy(&json_filename[json_filename_len - 5], ".json", 5);

    get_frame_durations(current_node, json_filename);

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "sdl initialization failed: %s\n", SDL_GetError());

        return 1;
    }

    if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "couldn't enable linear filtering");
    }

    if(SDL_CreateWindowAndRenderer(640, 420,
       SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE, &w, &r)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "window/renderer creation failed: %s", SDL_GetError());
        return 1;
    }

    generate_texture((Frame*)current_node->data, r);

    int width, height;
    SDL_QueryTexture(((Frame*)current_node->data)->texture,
                     NULL, NULL, &width, &height);

    SDL_SetWindowTitle(w, filename);
    SDL_SetWindowSize(w, width, height);
    SDL_ShowWindow(w);

    uint32_t frame_time = SDL_GetTicks();

    SDL_Event e;

    Node *redraw_node = current_node;

    Frame     *current_frame;
    Frame     *prev_frame;
    uint16_t   duration;

    for(;;) {
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_WINDOWEVENT:
                if(e.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    render_frame(current_node, r);
                }
                break;
            case SDL_KEYUP:
                if(e.key.keysym.mod != KMOD_NONE) {
                        break;
                }

                switch(e.key.keysym.sym) {
                case SDLK_q: {
                    SDL_Event quit_event;
                    quit_event.type = SDL_QUIT;

                    SDL_PushEvent(&quit_event);
                    break;
                }
                case SDLK_a:
                    switch_filtering_mode();
                    redraw_node = current_node;
                    for(Node *i = list_head(current_node);
                        i != NULL; i = i->next) {
                        ((Frame *)i->data)->need_redraw = true;
                    }
                    break;
                case SDLK_SPACE:
                    paused = !paused;
                    break;
                }
                break;
            case SDL_QUIT:
                SDL_Log("got quit event");

                Node *temp;

                current_node = list_head(current_node);
                assert(current_node != NULL);

                do {
                    Frame *frame = (Frame*)current_node->data;

                    frame_destroy(frame);

                    temp = current_node->next;

                    free(current_node);
                    current_node = NULL;

                    current_node = temp;
                } while(current_node != NULL);
                assert(temp == NULL);

                SDL_DestroyRenderer(r);
                SDL_DestroyWindow(w);
                exit(0);
            }
        }

        current_frame = (Frame *)current_node->data;
        if(current_node->prev) {
            prev_frame = (Frame *)current_node->prev->data;
        } else {
            prev_frame = (Frame *)list_last(current_node)->data;
        }
        duration = (prev_frame->duration ? prev_frame->duration : 1000);

        if(((Frame *)redraw_node->data)->need_redraw) {
            SDL_Log("generating %s", ((Frame *)redraw_node->data)->filename);
            generate_texture((Frame *)redraw_node->data, r);
        }

        if(redraw_node->next) {
            redraw_node = redraw_node->next;
        } else {
            redraw_node = list_head(redraw_node);
        }

        // FIXME: pausing breaks timing
        if(!paused && (SDL_GetTicks() / duration > frame_time / duration)) {
            if(current_node->prev) {
                assert(current_node->prev->data != NULL);
            }

            assert(current_node->data != NULL);

            SDL_Log("current node: %p (%s, %d)", current_node,
                    current_frame->filename, current_frame->duration);
            render_frame(current_node, r);

            if(current_node->next != NULL) {
                current_node = current_node->next;
            } else {
                SDL_Log("head");
                current_node = list_head(current_node);
            }

            frame_time = SDL_GetTicks();
        }

        SDL_Delay(16);
    }
}
