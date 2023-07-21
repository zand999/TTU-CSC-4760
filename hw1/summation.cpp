#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]){

    //std::cout << argv[1];
    int num = atoi(argv[1]), sum = 0;

    
    for(int i = 1; i <=  num; i++){

        sum += i;
        std::cout << sum << "\n";

    }


}