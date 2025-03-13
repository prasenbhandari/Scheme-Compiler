#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define TOTAL_TOKEN 1024

int token_pointer = 0;


typedef enum{
	TOKEN_LPAREN,
	TOKEN_RPAREN,
    TOKEN_EQ,
	TOKEN_LITERAL,
    TOKEN_DEC,
    TOKEN_REAL,
    TOKEN_STR_LITERAL,
    TOKEN_ID,
    TOKEN_IF,
    TOKEN_DEFINE,
    TOKEN_DISPLAY,
    TOKEN_LAMBDA,
    TOKEN_LET,
    TOKEN_CDR,
    TOKEN_CAR,
    TOKEN_COND,

} token_type;


typedef struct{
    union {
        char* lexeme;\
        int int_value;
        double real_value;
    };
	token_type type;
} token;


token* tokens[TOTAL_TOKEN];
int buffer_pointer = 0;
char buffer[BUFFER_SIZE];


token* create_token(const char* lexeme, token_type type){
	token* temp = malloc(sizeof(token));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for token\n");
        exit(1);
    }

    temp->type = type;

    if(type == TOKEN_DEC){
        temp->int_value = atoi(lexeme);
    }else if(type == TOKEN_REAL){
        temp->real_value = atof(lexeme);
    }else{
        temp->lexeme = strdup(lexeme);
    }
    return temp;
}


token* try_keywords(const char* lexeme){

    if(lexeme[0] == 'i'){
        if(lexeme[1] == 'f'){
            return create_token(lexeme, TOKEN_IF);
        }
    }else if (lexeme[0] == 'd') {
        if(strncmp(lexeme + 1, "efine", 5)){
            return create_token(lexeme, TOKEN_DEFINE);
        }else if (strncmp(lexeme + 1, "isplay", 6)){
            return create_token(lexeme, TOKEN_DISPLAY);
        }
    }else if(lexeme[0] == 'l'){
        if(strncmp(lexeme + 1, "ambda", 5)){
            return create_token(lexeme, TOKEN_LAMBDA);
        }else if (strncmp(lexeme + 1, "et", 2)) {
            return create_token(lexeme, TOKEN_LAMBDA);
        }
    }else if(lexeme[0] == 'c'){
        if(strncmp(lexeme + 1, "dr", 2)){
            return create_token(lexeme, TOKEN_CDR);
        }else if (strncmp(lexeme + 1, "ar", 2)) {
            return create_token(lexeme, TOKEN_CDR);
        }else if(strncmp(lexeme + 1, "ond", 3)){
            return create_token(lexeme, TOKEN_COND);
        }
    }else{
        return create_token(lexeme, TOKEN_ID);
    }
}


token* next_token(FILE* file){


    if (buffer[buffer_pointer] == '\0' || buffer[buffer_pointer] == '\n'){
        if(fgets(buffer, sizeof(buffer), file) == NULL){
            return NULL;
        }
        buffer_pointer = 0;
    }

    while(isspace(buffer[buffer_pointer])){
        buffer_pointer++;
    }


    if (buffer[buffer_pointer] == '(') {
        buffer_pointer++;
        return create_token("(", TOKEN_LPAREN);
    }else if (buffer[buffer_pointer] == ')'){
        buffer_pointer++;
        return create_token(")", TOKEN_RPAREN);
    }if(buffer[buffer_pointer] == '='){
        buffer_pointer++;
    }


    if (isdigit(buffer[buffer_pointer])){
        int start = buffer_pointer;
        token_type type = TOKEN_DEC;

        while(isdigit(buffer[buffer_pointer])){
            buffer_pointer++;
        }

        if(buffer[buffer_pointer] == '.'){
            buffer_pointer++;
            type = TOKEN_REAL;
            while(isdigit(buffer[buffer_pointer])){
                buffer_pointer++;
            }
        }

        int length = buffer_pointer - start;
        char* number = (char*)malloc(length + 1);
        strncpy(number, &buffer[start], length);
        number[length] = '\0';

        token* t = create_token(number, type);
        free(number);
        return t;
    }


    if(buffer[buffer_pointer] == '"'){
        buffer_pointer++;
        int start = buffer_pointer;

        while(buffer[buffer_pointer] != '"' && buffer[buffer_pointer] != '\0'){
            buffer_pointer++;
        }

        if(buffer[buffer_pointer] == '\0'){
            return NULL;
        }

        int length = buffer_pointer - start;
        char* string = (char*)malloc(length + 1);
        strncpy(string, &buffer[start], length);
        string[length] = '\0';
        buffer_pointer++;

        token* t = create_token(string, TOKEN_STR_LITERAL);
        free(string);
        return t;
    }


    if(isalpha(buffer[buffer_pointer])){
        int start = buffer_pointer;

        while(isalnum(buffer[buffer_pointer])){
            buffer_pointer++;
        }
        
        int length = buffer_pointer - start;
        char* string = (char*)malloc(length + 1);
        strncpy(string, &buffer[start], length);
        string[length] = '\0';

        token* t = try_keywords(string);
        free(string);
        return t;
    }

    /*if(isalnum(buffer[buffer_pointer])){*/
    /**/
    /*}*/

    return NULL;
}


int main(){
    FILE* file = fopen("test.scm", "r");
    if(!file){
        printf("Failed to open file\n");
        return 1;
    }

    token* t;
    while ((t = next_token(file)) != NULL && buffer[buffer_pointer] != EOF) {
        
        if (t->type == TOKEN_DEC) {
            printf("Token: %d, Type: %d\n", t->int_value, t->type);
        } else if (t->type == TOKEN_REAL) {
            printf("Token: %f, Type: %d\n", t->real_value, t->type);
        } else {
            printf("Token: %s, Type: %d\n", t->lexeme, t->type);
        }

        free(t->lexeme);  
        free(t);          
    }

    fclose(file);
    return 0;

}
