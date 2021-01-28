%skeleton "lalr1.cc"
%debug
%define parse.error verbose
%defines
%locations
%define api.namespace {tac}
%define api.parser.class {Parser}

%code requires
{
    #include <utility>
    #include <set>
    #include <sstream>
    #include <cstring>
    
    #include "error.hpp"
    #include "instruction.hpp"
    
    #define ON_ERROR(msg, p) errors.push_back(Error(ERROR, msg, *p.filename, p.line, p.column))
    
    namespace tac
    {
        class Scanner;
        struct Type;
    }
}

%union
{
    std::string *sval;
    int ival;
    char cval;
    float fval;
    Type *type;
    Symbol *symbol;
    std::vector<Symbol*> *symbol_list;
    Instruction::OpCode opcode;
    Instruction* instr;
    std::vector<Instruction*> *instr_list;
};

%lex-param { Scanner& scanner }
%parse-param { Scanner& scanner }
%parse-param { SymbolTable* table }
%parse-param { std::vector<Instruction*>*& instructions }
%parse-param { uint& start_address }
%parse-param { std::string file }
%parse-param { std::list<tac::Error>& errors }

%token <sval> IDENTIFIER "identifier"
%token <ival> TEMPORARY "temp_ref"
%token <ival> SP_TEMPORARY "special reg"
%token <ival> PARAMETER "param_ref"
%token <ival> I_CONSTANT "integer constant"
%token <cval> C_CONSTANT "char constant"
%token <fval> F_CONSTANT "float constant"
%token <sval> STRING_LITERAL "literal"

%token TABLE ".table"
%token CODE ".code"

%token CHAR INT FLOAT
%token ADD SUB MUL DIV AND OR BAND BOR BXOR SHL SHR MOD SEQ SLT SLEQ
%token MINUS NOT BNOT CHTOINT CHTOFL INTTOFL INTTOCH FLTOINT FLTOCH BRZ BRNZ
%token MOV
%token JUMP PARAM PRINT PRINTLN SCANC SCANI SCANF MEMA MEMF RAND CALL RETURN PUSH POP NOP
%token EOL

%type <symbol> symbol
%type <symbol> name_and_type
%type <type> type
%type <symbol> constant
%type <symbol> arr_size
%type <symbol_list> arr_constant
%type <symbol_list> constant_list
%type <instr_list> code_block
%type <instr_list> instruction_list
%type <instr> instruction
%type <instr> cmd
%type <symbol> addressable
%type <symbol> target
%type <symbol> operand
%type <instr> binary_operation
%type <opcode> binary_opname
%type <instr> unary_operation
%type <opcode> unary_opname
%type <instr> assignment
%type <instr> single_operand_cmd
%type <instr> mema
%type <instr> branch
%type <instr> call
%type <instr> ret

%start tac_program

/* Done beefore parsing starts. */
%initial-action
{
    /* Stores the root symbol table. */
    g_root_table = table;
    
    /* Resets program counter. */
    g_program_counter = 0;

    /* Sets the initial location. */
    @$.begin.filename = @$.end.filename = &file;
};

/* Destroys a string. */
%destructor { delete $$; } <sval>

/* Never destroy symbols if the table has them... */
%destructor
{
    if ($$ && (!$$->registered))
        delete $$;
} <symbol>
%destructor { destroy_list($$); } <symbol_list>

%code
{
    using namespace tac;
    
    static SymbolTable* g_root_table = 0;
    static uint g_program_counter = 0;
    
    static int yylex(
        Parser::semantic_type* yylval,
        Parser::location_type* yylloc,
        Scanner& scanner);
    
    Symbol* make_char_const(char, const location&);
    std::vector<Symbol*>* make_char_array(std::string*, const location&);
    std::vector<Symbol*>* make_empty_array();
    void destroy_list(std::vector<Symbol*>*);
    bool register_symbol(Symbol*, std::list<Error>&);
}

%%

tac_program:
    table_block code_block { instructions = $2; start_address = table->upper_bound(); }
    | code_block { instructions = $1; }
;


table_block:
    EOL table_block
    | TABLE symbol_list { g_program_counter = table->upper_bound(); }
;


/* Registers all valid symbols at the table. */
symbol_list:
    /* empty */
    
    | symbol_list symbol
    {
        Symbol *s = $2;
        if (s) register_symbol(s, errors);
    }
    
    | symbol_list EOL
;


symbol:
    name_and_type EOL { $$ = $1; }
    
    | name_and_type '=' constant EOL
    {
        Symbol *s = $1;
        Symbol *c = $3;
        $$ = 0;
        
        if (s && c)
        {
            if (s->type->kind != c->type->kind)
            {
                ON_ERROR("invalid constant initializer for type '" +  s->type->to_str() + "'", @3.begin);
                delete s;
            }
            else
            {
                s->value = c->value;
                memset(&c->value, 0, sizeof(c->value));
                $$ = s;
            }
        }
        delete c;
    }
    
    | name_and_type arr_size EOL
    {
        Symbol *s = $1;
        Symbol *size = $2;
        $$ = 0;
        
        if (s && size)
        {
            s->type->array_size = (size_t) size->value.ival;
            s->value.arrval = new std::vector<Symbol*>();
            for (uint i = 0; i < s->type->array_size; ++i)
                s->value.arrval->push_back(
                    new Symbol(SymbolTable::unique_id(0), @$, Symbol::VAR, new Type(*s->type))
                );
            $$ = s;
        }
        else
            delete s;

        delete size;
    }
    
    | name_and_type '[' ']' '=' arr_constant EOL
    {
        Symbol *s = $1;
        std::vector<Symbol*> *init = $5;
        $$ = 0;
        
        if (s && init)
        {
            s->type->array_size = init->size();
            s->value.arrval = init;
            $$ = s;
        }
        else
        {
            delete s;
            destroy_list(init);
        }
    }
    
    | name_and_type arr_size '=' arr_constant EOL
    {
        Symbol *s = $1;
        Symbol *size = $2;
        std::vector<Symbol*> *init = $4;
        $$ = 0;
        
        if (s && size && init)
        {
            if (s->type->kind == init->back()->type->kind)
            {
                if (size->value.ival == (int) init->size())
                {
                    s->type->array_size = (size_t) size->value.ival;
                    s->value.arrval = init;
                    $$ = s;
                }
                else
                    ON_ERROR("array initializer with incompatible size", @4.begin);
            }
            else
                ON_ERROR("invalid constant array initializer for type '" +  s->type->to_str() + "'", @4.begin);
        }
        
        if (!$$)
        {
            delete s;
            destroy_list(init);
        }
        delete size;
    }
;


name_and_type:
    type IDENTIFIER { $$ = new Symbol($2, @2, Symbol::VAR, $1); }
    ;


type:
    CHAR { $$ = new Type(Type::CHAR); }
    
    | INT { $$ = new Type(Type::INT); }
    
    | FLOAT { $$ = new Type(Type::FLOAT); }
;


constant:
    I_CONSTANT
    {
        $$ = new Symbol(SymbolTable::unique_id($1), @1, Symbol::CONST, new Type(Type::INT));
        $$->value.ival = $1;
    }
    
    | C_CONSTANT
    {
        $$ = make_char_const($1, @1);
    }
    
    | F_CONSTANT
    {
        $$ = new Symbol(SymbolTable::unique_id($1), @1, Symbol::CONST, new Type(Type::FLOAT));
        $$->value.fval = $1;
    }
    ;


arr_size:
    '[' I_CONSTANT ']'
    {
        $$ = 0;
        if ($2 > 0)
        {
            $$ = new Symbol(SymbolTable::unique_id($2), @2, Symbol::CONST, new Type(Type::INT));
            $$->value.ival = $2;
        }
        else
            ON_ERROR("array size must be a positive integer", @2.begin);
    }
    ;


arr_constant:
    STRING_LITERAL
    {
        $$ = make_char_array($1, @1);
        delete $1;
    }

    | '{' constant_list '}' { $$ = $2; }
    ;


constant_list:
    constant
    {
        $$ = new std::vector<Symbol*>();
        $$->push_back($1);
    }

    | constant_list ',' constant
    {
        std::vector<Symbol*>* list = $1;
        Symbol* c = $3;
        $$ = 0;
        
        if (list)
        {
            if (list->back()->type->kind == c->type->kind)
            {
                list->push_back(c);
                $$ = list;
            }
            else
            {
                ON_ERROR("different constant types in array initializer", @3.begin);
                delete c;
                destroy_list(list);
            }
        }
        else
            delete c;
    }
    ;


code_block:
    CODE instruction_list { $$ = $2; }
;


instruction_list:
    /* empty */
    { $$ = new std::vector<Instruction*>(); }
    
    | instruction_list EOL { $$ = $1; }
    
    | instruction_list instruction EOL
    {
        if ($2)
        {
            $$ = $1;
            $$->push_back($2);
            ++g_program_counter;
        }
    }
;


instruction:
    label_list cmd { $$ = $2; }
;


label_list:
    /* empty */
    | label_list label
;


label:
    IDENTIFIER ':' empty_lines
    {
        Symbol *s = new Symbol($1, @1, Symbol::LABEL, new Type(Type::ADDR));
        s->value.addrval = g_program_counter;
        
        if (!register_symbol(s, errors))
            delete s;
    }
;


empty_lines:
    /* empty */
    | empty_lines EOL
;


cmd:
    binary_operation
    | unary_operation
    | assignment
    | single_operand_cmd
    | mema
    | branch
    | call
    | ret
;

addressable:
    IDENTIFIER { $$ = new Symbol($1, @1, Symbol::VAR); }
    
    | PARAMETER
    {
        $$ = new Symbol(SymbolTable::unique_id((int) ($1)), @1, Symbol::PARAM);
        $$->value.addrval = $1;
    }
    ;

    
target:
    addressable { $$ = $1; }

    | TEMPORARY
    {
        $$ = new Symbol(SymbolTable::unique_id((int) ($1)), @1, Symbol::TEMP);
        $$->value.addrval = $1;
    }
    ;


operand:
    target
    
    | SP_TEMPORARY
    {
        $$ = new Symbol(SymbolTable::unique_id((int) ($1)), @1, Symbol::TEMP);
        $$->value.addrval = $1;
    }
    
    | constant
    ;

    
binary_operation:
    binary_opname target ',' operand ',' operand { $$ = new Instruction(@$, $1, $2, $4, $6); }
    ;


binary_opname:
    ADD { $$ = Instruction::ADD; }
    | SUB { $$ = Instruction::SUB; }
    | MUL { $$ = Instruction::MUL; }
    | DIV { $$ = Instruction::DIV; }
    | AND { $$ = Instruction::AND; }
    | OR { $$ = Instruction::OR; }
    | BAND { $$ = Instruction::BAND; }
    | BOR { $$ = Instruction::BOR; }
    | BXOR { $$ = Instruction::BXOR; }
    | SHL { $$ = Instruction::SHL; }
    | SHR { $$ = Instruction::SHR; }
    | MOD { $$ = Instruction::MOD; }
    | SEQ { $$ = Instruction::SEQ; }
    | SLT { $$ = Instruction::SLT; }
    | SLEQ { $$ = Instruction::SLEQ; }
    ;


unary_operation:
    unary_opname target ',' operand { $$ = new Instruction(@$, $1, $2, $4); }
    ;


unary_opname:
    MINUS { $$ = Instruction::MINUS; }
    | NOT { $$ = Instruction::NOT; }
    | BNOT { $$ = Instruction::BNOT; }
    | CHTOINT { $$ = Instruction::CHTOINT; }
    | CHTOFL { $$ = Instruction::CHTOFL; }
    | INTTOFL { $$ = Instruction::INTTOFL; }
    | INTTOCH { $$ = Instruction::INTTOCH; }
    | FLTOINT { $$ = Instruction::FLTOINT; }
    | FLTOCH { $$ = Instruction::FLTOCH; }
    ;


assignment:
    MOV target ',' operand { $$ = new Instruction(@$, Instruction::MOVVV, $2, $4); }
    | MOV target ',' '*' target { $$ = new Instruction(@$, Instruction::MOVVD, $2, $5); }
    | MOV target ',' '&' addressable { $$ = new Instruction(@$, Instruction::MOVVA, $2, $5); }
    | MOV target ',' target '[' operand ']' { $$ = new Instruction(@$, Instruction::MOVVI, $2, $4, $6); }
    | MOV '*' target ',' operand { $$ = new Instruction(@$, Instruction::MOVDV, $3, $5); }
    | MOV '*' target ',' '*' target { $$ = new Instruction(@$, Instruction::MOVDD, $3, $6); }
    | MOV '*' target ',' '&' addressable { $$ = new Instruction(@$, Instruction::MOVDA, $3, $6); }
    | MOV '*' target ',' target '[' operand ']' { $$ = new Instruction(@$, Instruction::MOVDI, $3, $7, $5); }
    | MOV target '[' operand ']' ',' operand { $$ = new Instruction(@$, Instruction::MOVIV, $2, $7, $4); }
    | MOV target '[' operand ']' ',' '*' target { $$ = new Instruction(@$, Instruction::MOVID, $2, $8, $4); }
    | MOV target '[' operand ']' ',' '&' addressable { $$ = new Instruction(@$, Instruction::MOVIA, $2, $8, $4); }
    ;


single_operand_cmd:
    JUMP operand { $$ = new Instruction(@$, Instruction::JUMP, $2); }
    | PARAM operand { $$ = new Instruction(@$, Instruction::PARAM, $2); }
    | PRINT operand { $$ = new Instruction(@$, Instruction::PRINT, $2); }
    | PRINTLN { $$ = new Instruction(@$, Instruction::PRINTLN); }
    | PRINTLN operand { $$ = new Instruction(@$, Instruction::PRINTLN, $2); }
    | SCANC target { $$ = new Instruction(@$, Instruction::SCANC, $2); }
    | SCANI target { $$ = new Instruction(@$, Instruction::SCANI, $2); }
    | SCANF target { $$ = new Instruction(@$, Instruction::SCANF, $2); }
    | MEMF operand { $$ = new Instruction(@$, Instruction::MEMF, $2); }
    | PUSH operand { $$ = new Instruction(@$, Instruction::PUSH, $2); }
    | POP target { $$ = new Instruction(@$, Instruction::POP, $2); }
    | RAND target { $$ = new Instruction(@$, Instruction::RAND, $2); }
    | NOP { $$ = new Instruction(@$, Instruction::NOP); }
    ;


mema:
    MEMA target ',' operand { $$ = new Instruction(@$, Instruction::MEMA, $2, $4); }
    ;


branch:
    BRZ operand ',' operand { $$ = new Instruction(@$, Instruction::BRZ, $2, $4); }
    | BRNZ operand ',' operand { $$ = new Instruction(@$, Instruction::BRNZ, $2, $4); }


call:
    CALL operand
    {
        Symbol *s = new Symbol(SymbolTable::unique_id(0), @2, Symbol::CONST, new Type(Type::INT));
        s->value.ival = 0;
        $$ = new Instruction(@$, Instruction::CALL, $2, s);
    }
    
    | CALL operand ',' I_CONSTANT
    {
        $$ = 0;
        
        if ($4 >= 0)
        {
            Symbol *s = new Symbol(SymbolTable::unique_id($4), @4, Symbol::CONST, new Type(Type::INT));
            s->value.ival = $4;
            $$ = new Instruction(@$, Instruction::CALL, $2, s);
        }
        else
            ON_ERROR("parameter count must be a non-negative number", @4.begin);
    }
    ;


ret:
    RETURN { $$ = new Instruction(@$, Instruction::RETURN); }
    | RETURN operand { $$ = new Instruction(@$, Instruction::RETURN, $2); }
    ;


%%

#include "scanner.hpp"

void Parser::error(const Parser::location_type& l, const std::string& msg)
{
    std::string& file = *l.begin.filename;
    int line = l.begin.line;
    int col = l.begin.column;
    std::ostringstream out;
    
    if (file.size() > 0)
    {
        out << file.c_str();
        if (line > 0)
        {
            out << "(" << line;
            if (col > 0)
                out << "," << col;
            out << ")";
        }
        out << ": ";
    }
    out << msg.c_str();

    errors.push_back(Error(ERROR, out.str()));
}

static int yylex(
    Parser::semantic_type* yylval,
    Parser::location_type* yylloc,
    Scanner& scanner)
{
    return scanner.yylex(yylval, yylloc);
}

Symbol* make_char_const(char c, const location &loc)
{
    std::string *id =
            (c >= 32) ? new std::string(std::string() + c) : SymbolTable::unique_id((int) c);
    Symbol *s = new Symbol(id, loc, Symbol::CONST, new Type(Type::CHAR));
    s->value.cval = c;
    return s;
}

std::vector<Symbol*>* make_char_array(std::string *str, const location &loc)
{
    std::vector<Symbol*>* v = new std::vector<Symbol*>();
    v->reserve(str->size() + 1);
    for (auto c : *str)
        v->push_back(make_char_const(c, loc));
    v->push_back(make_char_const('\0', loc));
    return v;
}

void destroy_list(std::vector<Symbol*> *list)
{
    if (list)
    {
        for (auto s : *list)
            if (!s->registered)
                delete s;
        delete list;
    }
}

bool register_symbol(Symbol *s, std::list<Error> &errors)
{
    const Symbol *aux = g_root_table->get(*s->id);
    if (aux)
    {
        std::ostringstream ss;
        ss  << "duplicate symbol '"
            << *s->id
            << "', previously defined at ("
                << aux->loc.begin.line << "," << aux->loc.begin.column
            << ")";
        ON_ERROR(ss.str(), s->loc.begin);
        return false;
    }
    
    g_root_table->put(s);
    return true;
}
