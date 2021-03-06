#include <unistd.h>
#include "mraa.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "aws_grid_update.h"
#include "togg_grid.h"
#define ROW 4
#define COL 4

//TCP server
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <time.h>


static volatile int counter_2 = 0;
static volatile int oldcounter_2 = 0;
static volatile int count_zero_2 = 0;
static volatile int delta_2 = 0;

static volatile int counter_3 = 0;
static volatile int oldcounter_3 = 0;
static volatile int count_zero_3 = 0;
static volatile int delta_3 = 0;

static volatile int counter_4 = 0;
static volatile int oldcounter_4 = 0;
static volatile int count_zero_4 = 0;
static volatile int delta_4 = 0;

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
   int * grid_positions = (int *) calloc(9, sizeof(int));
   int * prev_grid_positions = (int *) calloc(9, sizeof(int));
   int prev_len = 0;
   int grid[ROW][COL] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
   int flag2 = 0;
   int flag3 = 0;
   int flag4 = 0;
    
    //init gpio pins
  mraa_init();
  mraa_gpio_context low;
  mraa_gpio_context medium;
  mraa_gpio_context high;
  mraa_gpio_context relay_signal;
    
    //setup gpio pins
  low = mraa_gpio_init(20);
  medium = mraa_gpio_init(25);
  high = mraa_gpio_init(13);
  relay_signal = mraa_gpio_init(8);
  if (low == NULL || medium == NULL || high == NULL || relay_signal == NULL) {
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

   //TCP server setup
   int listenfd = 0, connfd = 0, n = 0;
   struct sockaddr_in serv_addr;
   char recvBuff[1024];

   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   memset(&serv_addr, '0', sizeof(serv_addr));
   memset(recvBuff, '0', sizeof(recvBuff));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(5000); 

   //set up JSON struct for AWS thing shadow
   //char grid_positions_string[1] = {'0'};
   char grid_positions_string;
   jsonStruct_t grid_tracker;
   grid_tracker.cb = windowActuate_Callback;
   grid_tracker.pData = &grid_positions_string;
   grid_tracker.pKey = "active_grids";
   grid_tracker.type = SHADOW_JSON_STRING;
   thing_shadow_setup(grid_tracker);
  
   mraa_gpio_isr(low,    edge2, &interrupt2, NULL);
   mraa_gpio_isr(medium, edge3, &interrupt3, NULL);
   mraa_gpio_isr(high,   edge4, &interrupt4, NULL);

   bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

   listen(listenfd, 5);


    for (;;) {
	delta_2 = 0;
	delta_3 = 0;
	delta_4 = 0;
	int client_grid[3] = {0,0,0};
	int client2_grid[3] = {0,0,0};
	int client3_grid[3] = {0,0,0};
	int server_grid[3] = {0,0,0};
	int client_collected = 0,  client2_collected = 0, client3_collected = 0;
	//read grid information from client grid
	
	while (client_collected + client2_collected + client3_collected != 3) {
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		if ((n = read(connfd, recvBuff, sizeof(recvBuff))) > 0) {
			recvBuff[n] = 0;
			if(fputs(recvBuff, stdout) == EOF)
			{
				printf("\n Error : Fputs error\n");
			}
			char *tokens = strtok(recvBuff, ",");
			int i;
			printf("recvBuff: %c,%c,%c,%c,%c,%c,%c\n", recvBuff[0], recvBuff[1], recvBuff[2], recvBuff[3], recvBuff[4],recvBuff[5], recvBuff[6]);
			if (recvBuff[0] -'0' == 1) {
		   		client_grid[0] = recvBuff[2] - '0';
		   		client_grid[1] = recvBuff[4] - '0';
		   		client_grid[2] = recvBuff[6] - '0';
				client_collected = 1;
			} else if (recvBuff[0] - '0' == 2) {
		   		client2_grid[0] = recvBuff[2] - '0';
		   		client2_grid[1] = recvBuff[4] - '0';
		   		client2_grid[2] = recvBuff[6] - '0';
				client2_collected= 1;
			} else if (recvBuff[0] - '0' == 3) {
				client3_grid[0] = recvBuff[2] - '0';
				client3_grid[1] = recvBuff[4] - '0';
				client3_grid[2] = recvBuff[6] - '0';
				client3_collected = 1;
			}
		}
		close(connfd);


	}

        //for sensor 2
        if (counter_2 != oldcounter_2) {
	    delta_2 = counter_2 - oldcounter_2;
	    printf("delta_2: %i\n", delta_2);
	    oldcounter_2 = counter_2; 
        } else {
	    oldcounter_2 = 0;
            counter_2 = 0;
        } 
        //for sensor 3
        if (counter_3 == oldcounter_3) {
            oldcounter_3 = 0;
            counter_3 = 0; 
        } else {
	    delta_3 = counter_3 - oldcounter_3;
	    printf("delta_3: %i\n", delta_3);
            oldcounter_3 = counter_3; 
        }
        //for sensor 4
        if (counter_4 == oldcounter_4) {
            oldcounter_4 = 0;
            counter_4 = 0; 
        } else {
	    delta_4 = counter_4 - oldcounter_4;
	    printf("delta_4: %i\n", delta_4);
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
            count_zero_2 = 0;
	    mraa_gpio_write(relay_signal, 0); 
            flag2 = 0;
	    fprintf(stdout, "2 no motion\n"); 
        }  
        
        if(delta_2 > 4){ 
	    count_zero_2 = 0;
            mraa_gpio_write(relay_signal, 1);
	    flag2 = 1;
            fprintf(stdout, "2 motion!!!\n"); 
        }
        //for sensor 3
        if (count_zero_3 >= 10) {  
            count_zero_3 = 0;
            flag3 = 0;
	    fprintf(stdout, "\t\t\t3 no motion\n");
        }
        if(delta_3 > 4) { 
	    count_zero_3 = 0; 
            flag3 = 1;
	    fprintf(stdout, "\t\t\t3 motion!!!\n"); 
        }


        //for sensor 4
        if (count_zero_4 >= 10) {  
            count_zero_4 = 0;
            flag4 = 0;
	            }
        if(delta_4 > 4) {
	    count_zero_4 = 0; 
            flag4 = 1;
	    
        }
	server_grid[0] = flag2;
	server_grid[1] = flag3;
	server_grid[2] = flag4;

	int grid_bits = 0;
	int j;
	for (j =0; j < 3; j++) {
		grid_bits <<= 1; 
		grid_bits |= server_grid[j];
		printf("server grid: %i\n", server_grid[j]);
	}
	for (j=0; j <3; j++) {
		grid_bits = (grid_bits << 1) | client_grid[j];
		printf("client grid: %i\n", client_grid[j]);
	}
	for (j=0; j<2; j++) {
		grid_bits = (grid_bits << 1) | client2_grid[j];
		printf("client2 grid: %i\n", client2_grid[j]);
	}
	for (j =0; j<2; j++) {
		grid_bits = (grid_bits << 1) | client3_grid[j];
		printf("client3 grid: %i\n", client3_grid[j]);
	}
	printf("grid_bits: %i\n", grid_bits);

	int positions = grid_number(grid_bits, grid_positions);

	//print out all the positions
	int z;
	for(z = 0; z < positions; z++) {
		printf("positions: %i\n", grid_positions[z]);
	}
	//See if there is a Delta needs to be reported
	if (positions != 0  &&  (positions != prev_len || !same_positions(grid_positions, prev_grid_positions, positions))) {
		printf("Passed the if statement\n");
		memset(prev_grid_positions, 0, prev_len);
		prev_len = positions;
		int i;
		for(i = 0; i < positions; i++) {
			grid_positions_string = grid_positions[i] + '0';
			prev_grid_positions[i] = grid_positions[i];

		} 

		report_to_thing_shadow(grid_tracker);
	}

        usleep(350* 1000);
    }

    return 1;
}
