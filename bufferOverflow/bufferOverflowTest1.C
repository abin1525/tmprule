#include <stdlib.h>

int main() {
  char* p = (char*)malloc(100);
  int i;
  for(i=0;i<10;++i)
    p[222] = 333;
}
