#ifndef AST_HH
#define AST_HH

#include <llvm/IR/Value.h>
#include <string>
#include <vector>

struct LLVMCompiler;

/**
Base node class. Defined as `abstract`.
*/
struct Node {
    enum NodeType {
        BIN_OP, INT_LIT, STMTS, ASSN, DBG, IDENT
    } type;

    virtual std::string to_string() = 0;
    virtual llvm::Value *llvm_codegen(LLVMCompiler *compiler) = 0;
};

/**
    Node for list of statements
*/
struct NodeStmts : public Node {
    std::vector<Node*> list;

    NodeStmts();
    void push_back(Node *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeArg : public Node {
    std::string identifier;
    std::string dtype;

    NodeArg(std::string id, std::string d);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};
struct NodeArgs : public Node {
    std::vector<NodeArg*> list;

    NodeArgs();
    void push_back(NodeArg *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeParams : public Node {
    std::vector<Node*> list;

    NodeParams();
    void push_back(Node *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for binary operations
*/
struct NodeBinOp : public Node {
    enum Op {
        PLUS, MINUS, MULT, DIV
    } op;

    Node *left, *right;

    NodeBinOp(Op op, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for integer literals
*/
struct NodeInt : public Node {
    long long value;

    NodeInt(long long val);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for variable assignments
*/
struct NodeDecl : public Node {
    std::string identifier;
    Node *expression;
    std::string dtype;

    NodeDecl(std::string id, Node *expr, std::string d);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for `dbg` statements
*/
struct NodeDebug : public Node {
    Node *expression;

    NodeDebug(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for idnetifiers
*/
struct NodeIdent : public Node {
    std::string identifier;

    NodeIdent(std::string ident);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeFunc : public Node {
    std::string identifier;
    std::string dtype;
    NodeStmts *stmtlist;
    NodeArgs *arglist;

    NodeFunc(std::string ident, std::string d, NodeStmts *stmts, NodeArgs *args);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeCall : public Node {
    std::string identifier;
    NodeParams *paramlist;
    NodeCall(std::string ident, NodeParams* params);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeReturn : public Node {
    Node *expression;
    NodeReturn(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};


struct NodeIfExpr : public Node {
    Node* Cond;
    Node* Then;
    Node* Else;

    NodeIfExpr(Node* Cond, Node* Then, Node* Else);
   
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);

};



#endif