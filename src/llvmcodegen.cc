#include "llvmcodegen.hh"
#include "ast.hh"
#include <iostream>
#include <string>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <vector>

#define MAIN_FUNC compiler->module.getFunction("main")

/*
The documentation for LLVM codegen, and how exactly this file works can be found
ins `docs/llvm.md`
*/

void LLVMCompiler::compile(Node *root) {
    /* Adding reference to print_i in the runtime library */
    // void printi();
    FunctionType *printi_func_type = FunctionType::get(
        builder.getVoidTy(),
        {builder.getInt64Ty()},
        false
    );
    Function::Create(
        printi_func_type,
        GlobalValue::ExternalLinkage,
        "printi",
        &module
    );
    /* we can get this later 
        module.getFunction("printi");
    */

    /* Main Function */
    // int main();
    FunctionType *main_func_type = FunctionType::get(
        builder.getInt32Ty(), {}, false /* is vararg */
    );
    Function *main_func = Function::Create(
        main_func_type,
        GlobalValue::ExternalLinkage,
        "main",
        &module
    );

    // create main function block
    BasicBlock *main_func_entry_bb = BasicBlock::Create(
        *context,
        "entry",
        main_func
    );

    // move the builder to the start of the main function block
    builder.SetInsertPoint(main_func_entry_bb);

    root->llvm_codegen(this);

    // return 0;
    builder.CreateRet(builder.getInt32(0));
}

void LLVMCompiler::dump() {
    outs() << module;
}

void LLVMCompiler::write(std::string file_name) {
    std::error_code EC;
    raw_fd_ostream fout(file_name, EC, sys::fs::OF_None);
    WriteBitcodeToFile(module, fout);
    fout.flush();
    fout.close();
}

//  ┌―――――――――――――――――――――┐  //
//  │ AST -> LLVM Codegen │  //
// └―――――――――――――――――――――┘   //

// codegen for statements
Value *NodeStmts::llvm_codegen(LLVMCompiler *compiler) {
    Value *last = nullptr;
    for(auto node : list) {
        last = node->llvm_codegen(compiler);
    }

    return last;
}

Value *NodeDebug::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);
    Value *temp = compiler->builder.CreateIntCast(expr, compiler->builder.getInt64Ty(), true);

    Function *printi_func = compiler->module.getFunction("printi");
    compiler->builder.CreateCall(printi_func, {temp});

    return expr;
}

Value *NodeInt::llvm_codegen(LLVMCompiler *compiler) {
    if(std::abs(value) <= 32767) {
        return compiler->builder.getInt16(value);
    }
    else if(std::abs(value) <= 2147483647){
        return compiler->builder.getInt32(value);
    }
    else {
        return compiler->builder.getInt64(value);
    }
}

Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler) {
    Value *left_expr = left->llvm_codegen(compiler);
    Value *right_expr = right->llvm_codegen(compiler);

    switch(op) {
        case PLUS:
        return compiler->builder.CreateAdd(left_expr, right_expr, "addtmp");
        case MINUS:
        return compiler->builder.CreateSub(left_expr, right_expr, "minustmp");
        case MULT:
        return compiler->builder.CreateMul(left_expr, right_expr, "multtmp");
        case DIV:
        return compiler->builder.CreateSDiv(left_expr, right_expr, "divtmp");
    }
}


Value *NodeDecl::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    IRBuilder<> temp_builder(
        &MAIN_FUNC->getEntryBlock(),
        MAIN_FUNC->getEntryBlock().begin()
    );

    Type *ty;
    if(compiler->type_scope[dtype] == 16) {
        ty = compiler->builder.getInt16Ty();
    }
    else if(compiler->type_scope[dtype] == 32) {
        ty = compiler->builder.getInt32Ty();
    }
    else {
        ty = compiler->builder.getInt64Ty();
    }

    AllocaInst *alloc = temp_builder.CreateAlloca(ty, 0, identifier);


    compiler->locals[identifier] = alloc;
    Value *temp = compiler->builder.CreateIntCast(expr, ty, true);

    std::string type_str;
    llvm::raw_string_ostream rso(type_str);
    expr->getType()->print(rso);
    std::cout<<identifier<<": "<<rso.str()<<std::endl;

    if(compiler->type_scope[dtype] < compiler->type_scope[rso.str()]) {
        std::cerr << "Error: Value bigger datatype than assignment" << std::endl;
        exit(1);
    }
    return compiler->builder.CreateStore(temp, alloc); // Apparently it implicitly converts now, idk what changed
}

Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler) {
    AllocaInst *alloc = compiler->locals[identifier];

    // if your LLVM_MAJOR_VERSION >= 14
    return compiler->builder.CreateLoad(alloc->getAllocatedType(), alloc, identifier);
}

Value *NodeFunc::llvm_codegen(LLVMCompiler *compiler) {
    FunctionType *main_func_type = FunctionType::get(
        compiler->builder.getInt32Ty(), {}, false /* is vararg */
    );
    Function *main_func = Function::Create(
        main_func_type,
        GlobalValue::ExternalLinkage,
        "main",
        &(compiler->module)
    );

    // create main function block
    BasicBlock *main_func_entry_bb = BasicBlock::Create(
        *(compiler->context),
        "entry",
        main_func
    );

    // move the builder to the start of the main function block
    compiler->builder.SetInsertPoint(main_func_entry_bb);

    // root->llvm_codegen(this);

    // return 0;
    compiler->builder.CreateRet(compiler->builder.getInt32(0));
}

#undef MAIN_FUNC