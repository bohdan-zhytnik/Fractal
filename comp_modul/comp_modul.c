#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include<unistd.h>
#include <string.h>

#include"utils.h"
#include "messages.h"
#include "event_queue.h"
#include "prg_io_nonblock.h"
#include"computation.h"

#ifndef IO_READ_TIMEOUT_MS
#define IO_READ_TIMEOUT_MS 100
#endif


void* M_read_pipe_thread(void* d);
void* M_write_pipe_thread(void* d);
bool write_to_pipe(message*, int);
bool write_to_pipe_BURST(message*, int);

int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;
    const char *fname_pipe_in = argc > 1 ? argv[1]: "/tmp/computational_module.in";
    const char *fname_pipe_out = argc > 2 ? argv[2]: "/tmp/computational_module.out";

    int pipe_in = io_open_read(fname_pipe_in);
    int pipe_out = io_open_write(fname_pipe_out);
    printf("start\n");

    my_assert(pipe_in != -1 && pipe_out != -1, __func__, __LINE__, __FILE__);
    enum {READ_PIPE_THRD, WRITE_PIPE_THRD, NUM_THREADS};
    const char *thrd_names[] = { "READ_PIPE", "WRITE_PIPE"};
    void* (*thrd_functions[])(void*) = {M_read_pipe_thread, M_write_pipe_thread};
    pthread_t threads[NUM_THREADS];
    void* thrd_data[NUM_THREADS] ={};
    thrd_data[READ_PIPE_THRD]=&pipe_in;
    thrd_data[WRITE_PIPE_THRD]=&pipe_out;
    for (int i = 0; i < NUM_THREADS; ++i) {
      int r = pthread_create(&threads[i], NULL, thrd_functions[i], thrd_data[i]);
      printf("Create thread '%s' %s\r\n", thrd_names[i], ( r == 0 ? "OK" : "FAIL") );
    }
    int *ex;
    for (int i = 0; i < NUM_THREADS; ++i) {
        printf("Call join to the thread %s\r\n", thrd_names[i]);
        int r = pthread_join(threads[i], (void*)&ex);
        printf("Joining the thread %s has been %s\r\n", thrd_names[i], (r == 0 ? "OK" : "FAIL"));
    }
    
    io_close(pipe_in);
    io_close(pipe_out);
    return ret;
}


void* M_read_pipe_thread(void* d){
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_in = *(int*)d;

    fprintf(stderr, "read_pipe_thread - start\n"); 
    bool end = false;
    uint8_t msg_buf[sizeof(message)]; 
    int i =0;
    int len = 0;
    unsigned char c;
    while (!end) {
        int r = io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c);
        if (r > 0){//char has been read
            if (i == 0 ){
                len = 0;
                if (get_message_size(c, &len)){
                    msg_buf[i++]=c;
                }else {
                    fprintf(stderr, "ERROR: unknown message type0x%x\n", c); 
                }
            }else { //read remaining bytes of the message
                msg_buf[i++] = c;
            }
            if (i == len && len >0){
                message *msg = my_alloc(sizeof(message));
                if (parse_message_buf(msg_buf, len , msg)){
                    event ev = {.type =  EV_PIPE_IN_MESSAGE};
                    ev.data.msg = msg;
                    queue_push(ev);
                }else {
                    fprintf(stderr, "ERROR: cannot parse massage type %d\n", msg_buf[0]);  
                    free(msg); 
                }
                i = len = 0;
            }
        }else if(r==0){//timeout
        }else {//error
            fprintf(stderr, "ERROR: reading from pipe\n");
            set_quit();
            event ev = {.type = EV_QUIT};
            queue_push(ev);
        } 
        end = is_quit();
    }//end while    
    fprintf(stderr, "read_pipe_thread - finished\n"); 
    return NULL;
}


void* M_write_pipe_thread(void* d){
    my_assert(d !=NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int*)d;
    while(!is_quit()){
        event ev = queue_pop();
        uint8_t* msg_buf_b = NULL;
        switch(ev.data.msg->type){
            case MSG_QUIT:
                set_quit();
                break;
            case MSG_GET_VERSION:
                message msg_v;
                msg_v.type = MSG_VERSION;
                get_version(&msg_v);
                if (write_to_pipe(&msg_v, pipe_out)){
                    debug("send version to pipe_in");
                }else{
                    error("send version is fail!");
                }
                break;
            case MSG_SET_COMPUTE:
                message msg_s;
                msg_s.type = MSG_OK;
                set_compute(ev.data.msg);
                if (write_to_pipe(&msg_s, pipe_out)){
                    debug("send OK to pipe_in");
                }else{
                    error("send OK is fail!");
                }

                break;
            case MSG_COMPUTE:
                uint8_t y_max, x_max;
                set_compute_chunk(ev.data.msg);
                num_pixels(&y_max,&x_max);
                for (uint8_t y = (uint8_t)0; y < y_max;y++){
                    for (uint8_t x = (uint8_t)0 ;x < x_max;x++){
                        message msg;
                        msg.type = MSG_COMPUTE_DATA;
                        msg.data.compute_data.i_im = y;
                        msg.data.compute_data.i_re = x;
                        compute_chunk(y, x ,&msg);
                        if (write_to_pipe(&msg, pipe_out)){
                        }else{
                            error("send computed data is fail!");
                        }
                    }
                }
                message msg_d;
                msg_d.type = MSG_DONE;
                if (write_to_pipe(&msg_d, pipe_out)){
                    debug("send compute_done to pipe_in");
                }else{
                    error("send compute_done is fail!");
                }
                break;
            case MSG_COMP_BURST:
                message msg_b;
                msg_b.type = MSG_COMPUTE_DATA_BURST;
                set_compute_chunk_BURST(ev.data.msg);
                compute_BURST(&msg_b);

                msg_buf_b = malloc((int)get_size_iters()+ sizeof(message));
                int msg_len_b;

                my_assert(fill_message_buf(&msg_b, msg_buf_b, sizeof(message), &msg_len_b),__func__, __LINE__, __FILE__);
                if (write(pipe_out, msg_buf_b, msg_len_b)==msg_len_b){
                }else{
                    error("send computed data is fail!");
                }   
                free(msg_b.data.compute_data_BURST.iters);
                free(msg_buf_b);
                message msg_b_d;
                msg_b_d.type = MSG_DONE;
                if (write_to_pipe(&msg_b_d, pipe_out)){
                    debug("send compute_done to pipe_in");
                }else{
                    error("send compute_done is fail!");
                }
                break;
            case MSG_ABORT:
                break;
            default:
                break;
        }
        free(ev.data.msg);
        ev.data.msg = NULL;
    }
    queue_cleanup();
    return NULL;
}

bool write_to_pipe(message* msg, int pipe_out){
    uint8_t msg_buf[sizeof(message)];
    int msg_len;
    my_assert(fill_message_buf(msg, msg_buf, sizeof(message), &msg_len),__func__, __LINE__, __FILE__);
    if (write(pipe_out, msg_buf, msg_len)==msg_len){
        return true;
    }else{
        return false;
    }
}


bool write_to_pipe_BURST(message* msg, int pipe_out){
    uint8_t msg_buf_b[(int)get_size_iters()+ sizeof(message)];
    int msg_len_b;
    my_assert(fill_message_buf(msg, msg_buf_b, sizeof(message), &msg_len_b),__func__, __LINE__, __FILE__);
    if (write(pipe_out, msg_buf_b, msg_len_b)==msg_len_b){
        return true;
    }else{
        return false;
    }
}