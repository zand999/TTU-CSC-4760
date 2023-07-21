#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

struct Cell{

    int x;
    int y;
    double temp;

};

//take filename and return array of heater location data and temperature
Cell* parseHeaterCells(string filename){

    ifstream get;
    get.open(filename);
    string input;
    if(get.is_open()){
        
        getline(get , input, '\n');
        cout << "Heaters: " << input << "\n";
        int entrynum = stoi(input);
        Cell* list = new Cell[entrynum];
        for(int i = 0; i < entrynum; i++){

            
            getline(get , input, ' ');
            list[i].x = stoi(input);
            getline(get , input, ' ');
            list[i].y = stoi(input);
            getline(get , input, '\n');
            list[i].temp = stod(input);

            //cout << "\n" << list[i].x << " " << list[i].y<< " " << list[i].temp<< " " << list[i].heater;
            
        }//cout << "\n";
        get.close();
        return list;
    }else{
        cout << "Invalid Filename Exiting...\n";
        exit(1);
    }
    return NULL;
}

//helper function to print grid
void printGrid(double **grid,int numRows, int numCols){
    for(int i = 0; i < numRows; i++){
        for(int j = 0; j< numCols; j++){
            cout << grid[i][j]<< "\t";
        }
        cout << "\n";
    }

}

void converttocsv(string filename, double **grid,int numRows, int numCols){
    //convert 2d array to csv
    ofstream fout;
    fout.open(filename);
    if(fout.is_open()){
        for(int i = 0; i < numRows; i++){
            for(int j = 0; j< numCols; j++){
                
                if(j+1 == numCols){
                    fout << grid[i][j]<< "\n";
                }else{
                    fout << grid[i][j]<< ",";
                }
                
            }
        }
    }else{

        cout << "Failed to create and export to " << filename << " \n exiting...\n";

    }
    
    fout.close();

}

bool isHeater(Cell *heaters,int num, int row, int col){

    //check if point on grid is heater
    for(int i = 0 ; i < num ;i++){

        if(heaters[i].x == row && heaters[i].y == col){
            return true;
        }

    }

    return false;

}

double calculateTemp(double **grid, int maxrow, int maxcol, int row, int col, double k, double base_temp){
    //Tnew = (Told + (k*(Ttop + Tbottom + Tleft + Tright + Ttop_left + Tbottom_left +
    // Ttop_right + Tbottom_right))/8.0)/2.0
    double oldtemp = grid[row][col];
    double top=base_temp, bottom=base_temp, left=base_temp, right=base_temp, top_left=base_temp;
    double bottom_left=base_temp, top_right=base_temp, bottom_right=base_temp;

    //checking each cell bounds prob better way but dont care
    // lol it would be faster to just assign values without bounds checking and then bounds chack at the end to
    // fix them, nevermind assigning values outside of the scope of the matrix ;)
    //also would be faster to have grid 2 sizes larger and fill it
    int cellx, celly;
    cellx = row - 1;
    celly = col - 1;
    
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        top_left = grid[cellx][celly];
    }
    cellx = row-1;
    celly = col;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        top = grid[cellx][celly];
    }
    cellx = row - 1;
    celly = col + 1;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        top_right = grid[cellx][celly];
    }

    cellx = row;
    celly = col-1;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        left = grid[cellx][celly];
    }
    cellx = row;
    celly = col+1;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        right = grid[cellx][celly];
    }

    cellx = row + 1;
    celly = col-1;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        bottom_left = grid[cellx][celly];
    }
    cellx = row + 1;
    celly = col;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        bottom = grid[cellx][celly];
    }
    cellx = row + 1;
    celly = col + 1;
    if(cellx >= 0 && cellx < maxrow && celly >= 0 && celly < maxcol){
        bottom_right = grid[cellx][celly];
    }
    
    //return calculation
    return (double)(oldtemp + (k* (top + bottom + left + right + top_left + bottom_left +top_right + bottom_right)) / 8.0) / 2.0;

}


int main(int argc, char* argv[])
{

    
    //./heat_transfer  <num_threads> <numRows> <numCols> <base temp> <k> <timesteps> <heaterFileName> <outputFileName>
	//declare variables
    double truntimes = omp_get_wtime();
    cout << "INPUT(" << argc << "): ";
	int num_threads = atoi(argv[1]), numRows = atoi(argv[2]), numCols = atoi(argv[3]);
    double base_temp = atoi(argv[4]), k = stod(argv[5]);
    int timesteps = atoi(argv[6]);
    string heaterFileName = argv[7], outputFileName = argv[8];
	cout << num_threads <<", " << numRows << ", " << numCols << ", " << base_temp << ", " << k << ", " << timesteps << ", " << heaterFileName << ", " << outputFileName <<"\n";
    omp_set_num_threads(num_threads);
    int numofheaters = 0;
    string input;


    double **grid = new double*[numRows];//not memory nice but whatever
    double **gridbuff = new double*[numRows];

    //pointers to arrays
    double **gridptr = grid;
    double **gridbuffptr = gridbuff;
    double **temp;

    //initlize grid with data
    for(int i = 0; i < numRows; i++){
        grid[i] = new double[numCols];

        #pragma omp parallel
        {
            #pragma omp parallel for 
                //fill with defualt values
                for(int j = 0; j< numCols; j++){
                    grid[i][j] = base_temp;
                }
        }
    }
    for(int i = 0; i < numRows; i++){
        gridbuff[i] = new double[numCols];
    }

    //get num of heaters from file
    ifstream get;
    get.open(heaterFileName);
    if(get.is_open()){
        getline(get , input, '\n');
        numofheaters = stoi(input);
        get.close();
    }else{
        cout << "Invalid Filename Exiting...\n";
        exit(1);
    }

    //initlize heaters
    Cell *heaters = parseHeaterCells(heaterFileName);
    //print heater file to show parsed properly
    for(int i = 0 ; i < numofheaters ;i++)
        cout << heaters[i].x << " " << heaters[i].y<< " " << heaters[i].temp << "\n";
    
    //apply heater data to grid small enough that does not need to be parallel
    for(int i = 0; i < numofheaters; i++){
        //bounds checking
        if(0 <= heaters[i].x && heaters[i].x < numRows && 0 <= heaters[i].y && heaters[i].y < numCols){
            grid[heaters[i].x][heaters[i].y] = heaters[i].temp;
        }else{
            cout << "Invalid Heater Cords! Location: " << heaters[i].x << "," << heaters[i].y << " Gridsize:" << numRows << ", " << numCols;
            cout << "\nExiting...\n";
            
            exit(1);
        }
    }
	
    
    
    //run for timesteps 
    cout << endl;
    for(int i = 0; i < timesteps; i++){
        //parallize temp update
        #pragma omp parallel
        {
            
            //cout << "Thread "<<omp_get_thread_num() <<": ";
            #pragma omp parallel for collapse(2)
                for(int i = 0; i < numRows; i++){
                    for(int j=0; j < numCols; j++){

                        //first check if location to not change
                        if(!isHeater(heaters,numofheaters,i,j)){
                            //do math + update value to buffer
                            gridbuffptr[i][j] = calculateTemp(gridptr, numRows, numCols, i,j,k, base_temp);


                        }else{
                            gridbuffptr[i][j] = gridptr[i][j];
                        }
                        

                    }
                }
        }
        //swap buffer and original grid
        temp = gridbuffptr;
        gridbuffptr = gridptr;
        gridptr = temp;

        //progress report
        cout << i << "/" << timesteps << "\n";
    }

    //print to csv
	converttocsv(outputFileName,gridptr,numRows,numCols);
    printf("Runtime: %f\n", omp_get_wtime()- truntimes);
}