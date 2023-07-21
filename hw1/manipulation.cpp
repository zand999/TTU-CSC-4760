#include <iostream>
#include <string>
using namespace std;

void printarray(string name, int *array);

int main(){

    
    int array1[100], array2[100];
    

    
    for(int i=1 ,j =0;i<=100;i++, j++){

        array1[j] = i*2;
        array2[j] = i*3;

    }
    
    int *arrayptr1 =  array1, *arrayptr2 = array2, *temp;
    printarray("Arrayptr1",arrayptr1);
    printarray("Arrayptr2",arrayptr2);
    cout << "\n";

    temp = arrayptr1;
    arrayptr1 = arrayptr2;
    arrayptr2 = temp;

    printarray("Arrayptr1",arrayptr1);
    printarray("Arrayptr2",arrayptr2);  


}


void printarray(string name, int *array){

    cout << name << ": {";
    for(int i = 0; i < 99 ; i++){
        cout << *(array+i )<< ",";
    }
    cout << array[99] << "}\n";

}