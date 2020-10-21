#include <stdio.h>
typedef struct Node {
	int i;
	char c;
	int j;
} Node;

int main(void) {
	Node node;
	node.i = 1;
	node.c = 'a';
	node.j = 2;
	printf("%d", node.i);
	return 0;
}
