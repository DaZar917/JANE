#include <string.h>

#include "generator.h"

#define WORK_MEM (short)2
#define VAR_OFFSET (short)3
#define MAX_CODE_LENGTH 20000
#define STACK_START 20000

enum OP_Code {
    NOP, BZE, JMP, JSR, RTS, EXIT,
    INPC, INP, INPR, OUTC, OUT, OUTR,
    POP, POPR, PUSH, PUSHR,
    LDA, LDAM, LDAI, LDAX,
    LDR, LDRI, STA,
    STAI, STRI,
    LDX, STX, LDS, STS,
    OR, AND, NOT,
    EQ, NE, LT, LE, GT, GE,
    EQR, NER, LTR, LER, GTR, GER,
    ADD, ADDM, SUB, SUBM,
    MUL, DIV, NEG,
    ADDR, SUBR, MULR, DIVR, NEGR
};

FILE *output_stream;
short *code_list;
short address;

short get_address(){
    return address;
}

void put_word(short value){
    code_list[address] = value;
    address++;
}

void put_op_attr(short op, short value){
    put_word(op);
    put_word(value);
}



typedef union {
    float R;

    struct {
        unsigned short RL;
        unsigned short RH;
    } Dword;
} RealRegister;

void put_real(float arg) {
    RealRegister rvalue;
    rvalue.R = arg;
    put_word(rvalue.Dword.RL);
    put_word(rvalue.Dword.RH);
}
void init_generator(FILE *output) {
    address = 0;
    code_list = calloc(sizeof(short), MAX_CODE_LENGTH);
    output_stream = output;
}

void generate_output(){
    fwrite(code_list, sizeof(short), (size_t) address, output_stream);
}

void write_begin(short num_vars) {
    put_op_attr(JMP,VAR_OFFSET + num_vars);
    put_word(STACK_START);
    for (int i = 0; i < num_vars; i++) {
        put_word(NOP);
    }
    put_op_attr(LDS,2);
}

void write_end(){
    put_word(EXIT);
}

void write_result() {
    put_word(POP);
    put_word(OUT);
}

void write_number(short value) {
    put_op_attr(LDAM, value);
    put_word(PUSH);
}

void write_var(short index) {
    put_op_attr(LDA, index);
    put_word(PUSH);
}

void write_add() {
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(ADD, WORK_MEM);
    put_word(PUSH);
}

void write_sub() {
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(SUB, WORK_MEM);
    put_word(PUSH);
}

void write_pow(int value){
    put_word(POP);
    put_word(POP);
    write_number(value);
}

void write_mul() {
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(MUL, WORK_MEM);
    put_word(PUSH);
}

void write_div() {
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(DIV, WORK_MEM);
    put_word(PUSH);
}

void write_ask_var(short index, char *name) {
    put_word(LDAM);
    put_word(name[0]);
    put_word(OUTC);
    put_word(LDAM);
    put_word('=');
    put_word(OUTC);
    put_word(INP);
    put_word(STA);
    put_word(index);
}

void write_EQUAL(){
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(EQ, WORK_MEM);
    put_word(PUSH);
}

void write_NOT(){
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(NE, WORK_MEM);
    put_word(PUSH);
}

void write_LT(){
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(LT, WORK_MEM);
    put_word(PUSH);
}

void write_GT(){
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(GT, WORK_MEM);
    put_word(PUSH);
}

void write_LESSEQUAL(){
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(LE, WORK_MEM);
    put_word(PUSH);
}

void write_GTEQUAL(){
    put_word(POP);
    put_op_attr(STA, WORK_MEM);
    put_word(POP);
    put_op_attr(EQ, WORK_MEM);
    put_word(PUSH);
}
