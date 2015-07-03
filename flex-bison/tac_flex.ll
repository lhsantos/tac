%{
    #include <string>
    #include <cmath>
    #include <stdint.h>
    
    #include "scanner.hpp"
    #include "interpreter.hpp"

    //#define YY_NO_UNISTD_H
    
    typedef tac::Parser::token token;
    
%}

%option debug
%option yyclass="Scanner" 
%option noyywrap 
%option c++
%option never-interactive

%x STRING_LIT
%x CHAR_LIT
%x LN_COMMENT

S   [+\-]
O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
EOL  "\n"|"\r"|"\r\n"
WS  [ \t\v\f]


%%

%{
    yylloc->step();
%}

"//"                { BEGIN(LN_COMMENT); }
<LN_COMMENT>{EOL}   { yylloc->lines(1); yylloc->step(); BEGIN(INITIAL); return token::EOL; }
<LN_COMMENT>.       { }

".table"            { yylloc->columns(yyleng); return token::TABLE; }
".code"             { yylloc->columns(yyleng); return token::CODE; }
"char"              { yylloc->columns(yyleng); return token::CHAR; }
"int"               { yylloc->columns(yyleng); return token::INT; }
"float"             { yylloc->columns(yyleng); return token::FLOAT; }
"add"               { yylloc->columns(yyleng); return token::ADD; }
"sub"               { yylloc->columns(yyleng); return token::SUB; }
"mul"               { yylloc->columns(yyleng); return token::MUL; }
"div"               { yylloc->columns(yyleng); return token::DIV; }
"and"               { yylloc->columns(yyleng); return token::AND; }
"or"                { yylloc->columns(yyleng); return token::OR; }
"band"              { yylloc->columns(yyleng); return token::BAND; }
"bor"               { yylloc->columns(yyleng); return token::BOR; }
"bxor"              { yylloc->columns(yyleng); return token::BXOR; }
"bnot"              { yylloc->columns(yyleng); return token::BNOT; }
"shl"               { yylloc->columns(yyleng); return token::SHL; }
"shr"               { yylloc->columns(yyleng); return token::SHR; }
"mod"               { yylloc->columns(yyleng); return token::MOD; }
"seq"               { yylloc->columns(yyleng); return token::SEQ; }
"slt"               { yylloc->columns(yyleng); return token::SLT; }
"sleq"              { yylloc->columns(yyleng); return token::SLEQ; }
"minus"             { yylloc->columns(yyleng); return token::MINUS; }
"not"               { yylloc->columns(yyleng); return token::NOT; }
"chtoint"           { yylloc->columns(yyleng); return token::CHTOINT; }
"chtofl"            { yylloc->columns(yyleng); return token::CHTOFL; }
"inttofl"           { yylloc->columns(yyleng); return token::INTTOFL; }
"inttoch"           { yylloc->columns(yyleng); return token::INTTOCH; }
"fltoint"           { yylloc->columns(yyleng); return token::FLTOINT; }
"fltoch"            { yylloc->columns(yyleng); return token::FLTOCH; }
"brz"               { yylloc->columns(yyleng); return token::BRZ; }
"brnz"              { yylloc->columns(yyleng); return token::BRNZ; }
"mov"               { yylloc->columns(yyleng); return token::MOV; }
"jump"              { yylloc->columns(yyleng); return token::JUMP; }
"param"             { yylloc->columns(yyleng); return token::PARAM; }
"print"             { yylloc->columns(yyleng); return token::PRINT; }
"println"           { yylloc->columns(yyleng); return token::PRINTLN; }
"scanc"             { yylloc->columns(yyleng); return token::SCANC; }
"scani"             { yylloc->columns(yyleng); return token::SCANI; }
"scanf"             { yylloc->columns(yyleng); return token::SCANF; }
"mema"              { yylloc->columns(yyleng); return token::MEMA; }
"memf"              { yylloc->columns(yyleng); return token::MEMF; }
"rand"              { yylloc->columns(yyleng); return token::RAND; }
"call"              { yylloc->columns(yyleng); return token::CALL; }
"return"            { yylloc->columns(yyleng); return token::RETURN; }
"push"              { yylloc->columns(yyleng); return token::PUSH; }
"pop"               { yylloc->columns(yyleng); return token::POP; }
"nop"               { yylloc->columns(yyleng); return token::NOP; }

{L}{A}*             { yylloc->columns(yyleng); yylval->sval = new std::string(yytext); return token::IDENTIFIER; }

"$"{D}+             { yylloc->columns(yyleng); yylval->ival = strtol(yytext + 1, 0, 10); return token::TEMPORARY; }
"$s"                { yylloc->columns(yyleng); yylval->ival = Interpreter::STACK_REG_CODE; return token::SP_TEMPORARY; }
"$f"                { yylloc->columns(yyleng); yylval->ival = Interpreter::FRAME_REG_CODE; return token::SP_TEMPORARY; }
"$pc"               { yylloc->columns(yyleng); yylval->ival = Interpreter::PC_REG_CODE; return token::SP_TEMPORARY; }
"$ra"               { yylloc->columns(yyleng); yylval->ival = Interpreter::RA_REG_CODE; return token::SP_TEMPORARY; }
"#"{D}+             { yylloc->columns(yyleng); yylval->ival = strtol(yytext + 1, 0, 10); return token::PARAMETER; }

{HP}{H}+{IS}?           { yylloc->columns(yyleng); yylval->ival = strtol(yytext, 0, 16); return token::I_CONSTANT; }
{S}?{NZ}{D}*{IS}?       { yylloc->columns(yyleng); yylval->ival = strtol(yytext, 0, 10); return token::I_CONSTANT; }
"0"{O}*{IS}?            { yylloc->columns(yyleng); yylval->ival = strtol(yytext, 0, 8); return token::I_CONSTANT; }

{S}?{D}+{E}{FS}?            { yylloc->columns(yyleng); yylval->fval = (float) strtod(yytext, 0); return token::F_CONSTANT; }
{S}?{D}*"."{D}+{E}?{FS}?    { yylloc->columns(yyleng); yylval->fval = (float) strtod(yytext, 0); return token::F_CONSTANT; }
{S}?{D}+"."{E}?{FS}?        { yylloc->columns(yyleng); yylval->fval = (float) strtod(yytext, 0); return token::F_CONSTANT; }
{HP}{H}+{P}{FS}?            { yylloc->columns(yyleng); yylval->fval = (float) strtod(yytext, 0); return token::F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?     { yylloc->columns(yyleng); yylval->fval = (float) strtod(yytext, 0); return token::F_CONSTANT; }
{HP}{H}+"."{P}{FS}?         { yylloc->columns(yyleng); yylval->fval = (float) strtod(yytext, 0); return token::F_CONSTANT; }


\'              {
                    yylloc->columns(yyleng);
                    m_buffer.clear();
                    BEGIN(CHAR_LIT);
                }
<CHAR_LIT>\'    {
                    yylloc->columns(yyleng);
                    yylval->cval = m_buffer.front();
                    BEGIN(INITIAL);
                    return token::C_CONSTANT;
                }
<CHAR_LIT>\\[\\\'\"\?]      { yylloc->columns(yyleng); m_buffer.push_back(yytext[1]); }
<CHAR_LIT>\\a               { yylloc->columns(yyleng); m_buffer.push_back('\a'); }
<CHAR_LIT>\\b               { yylloc->columns(yyleng); m_buffer.push_back('\b'); }
<CHAR_LIT>\\f               { yylloc->columns(yyleng); m_buffer.push_back('\f'); }
<CHAR_LIT>\\n               { yylloc->columns(yyleng); m_buffer.push_back('\n'); }
<CHAR_LIT>\\r               { yylloc->columns(yyleng); m_buffer.push_back('\r'); }
<CHAR_LIT>\\t               { yylloc->columns(yyleng); m_buffer.push_back('\t'); }
<CHAR_LIT>\\v               { yylloc->columns(yyleng); m_buffer.push_back('\v'); }
<CHAR_LIT>\\0[0-7]+         { yylloc->columns(yyleng); m_buffer.push_back((char) strtol(yytext + 1, 0, 8)); }
<CHAR_LIT>\\[0-9]+          { yylloc->columns(yyleng); m_buffer.push_back((char) strtol(yytext + 1, 0, 10)); }
<CHAR_LIT>\\x[a-fA-F0-9]+   { yylloc->columns(yyleng); m_buffer.push_back((char) strtol(yytext + 2, 0, 16)); }
<CHAR_LIT>[^\'\\\n\r]+      { yylloc->columns(yyleng); m_buffer.insert(m_buffer.end(), yytext, yytext + yyleng); }

\"              {
                    yylloc->columns(yyleng);
                    m_buffer.clear();
                    BEGIN(STRING_LIT);
                }
<STRING_LIT>\"  {
                    yylloc->columns(yyleng);
                    yylval->sval = new std::string(m_buffer.begin(), m_buffer.end());
                    BEGIN(INITIAL);
                    return token::STRING_LITERAL;
                }
<STRING_LIT>\\[\\\'\"\?]    { yylloc->columns(yyleng); m_buffer.push_back(yytext[1]); }
<STRING_LIT>\\a             { yylloc->columns(yyleng); m_buffer.push_back('\a'); }
<STRING_LIT>\\b             { yylloc->columns(yyleng); m_buffer.push_back('\b'); }
<STRING_LIT>\\f             { yylloc->columns(yyleng); m_buffer.push_back('\f'); }
<STRING_LIT>\\n             { yylloc->columns(yyleng); m_buffer.push_back('\n'); }
<STRING_LIT>\\r             { yylloc->columns(yyleng); m_buffer.push_back('\r'); }
<STRING_LIT>\\t             { yylloc->columns(yyleng); m_buffer.push_back('\t'); }
<STRING_LIT>\\v             { yylloc->columns(yyleng); m_buffer.push_back('\v'); }
<STRING_LIT>\\0[0-7]+       { yylloc->columns(yyleng); m_buffer.push_back((char) strtol(yytext + 1, 0, 8)); }
<STRING_LIT>\\[0-9]+        { yylloc->columns(yyleng); m_buffer.push_back((char) strtol(yytext + 1, 0, 10)); }
<STRING_LIT>\\x[a-fA-F0-9]+ { yylloc->columns(yyleng); m_buffer.push_back((char) strtol(yytext + 2, 0, 16)); }
<STRING_LIT>[^\"\\\n\r]+    { yylloc->columns(yyleng); m_buffer.insert(m_buffer.end(), yytext, yytext + yyleng); }

{EOL}   { yylloc->lines(1); return token::EOL; }
{WS}+   { yylloc->columns(yyleng); yylloc->step(); }
.       { yylloc->columns(yyleng); return *yytext; }

%%
