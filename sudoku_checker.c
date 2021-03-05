/* **************************************************************************************************************
Name: Tiffany Elliott (csci userid: @elliottt)
Date: October 2019
Course: CSCI360, FALL2019, Humayun Kabir
************************************************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PUZZLE_SIZE 9
#define NUMBER_OF_THREADS 27
#define LINE_MAX_LENGTH 100

int puzzle[PUZZLE_SIZE+1][PUZZLE_SIZE+1]; // Im memory representation of a sudoku puzzle solution
static int status_map[NUMBER_OF_THREADS]; // Status maps updated by corresponding worker thread

/*
 * Data structure to pass parameters to each worker thread 
 */
typedef struct {
	int thread_no;
	int x;
	int y;
} parameters;

parameters* worker_params[NUMBER_OF_THREADS]; // Array of worker thread parameter pointers, 
            //main thread passes an appropriate  parameter pointer to a corresponding worker thread
pthread_t workers[NUMBER_OF_THREADS]; // Array of worker threads, main thread needs to wait for all 
                                    //worker threads to finish before it compiles the final result 

void show_puzzle(){
// Shows in memory content of a sudoku puzzle solution
    printf("\n*************************************************\n");
    printf("\nYour sudoku puzzle is: \n");
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            printf("%d " ,puzzle[i][j]);
        }
        printf("\n");
    }
    printf("\n************************************************\n");
}

void read_from_file(FILE* sudoku_file){
// Reads the solution of a sudoku puzzle line by line from a given FILE pointer
// Parses individual values (separated by comma) from each line and puts it at a corresponding in memory location
// Reports error if reading fails
    char line[500];
    char *pt;
    int row = 0;
    int col = 0;

    //Get a line from the file 
    while(fgets(line, sizeof(line), sudoku_file)){

      //Tokenize by comma separated vals
      pt = strtok (line, ",");
      while(pt != NULL){
        int a = atoi(pt);
        puzzle[row][col] = a;
        pt = strtok (NULL, ",");
        col++;
      }
      
      col = 0; //reset the column count
      row++; //iterate to the next row
    }
}

void* row_worker(void* param){
// Checks whether the row of the sudoku puzzle solution referred by the param contains all the digits from 1 to 9
// Sets the appropriate status value in status_map.
    parameters *entry = (parameters *) param;
    int sRow = entry->x;
    int sCol = entry->y;
    int tNum = entry->thread_no;

    int row[10];//create a row checker array, when we see a value, mark the corresponding index in the row array
    int num;//current number in the puzzle 

    //go through each value of the current row in the puzzle and mark 
    for(int i = sCol; i < PUZZLE_SIZE; ++i){
        num = puzzle[sRow][i]; 

        //if the row checker has already been marked with a 1, there is a duplicate
        //mark the status_map and return
        if(row[num] != 0 ){ 
            status_map[tNum] = 0; 
            return (void *) 0; 
        //otherwise, we have not seen that integer yet, mark the row checker
            //to track that we have seen this number 
        } else {
            row[num] = 1;
        }
    }

    //We are keeping track of the numbers 1-9 by marking them to the corresponding
    //index.... this means that row[0] should have not been marked 1 or else we 
    //have seen a 0, which is incorrect
    if (row[0]!=1) {
        status_map[tNum] = 1; 
    } else {
        status_map[tNum] = 0;
    }

    return (void *) 0;
}


void* col_worker(void* param){
// Checks whether the column of the sudoku puzzle solution referred by the param contains all the digits from 1 to 9
// Sets the appropriate status value in status_map.
    parameters *entry = (parameters *) param;
    int sRow = entry->x;
    int sCol = entry->y;
    int tNum = entry->thread_no;
    int col[10];
    int num;

    //go through each value of the current row in the puzzle and mark 
    for(int i = sRow; i < PUZZLE_SIZE; ++i){
        
        //if the column checker has already been marked with a 1, there is a duplicate
        //mark the status_map and return
        num = puzzle[i][sCol];
        if(col[num] != 0 ){
            status_map[tNum] = 0;
            return (void *) 0;
        //otherwise, we have not seen that integer yet, mark the column checker
        //to track that we have seen this number 
        } else {
            col[num] = 1;
        }
    }
    //We are keeping track of the numbers 1-9 by marking them to the corresponding
    //index.... this means that row[0] should have not been marked 1 or else we 
    //have seen a 0, which is incorrect
    if (col[0]!=1) {
        status_map[tNum] = 1; 
    } else {
        status_map[tNum] = 0;
    }

    return (void *) 0;
}

void* subgrid_worker(void* param){
// Checks whether the subgrid of the sudoku puzzle solution referred by the param contains all the digits from 1 to 9
// Sets the appropriate status value in status_map.
    parameters *entry = (parameters *) param;
    int sRow = entry->x;
    int sCol = entry->y;
    int tNum = entry->thread_no;
    int grid[10];
    for (int i = 0; i < 10; i++) 
        grid[i] = 0;

    //go through each value of the grid in the puzzle and mark 
    for(int i = sRow; i < sRow + 3; ++i){
        for(int j = sCol; j < sCol + 3; ++j){

            //if the grid checker has already been marked with a 1, there is a duplicate
            //mark the status_map and return
            int num = puzzle[i][j];
            if(grid[num] != 0 ){
                status_map[tNum] = 0;
                return (void *) 0;
            //otherwise, we have not seen that integer yet, mark the grid checker
            //to track that we have seen this number 
            } else {
                grid[num] = 1;
            }
        }
    }

    //We are keeping track of the numbers 1-9 by marking them to the corresponding
    //index.... this means that row[0] should have not been marked 1 or else we 
    //have seen a 0, which is incorrect
    if (grid[0]!=1) {
        status_map[tNum] = 1;
    } else {
        status_map[tNum] = 0;
    }
    return (void *) 0;
}

int main(int argc, char** argv){
// Checks whether sudoku puzzle solution input file name is passed as an argument or not
// Reports error and quits, if the input file name is missing.

// Opens the input file.
    FILE *fptr;
    if ((fptr = fopen(argv[1], "r")) == NULL){
        printf("Sorry, couldn't open that file\n");
        // Program exits if file pointer returns NULL.
        exit(1);         
    }

// Reads from the input file and loads the solution in memory calling read_from_file() function.
    read_from_file(fptr);
    fclose(fptr);

// Shows the solution from memory calling show_puzzle() function.
    show_puzzle();

// Creates worker thread parameters
// Creates worker thread passing appropriate parameters and worker function.

    //Create worker threads to check rows
    for(int i = 0; i < 9; i++){
        worker_params[i] = (parameters *) malloc(sizeof(parameters));
        worker_params[i]->x = i;
        worker_params[i]->y = 0;
        worker_params[i]->thread_no = i;
        printf("Thread number %d is checking row %d.\n", worker_params[i]->thread_no, i);
        pthread_create(&workers[i], NULL, row_worker, (void *) worker_params[i]);

    }

    //Create worker threads to check columns
    for(int i = 9; i < 18; i++){
        worker_params[i] = (parameters *) malloc(sizeof(parameters));
        worker_params[i]->x = 0;
        worker_params[i]->y = i-9;
        worker_params[i]->thread_no = i;
        printf("Thread number %d is checking column %d.\n", worker_params[i]->thread_no, i);
        pthread_create(&workers[i], NULL, col_worker, (void *) worker_params[i]);
    }
    
    //Create worker threads to check the 3x3 grids
    int row = 0;
    int col = 0;
    for(int i = 18; i < 27; i++){
       if (row == 9) {
            row = 0;
            col += 3;
        }
            worker_params[i] = (parameters*)malloc(sizeof(parameters));
            worker_params[i]->thread_no = i;
            worker_params[i]->x = row;
            worker_params[i]->y = col;
            printf("Thread number %d is checking subgrid (%d,%d).\n", worker_params[i]->thread_no, row, col);
            pthread_create(&workers[i], NULL, subgrid_worker, (void *)worker_params[i]);
            row += 3;
    }

    // Integrates results produced by worker threads.
    int grid_col = 0;
    int grid_row = 0;
    col = 1;
    for(int i = 1; i < NUMBER_OF_THREADS; i++ ){
        pthread_join(workers[i], (void **) worker_params[i]);
        //Show user the portion of the puzzle was succesful  or unsucessful
        if (grid_row == 9) {
            grid_row = 0;
            grid_col +=3;
        }
        if(i<9){
            printf("Row %d returned with status %d.\n", i, status_map[i]); 
        } else if( i >= 9 && i < 18){
            printf("Column %d returned with status %d.\n", col, status_map[i]);
            col++;            
        } else {
            printf("The 3x3 grid (%d,%d) returned with status %d.\n", grid_row, grid_col, status_map[i]); 
            grid_row += 3;
        }
    }


    //show puzzle solution
    for(int i = 0; i < NUMBER_OF_THREADS; i++){
        if(status_map[i] == 0){
            printf("\n********************************************************************\n");
            printf("\t\tSorry, that sudoku puzzle is invalid.\n");
            printf("\n********************************************************************\n");
            exit(1);
        }
    }
    printf("\n********************************************************************\n");
    printf("\t\t\tSudoku puzzle is correct.\n");
    printf("\n********************************************************************\n");

    // Allocates and frees memory whenever necessary.
    for(int i = 0; i < NUMBER_OF_THREADS; i++){
        free((void**) worker_params[i]);
    }

    return 0;
}	
