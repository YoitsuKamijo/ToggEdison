#include <unistd.h>
#include "mraa.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
static volatile int counter = 0;
static volatile int oldcounter = 0;
static volatile int count_zero = 0;
void interrupt(void* args)
{
	    ++counter;
}

int main()
{
	//init gpio pins       
	mraa_init();
       	mraa_gpio_context x;
       	mraa_gpio_context relay_signal;
        x = mraa_gpio_init(2);
	relay_signal = mraa_gpio_init(8);
    	if (x == NULL || relay_signal == NULL) {
		 return 1;
	}

	//setup gpio direction	       
	mraa_gpio_dir(x, MRAA_GPIO_IN);
       	mraa_gpio_dir(relay_signal, MRAA_GPIO_OUT);
	mraa_gpio_write(relay_signal, 0);
    	mraa_gpio_edge_t edge = MRAA_GPIO_EDGE_BOTH;
	mraa_gpio_isr(x, edge, &interrupt, NULL);
	
	for (;;) {
		if (counter == oldcounter) {
			oldcounter = 0;
			counter = 0;
		} else {
			oldcounter = counter;
		}
		if (oldcounter == 0 && counter == 0) {
			count_zero++;
		}
		if (count_zero >= 10) {
			mraa_gpio_write(relay_signal, 0);
			fprintf(stdout, "no motion\n");
			count_zero = 0;
		}
       	
		if(counter > 0){
			mraa_gpio_write(relay_signal, 1);
			count_zero = 0;
			fprintf(stdout, "motion!!!\n");
		}

	 usleep(350* 1000);
	 }
	mraa_gpio_close(x);
	return MRAA_SUCCESS;
}
