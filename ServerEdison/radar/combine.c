#include <unistd.h>
#include "mraa.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ROW 4
#define COL 4

static volatile int counter_2 = 0;
static volatile int oldcounter_2 = 0;
static volatile int count_zero_2 = 0;

static volatile int counter_3 = 0;
static volatile int oldcounter_3 = 0;
static volatile int count_zero_3 = 0;

static volatile int counter_4 = 0;
static volatile int oldcounter_4 = 0;
static volatile int count_zero_4 = 0;

void interrupt2(void* args) {
            ++counter_2;
}

void interrupt3(void* args) {
            ++counter_3;
}

void interrupt4(void* args) {
            ++counter_4;
}

int main() {
    //set up the grid
    int grid[ROW][COL] = {{0,0,0,0}, {0,0,0,0},{0,0,0,0},{0,0,0,0}};
    //init gpio pins
    mraa_init();
    mraa_gpio_context low;
    mraa_gpio_context medium;
    mraa_gpio_context high;
    mraa_gpio_context relay_signal;
    //setup gpio pins
    low = mraa_gpio_init(2);
    medium = mraa_gpio_init(3);
    high = mraa_gpio_init(4);
    relay_signal = mraa_gpio_init(8);
    if (low == NULL || medium == NULL || high == NULL || relay_signal == 
NULL) {
        return 1;
    }

    //setup gpio direction         
    mraa_gpio_dir(low, MRAA_GPIO_IN);
    mraa_gpio_dir(medium, MRAA_GPIO_IN);
    mraa_gpio_dir(high, MRAA_GPIO_IN);
    mraa_gpio_dir(relay_signal, MRAA_GPIO_OUT);
    mraa_gpio_write(relay_signal, 0);
    //interrupt edge trigger
    mraa_gpio_edge_t edge2 = MRAA_GPIO_EDGE_BOTH;
    mraa_gpio_edge_t edge3 = MRAA_GPIO_EDGE_BOTH;
    mraa_gpio_edge_t edge4 = MRAA_GPIO_EDGE_BOTH;
    mraa_gpio_isr(low, edge2, &interrupt2, NULL);
    mraa_gpio_isr(medium, edge3, &interrupt3, NULL);
    mraa_gpio_isr(high, edge4, &interrupt4, NULL);

    for (;;) {
        //for sensor 2
        if (counter_2 == oldcounter_2) {
            oldcounter_2 = 0;
            counter_2 = 0; 
        } else { 
            oldcounter_2 = counter_2; 
        } 
        //for sensor 3
        if (counter_3 == oldcounter_3) {
            oldcounter_3 = 0;
            counter_3 = 0; 
        } else { 
            oldcounter_3 = counter_3; 
        }
        //for sensor 4
        if (counter_4 == oldcounter_4) {
            oldcounter_4 = 0;
            counter_4 = 0; 
        } else { 
            oldcounter_4 = counter_4; 
        }
        //for sensor 2
        if (oldcounter_2 == 0 && counter_2 == 0) { 
            count_zero_2++; 
        }
        //for sensor 3
        if (oldcounter_3 == 0 && counter_3 == 0) { 
            count_zero_3++; 
        }
        //for sensor 4
        if (oldcounter_4 == 0 && counter_4 == 0) { 
            count_zero_4++; 
        }


        //for sensor 2
        if (count_zero_2 >= 10) { 
            mraa_gpio_write(relay_signal, 0); 
            fprintf(stdout, "2 no motion\n"); 
            count_zero_2 = 0; 
        }  
        
        if(counter_2 > 0){ 
            mraa_gpio_write(relay_signal, 1); 
            count_zero_2 = 0; 
            fprintf(stdout, "2 motion!!!\n"); 
        }
        //for sensor 3
        if (count_zero_3 >= 10) {  
            count_zero_3 = 0;
            fprintf(stdout, "\t3 no motion\n");
        }
        if(counter_3 > 0) { 
            count_zero_3 = 0; 
            fprintf(stdout, "\t3 motion!!!\n"); 
        }


        //for sensor 4
        if (count_zero_4 >= 10) {  
            count_zero_4 = 0;
            fprintf(stdout, "\t\t4 no motion\n");
        }
        if(counter_4 > 0) { 
            count_zero_4 = 0; 
            fprintf(stdout, "\t\t4 motion!!!\n"); 
        }
        usleep(350* 1000); 
        }
}
