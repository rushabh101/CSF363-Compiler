/* just like Unix wc */
%option noyywrap
%option prefix="foo"

%x comment
%x comment2
%x DEFINE
%x DEFINE2
%x UNDEF
%x ifdefMacro1
%x skipMacro
%x trueSkipMacro
%s ifdefMacro

%{
#include <string>
#include <unordered_map>
using namespace std;

string key;
unordered_map<string, string> map;
%}
%%

"#def " {BEGIN(DEFINE); return 1;}
<DEFINE>[a-zA-Z0-9_]+ {key = yytext; map[key]="1"; return 1;}
<DEFINE>[\n]+ {BEGIN(INITIAL); return 1;}
<DEFINE>" " {BEGIN(DEFINE2); return 1;}
<DEFINE2>[^\\\n]+ {if(map[key] == "1") map[key] = ""; map[key] += yytext; return 5;}
<DEFINE2>"\\\n" {return 1;}
<DEFINE2>[\n]+ {BEGIN(INITIAL); return 1;}

"#undef " {BEGIN(UNDEF); return 2;}
<UNDEF>[a-zA-Z0-9_]+ {map.erase(yytext); return 2;}
<UNDEF>[ \n]+ {BEGIN(INITIAL); return 2;}


"/*"         BEGIN(comment);
<comment>[^*]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/]*   /* eat up '*'s not followed by '/'s */
<comment>"*"+"/"        {BEGIN(INITIAL);}

"//"    BEGIN(comment2);
<comment2>. /* om nom */
<comment2>[ \n]+ {BEGIN(INITIAL);}

"#ifdef "   {BEGIN(ifdefMacro1);}
<ifdefMacro1>[a-zA-Z0-9]+ { 
            if (map.find(yytext) != map.end()) {
                // printf("YESSS\n");
                BEGIN(ifdefMacro);
            } else {
                BEGIN(skipMacro);
            }
        }
<skipMacro>"#elif" {BEGIN(ifdefMacro1);}
<ifdefMacro>"#elif" {BEGIN(trueSkipMacro);}
<ifdefMacro>"#else" {BEGIN(trueSkipMacro);}
<skipMacro>"#else" {BEGIN(ifdefMacro);}
<ifdefMacro,skipMacro,trueSkipMacro>"#endif" {BEGIN(INITIAL);}
<trueSkipMacro,skipMacro>.|\n {}

[a-zA-Z0-9_]+ {return 3;}
.|\n {return 4;}
%%