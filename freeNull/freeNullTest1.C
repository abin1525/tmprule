#include <stdlib.h>

void func(void* p){
    free(p);
    *(char*)p = 'a';
}

int main(){
    void* p = malloc(100);
    func(p);
    return 0;
}
