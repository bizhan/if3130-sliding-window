#include <stdio.h>
#include <stdlib.h>

int main(){
	char* p1 = malloc(100*sizeof(char));
	int n = 0;
	for(char i='a'; i<='z'; i++)
		p1[n++] = i;
	printf("%s\n", p1);
	return 0;
}