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

    root->llvm_codegen(this);

    // // return 0;
    // builder.CreateRet(builder.getInt32(0));
}

Value* TypeConversion(Value *expr, Type* ty, LLVMCompiler *compiler) {
    std::string type_str;
    llvm::raw_string_ostream rso(type_str);
    expr->getType()->print(rso);

    std::string type_str2;
    llvm::raw_string_ostream rso2(type_str2);
    ty->print(rso2);

    // std::cout<<rso.str()<<rso2.str()<<std::endl;
    if(compiler->type_scope[rso2.str()] < compiler->type_scope[rso.str()]) {
        std::cerr << "Error: Value bigger datatype than variable" << std::endl;
        exit(1);
    }

    return compiler->builder.CreateIntCast(expr, ty, true);
}
AllocaInst *CreateEntryBlockAlloca(Function *TheFunction,
                                          StringRef VarName, Type *ty) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(ty, nullptr, VarName);
}

Type* gType(std::string dtype, LLVMCompiler *compiler)  {
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
    return ty;
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

    Type *ty = gType(dtype, compiler);

    std::cout<<"DEBUG: creating alloca for "<<identifier<<" of type "<<dtype<<std::endl;
    Function *TheFunction = compiler->builder.GetInsertBlock()->getParent();
    AllocaInst *alloc = CreateEntryBlockAlloca(TheFunction, identifier, ty);

    compiler->locals[identifier] = alloc;

    // std::string type_str;
    // llvm::raw_string_ostream rso(type_str);
    // expr->getType()->print(rso);

    // if(compiler->type_scope[dtype] < compiler->type_scope[rso.str()]) {
    //     std::cerr << "Error: Value bigger datatype than assignment" << std::endl;
    //     exit(1);
    // }

    Value *temp = TypeConversion(expr, ty, compiler);

    return compiler->builder.CreateStore(temp, alloc);
}

Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler) {
    AllocaInst *alloc = compiler->locals[identifier];

    // if your LLVM_MAJOR_VERSION >= 14
    return compiler->builder.CreateLoad(alloc->getAllocatedType(), alloc, identifier);
}

Value *NodeFunc::llvm_codegen(LLVMCompiler *compiler) {
    Type *ty = gType(dtype, compiler);

    std::vector<Type*> argsT;
    for (auto i: arglist->list) {
        argsT.push_back(gType(i->dtype, compiler));
    }
    FunctionType *main_func_type = FunctionType::get(
        ty, argsT, false /* is vararg */
    );

    Function *main_func = Function::Create(
        main_func_type,
        GlobalValue::ExternalLinkage,
        identifier,
        &(compiler->module)
    );

    // create main function block
    BasicBlock *main_func_entry_bb = BasicBlock::Create(
        *(compiler->context),
        "entry",
        main_func
    );

    int cnt=0;
    for (auto &i: main_func->args()) {
        i.setName(arglist->list[cnt++]->identifier);
    }
    // move the builder to the start of the main function block
    compiler->builder.SetInsertPoint(main_func_entry_bb);

    std::cout<<"DEBUG: allocation arg memory for "<<identifier<<std::endl;
    cnt=0;
    for(auto &i: main_func->args()) {
        AllocaInst *alloca = CreateEntryBlockAlloca(main_func, i.getName(), gType(arglist->list[cnt++]->dtype, compiler));
        compiler->builder.CreateStore(&i, alloca);
        compiler->locals[std::string(i.getName())] = alloca;
    }

    std::cout<<"DEBUG: starting codegen for "<<identifier<<std::endl;

    compiler->current_function.push(identifier);
    Value *r = stmtlist->llvm_codegen(compiler);
    compiler->current_function.pop();
    // return 0;
    if(main_func_entry_bb->getTerminator() == 0) {
        compiler->builder.CreateRet(compiler->builder.CreateIntCast(compiler->builder.getInt32(0), ty, true));
    }
    // compiler->builder.CreateRet(compiler->builder.CreateIntCast(compiler->builder.getInt32(0), ty, true));

    return r;
}

Value *NodeCall::llvm_codegen(LLVMCompiler *compiler) {
    Function *CalleeF = compiler->module.getFunction(identifier);

    std::vector<Value*> params;
    for(auto i: paramlist->list) {
        params.push_back(i->llvm_codegen(compiler));
    }
    return compiler->builder.CreateCall(CalleeF, params, "calltmp");
}

Value *NodeReturn::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);
    Function *f = compiler->module.getFunction(compiler->current_function.top());
    Type *ty = f->getReturnType();
    return compiler->builder.CreateRet(TypeConversion(expr, ty, compiler));
}

Value *NodeArgs::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}

Value *NodeArg::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}

Value *NodeParams::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}

#undef MAIN_FUNC