#ifndef OPCODE_S_H_
#define OPCODE_S_H_
enum OPCODE
{
    LOAD=1,//LOAD <type> <value>
    CALLMETHOD=2,
    ADD=3,//ADD
    MUL=4,//MUL
    DIV=5,//DIV
    SUB=6,//SUB
    JMP=7,//JMP <where>
    CALL=8,//CALL <name>
    XOR=9,//XOR
    ASSIGN = 10,//ASSIGN <name>
    JMPIF = 11,//JMPIf <where>
    EQ=12,//EQ
    NOTEQ=13,//NOTEQ
    SMALLERTHAN=14,
    GREATERTHAN=15,
    CALLUDF=16,
    INPLACE_INC=17,
    BACKJMP=18,
    JMPIFFALSE=19,
    NPOP_STACK = 20,
    MOD = 21,
    AND = 22,
    OR  = 23,
    RETURN = 24,
    JMPNPOPSTACK=25,
    GOTONPSTACK=26,
    GOTO=27,
    POP_STACK = 28,
    LOAD_CONST = 29,
    GEN_STOP = 30, //stops a generator
    SMOREQ = 31,
    GROREQ = 32,
    NEG = 33,
    NOT = 34,
    INDEX = 35,
    ASSIGNINDEX = 36,
    IMPORT = 37,
    BREAK = 38,
    CALLFORVAL = 39,
    INC_GLOBAL = 40,
    CONT = 41,
    LOAD_GLOBAL = 42,
    MEMB = 43,
    LOAD_NAME = 44,
    ASSIGNMEMB = 45,
    BUILD_CLASS = 46,
    ASSIGN_GLOBAL = 47,
    LOAD_FUNC = 48,
    OP_EXIT = 49,
    LSHIFT = 50,
    RSHIFT = 51,
    BITWISE_AND = 52,
    BITWISE_OR = 53,
    COMPLEMENT = 54,
    BUILD_DERIVED_CLASS = 55,
    GOTO_CALLSTACK = 56,
    IS = 57,
    ONERR_GOTO = 58,
    POP_EXCEP_TARG = 59,
    THROW = 60,
    LOAD_GEN = 61,
    YIELD = 62
};
#endif
