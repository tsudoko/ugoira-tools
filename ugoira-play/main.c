/*
 * This work is subject to the CC0 1.0 Universal (CC0 1.0) Public Domain
 * Dedication license. Its contents can be found in the LICENSE file or at
 * <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_image.h>

#include "list.h"
#include "archive.h"

void handle_events(void)
{
    SDL_Event e;

    SDL_Window   *w;
    SDL_Renderer *r;

    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_WINDOWEVENT: {
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_EXPOSED: {
                        //SDL_Log("window %d expose", e.window.windowID);
                        w = SDL_GetWindowFromID(e.window.windowID);

                        if(!w) {
                            SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                                         "couldn't get window: %s",
                                         SDL_GetError());
                            break; // XXX
                        }

                        r = SDL_GetRenderer(w);

                        if(!r) {
                            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                                       "couldn't get renderer: %s",
                                         SDL_GetError());
                            break; // XXX
                        }

                        SDL_RenderClear(r);
                        SDL_RenderPresent(r);
                        break;
                    }
                }
                break;
            }
            case SDL_QUIT: {
                exit(0);
            }
        }
    }
}

Node* list_rwops_to_texture(Node *node, SDL_Renderer *r)
{
    assert(node->prev == NULL);

    for(;;) {
        SDL_RWops   *current_rwop;
        SDL_Texture *current_texture;
        SDL_Surface *current_surface;
    
        current_rwop = (SDL_RWops*)node->data;
    
        if(IMG_isJPG(current_rwop)) {
            SDL_Log("(%p) loaded image is a JPG", node);
        } else {
            SDL_Log("(%p) loaded image is not a JPG", node);
        }
    
        if(!current_rwop) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "couldn't create RWops: %s", SDL_GetError());
        }
    
        current_surface = IMG_Load_RW(current_rwop, 0);
    
        if(!current_surface) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "couldn't load image: %s", IMG_GetError());
        }
    
        current_texture = SDL_CreateTextureFromSurface(r, current_surface);
    
        if(!current_texture) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "couldn't create texture: %s", IMG_GetError());
        }
    
        SDL_FreeSurface(current_surface);

        node->data = (SDL_Texture*)current_texture;

        if(node->next == NULL) {
            break;
        }

        node = node->next;
    }

    return list_head(node);
}

int main(int argc, char **argv)
{
    SDL_Window   *w;
    SDL_Renderer *r;

    Node *current_node;
    char *filename;

    if(argc <= 1) {
        fprintf(stderr, "usage stub\n");
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

    w = SDL_CreateWindow(filename,
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         640,
                         420,
                         SDL_WINDOW_RESIZABLE);

    if(!w) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                     "window creation failed: %s", SDL_GetError());
        return 1;
    }

    r = SDL_CreateRenderer(w, -1, 0);

    if(!r) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                     "renderer creation failed: %s", SDL_GetError());
    }

    current_node = list_rwops_to_texture(current_node, r);

    time_t frame_time = time(NULL);
    assert(frame_time - 1 > 0);
    frame_time -= 1;

    for(;;) {
        handle_events();

        if(current_node->prev) {
            assert(current_node->prev->data != NULL);
        }

        assert(current_node->data != NULL);

        // TODO: move to handle_events()
        SDL_RenderClear(r);
        SDL_RenderCopy(r, (SDL_Texture*)current_node->data, NULL, NULL);
        SDL_RenderPresent(r);

        if(time(NULL) > frame_time) {
            SDL_Log("current node: %p", current_node);

            if(current_node->next != NULL) {
                current_node = current_node->next;
            } else {
                SDL_Log("head");
                current_node = list_head(current_node);
            }

            frame_time = time(NULL);
        }

        SDL_Delay(16);
    }
}
