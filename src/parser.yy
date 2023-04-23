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
%token TLET TDBG TFUN TRET
%token TSCOL TLPAREN TRPAREN TLCURL TRCURL TEQUAL TCOMMA
%token TQM TCOLON
%token TIF TELSE 

%type <node> Expr Stmt
%type <arg> Arg
%type <stmts> Program StmtList
%type <args> ArgList
%type <params> ParaList



%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | {func_table.scope();} StmtList 
        { final_values = $2; }
	    ;

StmtList :
         { $$ = new NodeStmts(); } 
         | Stmt                
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt 
         { $$->push_back($2); }
	     ;

Stmt : TFUN {symbol_table.scope();} TIDENT TLPAREN ArgList TRPAREN TCOLON DTYPE TLCURL StmtList TRCURL
     {
        if(func_table.contains($3)) {
            // tried to redeclare function, so error
            yyerror("tried to redeclare function.\n");
        } else {
            func_table.insert($3);
            $$ = new NodeFunc($3, $8 ,$10, $5);
        }

        symbol_table.unscope();
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
     | TRET Expr TSCOL
     {
        $$ = new NodeReturn($2);
     }
     | TIF {symbol_table.scope();} Expr TLCURL StmtList TRCURL TELSE {symbol_table.unscope(); symbol_table.scope();} TLCURL StmtList TRCURL
     {
        if (typeid(*$3) == typeid(NodeInt)){
            std::cout << "Integer Found in IF" << std::endl;
            NodeInt* temp_3 = dynamic_cast<NodeInt*>($3);
            if(temp_3->value != 0)
                $$ = $5;
            else
                $$ = $10;
        }
        else{
            $$ = new NodeIfExpr($3, $5, $10);
        }

        symbol_table.unscope();
        
     }
     | Expr TSCOL
     {
        $$ = $1;
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
     { 
        if (typeid(*$1) == typeid(NodeInt)){
            NodeInt* temp_1 = dynamic_cast<NodeInt*>($1);
            NodeInt* temp_3 = dynamic_cast<NodeInt*>($3);
            $$ = new NodeInt(temp_1->value + temp_3->value);
        }
        else{
            $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); 
        }

     }
     | Expr TDASH Expr
     { 
        if (typeid(*$1) == typeid(NodeInt)){
            NodeInt* temp_1 = dynamic_cast<NodeInt*>($1);
            NodeInt* temp_3 = dynamic_cast<NodeInt*>($3);
            $$ = new NodeInt(temp_1->value - temp_3->value);
        }
        else{
            $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); 
        }
     }
     | Expr TSTAR Expr
     { 
        if (typeid(*$1) == typeid(NodeInt)){
            NodeInt* temp_1 = dynamic_cast<NodeInt*>($1);
            NodeInt* temp_3 = dynamic_cast<NodeInt*>($3);
            $$ = new NodeInt(temp_1->value * temp_3->value);
        }
        else{
            $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); 
        }
     }
     | Expr TSLASH Expr
     { 
        if (typeid(*$1) == typeid(NodeInt)){
            NodeInt* temp_1 = dynamic_cast<NodeInt*>($1);
            NodeInt* temp_3 = dynamic_cast<NodeInt*>($3);
            $$ = new NodeInt(temp_1->value / temp_3->value);
        }
        else{
            $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); 
        } 
     }
     | TLPAREN Expr TRPAREN { $$ = $2; }
     | TIDENT TLPAREN ParaList TRPAREN
     {
        if(!func_table.contains($1)) {
            yyerror("Function not declared.\n");
        }

        $$ = new NodeCall($1, $3);
     }
     ;


ArgList :
        {
            $$ = new NodeArgs();
        }
        | Arg
        {
            $$ = new NodeArgs();
            $$->push_back($1);
        }
        |
          ArgList TCOMMA Arg
        {
            $$->push_back($3);
        }
        ;
        
Arg     : TIDENT TCOLON DTYPE
        {
            if(symbol_table.contains($1)) {
                // tried to redeclare variable, so error
                yyerror("tried to redeclare variable.\n");
            } else {
                symbol_table.insert($1);
                $$ = new NodeArg($1, $3);

            }
        }
        ;

ParaList :
        {
            $$ = new NodeParams();
        }
        | Expr
        {
            $$ = new NodeParams();
            $$->push_back($1);
        }
        |
          ParaList TCOMMA Expr
        {
            $$->push_back($3);
        }
        ;

%%
int yyerror(std::string msg) {
    std::cerr << "Error: Invalid Syntax " << msg << std::endl;
    exit(1);
}
