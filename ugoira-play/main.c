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

void generate_texture(Frame *frame, SDL_Renderer *r)
{
    SDL_RWops   *current_rwops;
    SDL_Texture *current_texture;

    current_rwops = SDL_RWFromMem(frame->image, frame->image_size);

    if(!current_rwops) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "couldn't create RWops: %s", SDL_GetError());
    }

    if(IMG_isJPG(current_rwops)) {
        SDL_Log("(%s) loaded image is a JPG", frame->filename);
    } else {
        SDL_Log("(%s) loaded image is not a JPG", frame->filename);
    }

    current_texture = IMG_LoadTexture_RW(r, current_rwops, true);

    if(!current_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "couldn't create texture: %s", IMG_GetError());
    }

    frame->texture = current_texture;
    frame->need_redraw = false;
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
        fprintf(stderr, "no frames\n");
        return 1;
    }

    char     *json_filename;
    uint64_t  json_filename_length;

    json_filename_length = strlen(filename) + 1; // foo.zip -> foo.json
    json_filename = (char *)malloc((json_filename_length + 1)* sizeof(char));

    if(!json_filename) {
        fprintf(stderr, "couldn't allocate memory for json_filename\n");
        return 1;
    }


    strcpy(json_filename, filename);

    json_filename[json_filename_length - 4] =  'j';
    json_filename[json_filename_length - 3] =  's';
    json_filename[json_filename_length - 2] =  'o';
    json_filename[json_filename_length - 1] =  'n';
    json_filename[json_filename_length]     = '\0';

    get_frame_durations(current_node, json_filename);
    free(json_filename);

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
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

    generate_texture((Frame*)current_node->data, r);

    int width, height;
    SDL_QueryTexture(((Frame*)current_node->data)->texture,
                     NULL, NULL, &width, &height);

    SDL_SetWindowSize(w, width, height);
    SDL_ShowWindow(w);

    uint32_t frame_time = SDL_GetTicks();
    //assert((int64_t)(frame_time - 1000) > 0);
    //frame_time -= 1000;

    SDL_Event e;

    Frame     *current_frame;
    Frame     *prev_frame;
    uint16_t   duration;

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
                                for(Node *i = list_head(current_node);
                                    i != NULL; i = i->next) {
                                    ((Frame *)i->data)->need_redraw = true;
                                }
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
                    assert(temp == NULL);

                    SDL_DestroyRenderer(r);
                    SDL_DestroyWindow(w);
                    exit(0);
                }
            }
        }

        current_frame = (Frame *)current_node->data;
        if(current_node->prev) {
            prev_frame = (Frame *)current_node->prev->data;
        } else {
            prev_frame = (Frame *)list_last(current_node)->data;
        }
        duration = (prev_frame->duration ? prev_frame->duration : 1000);

        if(((Frame *)current_node->data)->need_redraw) {
            generate_texture((Frame *)current_node->data, r);
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
