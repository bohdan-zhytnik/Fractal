#include<stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>

#include "computation.h"
#include "messages.h"
#include "utils.h"

#define MAJOR 1
#define MINOR 0
#define PATCH 1

static struct{
    double c_re;
    double c_im;
    int n;

    double range_re_min;
    double range_re_max;
    double range_im_min;
    double range_im_max;

    uint8_t ver_major;
    uint8_t ver_minor;
    uint8_t ver_patch;

    int grid_w;
    int grid_h;

    int cur_x;
    int cur_y;

    double d_re;
    double d_im;

    uint8_t cid;
    double chunk_re;
    double chunk_im;

    uint8_t chunk_n_re;
    uint8_t chunk_n_im;

    double lenght_BURST; 
    uint8_t *iters_BURST;
    int grid_w_BURST;

    bool abort;
    bool done;

}comp={
    .iters_BURST = NULL,


    .ver_major = (uint8_t)MAJOR,
    .ver_minor = (uint8_t)MINOR,
    .ver_patch = (uint8_t)PATCH,
};


void get_version(message* msg){
    msg->data.version.major = comp.ver_major;
    msg->data.version.minor = comp.ver_minor;
    msg->data.version.patch = comp.ver_patch;
}


void set_compute(message* msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    comp.c_re = msg->data.set_compute.c_re;
    comp.c_im = msg->data.set_compute.c_im;
    comp.d_re = msg->data.set_compute.d_re;
    comp.d_im = msg->data.set_compute.d_im;
    comp.n = msg->data.set_compute.n;
    comp.done = false;
}

void set_compute_chunk(message* msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    comp.cid = msg->data.compute.cid;
    comp.chunk_re = msg->data.compute.re;
    comp.chunk_im = msg->data.compute.im;
    comp.chunk_n_re = msg->data.compute.n_re;
    comp.chunk_n_im = msg->data.compute.n_im;
    comp.cur_x=0;
    comp.cur_y=0;
}


void set_compute_chunk_BURST(message* msg){
    comp.cid = msg->data.comp_BURST.cid;
    comp.chunk_re = msg->data.comp_BURST.re;
    comp.chunk_im = msg->data.comp_BURST.im;
    comp.lenght_BURST = msg->data.comp_BURST.length;
    comp.grid_w_BURST = msg->data.comp_BURST.grid_w;
}
double get_size_iters (void){
    return comp.lenght_BURST;
}


void num_pixels (uint8_t* y_max, uint8_t* x_max){
    *y_max = comp.chunk_n_im;
    *x_max = comp.chunk_n_re;
}

void compute_chunk(uint8_t y, uint8_t x, message* msg){
    double real = comp.chunk_re + ((double)x * comp.d_re);
    double imag = comp.chunk_im + ((double)y * comp.d_im);
    int k = julia(real, imag, comp.c_re, comp.c_im );
    msg->data.compute_data.cid = comp.cid;
    msg->data.compute_data.iter = (uint8_t)k;
}

void compute_BURST(message* msg){
    int x = 0; 
    int y = 0;
    msg->data.compute_data_BURST.iters = malloc(comp.lenght_BURST*sizeof(uint8_t)+1);
    for (int i=0; i < comp.lenght_BURST; i++){
        x = i % comp.grid_w_BURST;
        if ((i / comp.grid_w_BURST) > y ){
            y++;
        }
        double real = comp.chunk_re + ((double)x * comp.d_re);
        double imag = comp.chunk_im + ((double)y * comp.d_im);
        uint8_t k = (uint8_t)julia(real, imag, comp.c_re, comp.c_im );
        msg->data.compute_data_BURST.iters[i]=k;
    }
}

int julia(double real0, double imag0, double c_re, double c_im)
{
    for (int k = 0; k < comp.n; k++) {
        double c_abs = sqrt((real0*real0) + (imag0*imag0));
        if (c_abs > 2.0){ 
            return k;
        }
        double temp_real = real0;
        real0 = ((real0*real0)-(imag0*imag0)+c_re);
        imag0 = (2.0*temp_real*imag0) + c_im;
    }
    return comp.n;
}
