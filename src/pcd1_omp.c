#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <omp.h>
#include <sys/time.h>
#include <time.h>

/*
 *	(x,y) = (1,1) for Glider.
 *	(x,y) = (10,30) for R-pentomino
 */

#define GLIDER_X 1
#define GLIDER_Y 1
#define R_PENTOMINO_X 10
#define R_PENTOMINO_Y 30

#define GENERATIONS 2000
#define GRID_SIZE 2048

/*
 *	Programacao Concorrente e Distribuida 2023 - Rainbow Game of Life
 *	Autor: Gustavo dos Santos Ferreira 140731
 */

float time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}

void initialShape(float*** grid) {
	/* GLIDER */
	(*grid)[GLIDER_X][GLIDER_Y+1] = 1.0;
	(*grid)[GLIDER_X+1][GLIDER_Y+2] = 1.0;
	(*grid)[GLIDER_X+2][GLIDER_Y] = 1.0;
	(*grid)[GLIDER_X+2][GLIDER_Y+1] = 1.0;
	(*grid)[GLIDER_X+2][GLIDER_Y+2] = 1.0;
	/* R-PENTOMINO */
	(*grid)[R_PENTOMINO_X][R_PENTOMINO_Y+1] = 1.0;
	(*grid)[R_PENTOMINO_X][R_PENTOMINO_Y+2] = 1.0;
	(*grid)[R_PENTOMINO_X+1][R_PENTOMINO_Y] = 1.0;
	(*grid)[R_PENTOMINO_X+1][R_PENTOMINO_Y+1] = 1.0;
	(*grid)[R_PENTOMINO_X+2][R_PENTOMINO_Y+1] = 1.0;
}

int getNeighbours(float** grid, int x, int y) {
	int i;
	int n;
	n = 0;
	for (i = 0; i < 9; i++) {
		int posX, posY;
		if (i == 4) continue;
		posX = x - 1 + (i%3);
		posY = y - 1 + (i/3);
		/* Infinite grid */
		if (posX < 0) posX = GRID_SIZE - 1;
		else if (posX > GRID_SIZE) posX = 0;
		if (posY < 0) posY = GRID_SIZE - 1;
		else if (posY > GRID_SIZE) posY = 0;
		if (grid[posX][posY]) n++;
	}
	return n;
}

float calcMeanNeighbours(float*** grid, int x, int y) {
	int i;
	int n;
	float val;
	n = 0;
	val = 0;
	for (i = 0; i < 9; i++) {
		int posX, posY;
		if (i == 4) continue;
		posX = x - 1 + (i%3);
		posY = y - 1 + (i/3);
		/* Infinite grid */
		if (posX < 0) posX = GRID_SIZE - 1;
		else if (posX > GRID_SIZE - 1) posX = 0;
		if (posY < 0) posY = GRID_SIZE - 1;
		else if (posY > GRID_SIZE - 1) posY = 0;
		if ((*grid)[posX][posY]) {
			n++;
			val += (*grid)[posX][posY];
		}
	}
	if (n != 2 && n != 3) {
		val = 0;
	}
	else if (n == 2) {
		val = (*grid)[x][y];
	}
	else if (n == 3) {
		val = val / 8;
		if (val == 0) {
			val = 0.001;
		}
	}
	return val;
}

int countLivingCells(float*** grid) {
	int i;
	int j;
	int count;
	count = 0;
	for (i = 0; i < GRID_SIZE; i++) {
		for (j = 0; j < GRID_SIZE; j++) {
			if ((*grid)[i][j]) {
				count++;
			}
		}
	}
	return count;
}

int main(int argc, char* argv[]) {
	struct timeval start;
	struct timeval end;
	int i, j, k;
	float** cells;
	float** nextCells;
    int THREAD_COUNT = 1;

    if (argc > 1) {
        THREAD_COUNT = atoi(argv[1]);
    }
	printf("Running %d threads.\n", THREAD_COUNT);

	FILE *fptr;

	fptr = fopen("result.csv", "w");

	/* Initialize the arrays */
	cells = (float**) malloc(GRID_SIZE * sizeof(float*));
	for (i = 0; i < GRID_SIZE; i++) {
		cells[i] = (float*) malloc(GRID_SIZE * sizeof(float));
		for (j = 0; j < GRID_SIZE; j++) {
			cells[i][j] = 0.0;
		}
	}
	initialShape(&cells);
	nextCells = (float**) malloc(GRID_SIZE * sizeof(float*));
	for (i = 0; i < GRID_SIZE; i++) {
		nextCells[i] = (float*) malloc(GRID_SIZE * sizeof(float));
	}
	/* Execution goes here */
	gettimeofday(&start, NULL);
	for (k = 0; k < GENERATIONS; k++) {
		//printf("[GENERATION %d]\n", k);
		/* Procedural Code */
		/*
		for (i = 0; i < GRID_SIZE; i++) {
			for (j = 0; j < GRID_SIZE; j++) {
				nextCells[i][j] = calcMeanNeighbours(&cells, i, j);
			}
		}
		*/

		/* OpenMP Code */
        #pragma omp parallel num_threads(THREAD_COUNT) 
        {
            #pragma omp for private (i, j) 
            for (i = 0; i < GRID_SIZE; i++) {
                for (j = 0; j < GRID_SIZE; j++) {
                    nextCells[i][j] = calcMeanNeighbours(&cells, i, j);
                }
            }
			
        }

		// Printing
		// Uncomment this if you want to print the .csv
		/*
		if (k < 5) {
			for (i = 0; i < 50; i++) {
				for (j = 0; j < 50; j++) {
					fprintf(fptr, "%.4f,", cells[i][j]);
				}
				fprintf(fptr, "\n");
			}
			fprintf(fptr, "\n");
		}
		*/
		
		for (i = 0; i < GRID_SIZE; i++) {
			for (j = 0; j < GRID_SIZE; j++) {
				cells[i][j] = nextCells[i][j];
			}
		}
	}
	printf("[FINAL CELL COUNT %d] ", countLivingCells(&cells));
	gettimeofday(&end, NULL);
	printf("Time taken to run %d threads: %.6f seconds.\n", THREAD_COUNT, time_diff(&start, &end));

	/* Free the arrays */
	for (i = 0; i < GRID_SIZE; i++) {
		free(cells[i]);
	}
	free(cells);

	for (i = 0; i < GRID_SIZE; i++) {
		free(nextCells[i]);
	}
	free(nextCells);

	fclose(fptr);
	return 0;
}
