#include "sos.h"
//
// S T R I N G   O P T I O N S
//
//
//
//

int cmpstr(char* bufferA,char* bufferB,int size){
        for(int i = 0 ; i < size ; i++){
                if(bufferA[i]!=bufferB[i]){
                        return 0;
                }
        }
        return 1;
}

unsigned int strlen(char* buffer){
        int i = 0;
        char deze;
        while((deze = buffer[i++])!=0x00){}
        return i;
}

