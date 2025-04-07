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
#include "compiler.h"
#include "ast.h"
#include "builtin-map.h"
#include "builtinfunc.h"
#include "byte_src.h"
#include "convo.h"
#include "lntable.h"
#include "opcode.h"
#include "pair-vector.h"
#include "parser.h"
#include "ptr-vector.h"
#include "refgraph.h"
#include "str-vec.h"
#include "symtable.h"
#include "misc.h"
#include "vm.h"
#include "zbytearray.h"
#include "zobject.h"
#include "zuko-src.h"
#include <stdint.h>
#include <stdio.h>
#include "sizet_vector.h"
#include "repl.h"



static inline void instruction(compiler_ctx* ctx,enum OPCODE op)
{
    zbytearr_push(&ctx->bytecode,op);
    ctx->bytes_done++;
}
static inline void i32_operand(compiler_ctx* ctx,int32_t operand)
{
    size_t sz = ctx->bytecode.size;
    zbytearr_resize(&ctx->bytecode,sz + sizeof(int32_t));
    memcpy(ctx->bytecode.arr+sz,&operand,sizeof(int32_t));
    ctx->bytes_done += sizeof(int32_t);
}
static inline void i64_operand(compiler_ctx* ctx,int64_t operand)
{
    size_t sz = ctx->bytecode.size;
    zbytearr_resize(&ctx->bytecode,sz + sizeof(int64_t));
    memcpy(ctx->bytecode.arr+sz,&operand,sizeof(int64_t));
    ctx->bytes_done += sizeof(int64_t);
}

extern ptr_vector vm_strings;
extern ptr_vector vm_builtin;
extern ptr_vector vm_important;
void extractSymRef(str_vector* ref,refgraph* graph)//gives the names of symbols that are referenced and not deadcode
{
  str_vector q;
  str_vector_init(&q);
  str_vector_push(&q,".main");

  //there is no incoming edge on .main
  //ref.emplace(".main",true);
  const char* curr;
  //printf("--begin DFS--\n");
  while(q.size != 0)
  {
    curr = q.arr[--q.size];
    str_vector* adj = refgraph_getref(graph,curr);
    for(size_t i = 0;i<adj->size;i++)
    {
        const char* e = adj->arr[i];
        if(str_vector_search(ref,e) == -1) //node not already visited or processed
        {
            str_vector_push(&q,e);
            str_vector_push(ref,e);
        }
    }
  }
  str_vector_destroy(&q);
  //printf("--end DFS--\n");
}
/*static inline void addBytes(zbytearr* vec,int32_t x)
{
  size_t sz = vec->size;
  zbytearr_resize(vec,sz + sizeof(int32_t));
  memcpy(vec->arr+sz,&x,sizeof(int32_t));
}*/
compiler_ctx* create_compiler_context(zuko_src* p)
{
    compiler_ctx* ctx = malloc(sizeof(compiler_ctx));
    str_vector_init(&ctx->symRef);
    ctx->symRef.size = 0;
    extractSymRef(&ctx->symRef,&p->ref_graph);
    ctx->files = &p->files;
    ctx->sources = &p->sources;
    ctx->fileTOP = 0; 
    ctx->line_num_table = &p->line_num_table;
    ctx->filename = p->files.arr[0];
    sizet_vector_init(&ctx->and_jumps);
    sizet_vector_init(&ctx->or_jumps);
    symtable_init(&ctx->globals);
    ptr_vector_init(&ctx->locals);
    ptr_vector_init(&ctx->compiled_functions);
    str_vector_init(&ctx->curr_class_members);
    sizet_vector_init(&ctx->break_stmt_idx);
    sizet_vector_init(&ctx->continue_stmt_idx);
    zbytearr_init(&ctx->bytecode);
    pair_vector_init(&ctx->backpatches);
    //init builtins
    init_builtin_functions();
    init_builtin_methods();
    //reset all state
    ctx->in_constructor = false;
    ctx->in_coroutine = false;
    ctx->inclass = false;
    ctx->infunc = false;
    ctx->infor = false;
    //if(!REPL_MODE)
    //{
        ctx->bytecode.size = 0;
        symtable_clear(&ctx->globals);
        ctx->locals.size = 0;
        ctx->classname = "";
        ctx->curr_class_members.size  = 0;
        ctx->line_num = 1;
        ctx->and_jumps.size = 0;
        ctx->or_jumps.size = 0;
        str_vector_init(&ctx->prefixes);
        str_vector_push(&ctx->prefixes,"");
        ctx->bytes_done = 0;
    //}
    return ctx;

}
void compiler_set_source(compiler_ctx* ctx, zuko_src* p, size_t root_idx)
{
    if(root_idx >= p->files.size)
    {
        fprintf(stderr,"Compiler: set_source() failed. REASON: root_idx out of range!");
        exit(1);
    }

}
int find_compiled_function(ptr_vector* vec,const char* name)
{
    for(size_t i = 0; i < vec->size; i++)
    {
        compiled_fn* fn = vec->arr[i];
        if(strcmp(fn->fn_node->childs.arr[1]->val,name)==0)
            return i;
    }
    return -1;
}
void compileError(compiler_ctx* ctx,const char* type,const char* msg)
{
    fprintf(stderr,"\nFile %s\n",ctx->filename);
    fprintf(stderr,"%s at line %zu\n",type,ctx->line_num);
    int idx = str_vector_search(ctx->files,ctx->filename);
    const char* source_code = ctx->sources->arr[idx];
    size_t l = 1;
    size_t k = 0;
    while(source_code[k]!=0 && l<=ctx->line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==ctx->line_num)
            fputc(source_code[k],stderr);
        k+=1;
    }
    fprintf(stderr,"\n%s\n",msg);
    if(REPL_MODE)
    {
        repl();
    }
    exit(1);
}
int32_t add_to_vm_strings(const char* n)
{
    size_t i = 0;
    for(size_t i=0; i< vm_strings.size; i++)
    {
        zstr* e = (zstr*)(vm_strings.arr[i]);
        if(strcmp(e->val,n) == 0)
            return i;
        i++;
    }
    size_t len = strlen(n);
    char* arr = malloc(sizeof(char)*(len+1)); //will be freed by VM's destructor
    strcpy(arr,n);
    zstr* str = malloc(sizeof(zstr));
    str->len = len;
    str->val = arr;
    ptr_vector_push(&vm_strings,str);
    return (int32_t)vm_strings.size-1;
}
void add_lntable_entry(compiler_ctx* ctx,size_t opcodeIdx)
{
    byte_src tmp = {ctx->fileTOP,ctx->line_num};
    lntable_emplace(ctx->line_num_table,opcodeIdx,tmp);
}
int32_t add_builtin_to_vm(const char* name)
{
    size_t index;
    BuiltinFunc fnAddr;
    bmap_get(&funcs,name,&fnAddr);
    for(index = 0;index < vm_builtin.size;index+=1)
    {
        if(vm_builtin.arr[index]==fnAddr)
        return (size_t)index;
    }
    ptr_vector_push(&vm_builtin,fnAddr);
    return (int32_t)vm_builtin.size-1;
}
int32_t resolve_name(compiler_ctx* ctx,const char* name,bool* isglobal,bool blowup,bool* isfromself)
{
    *isglobal = false;
    for(int32_t i=ctx->locals.size-1;i>=0;i-=1)
    {
        size_t tmp;
        if(symtable_get((symtable*)ctx->locals.arr[i],name,&tmp))
            return tmp;
    }

    if(isfromself)
    {
        char buffer[50];
        snprintf(buffer,50,"@%s",name);
        if(ctx->inclass && ctx->infunc && str_vector_search(&ctx->curr_class_members,name)!=-1 || str_vector_search(&ctx->curr_class_members,buffer)!=-1 )
        {
            *isfromself = true;
            return -2;
        }
    }
    for(int32_t i=ctx->prefixes.size-1;i>=0;i--)
    {
        const char* prefix = ctx->prefixes.arr[i];
        size_t val;
        char* new_name = merge_str(prefix,name);
        if(symtable_get(&ctx->globals,new_name,&val))
        {
            *isglobal = true;
            free((void*)new_name);
            return val;
        }
        free(new_name);
    }
    if(blowup)
    {
        char buffer[60];
        snprintf(buffer,60,"Error name %s is not defined!", name);
        compileError(ctx,"NameError",buffer);
    }
    return -1;
}

int32_t foo;
void expr_bytecode(compiler_ctx* ctx,Node* ast)
{
    zobject reg;
    if (ast->childs.size == 0)
    {
        if (ast->type == num_node)
        {
            const char* n = ast->val;
            if (is_int32(n))
            {
                instruction(ctx,LOAD_INT32);
                i32_operand(ctx,str_to_int32(n));
            }
            else if (is_int64(n))
            {
                int64_t tmp = str_to_int64(n);
                instruction(ctx,LOAD_INT64);
                i64_operand(ctx,tmp);
            }
            else
            {
                char buffer[50];
                snprintf(buffer,50,"Error integer %s causes overflow!",n);
                compileError(ctx,"OverflowError", buffer);
            }
            return;
        }
        else if (ast->type == float_node)
        {
            const char* n = ast->val;
            double tmp = str_to_double(n);
            zbytearr_push(&ctx->bytecode,LOAD_DOUBLE);
            zbytearr_resize(&ctx->bytecode,ctx->bytecode.size+8);
            memcpy(ctx->bytecode.arr+ctx->bytes_done+1,&tmp,8);
            ctx->bytes_done+=9;
            return;
        }
        else if (ast->type ==  bool_node)
        {
            const char* n = ast->val;
            instruction(ctx,strcmp(n,"true") ? LOAD_FALSE : LOAD_TRUE);;
            return;
        }
        else if (ast->type == str_node)
        {
            instruction(ctx,LOAD_STR);
            foo = add_to_vm_strings(ast->val);
            i32_operand(ctx,foo);
            return;
        }
        else if (ast->type == nil_node)
        {
            instruction(ctx,LOAD_NIL);
            return;
        }
        else if (ast->type == id_node)
        {
            const char* name = ast->val;
            bool isGlobal = false;
            bool isSelf = false;
            foo = resolve_name(ctx,name,&isGlobal,true,&isSelf);
            if(isSelf)
            {
                add_lntable_entry(ctx,ctx->bytes_done);
                instruction(ctx,SELFMEMB);
                foo = add_to_vm_strings(ast->val);
                i32_operand(ctx,foo);
                return;
            }
            else if(!isGlobal)
            {
                instruction(ctx,LOAD_LOCAL);
                i32_operand(ctx,foo);
                return;
            }
            else
            {
                instruction(ctx,LOAD_GLOBAL);
                i32_operand(ctx,foo);
                return;
            }
        }
        else if (ast->type == byte_node)
        {
            instruction(ctx,LOAD_BYTE);
            zbytearr_push(&ctx->bytecode,hex_to_uint8(ast->val));
            ctx->bytes_done++;
            return;
        }

    }
    if (ast->type == list_node)
    {
        
        for (size_t k = 0; k < ast->childs.size; k += 1)
        {
            expr_bytecode(ctx,ast->childs.arr[k]);
        }
        zbytearr_push(&ctx->bytecode,LOAD);
        zbytearr_push(&ctx->bytecode,'j');
        foo = ast->childs.size;
        i32_operand(ctx,foo);
        ctx->bytes_done += 2;
        return;
    }
    if (ast->type == dict_node)
    {
        
        for (size_t k = 0; k < ast->childs.size; k += 1)
        {
            expr_bytecode(ctx,ast->childs.arr[k]);
        }
        add_lntable_entry(ctx,ctx->bytes_done);//runtime errors can occur
        zbytearr_push(&ctx->bytecode,LOAD);
        zbytearr_push(&ctx->bytecode,'a');
        foo = ast->childs.size / 2;//number of key value pairs in dictionary
        i32_operand(ctx,foo);
        ctx->bytes_done += 2;
        return;
    }
    if (ast->type == add_node)
    {
        if(ast->childs.arr[0]->type == id_node && ast->childs.arr[1]->type == num_node && is_int32(ast->childs.arr[1]->val))
        {
            bool self1 = false;
            bool is_global1;
            
            int i = resolve_name(ctx,ast->childs.arr[0]->val,&is_global1,true,&self1);
            if(!self1)
            {
                zbytearr_push(&ctx->bytecode,LOADVAR_ADDINT32);
                zbytearr_push(&ctx->bytecode,(is_global1)? 1 : 0);
                i32_operand(ctx, i);
                i32_operand(ctx, (int32_t)atoi(ast->childs.arr[1]->val));
                ctx->bytes_done += 2;
                return;
            }
            
        }
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,ADD);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == sub_node)
    {
        if(ast->childs.arr[0]->type == id_node && ast->childs.arr[1]->type == num_node && is_int32(ast->childs.arr[1]->val))
        {
            bool self1 = false;
            bool is_global1;
            
            int i = resolve_name(ctx,ast->childs.arr[0]->val,&is_global1,true,&self1);
            if(!self1)
            {
                zbytearr_push(&ctx->bytecode,LOADVAR_SUBINT32);
                zbytearr_push(&ctx->bytecode,(is_global1)? 1 : 0);
                i32_operand(ctx, i);
                i32_operand(ctx, (int32_t)atoi(ast->childs.arr[1]->val));
                ctx->bytes_done += 2;
                return;
            }
            
        }
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,SUB);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
    
        return;
    }
    if (ast->type == div_node)
    {
        
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,DIV);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == mul_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,MUL);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == xor_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,XOR);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == mod_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,MOD);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == lshift_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,LSHIFT);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == rshift_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,RSHIFT);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == bitwiseand_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,BITWISE_AND);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == bitwiseor_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,BITWISE_OR);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == memb_node)
    {
        
        if (ast->childs.arr[1]->type == call_node)
        {
            Node* callnode = ast->childs.arr[1];

            expr_bytecode(ctx,ast->childs.arr[0]);
            Node* args = callnode->childs.arr[2];
            for (size_t f = 0; f < args->childs.size; f += 1)
            {
                expr_bytecode(ctx,args->childs.arr[f]);
            }
            zbytearr_push(&ctx->bytecode,CALLMETHOD);
            add_lntable_entry(ctx,ctx->bytes_done);
            byte_src tmp;
            const char* memberName = callnode->childs.arr[1]->val;
            foo = add_to_vm_strings(memberName);
            i32_operand(ctx,foo);
            zbytearr_push(&ctx->bytecode,args->childs.size);
            ctx->bytes_done += 2;
            return;
        }
        else
        { 
            expr_bytecode(ctx,ast->childs.arr[0]);
            add_lntable_entry(ctx,ctx->bytes_done);
            zbytearr_push(&ctx->bytecode,MEMB);
            if (ast->childs.arr[1]->type != id_node)
                compileError(ctx,"SyntaxError", "Invalid Syntax");
            const char* name = ast->childs.arr[1]->val;
            foo = add_to_vm_strings(name);
            i32_operand(ctx,foo);
            ctx->bytes_done++;
            return;
        }
    }
    if (ast->type == and_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        zbytearr_push(&ctx->bytecode,JMPIFFALSENOPOP);
        sizet_vector_push(&ctx->and_jumps,ctx->bytes_done);
        int32_t I = ctx->bytecode.size;
        zbytearr_push(&ctx->bytecode,0);
        zbytearr_push(&ctx->bytecode,0);
        zbytearr_push(&ctx->bytecode,0);
        zbytearr_push(&ctx->bytecode,0);
        ctx->bytes_done+=5;
        size_t before = ctx->bytes_done;
        expr_bytecode(ctx,ast->childs.arr[1]);
        size_t after = ctx->bytes_done;
        foo = after-before;
        memcpy(ctx->bytecode.arr+I,&foo,sizeof(int32_t));
        return;
    }
    if (ast->type == is_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,IS);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == or_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        zbytearr_push(&ctx->bytecode,NOPOPJMPIF);
        sizet_vector_push(&ctx->or_jumps,ctx->bytes_done);
        int32_t I = ctx->bytecode.size;
        zbytearr_push(&ctx->bytecode,0);
        zbytearr_push(&ctx->bytecode,0);
        zbytearr_push(&ctx->bytecode,0);
        zbytearr_push(&ctx->bytecode,0);
        ctx->bytes_done+=5;
        size_t before = ctx->bytes_done;
        expr_bytecode(ctx,ast->childs.arr[1]);
        size_t after = ctx->bytes_done;
        foo = after - before;
        memcpy(ctx->bytecode.arr + I,&foo,sizeof(int32_t));
        return;
    }
    if (ast->type == lt_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,SMALLERTHAN);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == gt_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,GREATERTHAN);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == equal_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,EQ);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == noteq_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,NOTEQ);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == gte_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,GROREQ);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == lte_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        zbytearr_push(&ctx->bytecode,SMOREQ);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == neg_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        zbytearr_push(&ctx->bytecode,NEG);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == complement_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        zbytearr_push(&ctx->bytecode,COMPLEMENT);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == not_node)
    {
        expr_bytecode(ctx,ast->childs.arr[0]);
        zbytearr_push(&ctx->bytecode,NOT);
        add_lntable_entry(ctx,ctx->bytes_done);
        ctx->bytes_done += 1;
        return;
    }
    if (ast->type == index_node)
    {
        int32_t idx1;
        int32_t idx2;

        if(ast->childs.arr[0]->type == id_node && ast->childs.arr[1]->type == id_node)
        {
            bool self1 = false;
            bool self2 = false;
            bool is_global1;
            bool is_global2;
            
            int i = resolve_name(ctx,ast->childs.arr[0]->val,&is_global1,true,&self1);
            int j = resolve_name(ctx,ast->childs.arr[1]->val,&is_global2,true,&self2);
            if(!self1 && !self2)
            {
                zbytearr_push(&ctx->bytecode,INDEX_FAST);
                zbytearr_push(&ctx->bytecode,(is_global1)? 1 : 0);
                zbytearr_push(&ctx->bytecode,(is_global2)? 1 : 0);
                i32_operand(ctx, i);
                i32_operand(ctx, j);
                ctx->bytes_done += 3;
                return;
            }
            
        }
        expr_bytecode(ctx,ast->childs.arr[0]);
        expr_bytecode(ctx,ast->childs.arr[1]);
        add_lntable_entry(ctx,ctx->bytes_done);
        instruction(ctx,INDEX);
        return;
    }
    if (ast->type == call_node)
    {
        const char* name = ast->childs.arr[1]->val;
        bool udf = false;
        if (!function_exists(name))//check if it is builtin function
            udf = true;
        if (udf)
        {
            bool isGlobal = false;
            bool isSelf = false;
            int foo = resolve_name(ctx,name,&isGlobal,true,&isSelf);
            int idx;
            if(isGlobal && (idx = find_compiled_function(&ctx->compiled_functions,name))!=-1) // Make direct call
            {
                Node* fn = ((compiled_fn*)ctx->compiled_functions.arr[idx])->fn_node;
                int32_t stack_idx = ((compiled_fn*)ctx->compiled_functions.arr[idx])->offset;
                int32_t N = ast->childs.arr[2]->childs.size; //arguments given
                int32_t args_required = fn->childs.arr[2]->childs.size;
                int32_t opt_args = 0;
                int32_t def_params = 0;
                int32_t def_begin = -1;
                for(size_t i=0;i<fn->childs.arr[2]->childs.size;i++)
                {
                    Node* arg = fn->childs.arr[2]->childs.arr[i];
                    if(arg->childs.size!=0)
                    {
                        def_params++;
                        if(def_begin==-1)
                            def_begin = i;
                    }
                }
                if ( (size_t)N + def_params < args_required || (size_t)N > args_required)
                {
                    char error_buffer[100];
                    snprintf(error_buffer,100,"Function %s takes %d arguments, %d given!",name,args_required,N);
                    compileError(ctx,"ArgumentError", error_buffer);
                }
                //load all args
                for(size_t i=0;i<ast->childs.arr[2]->childs.size;i++)
                {
                    Node* arg = ast->childs.arr[2]->childs.arr[i];
                    expr_bytecode(ctx,arg);
                }
                //load optional args
                for (size_t i = def_params - (args_required - N); i < def_params; i++)
                {
                    Node* arg = fn->childs.arr[2]->childs.arr[def_begin+i]->childs.arr[0];
                    expr_bytecode(ctx,arg);
                }
                zbytearr_push(&ctx->bytecode,CALL_DIRECT);
                i32_operand(ctx, stack_idx);
                ctx->bytes_done++;
                return;
            }
            if(isSelf)
            {
                zbytearr_push(&ctx->bytecode,LOAD_LOCAL);
                i32_operand(ctx,0);//load self to pass as first argument
                ctx->bytes_done++;
            }
            for (size_t k = 0; k < ast->childs.arr[2]->childs.size; k += 1)
            {
                expr_bytecode(ctx,ast->childs.arr[2]->childs.arr[k]);
            }
            if(isSelf)
            {
                add_lntable_entry(ctx,ctx->bytes_done);
                zbytearr_push(&ctx->bytecode,SELFMEMB);
                foo = add_to_vm_strings(name);
                i32_operand(ctx,foo);
                ctx->bytes_done++;
                add_lntable_entry(ctx,ctx->bytes_done);
                zbytearr_push(&ctx->bytecode,CALLUDF);
                zbytearr_push(&ctx->bytecode,ast->childs.arr[2]->childs.size+1);
                ctx->bytes_done+=2;
                return;
            }
            else if(isGlobal)
                zbytearr_push(&ctx->bytecode,LOAD_GLOBAL);
            else
                zbytearr_push(&ctx->bytecode,LOAD_LOCAL);
            i32_operand(ctx,foo);
            ctx->bytes_done++;
            add_lntable_entry(ctx,ctx->bytes_done);
            zbytearr_push(&ctx->bytecode,CALLUDF);
            zbytearr_push(&ctx->bytecode,ast->childs.arr[2]->childs.size);
            ctx->bytes_done+=2;
            return;
        }
        else
        {
            for (size_t k = 0; k < ast->childs.arr[2]->childs.size; k += 1)
            {
                expr_bytecode(ctx,ast->childs.arr[2]->childs.arr[k]);
            }
            add_lntable_entry(ctx,ctx->bytes_done);
            zbytearr_push(&ctx->bytecode,CALLFORVAL);
            bool add = true;
            size_t index = 0;
            BuiltinFunc fnAddr;
            bmap_get(&funcs,name,&fnAddr);
            for(index = 0;index < vm_builtin.size;index+=1)
            {

                if(vm_builtin.arr[index]==fnAddr)
                {
                    add = false;
                    break;
                }
            }
            if(add)
            {
                ptr_vector_push(&vm_builtin,fnAddr);
                foo = vm_builtin.size-1;
            }
            else
                foo = index;
            i32_operand(ctx,foo);
            zbytearr_push(&ctx->bytecode,(char)ast->childs.arr[2]->childs.size);
            ctx->bytes_done += 2;
            return;
        }
    }
    if (ast->type == yield_node)
    {
        if(ctx->in_constructor)
            compileError(ctx,"SyntaxError","Error class constructor can not be generators!");
        
        expr_bytecode(ctx,ast->childs.arr[1]);         
        add_lntable_entry(ctx,ctx->bytes_done);
        zbytearr_push(&ctx->bytecode,YIELD_AND_EXPECTVAL);
        ctx->bytes_done += 1;
        return;
    }  
    
    compileError(ctx,"SyntaxError", "Invalid syntax in expression");
    return;//to prevent compiler warning
}
static char error_buffer[100];
void scan_class(compiler_ctx* ctx,Node* ast)
{
    ctx->curr_class_members.size = 0;
    while(ast->type!=eop_node)
    {
        if(ast->type==declare_node)
        {
            const char* n = ast->val;
            if(n[0]=='@')
                n++;
            if(str_vector_search(&ctx->curr_class_members,n)!=-1 || str_vector_search(&ctx->curr_class_members,(ast->val))!=-1)
            {
                ctx->line_num = atoi(ast->childs.arr[0]->val);
                snprintf(error_buffer,100,"Error redeclaration of %s",n);
                compileError(ctx,"NameError",error_buffer);
            }
            str_vector_push(&ctx->curr_class_members,ast->val);
        }
        else if(ast->type == func_node)
        {
            const char* n = ast->childs.arr[1]->val;
            if(n[0]=='@')
                n++;
            if(str_vector_search(&ctx->curr_class_members,n)!=-1 || str_vector_search(&ctx->curr_class_members,ast->val)!=-1)
            {
                ctx->line_num = atoi(ast->childs.arr[0]->val);
                snprintf(error_buffer,100,"Error redeclaration of %s",n);
                compileError(ctx,"NameError",error_buffer);
            }
            str_vector_push(&ctx->curr_class_members,ast->childs.arr[1]->val);
        }
        else if(ast->type == coro_node) //generator function or coroutine
        {
            ctx->line_num = atoi(ast->childs.arr[0]->val);
            compileError(ctx,"NameError","Error coroutine inside class not allowed.");
        }
        
        else if(ast->type==class_node)
        {
            ctx->line_num = atoi(ast->childs.arr[0]->val);
            compileError(ctx,"SyntaxError","Error nested classes not supported");     
        }
        ast = ast->childs.arr[ast->childs.size-1];
    }
}

size_t compile(compiler_ctx* ctx,Node* ast)
{
    size_t bytes_done_before = ctx->bytes_done; 
    bool isGen = false;
    bool dfor = false;
    while (ast->type != eop_node)
    {
        if(ast->childs.size >= 1 && ast->childs.arr[0]->type == line_node)
                ctx->line_num = str_to_int32(ast->childs.arr[0]->val);
        if (ast->type == declare_node)
        {
            expr_bytecode(ctx,ast->childs.arr[1]);
            const char* name = ast->val;
            size_t tmp;
            if (ctx->locals.size == 0)
            {
                if (symtable_get(&ctx->globals, name, &tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError(ctx,"NameError", error_buffer);
                }
                foo = ctx->STACK_SIZE;
                if(!ctx->inclass)
                    symtable_emplace(&ctx->globals,ast->val,foo);
                ctx->STACK_SIZE+=1;
            }
            else
            {
                symtable* last = (symtable*)ctx->locals.arr[ctx->locals.size-1];
                if(symtable_get(last,name,&tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError(ctx,"NameError", error_buffer);
                }
                foo = ctx->STACK_SIZE;
                if(ctx->inclass && !ctx->infunc);
                else
                    symtable_emplace(last,name,foo);
                ctx->STACK_SIZE+=1;
            }

        }
        else if (ast->type == import_node || ast->type==importas_node)
        {
            const char* name = ast->childs.arr[1]->val;
            const char* vname = (ast->type == importas_node) ? ast->childs.arr[2]->val : name;
            bool f;
            if(resolve_name(ctx,vname,&f,false,NULL)!=-1)
            {
                char buffer[50];
                snprintf(buffer,50,"Error redeclaration of name %s",vname);
                compileError(ctx,"NameError",buffer);
            }
            add_lntable_entry(ctx,ctx->bytes_done);
            zbytearr_push(&ctx->bytecode,IMPORT);
            foo = add_to_vm_strings(name);
            i32_operand(ctx,foo);
            ctx->bytes_done++;
            if(ctx->locals.size == 0)
                symtable_emplace(&ctx->globals,vname,ctx->STACK_SIZE);
            else
            {
                symtable* last = (symtable*)ctx->locals.arr[ctx->locals.size-1];
                symtable_emplace(last,vname,ctx->STACK_SIZE);
            }
            ctx->STACK_SIZE+=1;
        }
        else if (ast->type == assign_node)
        {        
            const char* name = ast->childs.arr[1]->val;
            bool doit = true;
            if (ast->childs.arr[1]->type == id_node)
            {
                if (ast->childs.arr[2]->childs.size == 2)
                {
                    if (ast->childs.arr[2]->type == add_node && ast->childs.arr[2]->childs.arr[0]->val == ast->childs.arr[1]->val && ast->childs.arr[2]->childs.arr[0]->type==id_node && ast->childs.arr[2]->childs.arr[1]->type==num_node && strcmp(ast->childs.arr[2]->childs.arr[1]->val,"1") == 0)
                    {
                        bool isGlobal = false;
                        bool isSelf = false;
                        int32_t idx = resolve_name(ctx,name,&isGlobal,false,&isSelf);
                        if(idx==-1)
                        {
                            char buffer[50];
                            snprintf(buffer,50,"Error name %s is not defined!",name);
                            compileError(ctx,"NameError",buffer);
                        }
                        if(isSelf)
                        {
                            expr_bytecode(ctx,ast->childs.arr[2]);
                        }
                        add_lntable_entry(ctx,ctx->bytes_done);
                        foo = idx;
                        if(isSelf)
                        {
                            foo = add_to_vm_strings(name);
                            zbytearr_push(&ctx->bytecode,ASSIGNSELFMEMB);
                        }
                        else if(!isGlobal)
                            zbytearr_push(&ctx->bytecode,INPLACE_INC);
                        else
                            zbytearr_push(&ctx->bytecode,INC_GLOBAL);
                        i32_operand(ctx,foo);
                        ctx->bytes_done++;
                        doit = false;
                    }
                }
                if (doit)
                {
                    bool isGlobal = false;
                    bool isSelf = false;
                    int32_t idx = resolve_name(ctx,name,&isGlobal,false,&isSelf);
                    if(idx==-1)
                    {
                        char buffer[50];
                        snprintf(buffer,50,"Error name %s is not defined!",name);
                        compileError(ctx,"NameError",buffer);
                    }
                    expr_bytecode(ctx,ast->childs.arr[2]);
                    add_lntable_entry(ctx,ctx->bytes_done);
                    if(isSelf)
                    {
                        idx = add_to_vm_strings(name);
                        zbytearr_push(&ctx->bytecode,ASSIGNSELFMEMB);
                    }
                    else if(!isGlobal)
                        zbytearr_push(&ctx->bytecode,ASSIGN);
                    else
                        zbytearr_push(&ctx->bytecode,ASSIGN_GLOBAL);
                    i32_operand(ctx,idx);
                    ctx->bytes_done ++;
                }
            }
            else if (ast->childs.arr[1]->type == index_node)
            {
                //reassign index
                expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[0]);
                expr_bytecode(ctx,ast->childs.arr[2]);
                expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[1]);
                zbytearr_push(&ctx->bytecode,ASSIGNINDEX);
                add_lntable_entry(ctx,ctx->bytes_done);
                ctx->bytes_done+=1;
            }
            else if (ast->childs.arr[1]->type == memb_node)
            {
                //reassign object member
                expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[0]);
                if(ast->childs.arr[1]->childs.arr[1]->type!=id_node)
                    compileError(ctx,"SyntaxError","Invalid Syntax");
                const char* mname = ast->childs.arr[1]->childs.arr[1]->val;
                expr_bytecode(ctx,ast->childs.arr[2]);
                zbytearr_push(&ctx->bytecode,ASSIGNMEMB);
                foo = add_to_vm_strings(mname);
                add_lntable_entry(ctx,ctx->bytes_done);
                i32_operand(ctx,foo);
                ctx->bytes_done++;

            }
        }
        else if (ast->type == memb_node)
        {
            Node* bitch = new_node(memb_node,".");
            nodeptr_vector_push(&(bitch->childs),ast->childs.arr[1]);
            nodeptr_vector_push(&(bitch->childs),ast->childs.arr[2]);
            expr_bytecode(ctx,bitch);
            nodeptr_vector_destroy(&bitch->childs);
            free(bitch);
            instruction(ctx,POP_STACK);
        }
        else if (ast->type == while_node || ast->type == dowhile_node)
        {
            size_t offset1_idx; 
            size_t offset2_idx;
            if(ast->type == dowhile_node)
            {
                //to skip the condition first time
                zbytearr_push(&ctx->bytecode,GOTO);
                offset1_idx = ctx->bytes_done+1;
                i32_operand(ctx,0);
                ctx->bytes_done++;
            }
            
            size_t L = ctx->bytes_done;
            expr_bytecode(ctx,ast->childs.arr[1]);
            zbytearr_push(&ctx->bytecode,GOTOIFFALSE);
            offset2_idx = ctx->bytes_done + 1;
            i32_operand(ctx,0);
            ctx->bytes_done ++;

            int32_t before = ctx->STACK_SIZE;
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&ctx->locals,&m);

            size_t break_stmt_idxSizeCopy = ctx->break_stmt_idx.size;
            size_t continue_stmt_idxSizeCopy = ctx->continue_stmt_idx.size;

            int32_t localsBeginCopy = ctx->localsBegin; //idx of vector locals, from where
            //the locals of this loop begin
            ctx->localsBegin = ctx->locals.size - 1;
            size_t loop_body_begin = ctx->bytes_done;
            size_t loop_body_size = compile(ctx,ast->childs.arr[2]);
            ctx->localsBegin = localsBeginCopy; //backtrack
            ctx->STACK_SIZE = before;

            
            int32_t whileLocals = m.size;
            ctx->locals.size--;
            //
            if(whileLocals!=0)
            {
                zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                foo = whileLocals;
                i32_operand(ctx,foo);
            }
            else
                zbytearr_push(&ctx->bytecode,GOTO);
            foo = L; // goto the start of loop
            i32_operand(ctx,foo);
            ctx->bytes_done++;
            // backpatch break and continue offsets
            int32_t a = (int)ctx->bytes_done;
            int32_t b = (int)L;
            
            for(size_t i=break_stmt_idxSizeCopy;i<ctx->break_stmt_idx.size;i++)
            {
                pair_vector_push(&ctx->backpatches,ctx->break_stmt_idx.arr[i],a);  
            }
            for(size_t i=continue_stmt_idxSizeCopy;i<ctx->continue_stmt_idx.size;i++)
            {
                pair_vector_push(&ctx->backpatches,ctx->continue_stmt_idx.arr[i],b);
            }
            if(ast->type == dowhile_node)
                pair_vector_push(&ctx->backpatches,offset1_idx,loop_body_begin);
            pair_vector_push(&ctx->backpatches,offset2_idx,ctx->bytes_done);
            //backtrack
            ctx->break_stmt_idx.size = break_stmt_idxSizeCopy;
            ctx->continue_stmt_idx.size  = continue_stmt_idxSizeCopy;
            symtable_destroy(&m);
        }
        else if((ast->type == for_node) || (dfor = (ast->type == dfor_node)))
        {
            bool decl_variable = (ast->childs.arr[1]->type == decl_node);
            size_t linenum_copy = ctx->line_num;
            const char* loop_var_name = ast->childs.arr[2]->val;
            int32_t lcv_idx;
            int32_t end_idx;
            int32_t step_idx;
            bool lcv_is_global = false;
            int32_t stack_before = ctx->STACK_SIZE;
            // bytecode to initialize loop variable
            if(decl_variable)
            {
                expr_bytecode(ctx,ast->childs.arr[3]);//will load initial value onto stack
                lcv_idx = ctx->STACK_SIZE;
                ctx->STACK_SIZE++; //this initial value becomes a local variable
            } 
            else
            {
                bool is_self = false;
                int tmp = resolve_name(ctx,loop_var_name,&lcv_is_global,true,&is_self);
                if(is_self)
                    compileError(ctx,"SyntaxError","for loop control variable can't be from 'self'");
                lcv_idx = tmp;
                //assign initial value
                expr_bytecode(ctx,ast->childs.arr[3]);
                if(lcv_is_global)
                    zbytearr_push(&ctx->bytecode,ASSIGN_GLOBAL);
                else
                    zbytearr_push(&ctx->bytecode,ASSIGN);
                i32_operand(ctx,lcv_idx);
                ctx->bytes_done++;
            }
            //load end value
            expr_bytecode(ctx,ast->childs.arr[4]);
            end_idx = ctx->STACK_SIZE++;
            //load step size
            expr_bytecode(ctx,ast->childs.arr[5]); // step value
            step_idx = ctx->STACK_SIZE++;
            
            // bytecode to skip loop body if condition is not met
            ctx->line_num = linenum_copy;
            add_lntable_entry(ctx,ctx->bytes_done);
            if(dfor)
                zbytearr_push(&ctx->bytecode,SETUP_DLOOP);
            else
                zbytearr_push(&ctx->bytecode,SETUP_LOOP);
            ctx->bytes_done+=1;
            if(lcv_is_global)
            {
                zbytearr_push(&ctx->bytecode,1);
                i32_operand(ctx,lcv_idx);
                i32_operand(ctx, end_idx);
                ctx->bytes_done++;
            }
            else
            {
                zbytearr_push(&ctx->bytecode,0);
                i32_operand(ctx,lcv_idx);
                ctx->bytes_done++;
            }
            int32_t skip_loop = ctx->bytes_done;
            i32_operand(ctx,0); // apply backpatch here

            // add loop body
            symtable m;
            symtable_init(&m);
            if(decl_variable)
                symtable_emplace(&m,loop_var_name,lcv_idx);
            ctx->localsBegin = ctx->locals.size;
            ptr_vector_push(&ctx->locals, &m);
            size_t stacksize_before_body = ctx->STACK_SIZE;
            size_t loop_start = ctx->bytes_done;
            bool infor_copy = ctx->infor;
            size_t break_stmt_idxSizeCopy = ctx->break_stmt_idx.size;
            size_t continue_stmt_idxSizeCopy= ctx->continue_stmt_idx.size;

            ctx->infor = true;
            size_t a = ctx->bytes_done;
            compile(ctx,ast->childs.arr[6]);
            size_t b = ctx->bytes_done;
            ctx->infor = infor_copy;
            void* t;
            ptr_vector_pop(&ctx->locals,&t);
            int body_locals = ctx->STACK_SIZE - stacksize_before_body;
            ctx->STACK_SIZE = stacksize_before_body;
            /*if(body_locals == 1)
            {
                zbytearr_push(&bytecode,POP_STACK);
                bytes_done++;
            }
            else if(body_locals > 1)
            {
                zbytearr_push(&bytecode,NPOP_STACK);
                addBytes(&bytecode,body_locals);
                bytes_done+=5;
            }*/
            int32_t cont_dest = (int32_t)ctx->bytes_done;
            // finally add LOOP instruction
            ctx->line_num = linenum_copy;
            add_lntable_entry(ctx,ctx->bytes_done);
            if(dfor)
                zbytearr_push(&ctx->bytecode,DLOOP);
            else
                zbytearr_push(&ctx->bytecode,LOOP);
            if(lcv_is_global)
                zbytearr_push(&ctx->bytecode,1);
            else
                zbytearr_push(&ctx->bytecode,0);
            ctx->bytes_done+=1;
            i32_operand(ctx, lcv_idx); //can be global or local
            if(lcv_is_global)
            {
                i32_operand(ctx, end_idx); // is a local ( see above we reserved stack space for it)
            }
            i32_operand(ctx, (int32_t)loop_start);
            ctx->bytes_done++;//10;//18;
            
            //cleanup the locals we created
            int32_t break_dest = (int32_t)ctx->bytes_done;
            zbytearr_push(&ctx->bytecode,NPOP_STACK);
            if(decl_variable)
                i32_operand(ctx,3);
            else
                i32_operand(ctx,2);
            ctx->bytes_done++;
            for(size_t i=break_stmt_idxSizeCopy;i<ctx->break_stmt_idx.size;i++)
            {
                pair_vector_push(&ctx->backpatches,ctx->break_stmt_idx.arr[i],break_dest);
            }
            for(size_t i=continue_stmt_idxSizeCopy;i<ctx->continue_stmt_idx.size;i++)
            {
                pair_vector_push(&ctx->backpatches,ctx->continue_stmt_idx.arr[i],cont_dest);
            }
            pair_vector_push(&ctx->backpatches,skip_loop,(int32_t)break_dest);
            ctx->break_stmt_idx.size = break_stmt_idxSizeCopy;
            ctx->continue_stmt_idx.size = continue_stmt_idxSizeCopy;
            ctx->STACK_SIZE = stack_before;
            symtable_destroy(&m);
        }
        else if (ast->type == foreach_node)
        {
            const char* loop_var_name = ast->childs.arr[1]->val;
            int32_t fnIdx = add_builtin_to_vm("len"); //builtin len()
            int32_t len_idx;
            int32_t lcv_idx;
            int32_t list_idx;
            size_t goto1_offset_idx;

            //load integer iterator
            instruction(ctx,LOAD_INT32);
            i32_operand(ctx,0);
            lcv_idx = ctx->STACK_SIZE++;
            //load the list and calculate length
            expr_bytecode(ctx,ast->childs.arr[2]); //load the list
            instruction(ctx,CALLFORVAL);
            i32_operand(ctx,fnIdx);
            zbytearr_push(&ctx->bytecode,1);
            ctx->bytes_done++;

            len_idx = ctx->STACK_SIZE;
            ctx->STACK_SIZE++;

            expr_bytecode(ctx,ast->childs.arr[2]); //load the list
            list_idx = ctx->STACK_SIZE;
            ctx->STACK_SIZE++;

            // Generate loop condition
            int32_t loop_start = ctx->bytes_done;
            instruction(ctx,LOAD_LOCAL);
            i32_operand(ctx,lcv_idx);
            instruction(ctx,LOAD_LOCAL);
            i32_operand(ctx,len_idx);
            
            instruction(ctx,SMALLERTHAN);
            add_lntable_entry(ctx,ctx->bytes_done);
            instruction(ctx,GOTOIFFALSE);
            goto1_offset_idx = ctx->bytes_done;
            i32_operand(ctx,0);
            
            //load the indexed element
            symtable m;
            symtable_init(&m);
            instruction(ctx,LOAD_LOCAL);
            i32_operand(ctx,list_idx);
            instruction(ctx,LOAD_LOCAL);
            i32_operand(ctx,lcv_idx);
            
            add_lntable_entry(ctx,ctx->bytes_done);
            instruction(ctx,INDEX);

            size_t stack_size_before = ctx->STACK_SIZE;
            symtable_emplace(&m,loop_var_name,ctx->STACK_SIZE++);
            ptr_vector_push(&ctx->locals,&m);

            size_t break_stmt_idxSizeCopy = ctx->break_stmt_idx.size;
            size_t continue_stmt_idxSizeCopy = ctx->continue_stmt_idx.size;
            int32_t localsBeginCopy = ctx->localsBegin; //idx of vector locals, from where
            //the locals of this loop begin
            ctx->localsBegin = ctx->locals.size - 1;
            bool a = ctx->infor;
            ctx->infor = false; // we are compiling foreach not for loop
            compile(ctx,ast->childs.arr[3]);
            ctx->infor = a;
            ctx->localsBegin = localsBeginCopy;
            ctx->locals.size--;
            ctx->STACK_SIZE = stack_size_before;

            size_t cont_target = ctx->bytes_done;
            instruction(ctx,INPLACE_INC);
            i32_operand(ctx,lcv_idx);
            int32_t whileLocals = m.size;
            if(whileLocals != 0)
            {
                instruction(ctx,GOTONPSTACK);
                i32_operand(ctx,whileLocals);
                i32_operand(ctx,loop_start);
            }
            else
            {
                instruction(ctx,GOTO);
                i32_operand(ctx,loop_start);
            }
        
            int32_t brk_target = ctx->bytes_done;
            instruction(ctx,NPOP_STACK);
            i32_operand(ctx,3);
                        
            //backpatching             
            for(size_t i=break_stmt_idxSizeCopy;i < ctx->break_stmt_idx.size;i++)
            {
                pair_vector_push(&ctx->backpatches,ctx->break_stmt_idx.arr[i],brk_target);
            }
            for(size_t i = continue_stmt_idxSizeCopy; i< ctx->continue_stmt_idx.size; i++)
            {
                size_t k = ctx->continue_stmt_idx.arr[i];
                int32_t x = m.size - 1;
                memcpy(ctx->bytecode.arr+k-4,&x,sizeof(int32_t));
                pair_vector_push(&ctx->backpatches,ctx->continue_stmt_idx.arr[i],cont_target);
            }
            pair_vector_push(&ctx->backpatches,goto1_offset_idx,brk_target);
            //backtrack
            ctx->break_stmt_idx.size = break_stmt_idxSizeCopy;
            ctx->continue_stmt_idx.size  = continue_stmt_idxSizeCopy;
            ctx->STACK_SIZE -= 3;
            symtable_destroy(&m);
        }
        else if(ast->type == namespace_node)
        {
            const char* name = ast->childs.arr[1]->val;
            char* prefix;
            for(size_t i = 0;i <ctx->prefixes.size;i++)
            {
                if(i == 0)
                    prefix = merge_str("",ctx->prefixes.arr[i]);
                else
                {
                    char* old = prefix;
                    prefix = merge_str(prefix,ctx->prefixes.arr[i]);
                    free(old);
                }
            }
            char* old = prefix;
            prefix = merge_str(prefix,name);
            char* tmp = merge_str(prefix,"::");
            free(old);
            free(prefix);
            str_vector_push(&ctx->prefixes,tmp);
            compile(ctx,ast->childs.arr[2]);
            char* t;
            str_vector_pop(&ctx->prefixes,&t);
            free(t);
            //Hasta La Vista Baby
        }
        else if (ast->type == if_node)
        {
            int32_t offset1_idx;
            //print_ast(ast,0);
            Node* return_stmt = NULL;
            bool is_global = false;
            bool isfromself = false;
            int32_t idx;
            if(ast->childs.arr[2]->type == return_node && (return_stmt = ast->childs.arr[2]) && (return_stmt->childs.arr[1]->type == num_node && is_int32(return_stmt->childs.arr[1]->val)))
            {
                //print_ast(return_stmt,0);
                Node* cond = ast->childs.arr[1]->childs.arr[0];
                int32_t val_to_return = str_to_int32(return_stmt->childs.arr[1]->val);
                expr_bytecode(ctx,cond);
                instruction(ctx,CONDITIONAL_RETURN_I32);
                i32_operand(ctx, val_to_return);
                ctx->last_stmt_type = ast->type;
                ast = ast->childs.arr[ast->childs.size-1];
                continue;
            }
            if(ast->childs.arr[2]->type == return_node && (return_stmt = ast->childs.arr[2]) && (return_stmt->childs.arr[1]->type == id_node) && (idx = resolve_name(ctx,return_stmt->childs.arr[1]->val,&is_global,false,&isfromself))!=-1 && !is_global && !isfromself )
            {
                //print_ast(return_stmt,0);
                Node* cond = ast->childs.arr[1]->childs.arr[0];
                int32_t val_to_return = str_to_int32(return_stmt->childs.arr[1]->val);
                expr_bytecode(ctx,cond);
                instruction(ctx,CONDITIONAL_RETURN_LOCAL);
                i32_operand(ctx,idx);
                ctx->last_stmt_type = ast->type;
                ast = ast->childs.arr[ast->childs.size-1];
                continue;
            }

            /*if(ast->childs.arr[1]->childs.arr[0]->type == NodeType::equal)
            {
                Node* cond = ast->childs.arr[1]->childs.arr[0];
                expr_bytecode(cond->childs.arr[0]);
                expr_bytecode(cond->childs.arr[1]);
                zbytearr_push(&bytecode,CMP_JMPIFFALSE);
                bytes_done+=1;
                zbytearr_push(&bytecode,0);
                jmp_offset = ++bytes_done;
                addBytes(&bytecode, 0);
                bytes_done += 4;
                
            }
            else
            {*/
                expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[0]); // load condition
                zbytearr_push(&ctx->bytecode,GOTOIFFALSE);
                offset1_idx = ++ctx->bytes_done;
                i32_operand(ctx,0);
            //}

            int32_t before = ctx->STACK_SIZE;
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&ctx->locals,&m);
            size_t block_size = compile(ctx,ast->childs.arr[2]); // block
            ctx->STACK_SIZE = before;
            bool hasLocals = (m.size!=0);
            if(hasLocals)
            {
                foo = m.size;
                instruction(ctx,NPOP_STACK);
                i32_operand(ctx,foo);
            }
            ctx->locals.size--;
            pair_vector_push(&ctx->backpatches,offset1_idx,ctx->bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type == ifelse_node)
        {
            size_t offset1_idx;
            size_t offset2_idx;
            size_t stack_size_before;
            size_t else_offset;
            symtable m;

            expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[0]); //bytecode to load if condition
            offset1_idx = ctx->bytes_done+1;
            instruction(ctx,GOTOIFFALSE);
            i32_operand(ctx,0);
            int32_t before = ctx->STACK_SIZE;
            symtable_init(&m);
            ptr_vector_push(&ctx->locals,&m);
            size_t ifblock_size = compile(ctx,ast->childs.arr[2]);
            ctx->STACK_SIZE = before;
            //
            int32_t iflocals = m.size;
            ctx->locals.size--;
            if(iflocals != 0)
            {
                zbytearr_push(&ctx->bytecode,GOTONPSTACK);      
                i32_operand(ctx,iflocals);
                offset2_idx = ctx->bytes_done+1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
            }          
            else
            {
                zbytearr_push(&ctx->bytecode,GOTO);
                offset2_idx = ctx->bytes_done + 1;
                i32_operand(ctx,0);
                ctx->bytes_done++;
            }

            else_offset = ctx->bytes_done;
            before = ctx->STACK_SIZE;
            symtable_clear(&m);
            ptr_vector_push(&ctx->locals,&m);
            size_t elseblock_size = compile(ctx,ast->childs.arr[3]);
            ctx->STACK_SIZE = before;
            int32_t elseLocals = m.size;
            ctx->locals.size--;
            if(elseLocals!=0)
            {
                instruction(ctx,NPOP_STACK);
                i32_operand(ctx,elseLocals);
            }
            pair_vector_push(&ctx->backpatches,offset1_idx,else_offset);
            pair_vector_push(&ctx->backpatches,offset2_idx,ctx->bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type == ifelifelse_node)
        {  
            size_t offset1_idx;
            size_t offset2_idx;
            size_t after_else_offset;
            expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[0]); //load if condition
            zbytearr_push(&ctx->bytecode,GOTOIFFALSE);
            offset1_idx = ctx->bytes_done + 1;
            i32_operand(ctx,0);

            ctx->bytes_done += 1;
            
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&ctx->locals,&m);
            int32_t before = ctx->STACK_SIZE;
            compile(ctx,ast->childs.arr[2]); // compile if block
            ctx->STACK_SIZE = before;
            int32_t iflocals = m.size;
            ctx->locals.size--;
            
            if(iflocals != 0)
            {
                zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                i32_operand(ctx,iflocals);
                offset2_idx = ctx->bytes_done + 1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
            }
            else
            {
                zbytearr_push(&ctx->bytecode,GOTO);
                offset2_idx = ctx->bytes_done+1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
            }

            pair_vector_push(&ctx->backpatches,offset1_idx,ctx->bytes_done);
            int32_t elifBlockcounter = 3;//third node of ast
            sizet_vector the_end;
            sizet_vector_init(&the_end);
            for (size_t k = 1; k < ast->childs.arr[1]->childs.size; k += 1)
            {
                size_t goto1_offset_idx;
                size_t goto2_offset_idx;

                expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[k]); // load elif condition
                zbytearr_push(&ctx->bytecode,GOTOIFFALSE);
                goto1_offset_idx = ctx->bytes_done + 1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
                before = ctx->STACK_SIZE;
                symtable_clear(&m);
                ptr_vector_push(&ctx->locals,&m);
                compile(ctx,ast->childs.arr[elifBlockcounter]); // compile elif block
                ctx->STACK_SIZE = before;
                int32_t eliflocals = m.size;
                ctx->locals.size--;
                
                if(eliflocals != 0)
                {
                    zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                    i32_operand(ctx,eliflocals);
                    goto2_offset_idx = ctx->bytes_done + 1;
                    i32_operand(ctx,0);
                    ctx->bytes_done += 1;
                }
                else
                {
                    zbytearr_push(&ctx->bytecode,GOTO);
                    goto2_offset_idx = ctx->bytes_done+1;
                    i32_operand(ctx, 0);
                    ctx->bytes_done++;
                }
                pair_vector_push(&ctx->backpatches,goto1_offset_idx,ctx->bytes_done);
                sizet_vector_push(&the_end,goto2_offset_idx);
                elifBlockcounter += 1;
            }

            symtable_clear(&m);
            ptr_vector_push(&ctx->locals,&m);
            before = ctx->STACK_SIZE;
            compile(ctx,ast->childs.arr[ast->childs.size - 2]); // compile else block
            ctx->STACK_SIZE = before;
            size_t elseLocals = m.size;
            ctx->locals.size--;
            if(elseLocals!=0)
            {
                zbytearr_push(&ctx->bytecode,NPOP_STACK);
                i32_operand(ctx,elseLocals);
                ctx->bytes_done++;
            }
            for(size_t i=0;i<the_end.size;i++)
                pair_vector_push(&ctx->backpatches,the_end.arr[i],ctx->bytes_done);
            pair_vector_push(&ctx->backpatches,offset2_idx,ctx->bytes_done);
            symtable_destroy(&m);
            sizet_vector_destroy(&the_end);
        }
        else if (ast->type == ifelif_node)
        {  
            size_t offset1_idx;
            size_t offset2_idx;
            size_t after_else_offset;
            expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[0]); //load if condition
            zbytearr_push(&ctx->bytecode,GOTOIFFALSE);
            offset1_idx = ctx->bytes_done + 1;
            i32_operand(ctx,0);
            ctx->bytes_done += 1;
            
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&ctx->locals,&m);
            int32_t before = ctx->STACK_SIZE;
            compile(ctx,ast->childs.arr[2]); // compile if block
            ctx->STACK_SIZE = before;
            int32_t iflocals = m.size;
            ctx->locals.size--;
            
            if(iflocals != 0)
            {
                zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                i32_operand(ctx,iflocals);
                offset2_idx = ctx->bytes_done + 1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
            }
            else
            {
                zbytearr_push(&ctx->bytecode,GOTO);
                offset2_idx = ctx->bytes_done+1;
                i32_operand(ctx,0);
                ctx->bytes_done++;
            }

            pair_vector_push(&ctx->backpatches,offset1_idx,ctx->bytes_done);
            int32_t elifBlockcounter = 3;//third node of ast
            sizet_vector the_end;
            sizet_vector_init(&the_end);
            for (size_t k = 1; k < ast->childs.arr[1]->childs.size; k += 1)
            {
                size_t goto1_offset_idx;
                size_t goto2_offset_idx;

                expr_bytecode(ctx,ast->childs.arr[1]->childs.arr[k]); // load elif condition
                zbytearr_push(&ctx->bytecode,GOTOIFFALSE);
                goto1_offset_idx = ctx->bytes_done + 1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
                before = ctx->STACK_SIZE;
                symtable_clear(&m);
                ptr_vector_push(&ctx->locals,&m);
                compile(ctx,ast->childs.arr[elifBlockcounter]); // compile elif block
                ctx->STACK_SIZE = before;
                int32_t eliflocals = m.size;
                ctx->locals.size--;
                
                if(eliflocals != 0)
                {
                    zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                    i32_operand(ctx,eliflocals);
                    goto2_offset_idx = ctx->bytes_done + 1;
                    i32_operand(ctx,0);
                    ctx->bytes_done += 1;
                }
                else if(k != ast->childs.arr[1]->childs.size - 1) // not last elif
                {
                    zbytearr_push(&ctx->bytecode,GOTO);
                    goto2_offset_idx = ctx->bytes_done+1;
                    i32_operand(ctx, 0);
                    ctx->bytes_done += 1;
                }
                pair_vector_push(&ctx->backpatches,goto1_offset_idx,ctx->bytes_done);
                if(k != ast->childs.arr[1]->childs.size - 1) // not last elif
                    sizet_vector_push(&the_end,goto2_offset_idx);
                elifBlockcounter += 1;
            }

            for(size_t i=0;i<the_end.size;i++)
                pair_vector_push(&ctx->backpatches,the_end.arr[i],ctx->bytes_done);
            pair_vector_push(&ctx->backpatches,offset2_idx,ctx->bytes_done);
            symtable_destroy(&m);
            sizet_vector_destroy(&the_end);
        }
        else if(ast->type==trycatch_node)
        {
            size_t offset1_idx;
            size_t offset2_idx;
            zbytearr_push(&ctx->bytecode,ONERR_GOTO);
            offset1_idx = ctx->bytes_done+1;
            i32_operand(ctx,0);
            ctx->bytes_done+=1;
            
            int32_t before = ctx->STACK_SIZE;
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&ctx->locals,&m);
            compile(ctx,ast->childs.arr[2]);
            
            ctx->STACK_SIZE = before;
            int32_t trylocals = m.size;
            ctx->locals.size--;

            zbytearr_push(&ctx->bytecode,POP_EXCEP_TARG);
            ctx->bytes_done+=1;
            
            if(trylocals!=0)
            {
                zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                instruction(ctx,GOTONPSTACK);
                i32_operand(ctx,trylocals);
                offset2_idx = ctx->bytes_done;
                i32_operand(ctx,0);
            }
            else
            {
                instruction(ctx,GOTO);
                offset2_idx = ctx->bytes_done;
                i32_operand(ctx,0);
            }

            pair_vector_push(&ctx->backpatches,offset1_idx,ctx->bytes_done);
            
            before = ctx->STACK_SIZE;
            symtable_clear(&m);
            symtable_emplace(&m,ast->childs.arr[1]->val,ctx->STACK_SIZE);
            ctx->STACK_SIZE+=1;
            ptr_vector_push(&ctx->locals,&m);
            compile(ctx,ast->childs.arr[3]);
            int32_t catchlocals = m.size;
            ctx->STACK_SIZE = before;
            ctx->locals.size--;
            if(catchlocals > 1)
            {
                zbytearr_push(&ctx->bytecode,NPOP_STACK);
                i32_operand(ctx,catchlocals);
                ctx->bytes_done++;
            }
            else
                instruction(ctx,POP_STACK);
            pair_vector_push(&ctx->backpatches,offset2_idx,ctx->bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type==func_node || (isGen = ast->type == coro_node))
        {
            int32_t selfIdx= 0;
            if(ctx->infunc)
            {
                ctx->line_num = atoi(ast->childs.arr[0]->val);
                compileError(ctx,"SyntaxError","Error function within function not allowed!");
            }
            const char* name = ast->childs.arr[1]->val;
            bool isRef = (str_vector_search(&ctx->symRef,name) != -1);
            size_t tmp;
            if(ctx->compile_deadcode || isRef || ctx->inclass)
            {
                symtable* last = (ctx->locals.size == 0) ? NULL : (symtable*)ctx->locals.arr[ctx->locals.size-1];
                size_t tmp;
                if(function_exists(name) && !ctx->inclass)
                {
                    snprintf(error_buffer,100,"Error redeclaration of builtin function %s",name);
                    compileError(ctx,"NameError", error_buffer);
                }
                if(ctx->locals.size==0 && symtable_get(&ctx->globals,name,&tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError(ctx,"NameError", error_buffer);
                }
                else if(ctx->locals.size!=0 && symtable_get(last,name,&tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError(ctx,"NameError", error_buffer);
                }
                if(!ctx->inclass)
                {
                    if(ctx->locals.size==0)// at global level
                    {
                        symtable_emplace(&ctx->globals,name,ctx->STACK_SIZE);
                        if(!isGen)
                        {
                            compiled_fn* fn = malloc(sizeof(compiled_fn));
                            fn->fn_node = ast;
                            fn->offset = ctx->STACK_SIZE;
                            ptr_vector_push(&ctx->compiled_functions,fn);
                        }
                    }
                    else
                        symtable_emplace(last,name,ctx->STACK_SIZE);
                    ctx->STACK_SIZE+=1;
                }
                symtable C;
                symtable_init(&C);
                uint8_t def_param =0;
                int32_t before = ctx->STACK_SIZE;
                ctx->STACK_SIZE = 0;
                if(ctx->inclass)
                {
                    symtable_emplace(last,"self",ctx->STACK_SIZE);
                    selfIdx = ctx->STACK_SIZE;
                    if(strcmp(name,"__construct__")==0)
                        ctx->in_constructor = true;
                    ctx->STACK_SIZE+=1;
                }
                for (size_t k =0; k<ast->childs.arr[2]->childs.size; k += 1)
                {
                    const char* n = ast->childs.arr[2]->childs.arr[k]->val;
                    if(ast->childs.arr[2]->childs.arr[k]->childs.size!=0)
                    {
                        if(isGen)
                            compileError(ctx,"SyntaxError","Error default parameters not supported for couroutines");
                        expr_bytecode(ctx,ast->childs.arr[2]->childs.arr[k]->childs.arr[0]);
                        def_param +=1;
                    }
                    symtable_emplace(&C,n,ctx->STACK_SIZE);
                    ctx->STACK_SIZE+=1;
                }
                if(isGen)
                    zbytearr_push(&ctx->bytecode,LOAD_CO);
                else
                    zbytearr_push(&ctx->bytecode,LOAD_FUNC);
                foo = ctx->bytes_done+2+JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+1 +((isGen) ? 0 : 1);
                i32_operand(ctx,foo);
                foo = add_to_vm_strings(name);
                i32_operand(ctx,foo);
                if(ctx->inclass)
                    zbytearr_push(&ctx->bytecode,ast->childs.arr[2]->childs.size+1);
                else
                    zbytearr_push(&ctx->bytecode,ast->childs.arr[2]->childs.size);
                //Push number of optional parameters
                if(!isGen)
                {
                    zbytearr_push(&ctx->bytecode,def_param);
                    ctx->bytes_done++;
                }
                ctx->bytes_done+=2;

                /////////
                zbytearr_push(&ctx->bytecode,GOTO);
                size_t offset1_idx = ctx->bytes_done+1;
                i32_operand(ctx,0);
                ctx->bytes_done += 1;
                
                ptr_vector_push(&ctx->locals,&C);
                
                ctx->infunc = true;
                ctx->in_coroutine = isGen;
                size_t funcBody_size = compile(ctx,ast->childs.arr[3]);
                ctx->infunc = false;
                
                ctx->in_constructor = false;
                ctx->in_coroutine = false;
                ctx->locals.size--;
                ctx->STACK_SIZE = before;
                if(strcmp(name,"__construct__")!=0)
                {
                    if (funcBody_size == 0 || ctx->last_stmt_type!=return_node)
                    {
                        zbytearr_push(&ctx->bytecode,LOAD_NIL);
                        if(isGen)
                            zbytearr_push(&ctx->bytecode,CO_STOP);
                        else
                            zbytearr_push(&ctx->bytecode,OP_RETURN);
                        ctx->bytes_done +=2;
                    }
                }
                else
                {
                    zbytearr_push(&ctx->bytecode,LOAD_LOCAL);
                    foo = selfIdx;
                    i32_operand(ctx,foo);
                    zbytearr_push(&ctx->bytecode,OP_RETURN);
                    ctx->bytes_done+=2;
                }
                pair_vector_push(&ctx->backpatches,offset1_idx,ctx->bytes_done);
                symtable_destroy(&C);
            }
            else
            {
                //printf("not compiling function %s because it is not fnReferenced\n",name.c_str());
            }
            isGen = false;
        }
        else if(ast->type==class_node || ast->type==extclass_node)
        {
            bool extendedClass = (ast->type == extclass_node);
            const char* name = ast->childs.arr[1]->val;
            bool isRef = (str_vector_search(&ctx->symRef,name) != -1);
            if(!REPL_MODE && !isRef)
            {
                //class not referenced in source code
                //no need to compile
                ctx->last_stmt_type = ast->type;
                ast = ast->childs.arr[ast->childs.size-1]; 
                continue;
            }
            size_t tmp;
            if(symtable_get(&ctx->globals,name,&tmp))
            {
                snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                compileError(ctx,"NameError", error_buffer);
            }
            symtable_emplace(&ctx->globals,name,ctx->STACK_SIZE);
            ctx->STACK_SIZE+=1;
            scan_class(ctx,ast->childs.arr[(extendedClass) ? 3: 2]); // sets curr_class_members
            if(extendedClass)
            {
                expr_bytecode(ctx,ast->childs.arr[2]);
            }
            for(size_t i=0;i<ctx->curr_class_members.size;i++)
            {
                const char* e = ctx->curr_class_members.arr[i];
                if(strcmp(e,"super")==0 || strcmp(e,"self")==0)
                {
                    compileError(ctx,"NameError","Error class not allowed to have members named \"super\" or \"self\"");
                }
                foo = add_to_vm_strings(e);
                zbytearr_push(&ctx->bytecode,LOAD_STR);
                i32_operand(ctx,foo);
                ctx->bytes_done+=1;
            }
            symtable M;
            symtable_init(&M);
            ptr_vector_push(&ctx->locals,&M);
            int32_t before = ctx->STACK_SIZE;
            
            ctx->inclass = true;
            ctx->classname = name;
            size_t block_size;
            if(extendedClass)
                block_size = compile(ctx,ast->childs.arr[3]);
            else
                block_size = compile(ctx,ast->childs.arr[2]);
            ctx->inclass = false;
            symtable_destroy(&M);
            ctx->locals.size--;
            int32_t totalMembers = ctx->curr_class_members.size;
            ctx->curr_class_members.size = 0;
            ctx->STACK_SIZE = before;
            if(extendedClass)
            {
                zbytearr_push(&ctx->bytecode,BUILD_DERIVED_CLASS);
                add_lntable_entry(ctx,ctx->bytes_done);
            }
            else
                zbytearr_push(&ctx->bytecode,BUILD_CLASS);

            i32_operand(ctx,totalMembers);
            foo = add_to_vm_strings(name);
            i32_operand(ctx,foo);
            ctx->bytes_done+=1;
        }
        else if (ast->type == return_node)
        {
            if(ctx->in_constructor)
                compileError(ctx,"SyntaxError","Error class constructors should not return anything!");
            //stmt in it's block
            if(ast->childs.arr[1]->type == num_node && is_int32(ast->childs.arr[1]->val))
            {
                instruction(ctx,RETURN_INT32);
                i32_operand(ctx,atoi(ast->childs.arr[1]->val));
            }
            else
            {
                expr_bytecode(ctx,ast->childs.arr[1]);
                add_lntable_entry(ctx,ctx->bytes_done);
                if(ctx->in_coroutine)
                    zbytearr_push(&ctx->bytecode,CO_STOP);
                else
                    zbytearr_push(&ctx->bytecode,OP_RETURN);
                ctx->bytes_done += 1;
            }
        }
        else if (ast->type == throw_node)
        {   
            expr_bytecode(ctx,ast->childs.arr[1]);
            add_lntable_entry(ctx,ctx->bytes_done);
            zbytearr_push(&ctx->bytecode,THROW);
            ctx->bytes_done += 1;
        }
        else if (ast->type == yield_node)
        {
            if(ctx->in_constructor)
                compileError(ctx,"SyntaxError","Error class constructor can not be generators!");
            
            expr_bytecode(ctx,ast->childs.arr[1]);            
            add_lntable_entry(ctx,ctx->bytes_done);
            zbytearr_push(&ctx->bytecode,YIELD);
            ctx->bytes_done += 1;
        }
        else if (ast->type == break_node)
        {
            zbytearr_push(&ctx->bytecode,GOTONPSTACK);
            sizet_vector_push(&ctx->break_stmt_idx,ctx->bytes_done+5);
            foo = 0;
            for(size_t i=ctx->localsBegin;i<ctx->locals.size;i++)
                foo+=((symtable*)ctx->locals.arr[i])->size;
            if(ctx->infor)
                foo-=1; //don't pop loop control variable
            i32_operand(ctx,foo);
            foo = 0;
            i32_operand(ctx,foo);
            ctx->bytes_done += 1;
        }
        else if(ast->type==gc_node)
        {
            zbytearr_push(&ctx->bytecode,GC);
            ctx->bytes_done+=1;
        }
        else if (ast->type == continue_node)
        {
            if(ctx->infor)
            {
                zbytearr_push(&ctx->bytecode,GOTO);
                ctx->bytes_done+=1;
                sizet_vector_push(&ctx->continue_stmt_idx,ctx->bytes_done);
                i32_operand(ctx,0);
            }
            else
            {
                zbytearr_push(&ctx->bytecode,GOTONPSTACK);
                sizet_vector_push(&ctx->continue_stmt_idx,ctx->bytes_done+5);
                foo = 0;
                for(size_t i=ctx->localsBegin;i<ctx->locals.size;i++)
                    foo+=((symtable*)ctx->locals.arr[i])->size;
                if(ctx->infor)
                    foo -= 1;
                i32_operand(ctx,foo);
                foo = 0;
                i32_operand(ctx,foo);
                ctx->bytes_done += 1;
            }
        }
        else if (ast->type == call_node)
        {
            const char* name = ast->childs.arr[1]->val;
            bool udf = !function_exists(name);  
            if (udf)
            {
                expr_bytecode(ctx,ast);
                zbytearr_push(&ctx->bytecode,POP_STACK);
                ctx->bytes_done+=1;
            }
            else
            {
                for (size_t k = 0; k < ast->childs.arr[2]->childs.size; k += 1)
                {
                    expr_bytecode(ctx,ast->childs.arr[2]->childs.arr[k]);
                }
                add_lntable_entry(ctx,ctx->bytes_done);
                zbytearr_push(&ctx->bytecode,CALL);
                ctx->bytes_done += 1;
                size_t index = 0;
                bool add = true;
                BuiltinFunc fn_addr;
                bmap_get(&funcs,name,&fn_addr);
                for(index = 0;index < vm_builtin.size;index+=1)
                {
                    if(vm_builtin.arr[index]==fn_addr)
                    {
                        add = false;
                        break;
                    }
                }
                if(add)
                {
                    ptr_vector_push(&vm_builtin,fn_addr);
                    foo = vm_builtin.size-1;
                }
                else
                    foo = index;
                i32_operand(ctx,foo);
                ctx->bytes_done += 1;
                zbytearr_push(&ctx->bytecode,(char)ast->childs.arr[2]->childs.size);

            }
        }
        else if(ast->type == file_node)
        {
            if(ctx->infunc)
                compileError(ctx,"SyntaxError","Error file import inside function!");
            if(ctx->inclass)
                compileError(ctx,"SyntaxError","Error file import inside class!");
                
            const char* C = ctx->filename;
            ctx->filename = ast->childs.arr[1]->val;
            short K = ctx->fileTOP;
            ctx->fileTOP = str_vector_search(ctx->files,ctx->filename);
            bool a = ctx->infunc;
            bool b = ctx->inclass;
            ctx->infunc = false;
            ctx->inclass = false;
            size_t size = compile(ctx,ast->childs.arr[2]);
            ctx->infunc = a;
            ctx->inclass = b;
            ctx->filename = C;
            ctx->fileTOP = K;
        }
        else
        {
            printf("SyntaxError in file %s\n%s\nThe line does not seem to be a meaningful statement!\n", ctx->filename, NodeTypeNames[(int)ast->type]);
            exit(1);
        }
        ctx->last_stmt_type = ast->type;
        ast = ast->childs.arr[ast->childs.size-1];
    }
    return ctx->bytes_done - bytes_done_before;

}
void optimize_jmps(compiler_ctx* ctx,zbytearr* bytecode)
{
    for(size_t i=0; i < ctx->and_jumps.size; i++)
    {
        size_t e = ctx->and_jumps.arr[i];
        int offset;
        memcpy(&offset,bytecode->arr+e+1,4);
        size_t k = e+5+offset;
        int newoffset;
        while(k < ctx->bytes_done && bytecode->arr[k] == JMPIFFALSENOPOP)
        {
            memcpy(&newoffset,bytecode->arr+k+1,4);
            offset+=newoffset+5;
            k = k+5+newoffset;
        }
        memcpy(bytecode->arr+e+1,&offset,4);
    }
    //optimize short circuit jumps for or
    for(size_t i=0; i < ctx->or_jumps.size; i++)
    {
        size_t e = ctx->or_jumps.arr[i];
        int offset;
        memcpy(&offset,bytecode->arr+e+1,4);
        size_t k = e+5+offset;
        int newoffset;
        while(k < ctx->bytes_done && bytecode->arr[k] == NOPOPJMPIF)
        {
            memcpy(&newoffset,bytecode->arr+k+1,4);
            offset+=newoffset+5;
            k = k+5+newoffset;
        }
        memcpy(bytecode->arr+e+1,&offset,4);
    }
}
void compiler_reduce_stack_size(compiler_ctx* ctx,int size)//for REPL
{
    while((int)ctx->STACK_SIZE > size)
    {
        int lastidx = (int)ctx->STACK_SIZE-1;
        int firstidx = (int)size;
        for(size_t i = 0;i < ctx->globals.size; i++)
        {
            if(ctx->globals.table[i].stat == SYMT_OCCUPIED && ctx->globals.table[i].val >= firstidx && ctx->globals.table[i].val <= lastidx)
            {
                ctx->globals.table[i].stat = SYMT_DELETED;
                ctx->STACK_SIZE -= 1;
            }
        }
    }
}
zclass* make_error_class(compiler_ctx* ctx,const char* name,zclass* error)
{
    zclass* k = vm_alloc_zclass(name);
    //int32_t idx = add_to_vm_strings(name);
    strmap_assign(&(k->members),&(error->members));
    symtable_emplace(&ctx->globals,name,ctx->STACK_SIZE++);
    zlist_push(&STACK,zobj_from_class(k));
    return k;
}
void add_builtin(const char* name,BuiltinFunc fn)
{
    bmap_set(&funcs,name,fn);
}
zobject make_argv_list(int argc,const char* argv[])
{
    int32_t k = 2;
    zlist* l = vm_alloc_zlist();
    while (k < argc)
    {
        zstr* p = vm_alloc_zstr(strlen(argv[k]));
        memcpy(p->val,argv[k],strlen(argv[k]));
        zlist_push(l,zobj_from_str_ptr(p));
        k += 1;
    }
    return zobj_from_list(l);
}
zobject make_zfile(FILE* fp)
{
    zfile* f = vm_alloc_zfile();
    f->fp = fp;
    f->open = true;
    return zobj_from_file(f);
}
uint8_t* compile_program(compiler_ctx* ctx,Node* ast,int32_t argc,const char* argv[],int32_t options)
{
    //If prev vector is empty then this program will be compiled as an independent new one
    //otherwise this program will be an addon to the previous one
    //new = prev +curr
    ctx->bytes_done = ctx->bytecode.size;
    ctx->compile_deadcode = (options & OPT_COMPILE_DEADCODE);
    ctx->line_num = 1;
    ctx->and_jumps.size = 0;
    ctx->or_jumps.size = 0;
    ctx->backpatches.size = 0;
    if(ctx->bytecode.size == 0)
    {
        symtable_emplace(&ctx->globals,"argv",0);
        symtable_emplace(&ctx->globals,"stdin",1);
        symtable_emplace(&ctx->globals,"stdout",2);
        symtable_emplace(&ctx->globals,"stderr",3);
        STACK.size = 0;
        zlist_push(&STACK,make_argv_list(argc,argv));
        zlist_push(&STACK,make_zfile(stdin));
        zlist_push(&STACK,make_zfile(stdout));
        zlist_push(&STACK,make_zfile(stderr));
        ctx->STACK_SIZE = STACK.size;
        
        add_to_vm_strings("msg");
        
        zobject nil;
        nil.type = Z_NIL;

        Error = vm_alloc_zclass("Error");
        strmap_emplace(&(Error->members),"msg",nil);
        //Any class inheriting from Error will have 'msg'
        symtable_emplace(&ctx->globals,"Error",ctx->STACK_SIZE);
        add_to_vm_strings("__construct__");
        zfun* fun = vm_alloc_zfun();
        fun->name = "__construct__";
        fun->args = 2;
        fun->i = 5;
        fun->_klass = NULL;
        ptr_vector_push(&vm_important,fun);

        instruction(ctx,JMP);
        i32_operand(ctx,21);
        instruction(ctx,LOAD_LOCAL);
        i32_operand(ctx,0);
        instruction(ctx,LOAD_LOCAL);
        i32_operand(ctx,1);
        instruction(ctx,ASSIGNMEMB);
        i32_operand(ctx,0);
        instruction(ctx,LOAD_LOCAL);
        i32_operand(ctx,0);
        instruction(ctx,OP_RETURN);

        zobject construct;
        construct.type = Z_FUNC;
        construct.ptr = (void*)fun;
        strmap_emplace(&(Error->members),"__construct__",construct);
        zlist_push(&STACK,zobj_from_class(Error));
        ctx->STACK_SIZE+=1;
        TypeError = make_error_class(ctx,"TypeError",Error);
        ValueError = make_error_class(ctx,"ValueError",Error);
        MathError  = make_error_class(ctx,"MathError",Error);
        NameError = make_error_class(ctx,"NameError",Error);
        IndexError = make_error_class(ctx,"IndexError",Error);
        ArgumentError = make_error_class(ctx,"ArgumentError",Error);
        FileIOError = make_error_class(ctx,"FileIOError",Error);
        KeyError = make_error_class(ctx,"KeyError",Error);
        OverflowError = make_error_class(ctx,"OverflowError",Error);
        FileOpenError = make_error_class(ctx,"FileOpenError",Error);
        FileSeekError  = make_error_class(ctx,"FileSeekError",Error);
        ImportError = make_error_class(ctx,"ImportError",Error);
        ThrowError = make_error_class(ctx,"ThrowError",Error);
        MaxRecursionError =make_error_class(ctx,"MaxRecursionError",Error);
    }
    compile(ctx,ast);
    bool pop_globals = !(options & OPT_NOPOP_GLOBALS);
    if(ctx->globals.size!=0 && pop_globals)
    {
        foo = ctx->globals.size;
        instruction(ctx,NPOP_STACK);
        i32_operand(ctx,foo);
    }
    if(!(options & OPT_NOEXIT))
    {
        zbytearr_push(&ctx->bytecode,OP_EXIT);
        ctx->bytes_done+=1;
    }
    if (ctx->bytes_done != ctx->bytecode.size)
    {
        printf("Zuko encountered an internal error.\nError Code: 10\n");
        printf("%zu %zu\n",ctx->bytes_done,ctx->bytecode.size);
        exit(EXIT_FAILURE);
    }
    //final phase
    //apply backpatches
    for(size_t i=0;i<ctx->backpatches.size;i++)
    {
        zpair p = ctx->backpatches.arr[i];
        memcpy(ctx->bytecode.arr+p.x,&p.y,sizeof(int32_t));
    }
    //optimize short circuit jumps for 'and' and 'or' operators
    optimize_jmps(ctx,&ctx->bytecode);
    return ctx->bytecode.arr;
}

void compiler_destroy(compiler_ctx* ctx)
{
    for(size_t i = 0; i < ctx->compiled_functions.size; i++)
        free(ctx->compiled_functions.arr[i]);
    sizet_vector_destroy(&ctx->and_jumps);
    sizet_vector_destroy(&ctx->or_jumps);
    symtable_destroy(&ctx->globals);
    ptr_vector_destroy(&ctx->locals);
    ptr_vector_destroy(&ctx->compiled_functions);
    str_vector_destroy(&ctx->curr_class_members);
    sizet_vector_destroy(&ctx->break_stmt_idx);
    sizet_vector_destroy(&ctx->continue_stmt_idx);    
    str_vector_destroy(&ctx->prefixes);
    str_vector_destroy(&ctx->symRef);
    pair_vector_destroy(&ctx->backpatches);
    bmap_destroy(&funcs);
    free(ctx);
}
