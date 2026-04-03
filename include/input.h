#ifndef INPUT_H
#define INPUT_H

#include <iostream>

using std::cin;

void ignoreLine();
bool clearFailedExtract();

template<typename T>
T getVal(){
    T x;
    while(true){
        cin >> x;
        if (!clearFailedExtract()){
            ignoreLine();
            break;
        }
    }
    return x;
}

#endif