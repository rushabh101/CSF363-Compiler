#ifndef LLVMCODEGEN_HH
#define LLVMCODEGEN_HH

#include <llvm/IR/Instructions.h>
#include <string>
#include <stack>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <unordered_map>
#include <list>
#include "ast.hh"

using namespace llvm;

/**
    Compiler struct to store state of the LLVM IRBuilder.
    The `compile` method recursively calls the llvmcodegen method for a given 
    `Node`.
*/

struct SymbolsTable {
    std::list<std::unordered_map<std::string, AllocaInst*>> table;

    AllocaInst* find(std::string key);
    bool containsScope(std::string key, AllocaInst* alloca);
    void insert(std::string key, AllocaInst* alloca);
    void scope();
    void unscope();
};
struct LLVMCompiler {
    LLVMContext *context;
    IRBuilder<> builder;
    Module module;
    std::unordered_map<std::string, AllocaInst*> locals;
    SymbolsTable symbols;
    std::unordered_map<std::string, int> type_scope;

    std::stack<std::string> current_function;
    
    LLVMCompiler(LLVMContext *context, std::string file_name) : 
        context(context), builder(*context), module(file_name, *context) {
        type_scope["i16"] = 16;
        type_scope["i32"] = 32;
        type_scope["i64"] = 64;
        type_scope["short"] = 16;
        type_scope["int"] = 32;
        type_scope["long"] = 64;
        module.getFunction("printi");
    }
    
    void compile(Node *root);
    void dump();
    void write(std::string file_name);
};

#endif