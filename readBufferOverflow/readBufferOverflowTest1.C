#include <stdlib.h>

int main() {
  char* p = (char*)malloc(100);
  char c = p[110]; // WRONG
  p[99] = 'c'; // OK
}
