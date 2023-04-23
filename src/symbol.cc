#include "symbol.hh"

bool SymbolTable::contains(std::string key) {
    bool found = false;
    for(auto i : table) {
        if(i.find(key) != i.end()) {
            found = true;
            break;
        }
    }
    return found;
}

bool SymbolTable::containsScope(std::string key) {
    return table.back().find(key) != table.back().end();
}

void SymbolTable::insert(std::string key) {
    table.back().insert(key);
}

void SymbolTable::scope() {
    table.push_back(std::set<std::string>());
}

void SymbolTable::unscope() {
    table.pop_back();
}