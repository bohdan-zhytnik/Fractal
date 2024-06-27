#include <SDL.h>
#include<stdio.h>
#include <time.h>

#include "gui.h"
#include "xwin_sdl.h"
#include"utils.h"
#include "computation.h"
#include "event_queue.h"

#ifndef SDL_EVENT_POLL_WAIT_MS
#define SDL_EVENT_POLL_WAIT_MS 10
#endif

static struct{
    int w;
    int h;
    unsigned char *img;
}gui = { .img = NULL};

void gui_init(void){
    get_grind_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__,__LINE__, __FILE__);
    printf("Grid is inicialized\n");
}
void gui_cleanup(void){
    if (gui.img){
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}

void gui_refresh(void){
    if (gui.img){
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}

void* gui_win_thread(void* d){
    info("gui_win_thread - start");
    bool quit = false;
    SDL_Event event;
    struct event ev;
    while (!quit){
        ev.type = EV_TYPE_NUM;
        if (SDL_PollEvent(&event)){
            if (event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                    case SDLK_q: //quit
                        ev.type = EV_QUIT;
                        queue_push(ev);
                        info("quit");
                        break;
                    case SDLK_s: //set compute
                        ev.type = EV_SET_COMPUTE;
                        break;
                    case SDLK_a: // abort
                        ev.type = EV_ABORT;

                        struct timespec ts;
                        ts.tv_sec = 0;
                        ts.tv_nsec = 50 * 1000000; 
                        struct event ev1;
                        struct event ev2;
                        ev1.type = EV_ABORT;
                        ev2.type = EV_ABORT;
                        queue_push(ev1);
                        nanosleep(&ts, NULL);
                        queue_push(ev2);
                        nanosleep(&ts, NULL);

                        break;
                    case SDLK_g: //get version
                        ev.type = EV_GET_VERSION;
                        debug("g pressed");
                        break;
                    case SDLK_c: // enable_comp();
                        if (!is_computing() || is_was_abort()){
                            enable_comp();
                            ev.type = EV_COMPUTE;
                        }else if (!is_done()){ 
                            info("computing");
                        }else{ info("is done !");}
                        break;
                    case SDLK_b: // start BURST
                        if (!is_computing() || is_was_abort()){ 
                            ev.type = EV_COMP_BURST;
                            enable_comp();
                            debug("start BURST");
                        }else{ warn("abort computing first");}
                        break;
                    case SDLK_p: //animated picture 
                        set_animate();
                        break;
                }
            }
            // }else if (event.type == SDL_MOUSEMOTION ){
            // }
            //     info("mousemotion");
        }// end SDL_PollEvent
        if (ev.type != EV_TYPE_NUM){    
            queue_push(ev);
        }
        SDL_Delay(SDL_EVENT_POLL_WAIT_MS);
        // due to the use of SDL, in Valgrind I get a message after the program ends: 272 bytes of memory are still available
        quit = is_quit();
    }
    info("gui_win_thread - end");
    return NULL;
}