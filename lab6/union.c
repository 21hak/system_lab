#include <stdio.h>
typedef union Node {
	int i;
	char c;
	double d;
} Node;

int main (void) {
	Node node;
	node.i = 97 + 65536;
	printf("%c", node.c);
	return 0;
}

