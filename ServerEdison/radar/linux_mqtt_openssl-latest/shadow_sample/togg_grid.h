#ifndef TOGG_GRID_H
#define TOGG_GRID_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define GRID_POSITION_ONE  63
#define GRID_POSITION_TWO 31
#define GRID_POSITION_THREE  15
#define GRID_POSITION_FOUR  59
#define GRID_POSITION_FIVE 27
#define GRID_POSITION_SIX  11
#define GRID_POSITION_SEVEN 57
#define GRID_POSITION_EIGHT 25
#define GRID_POSITION_NINE 9

#define ALT_GRID_POSITION_ONE 13
#define ALT_GRID_POSITION_TWO 15
#define ALT_GRID_POSITION_THREE 15
#define ALT_GRID_POSITION_FOUR 13
#define ALT_GRID_POSITION_FIVE 25
#define ALT_GRID_POSITION_SIX 29
#define ALT_GRID_POSITION_SEVEN 29
#define ALT_GRID_POSITION_EIGHT 31
#define ALT_GRID_POSITION_NINE 29

//returns the number of active grids
int grid_number(int grid_bits, int * grids_p) {
	printf("I am in togg_update grid_number");
	int count = 0;
	int grids[9] = {ALT_GRID_POSITION_ONE, ALT_GRID_POSITION_TWO, ALT_GRID_POSITION_THREE, ALT_GRID_POSITION_FOUR, ALT_GRID_POSITION_FIVE, ALT_GRID_POSITION_SIX, ALT_GRID_POSITION_SEVEN, ALT_GRID_POSITION_EIGHT, ALT_GRID_POSITION_NINE };
	int i;
	//for (i =0; i < 9; i++) {
	//	if (grid_bits == grids[i]) {
	//		grids_p[i] = i + 1;
	//		count++;
	//	} else {
	//		grids_p[i] = 0;
	//	}
	//}
	//for (i=0; i < 9; i++) {
	//	if (grid_bits == grids[i]) {
	//	printf("Found the grid\n");
	//		grids_p[0] = i + 1;
	//		printf("Assigned the grid\n");
	//		count++;
	//		break;
	//	}
	//}

		if (grid_bits == 148) {
			grids_p[count] = 1;
			count++;
		}
		if (grid_bits == 32) {
			grids_p[count] = 2;
			count++;
		}
		if (grid_bits == 1) {
			grids_p[count] = 3;
			count++;
		}
		if (grid_bits == 412 || grid_bits == 404) {
			grids_p[count] = 4;
			count++;
		}
		if (grid_bits == 352) {
			grids_p[count] = 5;
			count++;
		}
		if (grid_bits == 33) {
			grids_p[count] = 6;
			count++;
		}
		if (grid_bits == 524) {
			grids_p[count] = 7;
			count++;
		}
		if (grid_bits == 64 || grid_bits == 96) {
			grids_p[count] = 8;
			count++;
		}
		if (grid_bits == 35) {
			grids_p[count] = 9;
			count++; 
		}
	return count;
}

bool same_positions(int * grid_one, int * grid_two, int len) {
	printf("checking if the positions are the same\n");
	int i;
	for (i =0; i < len; i++) {
		if (grid_one[i] != grid_two[i]) {
			return false;
		}
	}
	return true;
}

#endif
