#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <list>
#include <set>
#include <string>
#include "ast.hh"


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::list<std::set<std::string>> table;

    bool contains(std::string key);
    void insert(std::string key);
    void scope();
    void unscope();
};

#endif