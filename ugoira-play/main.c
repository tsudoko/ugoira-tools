#include "main.h"

void handle_events(void)
{
    SDL_Event e;

    SDL_Window   *w;
    SDL_Renderer *r;

    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_WINDOWEVENT: {
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        SDL_Log("window %d resize %d×%d", e.window.windowID,
                                e.window.data1, e.window.data2);

                        w = SDL_GetWindowFromID(e.window.windowID);

                        if(!w) {
                            SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                                         "couldn't get window for resizing: %s",
                                         SDL_GetError());
                            break; // XXX
                        }

                        r = SDL_GetRenderer(w);

                        if(!r) {
                            SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                                       "couldn't get renderer for resizing: %s",
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
                break;
            }
        }
    }
}

int main(int argc, char **argv)
{
    SDL_Window   *w;
    SDL_Renderer *r;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "sdl initialization failed: %s\n", SDL_GetError());

        return 1;
    }

    // shut up clang
    if(argc != 1) {
        SDL_Log("%d args", argc);
    } else {
        SDL_Log("%d arg", 1);
    }
    SDL_Log("--------");
    for(int i = 0; i < argc; i++) {
        SDL_Log("%s", argv[i]);
    }

    w = SDL_CreateWindow("ayy",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         240,
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

    SDL_SetRenderDrawColor(r, 249, 38, 114, 255);
    SDL_RenderClear(r);
    SDL_RenderPresent(r);

    for(;;) {
        handle_events();
        SDL_Delay(16);
    }
}
