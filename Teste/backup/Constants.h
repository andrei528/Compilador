#ifndef CONSTANTS_H
#define CONSTANTS_H

enum TokenId 
{
    EPSILON  = 0,
    DOLLAR   = 1,
    t_ID = 2,
    t_PR_VOID = 3,
    t_PR_IF = 4,
    t_PR_ELSE = 5,
    t_PR_INT = 6,
    t_PR_RETURN = 7,
    t_PR_COUT = 8,
    t_PR_CIN = 9,
    t_PR_FLOAT = 10,
    t_PR_CHAR = 11,
    t_PR_BOOL = 12,
    t_PR_STRING = 13,
    t_PR_FALSE = 14,
    t_PR_TRUE = 15,
    t_PR_WHILE = 16,
    t_PR_FOR = 17,
    t_PR_DO = 18,
    t_COMENTARIO = 19,
    t_COMENTARIO_BLOCO = 20,
    t_LIT_INT = 21,
    t_LIT_BIN = 22,
    t_LIT_HEX = 23,
    t_LIT_FLOAT = 24,
    t_LIT_CHAR = 25,
    t_LIT_STRING = 26,
    t_OP_INC = 27,
    t_OP_DEC = 28,
    t_OP_SUM = 29,
    t_OP_SUB = 30,
    t_OP_MULT = 31,
    t_OP_DIV = 32,
    t_OP_MOD = 33,
    t_OP_MAIOR = 34,
    t_OP_MENOR = 35,
    t_OP_MAIOR_IG = 36,
    t_OP_MENOR_IG = 37,
    t_OP_IG = 38,
    t_OP_DIF = 39,
    t_OP_AND = 40,
    t_OP_OR = 41,
    t_OP_NOT = 42,
    t_OP_BIT_SLL = 43,
    t_OP_BIT_SRL = 44,
    t_OP_BIT_AND = 45,
    t_OP_BIT_OR = 46,
    t_OP_BIT_COMP = 47,
    t_OP_BIT_XOR = 48,
    t_ATRIB = 49,
    t_P_VIRGULA = 50,
    t_PONTO = 51,
    t_VIRGULA = 52,
    t_DOIS_PONTOS = 53,
    t_ABRE_PAR = 54,
    t_FECHA_PAR = 55,
    t_ABRE_COL = 56,
    t_FECHA_COL = 57,
    t_ABRE_CHA = 58,
    t_FECHA_CHA = 59
};

const int STATES_COUNT = 56;

extern int SCANNER_TABLE[STATES_COUNT][256];

extern int TOKEN_STATE[STATES_COUNT];

extern int SPECIAL_CASES_INDEXES[61];

extern const char *SPECIAL_CASES_KEYS[16];

extern int SPECIAL_CASES_VALUES[16];

extern const char *SCANNER_ERROR[STATES_COUNT];

const int FIRST_SEMANTIC_ACTION = 114;

const int SHIFT  = 0;
const int REDUCE = 1;
const int ACTION = 2;
const int ACCEPT = 3;
const int GO_TO  = 4;
const int ERROR  = 5;

extern const int PARSER_TABLE[265][144][2];

extern const int PRODUCTIONS[122][2];

extern const char *PARSER_ERROR[265];

#endif
