#ifndef __COMPUTATION__
#define __COMPUTATION__

#include<stdbool.h>
#include <complex.h>

#include"messages.h"

void set_compute(message* );
void set_compute_chunk(message* );

void num_pixels (uint8_t* , uint8_t* );
void compute_chunk(uint8_t, uint8_t, message*);
void get_version(message*);
int julia(double, double, double, double);

void set_compute_chunk_BURST(message*);
void compute_BURST(message*);
double get_size_iters (void);

#endif