#include <stdio.h>
#include <string.h>

#define TABLE_SIZE 7 // Number of keywords
#define ALPHABETS 26
#define MAX_ATTEMPTS 5

const char* keywords[TABLE_SIZE] = {"if", "define", "lambda", "begin", "quote", "display", "newline"};
int letter_frequency[ALPHABETS] = {0};
int g[ALPHABETS] = {-1};
const char* hash_table[TABLE_SIZE] = {NULL};         

void compute_frequency(){
    for (int i = 0; i < TABLE_SIZE; i++) {
	int first_letter = keywords[i][0] - 'a';
	int last_letter = keywords[i][strlen(keywords[i]) - 1] - 'a';

	if (first_letter >= 0 && first_letter < ALPHABETS) {
            letter_frequency[first_letter]++;
        }
        if (last_letter >= 0 && last_letter < ALPHABETS) {
            letter_frequency[last_letter]++;
        }    }

    for (int i = 0; i < ALPHABETS; i++) {
	printf("%d\n", letter_frequency[i]);
    }
}

void sort_keywords_by_frequency() {
    for (int i = 0; i < TABLE_SIZE - 1; i++) {
        for (int j = i + 1; j < TABLE_SIZE; j++) {
            int first_i = keywords[i][0] - 'a';
            int last_i = keywords[i][strlen(keywords[i]) - 1] - 'a';
            int first_j = keywords[j][0] - 'a';
            int last_j = keywords[j][strlen(keywords[j]) - 1] - 'a';

            int score_i = letter_frequency[first_i] + letter_frequency[last_i];
            int score_j = letter_frequency[first_j] + letter_frequency[last_j];

            if (score_j > score_i) {
                const char *temp = keywords[i];
                keywords[i] = keywords[j];
                keywords[j] = temp;
            }
        }
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
	      printf("%s\n", keywords[i]);
    }

}

int compute_hash(const char* keyword, int first_letter, int last_letter){
    int length = strlen(keyword);

    return (length + first_letter + last_letter) % TABLE_SIZE;
}

int has_collision(const char* keyword, int hash_val){
    if(hash_table[hash_val] == NULL){
        return 0;
    }
    

    return (strcmp(hash_table[hash_val], keyword) != 0);
}

int assign_hash(int index){
    if (index == TABLE_SIZE){
        return 1;
    }

    const char *keyword = keywords[index];
    int first_letter = keyword[0] - 'a';
    int last_letter = keyword[strlen(keyword) - 1] - 'a';
   
    for (int i = 0; i < MAX_ATTEMPTS; i++){
        for(int j = 0; j < MAX_ATTEMPTS; j++){
            g[first_letter] = i;
            g[last_letter] = j;

            int hash_value = compute_hash(keyword, i, j);

            if(!has_collision(keyword, hash_value)){
                hash_table[hash_value] = keyword;
                if (assign_hash(index + 1)){
                    return 1;
                }
                hash_table[hash_value] = NULL;
            }

        }
    }
    
    return 0;
}

int look_up(const char* keyword){
	int first_letter = keyword[0] - 'a';
    int last_letter = keyword[strlen(keyword) - 1] - 'a';

	for (int i = g[first_letter]; i >= 0; i--) {
		for (int j = g[last_letter]; j >= 0; j--) {
			int hash_val = compute_hash(keyword, i, j);

			if(strcmp(hash_table[hash_val], keyword) == 0){
				return 1;
			}
		}
	}
    return 0;
}

int main(){
    compute_frequency();
    sort_keywords_by_frequency();
    

    if (assign_hash(0)) {
        printf("\nPerfect hash table generated successfully:\n");
        for (int i = 0; i < TABLE_SIZE; i++) {
            printf("[%d] = %s\n", i, hash_table[i] ? hash_table[i] : "(empty)");
        }
    } else {
        printf("Failed to generate a perfect hash table.\n");
    }

	const char* test_words[10] = {"if", "define", "hello", "lambda", "begin", "world", "quote", "display", "hi", "newline"};
	for (int i = 0; i < 10; i++) {
		if(look_up(test_words[i])){
			printf("{keyword = %s} found \n", test_words[i]);
		}else{
			printf("%s is not a keyword \n", test_words[i]);
		}
	}
}
