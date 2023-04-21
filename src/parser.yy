%define api.value.type { ParserValue }

%code requires {
#include <iostream>
#include <vector>
#include <string>

#include "parser_util.hh"
#include "symbol.hh"

}

%code {

#include <cstdlib>

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTable symbol_table, func_table;

int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT DTYPE
%token TLET TDBG TFUN
%token TSCOL TLPAREN TRPAREN TLCURL TRCURL TEQUAL
%token TQM TCOLON

%type <node> Expr Stmt
%type <stmts> Program StmtList



%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | StmtList 
        { final_values = $1; }
	    ;

StmtList : Stmt                
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt 
         { $$->push_back($2); }
	     ;

Stmt : TFUN TIDENT TLPAREN TRPAREN TCOLON DTYPE TLCURL StmtList TRCURL
     {
        if(func_table.contains($2)) {
            // tried to redeclare function, so error
            yyerror("tried to redeclare function.\n");
        } else {
            func_table.insert($2);
            $$ = new NodeFunc($2, $8);
        }
     }
     | TLET TIDENT TCOLON DTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2);
            $$ = new NodeDecl($2, $6, $4);
        }
     }
     | TDBG Expr TSCOL
     { 
        $$ = new NodeDebug($2);
     }
     ;

Expr : TINT_LIT               
     { $$ = new NodeInt(stoll($1)); }
     | TIDENT
     { 
        if(symbol_table.contains($1))
            $$ = new NodeIdent($1); 
        else
            yyerror("using undeclared variable.\n");
     }
     | Expr TPLUS Expr
     { $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); }
     | Expr TDASH Expr
     { $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); }
     | Expr TSTAR Expr
     { $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); }
     | Expr TSLASH Expr
     { $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); }
     | TLPAREN Expr TRPAREN { $$ = $2; }
     ;

%%

int yyerror(std::string msg) {
    std::cerr << "Error: Invalid Syntax " << msg << std::endl;
    exit(1);
}
