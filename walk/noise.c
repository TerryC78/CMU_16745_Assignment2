#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

char noise_method;
int32_t noise_seed;
double noise_mean;
double noise_std;
double noise_sample_time;

// linear interpolation
static double t_curr, t_next;
static double x_curr, x_next;

// generate random number with mu = 0, sigma = 1
double (*noise_gen)();
static double noise_gen_zero() { return 0; }
static double noise_gen_uniform() { return (rand()*(2.0l/RAND_MAX) - 1.0l)*sqrt(3); }

// scale and shift noise to desired mu/sigma
inline static double noise_shift(double x) {
    return x*noise_std + noise_mean;
}

void noise_init() {
    FILE* f_noise;
    f_noise = fopen("noise.par", "r");

    noise_method = fgetc(f_noise);
    switch(noise_method) {
        case 'U': //uniform (white)
            noise_gen = noise_gen_uniform;
            break;
        default: //no noise
            noise_gen = noise_gen_zero;
            return;
    }

    // NOTE: only one seed per execution
    fscanf(f_noise, "%d", &noise_seed);
    srand(noise_seed);

    fscanf(f_noise, "%lf%lf%lf", &noise_mean, &noise_std, &noise_sample_time);

    //initialize first sample
    t_next = 0;
    x_next = noise_shift(noise_gen());

    fclose(f_noise);
}

#define INTERP1(x1, x2, y1, y2, x) ( ((x)-(x1))/((x2)-(x1))*((y2)-(y1)) + (y1) )
double noise(double t) {
    while (t >= t_next) {
        t_curr = t_next;
        t_next = t_curr + noise_sample_time;
        x_curr = x_next;
        x_next = noise_shift(noise_gen());
    }
    return INTERP1(t_curr, t_next, x_curr, x_next, t);
}
#undef INTERP1
