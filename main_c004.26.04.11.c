#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <math.h>
#include <time.h> 

#ifdef _WIN32
    #define strdup _strdup
#endif

void set_unicode() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

/* ===================== VTOKENS ===================== */

typedef enum {
    VTOKEN_EOF, VTOKEN_NUMBER, VTOKEN_ASSIGN, VTOKEN_LET,
    VTOKEN_IDENTIFIER, VTOKEN_PLUS, VTOKEN_MINUS, VTOKEN_STAR, VTOKEN_EXPO, VTOKEN_MOD, VTOKEN_COMMA,
    VTOKEN_SLASH, VTOKEN_LPAREN, VTOKEN_RPAREN, VTOKEN_NEWLINE,
    VTOKEN_PRINTF, VTOKEN_INPUT, VTOKEN_STRING, VTOKEN_RAND,
    VTOKEN_LEN, VTOKEN_CLEAR, VTOKEN_ABS, VTOKEN_EXIT,
    VTOKEN_SQRT, VTOKEN_SLEEP, VVTOKEN_TYPE, VTOKEN_UPPER, VTOKEN_LOWER,
    VTOKEN_IF, VTOKEN_ELSE, VTOKEN_ELSEIF, VTOKEN_WHILE, VTOKEN_EQ, VTOKEN_GEQ, VTOKEN_SEQ, VTOKEN_NEQ, VTOKEN_G, VTOKEN_S, VTOKEN_AND, VTOKEN_OR, VTOKEN_NOT,
    VTOKEN_LBRACE, VTOKEN_RBRACE,
    VTOKEN_BREAK, VTOKEN_CONTINUE, VTOKEN_FN, VTOKEN_RETURN,
} VVTOKENType;

typedef struct {
    VVTOKENType type;
    const char* value;
} VTOKEN;

/* ===================== LEXER ===================== */

typedef struct {
    const char* src;
    int pos;
    int line;
    int col;
    char current;
} Lexer;

void advance(Lexer* l) {
    if (l->current == '\n') {
        l->line++;
        l->col = 0;
    }
    l->pos++;
    l->col++;
    l->current = l->src[l->pos];
}

void lexer_init(Lexer* l, const char* src) {
    l->src = src;
    l->pos = 0;
    l->line = 0;
    l->col = 1;
    l->current = src[0];
}

void error_alert(Lexer* l, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("\nvanta: error: ");
    vprintf(fmt, args);
    printf("\n  at line %d, col %d\n", l->line + 1, l->col);

    int line_start = l->pos - (l->col - 1);
    printf("    | ");
    for (int i = line_start; l->src[i] && l->src[i] != '\n'; i++) {
        putchar(l->src[i]);
    }
    printf("\n    | ");
    for (int i = 0; i < l->col - 1; i++) printf(" ");
    printf("^\n\n");
    va_end(args);
    exit(1);
}

void skip_whitespace(Lexer* l) { 
    while (l->current == ' ' || l->current == '\t' || l->current == '\r') advance(l);
}

char* read_string(Lexer* l) {
    char buffer[1024]; int i = 0;
    advance(l); 
    while (l->current != '"' && l->current != '\0') {
        if (l->current == '\\') {
            advance(l);
            if (l->current == 'n') buffer[i++] = '\n';
            else if (l->current == 't') buffer[i++] = '\t';
            else buffer[i++] = l->current;
        } else buffer[i++] = l->current;
        advance(l);
    }
    if (l->current != '"') error_alert(l, "unterminated string");
    advance(l); 
    buffer[i] = '\0';
    return strdup(buffer);
}

VTOKEN next_VTOKEN(Lexer* l) {
    skip_whitespace(l);

    while (l->current == '#') {
        while (l->current != '\n' && l->current != '\0') {
            advance(l);
        }
        skip_whitespace(l); 
    }

    VTOKEN t = {VTOKEN_EOF, NULL};
    if (l->current == '\0') return t;

    if (isdigit(l->current)) {
        char buf[64]; int i = 0;
        while (isdigit(l->current)) { buf[i++] = l->current; advance(l); }
        buf[i] = '\0'; t.type = VTOKEN_NUMBER; t.value = strdup(buf);
        return t;
    }

    if (isalpha((unsigned char)l->current) || l->current == '_' || (unsigned char)l->current > 127) {
        char buf[256]; int i = 0;
        while (isalnum((unsigned char)l->current) || l->current == '_' || (unsigned char)l->current > 127) {
            buf[i++] = l->current;
            advance(l);
            if (i >= 255) break; 
        }
        buf[i] = '\0';
        char* id = strdup(buf);
        
        if (strcmp(id, "let") == 0) t.type = VTOKEN_LET;
        else if (strcmp(id, "printf") == 0) t.type = VTOKEN_PRINTF;
        else if (strcmp(id, "input") == 0) t.type = VTOKEN_INPUT;
        else if (strcmp(id, "rand") == 0) t.type = VTOKEN_RAND;
        else if (strcmp(id, "len") == 0) t.type = VTOKEN_LEN;
        else if (strcmp(id, "clear") == 0) t.type = VTOKEN_CLEAR;
        else if (strcmp(id, "abs") == 0) t.type = VTOKEN_ABS;
        else if (strcmp(id, "exit") == 0) t.type = VTOKEN_EXIT;
        else if (strcmp(id, "sqrt") == 0) t.type = VTOKEN_SQRT;
        else if (strcmp(id, "sleep") == 0) t.type = VTOKEN_SLEEP;
        else if (strcmp(id, "type") == 0) t.type = VVTOKEN_TYPE;
        else if (strcmp(id, "upper") == 0) t.type = VTOKEN_UPPER;
        else if (strcmp(id, "lower") == 0) t.type = VTOKEN_LOWER;
        else if (strcmp(id, "if") == 0) t.type = VTOKEN_IF;
        else if (strcmp(id, "elseif") == 0) t.type = VTOKEN_ELSEIF;
        else if (strcmp(id, "else") == 0) t.type = VTOKEN_ELSE;
        else if (strcmp(id, "while") == 0) t.type = VTOKEN_WHILE;
        else if (strcmp(id, "or") == 0) t.type = VTOKEN_OR;
        else if (strcmp(id, "and") == 0) t.type = VTOKEN_AND;
        else if (strcmp(id, "not") == 0) t.type = VTOKEN_NOT;
        else if (strcmp(id, "break") == 0) t.type = VTOKEN_BREAK;
        else if (strcmp(id, "continue") == 0) t.type = VTOKEN_CONTINUE;
        else if (strcmp(id, "fn") == 0) t.type = VTOKEN_FN;
        else if (strcmp(id, "return") == 0) t.type = VTOKEN_RETURN;
        else t.type = VTOKEN_IDENTIFIER;
        t.value = id; return t;
    }

    if (l->current == '"') { t.type = VTOKEN_STRING; t.value = read_string(l); return t; }

    if (l->current == '=') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_EQ;
            return t;
        }
        advance(l);
        t.type = VTOKEN_ASSIGN;
        return t;
    }

    if (l->current == '!') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_NEQ;
            return t;
        } else { 
            error_alert(l, "unexpected character '!'");
            exit(1); 
        }
        advance(l);
        return t;
    }

    if (l->current == '>') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_GEQ;
            return t;
        }
        advance(l);
        t.type = VTOKEN_G;
        return t;
    }

    if (l->current == '<') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_SEQ;
            return t;
        }
        advance(l);
        t.type = VTOKEN_S;
        return t;
    }

    switch (l->current) {
        case '+': t.type = VTOKEN_PLUS; break;
        case '-': t.type = VTOKEN_MINUS; break;
        case '*': t.type = VTOKEN_STAR; break;
        case '/': t.type = VTOKEN_SLASH; break;
        case '^': t.type = VTOKEN_EXPO; break;
        case '%': t.type = VTOKEN_MOD; break;
        case ',': t.type = VTOKEN_COMMA; break;
        case '=': t.type = VTOKEN_ASSIGN; break;
        case '(': t.type = VTOKEN_LPAREN; break;
        case ')': t.type = VTOKEN_RPAREN; break;
        case '{': t.type = VTOKEN_LBRACE; break;
        case '}': t.type = VTOKEN_RBRACE; break;
        case '\n': t.type = VTOKEN_NEWLINE; break;
        default: error_alert(l, "unexpected character '%c'", l->current);
    }
    advance(l); return t;
}

/* ===================== AST ===================== */

typedef enum {
    AST_NUMBER, AST_BINOP, AST_VAR, AST_LET, AST_PRINT, AST_INPUT, 
    AST_STRING, AST_BUILTIN, AST_RAND, AST_IF, AST_WHILE, AST_BLOCK,
    AST_BREAK, AST_CONTINUE, AST_RETURN, AST_FUNCTION, AST_CALL,
} ASTType;

typedef struct ASTNode ASTNode;
struct ASTNode {
    ASTType type;
    union {
        int number;
        const char* string;
        const char* var_name;
        struct { ASTNode *left, *right; VVTOKENType op; } binop;
        struct { const char* name; ASTNode* value; } let;
        struct {
            ASTNode* cond;
            ASTNode* body;
            ASTNode* else_branch;
        } if_stmt;
        struct {
            ASTNode* cond;
            ASTNode* body;
        } while_stmt;
        struct {
            ASTNode** statements;
            int count;
        } block;
        struct {
            ASTNode* value;
        } return_stmt;

        struct {
            const char* name;
            char** params;
            int param_count;
            ASTNode* body;
        } function;

        struct {
            const char* name;
            ASTNode** args;
            int arg_count;
        } call;
        struct { ASTNode* value; } print;
        struct { ASTNode* prompt; } input;
        struct { ASTNode* min; ASTNode* max; } rand;
        struct { VVTOKENType type; ASTNode* arg; } builtin;
    };
};

ASTNode* new_node(ASTType type) {
    ASTNode* n = (ASTNode*)calloc(1, sizeof(ASTNode));
    n->type = type;
    return n;
}

/* ===================== PARSER ===================== */

typedef struct { Lexer lexer; VTOKEN current; } Parser;

void parser_advance(Parser* p) { p->current = next_VTOKEN(&p->lexer); }

void skip_newlines(Parser* p) {
    while (p->current.type == VTOKEN_NEWLINE) parser_advance(p);
}

ASTNode* parse_expr(Parser* p);

ASTNode* parse_factor(Parser* p) {
    VTOKEN t = p->current;

    if (t.type == VTOKEN_MINUS) {
        parser_advance(p); 
        ASTNode* operand = parse_factor(p); 
        
        ASTNode* zero = new_node(AST_NUMBER);
        zero->number = 0;
        
        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = zero;
        n->binop.op = VTOKEN_MINUS;
        n->binop.right = operand;
        return n;
    }

    if (t.type == VTOKEN_NUMBER) {
        ASTNode* n = new_node(AST_NUMBER);
        n->number = atoi(t.value);
        parser_advance(p);
        return n;
    }
    
    if (t.type == VTOKEN_STRING) {
        ASTNode* n = new_node(AST_STRING);
        n->string = t.value;
        parser_advance(p);
        return n;
    }

    if (t.type == VTOKEN_IDENTIFIER) {
        const char* name = t.value;
        parser_advance(p);

        if (p->current.type == VTOKEN_LPAREN) {
            parser_advance(p);

            ASTNode** args = malloc(sizeof(ASTNode*) * 32);
            int count = 0;

            while (p->current.type != VTOKEN_RPAREN) {
                args[count++] = parse_expr(p);

                if (p->current.type == VTOKEN_COMMA)
                    parser_advance(p);
            }

            parser_advance(p); 

            ASTNode* n = new_node(AST_CALL);
            n->call.name = name;
            n->call.args = args;
            n->call.arg_count = count;
            return n;
        }


        ASTNode* n = new_node(AST_VAR);
        n->var_name = name;
        return n;
    }

    if (t.type >= VTOKEN_PRINTF && t.type <= VTOKEN_LOWER) {
        VVTOKENType type = t.type;

        if (type == VTOKEN_RAND) {
            parser_advance(p); 
            parser_advance(p); 
            ASTNode* min_n = parse_expr(p);
            parser_advance(p); 
            ASTNode* max_n = parse_expr(p);
            parser_advance(p); 
            ASTNode* n = new_node(AST_RAND);
            n->rand.min = min_n; n->rand.max = max_n;
            return n;
        }

        parser_advance(p);
        ASTNode* arg = NULL;
        if (p->current.type == VTOKEN_LPAREN) {
            parser_advance(p);
            if (p->current.type != VTOKEN_RPAREN) arg = parse_expr(p);
            parser_advance(p); 
        }
        
        if (type == VTOKEN_PRINTF) { ASTNode* n = new_node(AST_PRINT); n->print.value = arg; return n; }
        if (type == VTOKEN_INPUT) { ASTNode* n = new_node(AST_INPUT); n->input.prompt = arg; return n; }
        if (type == VTOKEN_CLEAR || type == VTOKEN_EXIT) { ASTNode* n = new_node(AST_BUILTIN); n->builtin.type = type; return n; }
        
        ASTNode* n = new_node(AST_BUILTIN);
        n->builtin.type = type;
        n->builtin.arg = arg;
        return n;
    }

    if (t.type == VTOKEN_LPAREN) {
        parser_advance(p);
        ASTNode* n = parse_expr(p);
        parser_advance(p); 
        return n;
    }

    error_alert(&p->lexer, "syntax error at '%s'", t.value ? t.value : "EOF");
    return NULL;
}

ASTNode* parse_expo(Parser* p) {
    ASTNode* left = parse_factor(p);

    if (p->current.type == VTOKEN_EXPO) {
        VVTOKENType op = p->current.type;
        parser_advance(p);

        ASTNode* right = parse_expo(p); 

        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = left;
        n->binop.op = op;
        n->binop.right = right;

        return n;
    }

    return left;
}

ASTNode* parse_term(Parser* p) {
    ASTNode* node = parse_expo(p);   

    while (p->current.type == VTOKEN_STAR || p->current.type == VTOKEN_SLASH || p->current.type == VTOKEN_MOD) {
        VVTOKENType op = p->current.type;
        parser_advance(p);

        ASTNode* right = parse_expo(p);  

        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = node;
        n->binop.op = op;
        n->binop.right = right;

        node = n;
    }
    return node;
}

ASTNode* parse_statement(Parser* p);

ASTNode* parse_comparison(Parser* p);

ASTNode* parse_logic_not(Parser* p);
ASTNode* parse_logic_and(Parser* p);
ASTNode* parse_logic_or(Parser* p);

ASTNode* parse_block(Parser* p);

ASTNode* parse_if(Parser* p) {
    parser_advance(p); 

    ASTNode* cond = parse_logic_or(p);
    skip_newlines(p);

    ASTNode* body = (p->current.type == VTOKEN_LBRACE)
        ? parse_block(p)
        : parse_statement(p);

    ASTNode* node = new_node(AST_IF);
    node->if_stmt.cond = cond;
    node->if_stmt.body = body;
    node->if_stmt.else_branch = NULL;

    ASTNode* current = node;

    skip_newlines(p); 

    while (p->current.type == VTOKEN_ELSEIF) {
        parser_advance(p);

        ASTNode* elseif_cond = parse_logic_or(p);
        skip_newlines(p);

        ASTNode* elseif_body = (p->current.type == VTOKEN_LBRACE)
            ? parse_block(p)
            : parse_statement(p);

        ASTNode* elseif_node = new_node(AST_IF);
        elseif_node->if_stmt.cond = elseif_cond;
        elseif_node->if_stmt.body = elseif_body;
        elseif_node->if_stmt.else_branch = NULL;

        current->if_stmt.else_branch = elseif_node;
        current = elseif_node;
        skip_newlines(p); 
    }

    if (p->current.type == VTOKEN_ELSE) {
        parser_advance(p);
        skip_newlines(p);

        ASTNode* else_body = (p->current.type == VTOKEN_LBRACE)
            ? parse_block(p)
            : parse_statement(p);

        current->if_stmt.else_branch = else_body;
    }

    return node;
}

ASTNode* parse_while(Parser* p) {
    parser_advance(p); 

    ASTNode* cond = parse_logic_or(p);

    skip_newlines(p);

    ASTNode* body;

    if (p->current.type == VTOKEN_LBRACE)
        body = parse_block(p);
    else
        body = parse_statement(p);

    ASTNode* n = new_node(AST_WHILE);
    n->while_stmt.cond = cond;
    n->while_stmt.body = body;

    return n;
}

ASTNode* parse_expr(Parser* p) {
    ASTNode* node = parse_term(p);
    while (p->current.type == VTOKEN_PLUS || p->current.type == VTOKEN_MINUS) {
        VVTOKENType op = p->current.type; parser_advance(p);
        ASTNode* right = parse_term(p);
        ASTNode* n = new_node(AST_BINOP); n->binop.left = node; n->binop.op = op; n->binop.right = right;
        node = n;
    }
    return node;
}

ASTNode* parse_comparison(Parser* p) {
    ASTNode* left = parse_expr(p);

    while (
        p->current.type == VTOKEN_EQ  ||
        p->current.type == VTOKEN_GEQ ||
        p->current.type == VTOKEN_SEQ ||
        p->current.type == VTOKEN_G   ||
        p->current.type == VTOKEN_S   ||
        p->current.type == VTOKEN_NEQ 
    ) {
        VVTOKENType op = p->current.type;
        parser_advance(p);

        ASTNode* right = parse_expr(p);

        ASTNode* node = new_node(AST_BINOP);
        node->binop.left = left;
        node->binop.right = right;
        node->binop.op = op;

        left = node;
    }

    return left;
}

ASTNode* parse_logic_not(Parser* p) {
    if (p->current.type == VTOKEN_NOT) {
        parser_advance(p);

        ASTNode* expr = parse_logic_not(p);

        ASTNode* zero = new_node(AST_NUMBER);
        zero->number = 0;

        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = expr;
        n->binop.op = VTOKEN_EQ; 
        n->binop.right = zero;

        return n;
    }

    return parse_comparison(p);
}

ASTNode* parse_logic_and(Parser* p) {
    ASTNode* node = parse_logic_not(p);

    while (p->current.type == VTOKEN_AND) {
        parser_advance(p);

        ASTNode* right = parse_logic_not(p);

        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = node;
        n->binop.right = right;
        n->binop.op = VTOKEN_AND;

        node = n;
    }

    return node;
}

ASTNode* parse_logic_or(Parser* p) {
    ASTNode* node = parse_logic_and(p);

    while (p->current.type == VTOKEN_OR) {
        parser_advance(p);

        ASTNode* right = parse_logic_and(p);

        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = node;
        n->binop.right = right;
        n->binop.op = VTOKEN_OR;

        node = n;
    }

    return node;
}

ASTNode* parse_block(Parser* p) {
    parser_advance(p); 

    ASTNode* block = new_node(AST_BLOCK);
    block->block.statements = malloc(sizeof(ASTNode*) * 1024);
    block->block.count = 0;

    while (p->current.type != VTOKEN_RBRACE) {
        skip_newlines(p);

        if (p->current.type == VTOKEN_RBRACE)
            break;

        block->block.statements[block->block.count++] = parse_statement(p);

        skip_newlines(p);
    }

    parser_advance(p);

    return block;
}

ASTNode* parse_function(Parser* p) {
    parser_advance(p); 

    const char* name = p->current.value;
    parser_advance(p);

    parser_advance(p); 

    char** params = malloc(sizeof(char*) * 32);
    int count = 0;

    while (p->current.type != VTOKEN_RPAREN) {
        params[count++] = strdup(p->current.value);
        parser_advance(p);

        if (p->current.type == VTOKEN_COMMA)
            parser_advance(p);
    }

    parser_advance(p); 

    skip_newlines(p);

    ASTNode* body = (p->current.type == VTOKEN_LBRACE)
        ? parse_block(p)
        : parse_statement(p);

    ASTNode* n = new_node(AST_FUNCTION);
    n->function.name = name;
    n->function.params = params;
    n->function.param_count = count;
    n->function.body = body;

    return n;
}

ASTNode* parse_statement(Parser* p) {
    skip_newlines(p);

    if (p->current.type == VTOKEN_LBRACE) {
        return parse_block(p);
    }

    if (p->current.type == VTOKEN_LET) {
        parser_advance(p);
        const char* name = p->current.value;
        parser_advance(p);

        if (p->current.type != VTOKEN_ASSIGN)
            error_alert(&p->lexer, "expected '=' after %s", name);

        parser_advance(p);

        ASTNode* val = parse_logic_or(p);

        ASTNode* n = new_node(AST_LET);
        n->let.name = name;
        n->let.value = val;
        return n;
    }

    if (p->current.type == VTOKEN_IF) {
        return parse_if(p);
    }

    if (p->current.type == VTOKEN_ELSEIF || p->current.type == VTOKEN_ELSE) {
        error_alert(&p->lexer, "'%s' without preceding if",
            p->current.type == VTOKEN_ELSEIF ? "elseif" : "else");
    }

    if (p->current.type == VTOKEN_WHILE) {
        return parse_while(p);
    }

    if (p->current.type == VTOKEN_BREAK) {
        parser_advance(p);
        return new_node(AST_BREAK);
    }

    if (p->current.type == VTOKEN_CONTINUE) {
        parser_advance(p);
        return new_node(AST_CONTINUE);
    }

    if (p->current.type == VTOKEN_RETURN) {
        parser_advance(p);
        ASTNode* val = NULL;
        if (p->current.type != VTOKEN_NEWLINE && p->current.type != VTOKEN_RBRACE)
            val = parse_logic_or(p);

        ASTNode* n = new_node(AST_RETURN);
        n->return_stmt.value = val;
        return n;
    }

    if (p->current.type == VTOKEN_FN) {
        return parse_function(p);
    }

    return parse_logic_or(p);
}

/* ===================== EVALUATE ===================== */

typedef struct { int type; int i; char* s; } Value;
int break_flag = 0;
int continue_flag = 0;
int return_flag = 0;
Value return_value;

typedef struct { char* name; Value value; } Variable;
Variable vars[1000]; int var_count = 0;

typedef struct {
    char* name;
    ASTNode* node;
} Function;

Function functions[100];
int func_count = 0;

void set_var(const char* name, Value val) {
    for (int i = 0; i < var_count; i++) 
        if (strcmp(vars[i].name, name) == 0) { vars[i].value = val; return; }
    vars[var_count].name = strdup(name);
    vars[var_count].value = val;
    var_count++;
}

Value get_var(const char* name) {
    for (int i = 0; i < var_count; i++) 
        if (strcmp(vars[i].name, name) == 0) return vars[i].value;
    printf("runtime error: undefined variable '%s'\n", name); exit(1);
}

int power(int base, int exp) {
    int result = 1;
    while (exp > 0) {
        if (exp % 2 == 1)
            result *= base;
        base *= base;
        exp /= 2;
    }
    return result;
}

Value eval(ASTNode* node) {
    Value v = {0, 0, NULL}; if (!node) return v;
    switch (node->type) {
        case AST_NUMBER: v.type = 0; v.i = node->number; return v;
        case AST_STRING: v.type = 1; v.s = strdup(node->string); return v;
        case AST_VAR: return get_var(node->var_name);
        case AST_LET: { Value val = eval(node->let.value); set_var(node->let.name, val); return val; }
        case AST_PRINT: {
            Value val = eval(node->print.value);
            if (val.type == 0) printf("%d\n", val.i); else printf("%s\n", val.s);
            return val;
        }
        case AST_INPUT: {
            if (node->input.prompt) { Value p = eval(node->input.prompt); printf("%s", p.s); }
            char buf[1024]; fgets(buf, 1024, stdin); buf[strcspn(buf, "\n")] = 0;
            char* end; long res = strtol(buf, &end, 10);
            if (*end == '\0' && strlen(buf) > 0) { v.type = 0; v.i = (int)res; }
            else { v.type = 1; v.s = strdup(buf); }
            return v;
        }
        case AST_RAND: {
            Value min_val = eval(node->rand.min);
            Value max_val = eval(node->rand.max);
            v.type = 0;
            if (max_val.i <= min_val.i) v.i = min_val.i;
            else v.i = (rand() % (max_val.i - min_val.i + 1)) + min_val.i;
            return v;
        }
        case AST_BUILTIN: {
            if (node->builtin.type == VTOKEN_CLEAR) { system("cls"); return v; }
            if (node->builtin.type == VTOKEN_EXIT) { exit(0); }
            Value arg = eval(node->builtin.arg);
            switch(node->builtin.type) {
                case VTOKEN_LEN: v.type = 0; v.i = (arg.type == 1) ? (int)strlen(arg.s) : 0; break;
                case VTOKEN_ABS: v.type = 0; v.i = (arg.type == 0) ? abs(arg.i) : 0; break;
                case VTOKEN_SQRT: v.type = 0; v.i = (arg.type == 0) ? (int)sqrt(arg.i) : 0; break;
                case VTOKEN_SLEEP: if(arg.type == 0) Sleep(arg.i); break;
                case VVTOKEN_TYPE: v.type = 0; v.i = arg.type; break;
                case VTOKEN_UPPER: 
                    if(arg.type == 1) { v.type = 1; v.s = strdup(arg.s); for(int i=0; v.s[i]; i++) v.s[i] = toupper(v.s[i]); } break;
                case VTOKEN_LOWER:
                    if(arg.type == 1) { v.type = 1; v.s = strdup(arg.s); for(int i=0; v.s[i]; i++) v.s[i] = tolower(v.s[i]); } break;
                default: break;
            }
            return v;
        }
        case AST_BINOP: {
            Value l = eval(node->binop.left); Value r = eval(node->binop.right);
            v.type = 0;
            if (node->binop.op == VTOKEN_PLUS)       v.i = l.i + r.i;
            else if (node->binop.op == VTOKEN_MINUS) v.i = l.i - r.i;
            else if (node->binop.op == VTOKEN_STAR)  v.i = l.i * r.i;
            else if (node->binop.op == VTOKEN_SLASH) v.i = (r.i != 0) ? l.i / r.i : 0;
            else if (node->binop.op == VTOKEN_EXPO)  v.i = power(l.i, r.i);
            else if (node->binop.op == VTOKEN_MOD)   v.i = (r.i != 0) ? l.i % r.i : 0;
            else if (node->binop.op == VTOKEN_EQ)    v.i = (l.i == r.i);
            else if (node->binop.op == VTOKEN_NEQ)    v.i = (l.i != r.i);
            else if (node->binop.op == VTOKEN_G)     v.i = (l.i >  r.i);
            else if (node->binop.op == VTOKEN_S)     v.i = (l.i <  r.i);
            else if (node->binop.op == VTOKEN_GEQ)   v.i = (l.i >= r.i);
            else if (node->binop.op == VTOKEN_SEQ)   v.i = (l.i <= r.i);
            else if (node->binop.op == VTOKEN_AND)   v.i = (l.i && r.i);
            else if (node->binop.op == VTOKEN_OR)    v.i = (l.i || r.i);
            return v;
        }
        case AST_IF: {
            Value c = eval(node->if_stmt.cond);

            if (c.type == 0 && c.i) {
                return eval(node->if_stmt.body);
            } else if (node->if_stmt.else_branch) {
                return eval(node->if_stmt.else_branch);
            }

            return (Value){0};
        }
        case AST_WHILE: {
            while (1) {
                Value c = eval(node->while_stmt.cond);

                if (!(c.type == 0 && c.i))
                    break;

                eval(node->while_stmt.body);

                if (break_flag) {
                    break_flag = 0;
                    break;
                }

                if (continue_flag) {
                    continue_flag = 0;
                    continue;
                }
            }
            return v;
        }
        case AST_BLOCK: {
            Value last = {0};
            for (int i = 0; i < node->block.count; i++) {
                last = eval(node->block.statements[i]);
            }
            return last;
        }
        case AST_BREAK:
            break_flag = 1;
            return v;

        case AST_CONTINUE:
            continue_flag = 1;
            return v;

        case AST_RETURN:
            return_flag = 1;
            if (node->return_stmt.value)
                return_value = eval(node->return_stmt.value);
            return return_value;

        case AST_FUNCTION:
            functions[func_count].name = strdup(node->function.name);
            functions[func_count].node = node;
            func_count++;
            return v;
        case AST_CALL: {
            for (int i = 0; i < func_count; i++) {
                if (strcmp(functions[i].name, node->call.name) == 0) {

                    ASTNode* fn = functions[i].node;

                    for (int j = 0; j < fn->function.param_count; j++) {
                        Value arg = eval(node->call.args[j]);
                        set_var(fn->function.params[j], arg);
                    }

                    return_flag = 0;
                    eval(fn->function.body);

                    return return_value;
                }
            }

            printf("runtime error: function '%s' not found\n", node->call.name);
            exit(1);
        }
    }
    return v;
}

/* ===================== FILE HELPER ===================== */

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("vanta: error: could not open file '%s'\n", filename);
        exit(1);
    } 
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    
    char* buf = malloc(size + 1);
    size_t bytes_read = fread(buf, 1, size, f);
    buf[bytes_read] = '\0';
    fclose(f);
    return buf;
}

/* ===================== MAIN ===================== */

int main(int argc, char* argv[]) {
    srand(time(NULL));
    set_unicode();

    const char* filename;
    if (argc > 2) {
        printf("usage: vanta [filename]\n");
        return 1;
    } 
    else if (argc == 2) {
        filename = argv[1];
    } 
    else {
        printf("usage: vanta <filename.vt>\ndefault to main.vt\n");
        filename = "main.vt";
    }

    char* code = read_file(filename);
    
    Parser p;
    lexer_init(&p.lexer, code);
    parser_advance(&p);

    while (p.current.type != VTOKEN_EOF) {
        skip_newlines(&p);
        if (p.current.type == VTOKEN_EOF) break;
        eval(parse_statement(&p));
        skip_newlines(&p);
    }

    free(code);
    return 0;
}