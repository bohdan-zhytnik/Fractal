#include"keyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"
#include "event_queue.h"
#include"computation.h"
#include "xwin_sdl.h"

bool quit=false;
void* keyboard_thread(void* d){
    fprintf(stderr, "keyboard_thread - start\n");
    call_termios(0); 
    int c;
    event ev;
    while ((c = getchar()) != 'q') {
        ev.type = EV_TYPE_NUM;
        switch (c){
            case 'g':   //get version
                ev.type = EV_GET_VERSION;
                debug("g pressed");
                break;
            case 'a':   // abort
                ev.type = EV_ABORT;

                struct timespec ts;
                ts.tv_sec = 0;
                ts.tv_nsec = 50 * 1000000; 
                event ev1;
                event ev2;
                ev1.type = EV_ABORT;
                ev2.type = EV_ABORT;
                queue_push(ev1);
                nanosleep(&ts, NULL);
                queue_push(ev2);
                nanosleep(&ts, NULL);
                
                break;
            case 's':   //set compute
                ev.type = EV_SET_COMPUTE;
                break;
            case 'c':   // enable_comp();
                if (!is_computing() || is_was_abort()){
                    enable_comp();
                    ev.type = EV_COMPUTE;
                }else if (!is_done()){ 
                    info("computing");
                }else{ info("is done !");}
                //compute
                break;
            case 'b':
                if (!is_computing() || is_was_abort()){ 
                    ev.type = EV_COMP_BURST;
                    enable_comp();
                    debug("start BURST");
                }else{ warn("abort computing first");}
                break;
            case 'p': //animated picture 
                set_animate();
                break;
            default:
                break;
        }
        if(ev.type != EV_TYPE_NUM){
            queue_push(ev);
        }
    }//end while
    set_quit();
    ev.type = EV_QUIT;
    queue_push(ev);
    call_termios(1); // restore terminal settings
    fprintf(stderr, "keyboard_thread - finished\n");
    return NULL;
}

