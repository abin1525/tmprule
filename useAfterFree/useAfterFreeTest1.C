#include <stdlib.h>
void func(void* p){
    free(p);
    p = malloc(100);
    *(char*)p = 'a';
}
int main(){
    void* p = malloc(100);
    func(p);
    return 0;
}
