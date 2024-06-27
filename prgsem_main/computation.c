#include"computation.h"
#include "utils.h"
#include<stdio.h>

#include "gui.h"

static struct{
    double c_re; 
    double c_im;
    // C value parameters
    int n;
    //  number of iterations

    double range_re_min;
    double range_re_max;
    double range_im_min;
    double range_im_max;
    // min and max complex number

    int grid_w;
    int grid_h;
    // window size

    int cur_x;
    int cur_y;
    // number of computed window pixels

    double d_re;
    double d_im;
    // complex number increase 

    int nbr_chunks;
    //  number of cells
    int cid;
    // number of computed cells
    int reset_cid;
    // how many times the number of calculated cells was greater than 255
    double chunk_re;
    double chunk_im;
    // first pixel in cell

    uint8_t chunk_n_re;
    uint8_t chunk_n_im;
    // number of pixels in cell

    uint8_t *grid;
    // array of computed value 't'

    double lenght_BURST; 
    uint8_t *iters_BURST;


    bool was_abort;

    bool computing;
    bool abort;
    bool done;

    bool animate;

}comp = {
    .c_re = -0.4,
    .c_im =  0.6,    

    .n = 60,
    
    .range_re_min = -1.6,
    .range_re_max = 1.6,
    .range_im_min = -1.1,
    .range_im_max = 1.1,

    .reset_cid = 0,

    .grid = NULL,
    .iters_BURST = NULL,
    .grid_w = 640,
    .grid_h = 480,
    .chunk_n_re =64,
    .chunk_n_im = 48,

    .was_abort = false,

    .computing = false,
    .abort = false,
    .done = false,

    .animate = false

};


bool is_animate(void){
    return comp.animate;
}


void set_animate(void){
    comp.animate= !comp.animate;
}


int find_max_divisor(int number) {
    for(int i = number / 2; i > 1; i--) {
        if((number % i == 0) && i <= 100) {
            return i;
        }
    }
    return 1;
}

void write_info_param(void){
    fprintf(stderr, "\n");
    fprintf(stderr, "Please, input parameters\n");
    fprintf(stderr, "n - modify number of iterations. Note: it must be less than 255!\n");
    fprintf(stderr, "a - adjust 'min' value of the coplex number. Note: it must be less than 1.6 + 1.1*i or new 'b' value! \n");
    fprintf(stderr, "b - adjust 'max' value of the coplex number. Note: it must be larger than -1.6 - 1.1*i or new 'a' value!\n");
    fprintf(stderr, "c - adjust c parameter\n");
    fprintf(stderr, "s - modify window size. Note: it must be larger than 10x10 and less than 1921*1081!\n\n");

    fprintf(stderr, "Click on the corresponding button and enter the parameter next to it\n");

    fprintf(stderr, "Press 'f' to end, 'd' to set default parameters\n\n");

    fprintf(stderr, "Usage example: \n");
    fprintf(stderr, "s 640 480  *press ENTER*\n");
    fprintf(stderr, "a -1.4 0.1  *press ENTER*\n");
    fprintf(stderr, "\n");
}




void input_paramentrs(void){
   int num_iterations, win_width, win_height;
    double a_re, a_im,b_re, b_im, c_re, c_im;
    char option;
    while (1) {
        option = getchar();
        switch (option) {
            case 'n':
                if((scanf("%d", &num_iterations)==1) && num_iterations > 0 && num_iterations < 255){
                    comp.n = num_iterations;
                    printf("Number of iterations set to: %d\n", comp.n);
                }else{warn("Invalid input. Please try again.");}
                break;
            case 'a':
                if((scanf("%lf %lf", &a_re, &a_im) == 2) && a_re < comp.range_re_max && a_im < comp.range_im_max){
                    comp.range_re_min = a_re;
                    comp.range_im_min = a_im;
                    printf("'RE' and 'IMAG' values of the 'min' complex number set to: %lf + i*%lf\n", comp.range_re_min, comp.range_im_min);
                }else{warn("Invalid input. Please try again.");}
                break;
            case 'b':
                if ((scanf("%lf %lf", &b_re, &b_im) == 2) && b_re > comp.range_re_min && b_im > comp.range_im_min){
                    comp.range_re_max = b_re;
                    comp.range_im_max = b_im;
                    printf("'RE' and 'IMAG' values of the 'max' complex number set to: %lf + i*%lf\n", b_re, b_im);
                }else{warn("Invalid input. Please try again.");}
                break;
            case 'c':
                if(scanf("%lf %lf", &c_re, &c_im) == 2){
                comp.c_re = c_re;
                comp.c_im = c_im;
                printf("'RE' and 'IMAG' values of the complex number 'C' set to: %lf + i*%lf\n", c_re, c_im);
                }else{warn("Invalid input. Please try again.");}
                break;
            case 's':
                if ((scanf("%d %d", &win_width, &win_height) == 2) && win_width > 10 && win_height > 10 && win_width <= 1920 && win_height <= 1080){
                comp.grid_w = win_width;
                comp.grid_h = win_height;
                comp.chunk_n_re = find_max_divisor(win_width);
                comp.chunk_n_im = find_max_divisor(win_height);
                printf("Window size set to: %d x %d. Cell size set to:%d x %d\n", win_width, win_height, comp.chunk_n_re, comp.chunk_n_im);
                }else{warn("Invalid input. Please try again.");}
                break;
            case 'f':
                return;
            case 'q':
                info("Terminate the program");
                exit(100);
            case 'd':
                comp.n = 60;
                comp.range_re_min = -1.6;
                comp.range_im_min = -1.1;
                comp.range_re_max = 1.6;
                comp.range_im_max = 1.1;
                comp.c_re = -0.4;
                comp.c_im = 0.6;
                comp.grid_w = 640;
                comp.grid_h = 480;
                comp.chunk_n_re = 64;
                comp.chunk_n_im = 48;
                printf("Parameters set to default.\n");
                break;
            default:
                printf("Invalid input. Please try again.\n");
                break;
        }

        // Clear input buffer
        while ((option = getchar()) != '\n' && option != EOF) {}
    } 

}



void change_size(int w, int h){
    comp.grid_w = w;
    comp.grid_h = h;
}

void computation_init(void){
    comp.grid = my_alloc(comp.grid_w*comp.grid_h);
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1. * comp.grid_w);
    comp.d_im = -(comp.range_im_max - comp.range_im_min) / (1. * comp.grid_h);
    comp.nbr_chunks = (comp.grid_w * comp.grid_h) / (comp.chunk_n_re * comp.chunk_n_im);
}
void computation_cleanup(void){
    if(comp.grid){
        free(comp.grid);
    }
    comp.grid = NULL;   
}

bool is_computing(void){
    return comp.computing;
}
bool is_done(void){ return comp.done; }
bool is_abort(void){ return comp.abort; }
bool is_was_abort(void){ return comp.was_abort;}

void get_grind_size(int *w, int *h){
    *w=comp.grid_w;
    *h=comp.grid_h;
}

void abort_comp(void){ 
    comp.abort = true;
    comp.was_abort = true;
}
void enable_comp(void){ 
    comp.was_abort = false;
}

bool set_compute(message *msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    bool ret = !is_computing();
    if (ret){
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;  
        msg->data.set_compute.n = comp.n;
        comp.done = false;
    }
    return ret;
}

bool compute_BURST(message *msg){
    if(!is_computing()){ //fisrt chunk
        msg->data.comp_BURST.cid = comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0; //start computation of the whole image
        msg->data.comp_BURST.cid =0;
        msg->data.comp_BURST.re = comp.chunk_re = comp.range_re_min; // upper left corner
        msg->data.comp_BURST.im = comp.chunk_im = comp.range_im_max; // upper left corner
        msg->data.comp_BURST.length = comp.lenght_BURST = comp.grid_w * comp.grid_h;
        msg->data.comp_BURST.iters = comp.iters_BURST;
        msg->data.comp_BURST.grid_w = comp.grid_w;
    }else if(is_computing()){
        if(comp.cid + (256*comp.reset_cid) < comp.nbr_chunks){
            if (comp.cur_x < comp.grid_w){
                msg->data.comp_BURST.cid =comp.cid;
                msg->data.comp_BURST.re = comp.range_re_min;
                msg->data.comp_BURST.im = comp.chunk_im ;
                msg->data.comp_BURST.length = comp.lenght_BURST = ((comp.range_im_min-comp.chunk_im)/comp.d_im) * comp.grid_w;
                msg->data.comp_BURST.iters = comp.iters_BURST;
                msg->data.comp_BURST.grid_w = comp.grid_w;
            }
        }else{// all has been compute
        }
    }
    return is_computing();
}

double get_size_iters (void){
    return comp.lenght_BURST;
}

bool compute(message *msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    if(!is_computing()){ //fisrt chunk
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0; //start computation of the whole image
        comp.chunk_re = comp.range_re_min; // upper left corner
        comp.chunk_im = comp.range_im_max; // upper left corner
        msg->type = MSG_COMPUTE;
    } 
    else if ((!comp.abort) && is_computing()){ //next chunk
        comp.cid +=1;
        if (comp.cid == 256){
            comp.reset_cid ++;
            comp.cid =0 ;
            }
        if(comp.cid + (256*comp.reset_cid) < comp.nbr_chunks){
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w){
                comp.chunk_re = comp.range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
            }
            msg->type = MSG_COMPUTE;
        }else { // all has been compute
        }

    }
    else if (comp.abort && is_computing()){
        comp.cur_x -= comp.chunk_n_re;
        comp.chunk_re -= comp.chunk_n_re * comp.d_re;
        comp.abort = false;
        if(comp.cid + (256*comp.reset_cid) < comp.nbr_chunks){
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w){
                comp.chunk_re = comp.range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
            }
            msg->type = MSG_COMPUTE;
        }else { // all has been compute
        }
    }
    if(comp.computing && msg->type == MSG_COMPUTE){
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }
    return is_computing();
}

void update_image(int w, int h, unsigned char *img){
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__, __FILE__);
     for (int i = 0; i < w * h; ++i){
        const double t = 1. * comp.grid[i] / (comp.n + 1.0);
        *(img++) = 9 * (1-t)*t*t*t * 255;
        *(img++) = 15 * (1-t) * (1-t) *t*t*255;
        *(img++) = 8.5* (1-t)* (1-t)* (1-t) * t * 255;
     }
}
void update_data(const msg_compute_data *compute_data ){
    my_assert(compute_data != NULL, __func__, __LINE__, __FILE__);
    if(compute_data->cid == comp.cid){
        const int idx = comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im)*comp.grid_w;
        if (idx >=0 && idx < (comp.grid_w * comp.grid_h)){
            comp.grid[idx]=compute_data->iter;
        }
        if ((comp.cid+ (256*comp.reset_cid) +1) >= comp.nbr_chunks && (compute_data->i_re +1) == comp.chunk_n_re && (compute_data->i_im +1) == comp.chunk_n_im){
             comp.done = true;   
             comp.computing = false;
        } 
    }else{
        warn("Received chunk with unexpected chunk id (cid)");  
    }
}

int count=0;

void update_data_BURST(const msg_compute_data_done_BURST* compute_data_BURST){
    int idx_im = 0;
    comp.abort = false;
    for (int i=0; i < comp.lenght_BURST; i++){
        if ((i / comp.grid_w) > idx_im ){
            idx_im++;
            comp.cur_y += 1;
        }
        int idx = (i%comp.grid_w) + comp.cur_y*comp.grid_w;
        if (idx >=0 && idx < (comp.grid_w * comp.grid_h)){
            comp.grid[idx]=compute_data_BURST->iters[i];
        }
    }
    free(compute_data_BURST->iters);
    comp.computing = false;
    comp.done = true;   

    if (!is_was_abort() && is_animate()){
        comp.c_re +=0.1;
        comp.c_im +=0.1;
        count++;
        if (count==10){
            comp.c_re -=2;
            comp.c_im -=2;
            count=-10;
        }
    }
}
