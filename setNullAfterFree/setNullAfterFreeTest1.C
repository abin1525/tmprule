#include <stdlib.h>
#include <malloc.h>

int main(void) {
	int *x = (int *) malloc(sizeof(int));
	if (NULL != x) {
		*x = 1;
		free(x);
		x = NULL; //OK
	} else {
		free(x); //Error
		return (-1);
	}
	return 0;
}
