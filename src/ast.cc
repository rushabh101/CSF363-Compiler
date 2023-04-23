#include "ast.hh"

#include <string>
#include <vector>

NodeBinOp::NodeBinOp(NodeBinOp::Op ope, Node *leftptr, Node *rightptr) {
    type = BIN_OP;
    op = ope;
    left = leftptr;
    right = rightptr;
}

std::string NodeBinOp::to_string() {
    std::string out = "(";
    switch(op) {
        case PLUS: out += '+'; break;
        case MINUS: out += '-'; break;
        case MULT: out += '*'; break;
        case DIV: out += '/'; break;
    }

    out += ' ' + left->to_string() + ' ' + right->to_string() + ')';

    return out;
}

NodeInt::NodeInt(long long val) {
    type = INT_LIT;
    value = val;
}

std::string NodeInt::to_string() {
    return std::to_string(value);
}

NodeStmts::NodeStmts() {
    type = STMTS;
    list = std::vector<Node*>();
}

void NodeStmts::push_back(Node *node) {
    list.push_back(node);
}

std::string NodeStmts::to_string() {
    std::string out = "(begin";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    out += ')';

    return out;
}

NodeArgs::NodeArgs() {
    list = std::vector<NodeArg*>();
}

void NodeArgs::push_back(NodeArg *node) {
    list.push_back(node);
}

std::string NodeArgs::to_string() {
    std::string out = "(";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    out += ')';

    return out;
}

NodeArg::NodeArg(std::string id, std::string d) {
    identifier = id;
    dtype = d;
}

NodeParams::NodeParams() {
    list = std::vector<Node*>();
}

void NodeParams::push_back(Node *node) {
    list.push_back(node);
}

std::string NodeParams::to_string() {
    std::string out = "(";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    out += ')';

    return out;
}

std::string NodeArg::to_string() {
    return "(" + dtype + " " + identifier + ")";
}

NodeDecl::NodeDecl(std::string id, Node *expr, std::string d) {
    type = ASSN;
    identifier = id;
    expression = expr;
    dtype = d;
}

std::string NodeDecl::to_string() {
    return "(let (" + identifier + " " + dtype + ") " + expression->to_string() + ")";
}

NodeDebug::NodeDebug(Node *expr) {
    type = DBG;
    expression = expr;
}

std::string NodeDebug::to_string() {
    return "(dbg " + expression->to_string() + ")";
}

NodeIdent::NodeIdent(std::string ident) {
    identifier = ident;
}
std::string NodeIdent::to_string() {
    return identifier;
}

NodeFunc::NodeFunc(std::string ident, std::string d, NodeStmts *stmts, NodeArgs* args) {
    identifier = ident;
    dtype = d;
    stmtlist = stmts;
    arglist = args;
}

std::string NodeFunc::to_string() {
    return "(fun " + dtype + " " + identifier + " args" + arglist->to_string() + " body" + stmtlist->to_string() + ")";
}

NodeCall::NodeCall(std::string ident, NodeParams* params) {
    identifier = ident;
    paramlist = params;
}

std::string NodeCall::to_string() {
    return "(call " + identifier + " (" + paramlist->to_string() +"))"; 
}

NodeReturn::NodeReturn(Node *expr) {
    expression = expr;
}

std::string NodeReturn::to_string() {
    return "(ret " + expression->to_string() + ")";
}

NodeIfExpr::NodeIfExpr(Node* cond, Node* then, Node* el)
{
    Cond = cond;
    Then  = then;
    Else = el;
}

std::string NodeIfExpr::to_string()
{
    return "(if " + Cond->to_string() + " " + Then->to_string() + " " + Else->to_string() + " )";
}