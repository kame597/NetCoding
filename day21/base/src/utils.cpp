#include "utils.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>

void errif(bool condition, const char *errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}