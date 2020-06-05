#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

/* Nazvy symbolov (len pre ich jednoduchsi vypis) */
const char *SYM_NAMES[] = {
        [VALUE] = "VALUE", [ID] = "ID", [READ] = "READ", [PRINT] = "PRINT", [LOGIC] = "LOGIC",
        [PLUS] = "PLUS", [MINUS] = "MINUS", [MUL] = "MUL", [DIV] = "DIV", [POWER] = "POWER",
        [LPAR] = "LPAR", [RPAR] = "RPAR", [COMMA] = "COMMA",
        [SEOF] = "SEOF", [SERROR] = "SERROR",
        [IF] = "IF", [THEN] = "THEN", [ELSE] = "ELSE", [WHILE] = "WHILE",
        [VAR] = "VAR", [SET] = "SET", [SEMCOL] = "SEMCOL", [EQUAL] = "EQUAL",
        [LT] = "LESS", [GT] = "GREATER", [SETVAL] = "SETVAL", [LTEQUAL] = "LESSEQUAL", [GTEQUAL] = "GREATEREQUAL",
        [NOT] = "NOT",[AND]= "AND",[OR] = "OR"
};

/* Globalne premenne, "public" */
Symbol lex_symbol;
int lex_attr;

char *lex_ids[LEX_IDS_MAX];
int lex_ids_size; // Pocet ulozenych identifikatorov


/* Vstupne premenne */
static char *input;     // Vstupny retazec
static char c;          // Spracovavany vstupny znak
static int ic;          // Index dalsieho znaku vo vstupnom retazci


/* Inicializacia lex. analyzatora. Parametrom je vstupny retazec. */
void init_lexer(char *string)
{
    input = string;
    ic = 0;
    lex_ids_size = 0;
}


/* Ulozenie identifikatora `id` do tabulky identifikatorov ak tam este nie je.
 * Vracia index, na ktorom je identifikator ulozeny. */
int store_id(char *id) {
    int i = 0;
    while (i < lex_ids_size) {
        if (strcmp(id, lex_ids[i]) == 0)
            return i;
        i++;
    }
    lex_ids[i] = strdup(id);
    lex_ids_size++;
    return i;
}


/* Precitanie dalsieho symbolu.
 * Volanie nastavi nove hodnoty lex_symbol a lex_attr. */
void next_symbol()
{
    c = input[ic];
    ic++;
    while (isspace(c)) { // Preskocenie medzier
        c = input[ic];
        ic++;
    }
    switch (c) {
        case ';': lex_symbol = SEMCOL; break;
        case ',': lex_symbol = COMMA;  break;
        case '+': lex_symbol = PLUS;   break;
        case '-': lex_symbol = MINUS;  break;
        case '*': lex_symbol = MUL;    break;
        case '/': lex_symbol = DIV;    break;
        case '^': lex_symbol = POWER;  break;
        case '(': lex_symbol = LPAR;   break;
        case ')': lex_symbol = RPAR;   break;
        case '&':
            if (input[ic] == '&'){
                lex_symbol = AND;
            }else lex_symbol = SERROR;
            ic++;
            break;
        case '|':
            if (input[ic] == '|') {
                lex_symbol = OR;
            } else lex_symbol = SERROR;
            ic++;
            break;

        case '>':
            if(input[ic] == '='){ //>=
                ic++;
                lex_symbol = GTEQUAL;
            }
            else{
                lex_symbol = GT;
            }    
            break;
        case '<':
            if(input[ic] == '='){ //<=
                ic++;
                lex_symbol = LTEQUAL;
            }
            else if(input[ic] == '>'){ //<>
                ic++;
                lex_symbol = NOT;
            }
            else{
                lex_symbol = LT;
            }
            break;
        case ':':
            if(input[ic] == '='){ //>=
                ic++;
                lex_symbol = SETVAL;
            } else lex_symbol = SERROR;
            break;
        case '=': lex_symbol = EQUAL; break;
        case '\0': lex_symbol = SEOF; break; // Koniec retazce
        default:
            if (isdigit(c)) {
                int id_start = ic - 1; // Index zaciatku identifikatora
                do {
                    c = input[ic];
                    ic++;
                } while (isalnum(c));
                ic--;
                int id_len = ic - id_start;
                char *id = (char*)calloc(id_len + 1, sizeof(char));
                memcpy(id, &input[id_start], id_len);
                id[id_len] = 0;
                lex_attr = atoi(id);
                lex_symbol = VALUE;
                free(id);
            } else if (isalpha(c)) {
                int id_start = ic - 1; // Index zaciatku identifikatora
                do {
                    c = input[ic];
                    ic++;
                } while (isalnum(c));
                ic--; // "Vratenie" posledneho znaku
                // Skopirovanie identifikatora
                // char *id = strndup(&input[id_start], ic - id_start);
                int id_len = ic - id_start;
                char *id = malloc(id_len + 1);
                memcpy(id, &input[id_start], id_len);
                id[id_len] = 0;
                // Kontrola klucovych slov
                if (strcmp(id, "read") == 0){
                    lex_symbol = READ;
                } else if (strcmp(id, "logic") == 0) {
                    lex_symbol = LOGIC;
                } else if (strcmp(id, "print") == 0) {
                    lex_symbol = PRINT;
                } else if (strcmp(id, "if") == 0) {
                        lex_symbol = IF;
                } else if (strcmp(id, "then") == 0){
                    lex_symbol = THEN;
                } else if (strcmp(id, "else") == 0) {
                    lex_symbol = ELSE;
                } else if (strcmp(id, "while") == 0) {
                    lex_symbol = WHILE;
                } else if (strcmp(id, "var") == 0) {
                    lex_symbol = VAR;
                } else if (strcmp(id, "set") == 0) {
                    lex_symbol = SET;
                }
                else { // Ulozenie do tabulky identifikatorov
                    lex_attr = store_id(id);
                    lex_symbol = ID;
                }
                free(id);
            } else {
                lex_symbol = SERROR;
            }
    }
}


/* Nazov lexikalnej jednotky */
const char *symbol_name(Symbol symbol)
{
    return SYM_NAMES[symbol];
}


/* Vypis vsetky lexikalnych jednotiek zo vstupu */
void print_tokens()
{
    printf("\nOutput of lexical analysis (string of symbols)\n");
    do {
        next_symbol();
        printf("  [%2d] %s", lex_symbol, symbol_name(lex_symbol));
        if (lex_symbol == VALUE) printf(" <%d>", lex_attr);
        if (lex_symbol == ID) printf(" <%d> -> %s", lex_attr, lex_ids[lex_attr]);
        printf("\n");
    } while (lex_symbol != SEOF);
}
