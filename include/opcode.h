/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
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
    LOAD_NIL = 22,
    LOAD_INT32  = 23,
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
    LOAD_TRUE = 56,//simply goes to callstack.back() also pops the callstack
    IS = 57,//is operator
    ONERR_GOTO = 58,//begins error handling
    POP_EXCEP_TARG = 59,
    THROW = 60,
    LOAD_CO = 61,//builds a coroutine(not object!)
    YIELD = 62,//yields a value from coroutine
    YIELD_AND_EXPECTVAL = 63,//yields but also expects a value when coroutine is resumed
    LOAD_LOCAL = 64,
    GC=65,
    NOPOPJMPIF=66,
    SELFMEMB=67,
    ASSIGNSELFMEMB=68,
    LOAD_FALSE=69,
    LOAD_BYTE=70
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
   "LOAD_NIL",
   "LOAD_INT32 ",
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
   "LOAD_TRUE",
   "IS",
   "ONERR_GOTO",
   "POP_EXCEP_TARG",
   "THROW",
   "LOAD_CO",
   "YIELD",
   "YIELD_AND_EXPECTVAL",
   "SELFMEMB",
   "GC",
   "NOPOPJMPIF",
   "SELFMEMB",
   "ASSIGNSELFMEMB",
   "LOAD_BYTE",
   "LOAD_FALSE"
};
#endif
#endif
