#ifndef __COMPUTATION__
#define __COMPUTATION__

#include<stdbool.h>

#include"messages.h"

int find_max_divisor(int );

void write_info_param(void);
void input_paramentrs(void);

void change_size(int w, int h);
void computation_init(void);
void computation_cleanup(void);

void get_grind_size(int *w, int *h);
bool is_computing(void);
bool is_done(void);
bool is_abort(void);
bool is_was_abort(void);

void enable_comp(void);
void abort_comp(void);

bool set_compute(message *msg);
bool compute(message *msg);

bool compute_BURST(message *);

void update_image(int w, int h, unsigned char *img);
void update_data(const msg_compute_data *compute_data );

void update_data_BURST(const msg_compute_data_done_BURST *);
double get_size_iters (void);

bool is_animate(void);
void set_animate(void);

#endif