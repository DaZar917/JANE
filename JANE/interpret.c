#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "lexer.h"
#include "generator.h"

#define MAX_INPUT_SIZE 160
#define KeySet unsigned long int
#define E 1 <<

KeySet H_Term = E VALUE | E ID | E LPAR;
KeySet H_Power = E VALUE | E ID | E LPAR;
KeySet H_Mul = E VALUE | E ID | E LPAR;
KeySet H_Expr = E VALUE | E ID | E LPAR;
KeySet H_Stat = E PRINT | E READ | E SET | E VAR | E IF | E WHILE;
KeySet H_Logic = E GT | E LT | E GTEQUAL | E LTEQUAL | E EQUAL| E NOT | E AND | E OR;

int variables[LEX_IDS_MAX];
bool createdIdForVariables[LEX_IDS_MAX];
char* source;

void get_input_from_keyboard() {
    printf("Enter your code (You can type Enter): \n");
    source = (char *)calloc(1024, sizeof(char));
    char line[MAX_INPUT_SIZE];
    memset(line, 0, MAX_INPUT_SIZE);
    int rc;
    while((rc = scanf("%c", line)) != EOF) {
        if(rc == 1) {
            strcat(source, line);
        }
        line[0] = '\0';
    }
    source[strlen(source)] = '\0';
}

int exponentiation(int number, int exponentiation)  //pow int math.h
{
    int result=1;
    for (int k = exponentiation; k>0; k--){
        result = result * number;
    }
    return result;
}

void scan(int id_idx){
    int value;
    printf("%s = ", lex_ids[id_idx]);
    scanf("%d", &value);
    variables[id_idx] = value;
}

/*** Syntakticky analyzator a interpretator ***/

void error(const char *msg, KeySet K) {
    fprintf(stderr, "Ooops... %s\n", msg); /* chybové hlásenie */
    /* preskočenie nekľučových symbolov */
    while (!(E lex_symbol & K))
        next_symbol();
}
 
void check(const char *msg, KeySet K) {
    if (!(E lex_symbol & K))
        error(msg, K);
}

/* Overenie symbolu na vstupe a precitanie dalsieho.
 * Vracia atribut overeneho symbolu. */
int match(const Symbol expected, KeySet K)
{
    if (lex_symbol == expected) {
        int attr = lex_attr;
        next_symbol();
        return attr;
    } else {
        char *msg = malloc(100);
        snprintf(msg, 100, "Error (match): Expected symbol: %s, \tInstead of:  %s", symbol_name(expected), symbol_name(lex_symbol));
        error(msg, K);
        return 0;
    }
}




/***Gramatika***/
/* Term -> VALUE | "(" Expr ")" | ID
 * Power -> Term { ("^") Term }
 * Mul -> Power {("*"|"/") Power}
 * Expr -> Term {("+"|"-") Term}
 * Logic -> Expr ( LT | GT | EQUAL | GTEQUAL | LTEQUAL | NOT ) Expr

 * Print -> "print" Expr ";"
 * Print_Logic -> "logic" Expr ";"
 * Read -> "read" ID {"," ID} ";"
 * var -> "var" ID {"," ID} ";"
 * Set_var -> "set" ID ":=" Expr ";"
 * Stat -> Read | Print | Var | Set
 * Program -> { Stat }
 */

int term(KeySet K);
int power(KeySet K);
int mul(KeySet K);
int expr(KeySet K);
int logic(KeySet K);

void print(KeySet K);
void read(KeySet K);
void var(KeySet K);
void set(KeySet K);
void stat(KeySet K);
void program(KeySet K);






/* Term -> VALUE | "(" Expr ")" | ID*/
int term(KeySet K) {
    int value;
	check("Check (term): Waiting for VALUE or '(' ", K | H_Term);
    switch (lex_symbol) {
        case VALUE:
            value = lex_attr;
            write_number(value);
            next_symbol();
            break;
        case LPAR:
            next_symbol();
            value = expr(K | H_Term);
            match(RPAR, K);
            break;
        case ID:
            if(createdIdForVariables[lex_attr]){
                value = variables[lex_attr];
                write_var(lex_attr);
                next_symbol();
            }
            else {
                error("Error (term): Not declared in this scope!\n", K | H_Term);
                next_symbol();
            }
            break;
        default:
            error("Error (term): Operand error! ", K);
    }
	return value;
}

/* Power -> Term { ("^") Term } */
int power(KeySet K) {
	int leftOp, rightOp;
	Symbol operator;
	leftOp = term(K | E POWER | H_Term);
    check("Check (power): Waiting for operand: ^", E POWER | H_Term | E SEMCOL | K );
	while ((E lex_symbol) & (E POWER | H_Term))	{
		if ((E lex_symbol) & (E POWER)) {
            operator = lex_symbol;
            next_symbol();
        } else error("Error (power): Waiting for operand: ^", H_Term | K);
		rightOp = term(K | E POWER | H_Term);
		if (operator == POWER){
            leftOp = exponentiation(leftOp, rightOp);
            write_pow(leftOp);
		} else assert("Assert (power): Unexpected operator in power ()");

        check("Check (power): Waiting for operand: ^", E POWER | H_Term | E SEMCOL | K);
	}
	return leftOp;
}

/* Mul -> Power {("*"|"/") Power} */
int mul(KeySet K) {
    int leftOp, rightOp;
    Symbol operator;
    leftOp = power(K | E MUL | E DIV | H_Power);
    check("Check (mul): Waiting for multiplication ( * ) or division ( / )", E MUL | E DIV | H_Power | E SEMCOL | K);
    while((E lex_symbol) & (E MUL | E DIV | H_Power)){
        if ((E lex_symbol) & (E MUL | E DIV)) {
            operator = lex_symbol;
            next_symbol();
        } else error("Error (mul): Waiting for multiplication ( * ) or division ( / )", H_Power | K);
        rightOp = power(E MUL | E DIV | H_Power | K);
        switch (operator) {
            case MUL:
                leftOp = leftOp * rightOp;
                write_mul();
                break;
            case DIV: 
                leftOp = leftOp / rightOp;
                write_div();
                break;
            default:
                assert("Assert (mul): Unexpected operator in mul()");
        }
        check("Check (mul): Waiting for multiplication ( * ) or division ( / )", E MUL | E DIV | H_Power | E SEMCOL | K);
    }
    return leftOp;
}

/* Expr -> Mul {("+"|"-") Mul} */
int expr(KeySet K) {
    int leftOp, rightOp;
    Symbol operator;
    leftOp = mul(E PLUS | E MINUS | H_Mul | K);
    check("Check (expr): Waiting for adding ( + ) or subtraction ( - )", E PLUS | E MINUS | H_Mul | E SEMCOL | K);
    while ((E lex_symbol) & (E PLUS | E MINUS | H_Mul)){
        if ((E lex_symbol) & (E PLUS | E MINUS )) {
            operator = lex_symbol;
            next_symbol();
        } else error("Error (expr): Waiting for adding ( + ) or subtraction ( - )", H_Mul | K);
        rightOp = mul(K | E PLUS | E MINUS | H_Mul);
        switch (operator) {
            case PLUS:
                leftOp = leftOp + rightOp;
                write_add();
                break;
            case MINUS:
                leftOp = leftOp - rightOp;
                write_sub();
                break;

            default:
                assert("Assert (expr): Unexpected operator in expr()");
        }
        check("Check (expr): Waiting for adding ( + ) or subtraction ( - )", E PLUS | E MINUS | E SEMCOL| H_Mul | K);
    }
    return leftOp;
}

/* Logic -> Expr ( LT | GT | EQUAL | GTEQUAL | LTEQUAL | NOT ) Expr */
int logic(KeySet K) {
    int leftOp, rightOp;
    Symbol operator;
    leftOp = mul(H_Logic| H_Mul | K);
    check("Check (logic): Waiting for logic expression! (>,<,=,<>,>=,<=)", H_Logic| E SEMCOL | H_Mul | K);
    while ((E lex_symbol) & (H_Logic| H_Mul)){
        if ((E lex_symbol) & (H_Logic)) {
            operator = lex_symbol;
            next_symbol();
        } else error("Error (logic): Waiting for logic expression! (>,<,=,<>,>=,<=)", H_Mul | K);
        rightOp = mul(K |H_Logic| H_Mul);
        switch (operator) {
            case GT:
                leftOp = leftOp > rightOp;
                write_GT();
                break;
            case LT:
                leftOp = leftOp < rightOp;
                write_LT();
                break;
            case LTEQUAL:
                leftOp = leftOp <= rightOp;
                write_LESSEQUAL();
                break;
            case GTEQUAL:
                leftOp = leftOp >= rightOp;
                write_GTEQUAL();
                break;
            case EQUAL:
                leftOp = leftOp == rightOp;
                write_EQUAL();
                break;
            case NOT:
                leftOp = leftOp != rightOp;
                write_NOT();
                break;
            case AND:
                if (leftOp >= 1 && rightOp >= 1)
                    leftOp = 1;
                else leftOp = 0;
                break;
            case OR:
                if (leftOp == 0 && rightOp == 0)
                    leftOp = 0;
                else leftOp = 1;
                break;
            default:
                assert("Assert (logic): Unexpected operator in logic()");
        }
        check("Check (logic): Waiting for logic expression! (>,<,=,<>,>=,<=)",  E SEMCOL |H_Logic| H_Mul | K);
    }
    return leftOp;
}


/* Print -> "print" Expr ";"*/
void print(KeySet K){
	match(PRINT, H_Expr | E SEMCOL | K );
    int result = expr(H_Expr | K);
    printf("Result: %d\n", result);
    match(SEMCOL, K);
}

/* Print_logic -> "logic" Expr ";"*/
void print_Logic(KeySet K){
    match(LOGIC, H_Logic | E SEMCOL | K );
    int result = logic(H_Logic | K);
    printf("Result: %d\n", result);
    match(SEMCOL, K);
}

/* Read -> "read" ID {"," ID} ";" */
void read(KeySet K){
	next_symbol();
	match(ID, E COMMA | E ID | E SEMCOL | K);
    if(createdIdForVariables[lex_attr]) scan(lex_attr);
    else error("Error (read): Not declared in this scope!\n", K | SEMCOL);
	while((E lex_symbol) & (E COMMA))
	{
		next_symbol();
		match(ID, K | E COMMA | E ID);
		if(createdIdForVariables[lex_attr]){
		    scan(lex_attr);
		    write_ask_var(ID,"var");
		}
		else error("Error (read): Not declared in this scope!\n", K | SEMCOL);
	}
    match(SEMCOL, K);
}

/* Create -> "var" ID {"," ID} ";" */
void var(KeySet K){
    next_symbol();
    match(ID, E COMMA | E ID | E SEMCOL | K);
    if(!createdIdForVariables[lex_attr]){
        createdIdForVariables[lex_attr] = true;
        variables[lex_attr] = 0;
    } else error("Error (var): Duplication of a variable! The variable was declared earlier!\n", K | E SEMCOL);
    while((E lex_symbol) &(E COMMA)){
        next_symbol();
        match(ID, E COMMA | E ID | E SEMCOL | K);
        if(!createdIdForVariables[lex_attr]){
            createdIdForVariables[lex_attr] = true;
            variables[lex_attr] = 0;
        } 
        else error("Error (var): Duplication of a variable! The variable was declared earlier!\n", K | E SEMCOL);
    }
    match(SEMCOL, K);
}

/* Set -> "set" ID ":=" Expr ";"  */
void set(KeySet K){
    next_symbol();
    match(ID, E SETVAL | H_Expr);
    int prev_lex_attr = lex_attr;
    match(SETVAL, K | H_Expr);
    if(createdIdForVariables[prev_lex_attr]){
		variables[prev_lex_attr] = expr(K | H_Expr);

	} else error("Error (set): The variable was not declared!\n", K | E SEMCOL);
	match(SEMCOL, K);
}

/* Stat -> Read | Print | Create | Set */
void stat(KeySet K){
    switch (lex_symbol)
    {
        case READ:
            read(K | H_Stat);
            break;
        case PRINT:
            print(K | H_Stat);
            break;
        case LOGIC:
            print_Logic(K| H_Logic);
            break;
        case VAR:
            var(K | H_Stat);
            break;
        case SET:
            set(K | H_Stat);
            break;
        default:
            error("Error (stat): Expected some key word!", K);
    }
}

/* Program -> { Stat }*/
void program(KeySet K)
{
	while(lex_symbol != SEOF)
	{
		stat(K | H_Stat);
	}
}



int main(int argc, char** argv) {
    FILE *output_file = fopen("program.bin", "wb");
    init_generator(output_file);

    get_input_from_keyboard();
    write_begin(lex_ids_size);
    printf("Enter your code: ");
    init_lexer(source);
    print_tokens();
    printf("\nStarting the interpretation caused by syntax\n\n");
    init_lexer(source);
    next_symbol();
    program(E SEOF);

    write_result();
    write_end();

    generate_output();
    fclose(output_file);
    return 0;
}