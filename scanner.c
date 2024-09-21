#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define buffer_size 1024

int main() {
	FILE *file = fopen("test.scm", "r");
	char buffer[buffer_size];

	if (file == NULL) {
		printf("Error opening file");
		return 1;
	}

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		int i = 0;
		while ((buffer[i] != '\0') && i < buffer_size) {

			if (buffer[i] == '(') {
				printf("{%c: LPAREN}\n", buffer[i]);
			} else if (buffer[i] == ')') {
				printf("{%c:RPAREN}\n", buffer[i]);
			} else if (isdigit(buffer[i])) {
				int num = (buffer[i] - '0');
				i++;

				while (i < buffer_size && isdigit(buffer[i])) {
					num = num * 10 + (buffer[i] - '0');
					i++;
				}
				printf("{%d: LITERAL}\n", num);
			} else if (isalpha(buffer[i])) {
				char lexeme[50];
				int j = 0;

				while (i < buffer_size && isalnum(buffer[i])) {
					lexeme[j++] = buffer[i++];
				}
				lexeme[j] = '\0';
				i--;
			}

			i++;
		}
	}
}
