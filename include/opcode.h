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
    CALLUDF=16,//
    INPLACE_INC=17, //increments a local variable 
    LOAD_STR=18,
    JMPIFFALSE=19,
    NPOP_STACK = 20,
    MOD = 21,
    AND = 22,
    OR  = 23,
    RETURN = 24,
    JMPNPOPSTACK=25,// jump to a certain number of bytes and pop n values from stack
    GOTONPSTACK=26,//goto a certain bytecode index and pop n values from stack
    GOTO=27,//goto a certain bytecode index
    POP_STACK = 28,//pop stack
    LOAD_CONST = 29,//LOAD_CONST <index> loads a constant at a certain index in constant table onto the stack 
    CO_STOP = 30, //stops a co routine,marks it as dead and returns nil
    SMOREQ = 31,//smaller than or equal to
    GROREQ = 32,//greater than or equal to
    NEG = 33,//unary operator
    NOT = 34,//
    INDEX = 35,//to index a list or dictionary
    ASSIGNINDEX = 36,
    IMPORT = 37,//imports a native module (a shared library)
    BREAK = 38,
    CALLFORVAL = 39,//calls a builtin function and keeps it's return value on the stack
    INC_GLOBAL = 40,//increments a global variable
    CONT = 41,
    LOAD_GLOBAL = 42,//loads a global variable
    MEMB = 43,//member operator '.'
    JMPIFFALSENOPOP = 44,//
    ASSIGNMEMB = 45,//
    BUILD_CLASS = 46,//builds a class
    ASSIGN_GLOBAL = 47,//assigns a global variable
    LOAD_FUNC = 48,//builds a user defined function and loads it onto the stack
    OP_EXIT = 49,
    LSHIFT = 50,
    RSHIFT = 51,
    BITWISE_AND = 52,
    BITWISE_OR = 53,
    COMPLEMENT = 54,
    BUILD_DERIVED_CLASS = 55,//builds a derived class
    GOTO_CALLSTACK = 56,//simply goes to callstack.back() also pops the callstack
    IS = 57,//is operator
    ONERR_GOTO = 58,//begins error handling
    POP_EXCEP_TARG = 59,
    THROW = 60,
    LOAD_CO = 61,//builds a coroutine(not object!)
    YIELD = 62,//yields a value from coroutine
    YIELD_AND_EXPECTVAL = 63,//yields but also expects a value when coroutine is resumed
    DUP = 64,
    GC=65,
    NOPOPJMPIF=66,
    LOAD_LOCAL=67
};
#ifdef PLUTONIUM_PROFILE
const char* opNames[] = 
{
   "LOAD",
   "CALLMETHOD",
   "ADD",
   "MUL",
   "DIV",
   "SUB",
   "JMP",
   "CALL",
   "XOR",
   "ASSIGN",
   "JMPIF",
   "EQ",
   "NOTEQ",
   "SMALLERTHAN",
   "GREATERTHAN",
   "CALLUDF",
   "INPLACE_INC", 
   "LOAD_STR",
   "JMPIFFALSE",
   "NPOP_STACK",
   "MOD",
   "AND",
   "OR ",
   "RETURN",
   "JMPNPOPSTACK",
   "GOTONPSTACK",
   "GOTO",
   "POP_STACK",
   "LOAD_CONST",
   "CO_STOP", 
   "SMOREQ",
   "GROREQ",
   "NEG",
   "NOT",
   "INDEX",
   "ASSIGNINDEX",
   "IMPORT",
   "BREAK",
   "CALLFORVAL",
   "INC_GLOBAL",
   "CONT",
   "LOAD_GLOBAL",
   "MEMB",
   "JMPIFFALSENOPOP",
   "ASSIGNMEMB",
   "BUILD_CLASS",
   "ASSIGN_GLOBAL",
   "LOAD_FUNC",
   "OP_EXIT",
   "LSHIFT",
   "RSHIFT",
   "BITWISE_AND",
   "BITWISE_OR",
   "COMPLEMENT",
   "BUILD_DERIVED_CLASS",
   "GOTO_CALLSTACK",
   "IS",
   "ONERR_GOTO",
   "POP_EXCEP_TARG",
   "THROW",
   "LOAD_CO",
   "YIELD",
   "YIELD_AND_EXPECTVAL",
   "DUP",
   "GC",
   "NOPOPJMPIF",
   "LOAD_LOCAL"
};
#endif
#endif
