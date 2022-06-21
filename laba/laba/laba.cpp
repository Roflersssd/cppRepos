#include <stdio.h>

int main() {
	FILE* f = fopen("input.txt", "r");
	char str[100];
	while (!fgets(str, sizeof(str), f)) {
		printf("%s", str);
	}
}