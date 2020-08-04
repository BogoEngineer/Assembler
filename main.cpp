#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "FileManager.h"
#include "Assembler.h"

using namespace std;

int main(int argc, char *argv[]){
    if (argc != 3) {
        std::cout << "ERROR: Number of given parameters must be 3.\n" << endl;
        return -1;
    }
    Assembler* assembler = new Assembler(argv[1], argv[2]);
    assembler->start();
    return 0;
}