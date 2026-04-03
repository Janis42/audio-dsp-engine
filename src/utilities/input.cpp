#include <iostream>
#include <limits>

using std::cin;

void ignoreLine(){
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool clearFailedExtract(){
    if (!cin){
        if (cin.eof()){
            std::exit(0);
        }
        cin.clear();
        ignoreLine();
        return true;
    }
    return false;
}