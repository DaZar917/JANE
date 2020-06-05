#ifndef LEXER_H
#define LEXER_H

/* Velkost tabulky identifikatorov */
#define LEX_IDS_MAX 20

/* Typy symbolov - lexikalnych jednotiek */
typedef enum {
    VALUE, ID, READ, PRINT,LOGIC, PLUS, MINUS, MUL, DIV, POWER, LPAR, RPAR, COMMA,
    SERROR, IF, THEN, ELSE, WHILE, VAR, SET, LT,EQUAL, GT, SETVAL, LTEQUAL, GTEQUAL, NOT, SEMCOL, SEOF, AND, OR
} Symbol;

/* Vystupny symbol lexikalnej analyzy a jeho atribut.
 * Ak symbol je VALUE, atribut obsahuje jeho celociselnu hodnotu,
 * ak je to ID, atribut obsahuje index do tabulky identifikatorov. */
extern Symbol lex_symbol;
extern int lex_attr;

/* Tabulka identifikatorov */
extern char *lex_ids[LEX_IDS_MAX];
extern int lex_ids_size; // Pocet ulozenych identifikatorov

/* Inicializacia lex. analyzatora. Parametrom je vstupny retazec. */
void init_lexer(char *string);

/* Precitanie dalsieho symbolu.
 * Volanie nastavi nove hodnoty lex_symbol a lex_attr. */
void next_symbol();

/* Vypis vsetky lexikalnych jednotiek zo vstupu */
void print_tokens();

/* Nazov lexikalnej jednotky */
const char *symbol_name(Symbol symbol);

#endif /* LEXER_H */

