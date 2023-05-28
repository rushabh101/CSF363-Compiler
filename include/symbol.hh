#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <list>
#include <set>
#include <string>
#include "ast.hh"


// symbol table with scoping
struct SymbolTable {
    std::list<std::set<std::string>> table;

    bool contains(std::string key);
    bool containsScope(std::string key);
    void insert(std::string key);
    void scope();
    void unscope();
};

#endif