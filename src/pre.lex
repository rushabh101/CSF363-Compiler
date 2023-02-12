/* just like Unix wc */
%option noyywrap
%option prefix="foo"

%x comment
%x DEFINE
%x DEFINE2
%{
#include <string>
#include <unordered_map>
using namespace std;

string key;
unordered_map<string, string> map;
%}
%%

"#def " BEGIN(DEFINE);
<DEFINE>[a-zA-Z]+ {key = yytext;}
<DEFINE>" " BEGIN(DEFINE2);
<DEFINE2>[a-zA-Z]+ { map[key] = yytext; printf("mapped: %s\n", yytext);}
<DEFINE2>[ \n]+ BEGIN(INITIAL);

"/*"         BEGIN(comment);
<comment>[^*]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/]*   /* eat up '*'s not followed by '/'s */
<comment>"*"+"/"        BEGIN(INITIAL);
%%