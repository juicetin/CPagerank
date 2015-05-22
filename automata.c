#include <stdio.h>
#include <stdlib.h>

// Prints the board, using '.' for a dead cell, and 'X' for a live cell.
void print_board(int *board, int width) {
	for (int i = 0; i < width; i++) {
		printf("%c", (board[i] == 0) ? '.' : 'X');
	}
	printf("\n");
}

// Writes the next generation into the next array. Considers the cells
// to be living on a circle, in order to deal with endpoints.
void next_gen(int *current, int *next, int width) {
	for (int i = 0; i < width; i++) {
		int left = (i == 0) ? width-1 : i-1;
		int right = (i == width-1) ? 0 : i+1;
		next[i] = current[left] ^ current[right];
	}
}

int main(void) {
	int width = 64;
	int generations = 33;
	int *board1 = calloc(width, sizeof(int));
	int *board2 = calloc(width, sizeof(int));
	
	// Initial generation: only has one live cell in the middle.
	board1[width/2] = 1;
	
	for (int i = 0; i < generations; i++) {
		// Print the current generation.
		printf("Generation %02d: ", i);
		print_board(board1, width);
		
		// Compute the next generation.
		next_gen(board1, board2, width);
		
		// Swap the current and previous generations.
		int *tmp = board1;
		board1 = board2; 
		board2 = tmp;
	}
	
	free(board1);
	free(board2);
	return 0;
}
	