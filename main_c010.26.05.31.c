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

/* flag */

int skip_error = 0;

/* macro */

#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_BOLD   "\x1b[1m"
#define ANSI_RED   "\x1b[31m"
#define ANSI_RESET "\x1b[0m"
#define version "c010.26.05.28"

/* ansi enabler */

void enable_ansi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
}

/* ===================== VTOKENS ===================== */

typedef enum {
    VTOKEN_EOF, VTOKEN_NUMBER, VTOKEN_ASSIGN, VTOKEN_LET,
    VTOKEN_IDENTIFIER, VTOKEN_PLUS, VTOKEN_MINUS, VTOKEN_STAR, VTOKEN_EXPO, VTOKEN_MOD, VTOKEN_COMMA, VTOKEN_CONCAT,
    VTOKEN_ASPLUS, VTOKEN_ASMINUS, VTOKEN_ASSTAR, VTOKEN_ASEXPO, VTOKEN_ASMOD, VTOKEN_ASSLASH, VTOKEN_ASCONCAT, VTOKEN_ASINTSLASH,
    VTOKEN_SLASH, VTOKEN_INTSLASH, VTOKEN_LPAREN, VTOKEN_RPAREN, VTOKEN_LBRACKET, VTOKEN_RBRACKET, VTOKEN_NEWLINE,
    VTOKEN_PRINT, VTOKEN_INPUT, VTOKEN_STRING, VTOKEN_RAND,
    VTOKEN_LEN, VTOKEN_CLEAR, VTOKEN_ABS, VTOKEN_FABS, VTOKEN_EXIT,
    VTOKEN_SQRT, VTOKEN_SLEEP, VVTOKEN_TYPE, VTOKEN_UPPER, VTOKEN_LOWER,
    VTOKEN_IF, VTOKEN_ELSE, VTOKEN_ELSEIF, VTOKEN_WHILE, VTOKEN_EQ, VTOKEN_GEQ, VTOKEN_SEQ, VTOKEN_NEQ, VTOKEN_G, VTOKEN_S, VTOKEN_AND, VTOKEN_OR, VTOKEN_NOT,
    VTOKEN_TRUE, VTOKEN_FALSE,
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
    if (!skip_error) {
        va_list args;
        va_start(args, fmt);

        printf("vanta: " ANSI_RED ANSI_BOLD "error" ANSI_RESET ": ");
        vprintf(fmt, args);

        printf("\n  at line %d, col %d\n", l->line + 1, l->col);

        int line_start = l->pos - (l->col - 1);

        // in line
        printf("    | ");
        int line_len = 0;
        for (int i = line_start; l->src[i] && l->src[i] != '\n'; i++) {
            printf("%c", l->src[i]);
            line_len++;
        }

        printf("\n");

        printf("    | ");

        for (int i = 0; i < l->col - 1; i++)
            printf(" ");

        int underline_len = (line_len > 0 ? line_len - (l->col - 1) : 1);
        if (underline_len < 1) underline_len = 1;

        for (int i = 0; i < underline_len; i++)
            printf(ANSI_RED "~");

        printf("\n");

        printf(ANSI_RESET);
        va_end(args);
        exit(1);
    } else {}
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
        char buf[64];
        int i = 0;
        int dot_count = 0;

        while (isdigit(l->current) || l->current == '.') {
            if (l->current == '.') {
                dot_count++;
                if (dot_count > 1)
                    error_alert(l, "invalid number");
            }

            buf[i++] = l->current;
            advance(l);
        }

        buf[i] = '\0';

        t.type = VTOKEN_NUMBER;
        t.value = strdup(buf);
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
        else if (strcmp(id, "printf") == 0) t.type = VTOKEN_PRINT;  /* sửa thành hàm print, giữ printf để tương thích legacy
                                                                       changed to print, printf for legacy code */ 
        else if (strcmp(id, "print") == 0) t.type = VTOKEN_PRINT;
        else if (strcmp(id, "input") == 0) t.type = VTOKEN_INPUT;
        else if (strcmp(id, "rand") == 0) t.type = VTOKEN_RAND;
        else if (strcmp(id, "len") == 0) t.type = VTOKEN_LEN;           
        else if (strcmp(id, "clear") == 0) t.type = VTOKEN_CLEAR;
        else if (strcmp(id, "abs") == 0) t.type = VTOKEN_ABS;
        else if (strcmp(id, "fabs") == 0) t.type = VTOKEN_FABS;
        else if (strcmp(id, "exit") == 0) t.type = VTOKEN_EXIT;
        else if (strcmp(id, "sqrt") == 0) t.type = VTOKEN_SQRT;
        else if (strcmp(id, "sleep") == 0) t.type = VTOKEN_SLEEP;
        else if (strcmp(id, "type") == 0) t.type = VVTOKEN_TYPE;
        else if (strcmp(id, "upper") == 0) t.type = VTOKEN_UPPER;
        else if (strcmp(id, "lower") == 0) t.type = VTOKEN_LOWER;
        else if (strcmp(id, "if") == 0) t.type = VTOKEN_IF;
        else if (strcmp(id, "elseif") == 0) t.type = VTOKEN_ELSEIF;
        else if (strcmp(id, "else") == 0) t.type = VTOKEN_ELSE;
        else if (strcmp(id, "true") == 0) t.type = VTOKEN_TRUE;
        else if (strcmp(id, "false") == 0) t.type = VTOKEN_FALSE;
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

    if (l->current == '*') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_ASSTAR;
            return t;
        }
        if (l->src[l->pos + 1] == '*') {
            if (l->src[l->pos + 2] == '=') {
                advance(l);
                advance(l);
                advance(l);
                t.type = VTOKEN_ASEXPO;
                return t;
            }
            advance(l);
            advance(l);
            t.type = VTOKEN_EXPO;
            return t;
        }
        advance(l);
        t.type = VTOKEN_STAR;
        return t;
    }

    if (l->current == '+') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_ASPLUS;
            return t;
        }
        advance(l); 
        t.type = VTOKEN_PLUS;
        return t;
    }

    if (l->current == '-') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_ASMINUS;
            return t;
        }
        advance(l); 
        t.type = VTOKEN_MINUS;
        return t;
    }

    if (l->current == '.') {
        if (l->src[l->pos + 1] == '.') {
            if (l->src[l->pos + 2] == '=') {
                advance(l);
                advance(l);
                advance(l);
                t.type = VTOKEN_ASCONCAT;
                return t;
            }
            advance(l);
            advance(l);
            t.type = VTOKEN_CONCAT;
            return t;
        } else { 
            error_alert(l, "unexpected character '.'");
            exit(1); 
        }
        advance(l);
        return t;
    }

    if (l->current == '/') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_ASSLASH;
            return t;
        }

        if (l->src[l->pos + 1] == '/') {
            if (l->src[l->pos + 2] == '=') {
                advance(l);
                advance(l);
                advance(l);
                t.type = VTOKEN_ASINTSLASH;
                return t;
            }
            advance(l);
            advance(l);
            t.type = VTOKEN_INTSLASH;
            return t;
        }

        advance(l);
        t.type = VTOKEN_SLASH;
        return t;
    }

    if (l->current == '%') {
        if (l->src[l->pos + 1] == '=') {
            advance(l);
            advance(l);
            t.type = VTOKEN_ASMOD;
            return t;
        }
        advance(l); 
        t.type = VTOKEN_MOD;
        return t;
    }

    switch (l->current) {
        case ',': t.type = VTOKEN_COMMA; break;
        case '=': t.type = VTOKEN_ASSIGN; break;
        case '(': t.type = VTOKEN_LPAREN; break;
        case ')': t.type = VTOKEN_RPAREN; break;
        case '[': t.type = VTOKEN_LBRACKET; break;
        case ']': t.type = VTOKEN_RBRACKET; break;
        case '{': t.type = VTOKEN_LBRACE; break;
        case '}': t.type = VTOKEN_RBRACE; break;
        case '\n': t.type = VTOKEN_NEWLINE; break;
        default: error_alert(l, "unexpected character '%c'", l->current);
    }
    advance(l); return t;
}

/* ===================== AST ===================== */

typedef enum {
    AST_NUMBER, AST_BINOP, AST_BOOL, AST_VAR, AST_LET, AST_PRINT, AST_INPUT, 
    AST_STRING, AST_BUILTIN, AST_RAND, AST_IF, AST_WHILE, AST_BLOCK,
    AST_BREAK, AST_CONTINUE, AST_RETURN, AST_FUNCTION, AST_CALL,
    AST_LIST, AST_INDEX, AST_SPLIT,
} ASTType;

typedef struct ASTNode ASTNode;
struct ASTNode {
    ASTType type;
    union {
        double number;
        const char* string;
        const char* var_name;
        struct { ASTNode *left, *right; VVTOKENType op; } binop;
        struct { const char* name; ASTNode* value; } let;
        struct { const char* name; ASTNode* value; } letident;
        struct {
            ASTNode* cond;
            ASTNode* body;
            ASTNode* else_branch;
        } if_stmt;
        struct {
            ASTNode* cond;
            ASTNode* body;
        } while_stmt;
        struct { int tof; } blean;
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

        struct {
            ASTNode** items;
            int count;
        } list;

        struct {
            ASTNode* object;
            ASTNode* index;
        } index;
        struct {
            ASTNode* str;
            ASTNode* delim;
        } split;
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

int is_variable_defined(const char* name);

ASTNode* parse_expr(Parser* p);
ASTNode* parse_concat(Parser* p);
ASTNode* parse_comparison(Parser* p);
ASTNode* parse_logic_not(Parser* p);
ASTNode* parse_logic_and(Parser* p);
ASTNode* parse_logic_or(Parser* p);
ASTNode* parse_statement(Parser* p);
ASTNode* parse_block(Parser* p);
ASTNode* parse_primary(Parser* p);

ASTNode* parse_postfix(Parser* p) {
    ASTNode* node = parse_primary(p);

    while (1) {

        if (p->current.type == VTOKEN_LBRACKET) {

            parser_advance(p);

            ASTNode* idx = parse_concat(p);

            if (p->current.type != VTOKEN_RBRACKET)
                error_alert(&p->lexer, "expected ']'");

            parser_advance(p);

            ASTNode* n = new_node(AST_INDEX);

            n->index.object = node;
            n->index.index = idx;

            node = n;

            continue;
        }

        break;
    }

    return node;
}

ASTNode* parse_primary(Parser* p) {
    VTOKEN t = p->current;

    if (t.type == VTOKEN_MINUS) {
        parser_advance(p); 
        ASTNode* operand = parse_primary(p); 
        
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
        n->number = atof(t.value);
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
                    args[count++] = parse_concat(p);;

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

    if (t.type >= VTOKEN_PRINT && t.type <= VTOKEN_LOWER) {
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
            if (p->current.type != VTOKEN_RPAREN) arg = parse_concat(p);
            parser_advance(p); 
        }
        
        if (type == VTOKEN_PRINT) { ASTNode* n = new_node(AST_PRINT); n->print.value = arg; return n; }
        if (type == VTOKEN_INPUT) { ASTNode* n = new_node(AST_INPUT); n->input.prompt = arg; return n; }
        if (type == VTOKEN_CLEAR || type == VTOKEN_EXIT) { ASTNode* n = new_node(AST_BUILTIN); n->builtin.type = type; return n; }
        
        ASTNode* n = new_node(AST_BUILTIN);
        n->builtin.type = type;
        n->builtin.arg = arg;
        return n;
    }

    if (t.type == VTOKEN_LPAREN) {
        parser_advance(p);

        ASTNode* n = parse_concat(p);

        if (p->current.type != VTOKEN_RPAREN)
            error_alert(&p->lexer, "expected ')'");

        parser_advance(p);
        return n;
    }

    if (t.type == VTOKEN_LBRACKET) {
        parser_advance(p);

        ASTNode* n = new_node(AST_LIST);

        n->list.items = malloc(sizeof(ASTNode*) * 64);
        n->list.count = 0;

        while (
            p->current.type != VTOKEN_RBRACKET &&
            p->current.type != VTOKEN_EOF
        ) {
            n->list.items[n->list.count++] = parse_concat(p);

            if (p->current.type == VTOKEN_COMMA)
                parser_advance(p);
        }

        if (p->current.type != VTOKEN_RBRACKET)
            error_alert(&p->lexer, "expected ']'");

        parser_advance(p);

        return n;
    }

    if (t.type == VTOKEN_TRUE) {
        ASTNode* n = new_node(AST_BOOL);
        n->blean.tof = 1;
        parser_advance(p);
        return n;
    }

    if (t.type == VTOKEN_FALSE) {
        ASTNode* n = new_node(AST_BOOL);
        n->blean.tof = 0;
        parser_advance(p);
        return n;
    }

    error_alert(&p->lexer, "syntax error at '%s'", t.value ? t.value : "UNK");
    return NULL;
}

ASTNode* parse_expo(Parser* p) {
    ASTNode* left = parse_postfix(p);

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

    while (p->current.type == VTOKEN_STAR || p->current.type == VTOKEN_SLASH || p->current.type == VTOKEN_MOD || p->current.type == VTOKEN_INTSLASH) {
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

    ASTNode* cond = parse_concat(p);
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

        ASTNode* elseif_cond = parse_concat(p);
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
            ? parse_block(p) : parse_block(p);

        current->if_stmt.else_branch = else_body;
    }

    return node;
}

ASTNode* parse_while(Parser* p) {
    parser_advance(p); 

    ASTNode* cond = parse_concat(p);

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

ASTNode* parse_concat(Parser* p) {
    ASTNode* node = parse_logic_or(p);

    while (p->current.type == VTOKEN_CONCAT) {
        parser_advance(p);

        ASTNode* right = parse_concat(p);

        ASTNode* n = new_node(AST_BINOP);
        n->binop.left = node;
        n->binop.right = right;
        n->binop.op = VTOKEN_CONCAT;

        node = n;
    }

    return node;
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

        ASTNode* val = parse_concat(p);

        ASTNode* n = new_node(AST_LET);
        n->let.name = name;
        n->let.value = val;
        return n;
    }

    if (p->current.type == VTOKEN_IDENTIFIER) {
        VTOKEN ident = p->current;

        parser_advance(p);

        if (
            p->current.type == VTOKEN_ASSIGN     ||
            p->current.type == VTOKEN_ASPLUS     ||
            p->current.type == VTOKEN_ASMINUS    ||
            p->current.type == VTOKEN_ASSTAR     ||
            p->current.type == VTOKEN_ASEXPO     ||
            p->current.type == VTOKEN_ASMOD      ||
            p->current.type == VTOKEN_ASSLASH    ||
            p->current.type == VTOKEN_ASCONCAT   ||
            p->current.type == VTOKEN_ASINTSLASH
        ) {
            VVTOKENType op = p->current.type;

            parser_advance(p);

            if (is_variable_defined(ident.value)) {
                ASTNode* rhs = parse_concat(p);

                ASTNode* val = NULL;

                if (op == VTOKEN_ASSIGN) {
                    val = rhs;
                } else {
                    ASTNode* lhs = new_node(AST_VAR);
                    lhs->var_name = ident.value;

                    ASTNode* bin = new_node(AST_BINOP);

                    switch (op) {
                        case VTOKEN_ASPLUS:
                            bin->binop.op = VTOKEN_PLUS;
                            break;

                        case VTOKEN_ASMINUS:
                            bin->binop.op = VTOKEN_MINUS;
                            break;

                        case VTOKEN_ASSTAR:
                            bin->binop.op = VTOKEN_STAR;
                            break;

                        case VTOKEN_ASEXPO:
                            bin->binop.op = VTOKEN_EXPO;
                            break;

                        case VTOKEN_ASMOD:
                            bin->binop.op = VTOKEN_MOD;
                            break;

                        case VTOKEN_ASSLASH:
                            bin->binop.op = VTOKEN_SLASH;
                            break;

                        case VTOKEN_ASCONCAT:
                            bin->binop.op = VTOKEN_CONCAT;
                            break;

                        case VTOKEN_ASINTSLASH:
                            bin->binop.op = VTOKEN_INTSLASH;
                            break;

                        default:
                            error_alert(&p->lexer, "invalid assignment operator");
                    }

                    bin->binop.left = lhs;
                    bin->binop.right = rhs;

                    val = bin;
                }

                ASTNode* n = new_node(AST_LET);
                n->let.name = ident.value;
                n->let.value = val;

                return n;
            }

            if (!skip_error) {
                printf("runtime error: undefined variable '%s'\n", ident.value);
                exit(1);
            }
        }

        ASTNode* n = new_node(AST_VAR);
        n->var_name = ident.value;

        if (p->current.type == VTOKEN_LPAREN) {
            parser_advance(p);

            ASTNode** args = malloc(sizeof(ASTNode*) * 32);
            int count = 0;

            while (p->current.type != VTOKEN_RPAREN) {
                args[count++] = parse_concat(p);;

                if (p->current.type == VTOKEN_COMMA)
                    parser_advance(p);
            }

            parser_advance(p);

            ASTNode* call = new_node(AST_CALL);
            call->call.name = ident.value;
            call->call.args = args;
            call->call.arg_count = count;

            return call;
        }

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
            val = parse_concat(p);

        ASTNode* n = new_node(AST_RETURN);
        n->return_stmt.value = val;
        return n;
    }

    if (p->current.type == VTOKEN_FN) {
        return parse_function(p);
    }

    return parse_concat(p);
}



/* ===================== EVALUATE ===================== */

typedef struct Value Value;

typedef struct {
    Value* items;
    int count;
} List;

struct Value {
    int type;
    double num;
    char* s;
    List* list;
};

int break_flag = 0;
int continue_flag = 0;
int return_flag = 0;
Value return_value;

typedef struct { char* name; Value value; } Variable;
Variable vars[1000]; int var_count = 0;

// Hàm trả về 1 nếu biến đã tồn tại, ngược lại trả về 0
int is_variable_defined(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return 1; // Tìm thấy biến trong danh sách
        }
    }
    return 0; // Không tìm thấy
}

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
     if (skip_error) {} else {printf("runtime error: undefined variable '%s'\n", name); exit(1);}
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

int starts_with(const char* s, const char* prefix) {
    if (!s || !prefix) return 0;

    while (*prefix) {
        if (*s != *prefix) return 0;
        s++;
        prefix++;
    }
    return 1;
}

int ends_with(const char* s, const char* suffix) {
    if (!s || !suffix) return 0;

    size_t len_s = strlen(s);
    size_t len_p = strlen(suffix);

    if (len_p > len_s) return 0;

    return strcmp(s + (len_s - len_p), suffix) == 0;
}

typedef struct {
    char** items;
    int count;
} StringArray;

StringArray split(const char* s, const char* delimiter) {
    StringArray arr;

    arr.items = malloc(sizeof(char*) * 128);
    arr.count = 0;

    int delim_len = strlen(delimiter);

    const char* start = s;
    const char* pos;

    while ((pos = strstr(start, delimiter)) != NULL) {

        int len = pos - start;

        char* part = malloc(len + 1);

        strncpy(part, start, len);
        part[len] = '\0';

        arr.items[arr.count++] = part;

        start = pos + delim_len;
    }

    arr.items[arr.count++] = strdup(start);

    return arr;
}

char* replace_all(const char* str, const char* old, const char* repl) {
    if (!str || !old || !repl) return NULL;

    size_t str_len = strlen(str);
    size_t old_len = strlen(old);
    size_t repl_len = strlen(repl);

    if (old_len == 0) {
        char* out = malloc(str_len + 1);
        strcpy(out, str);
        return out;
    }

    int count = 0;
    const char* p = str;

    while ((p = strstr(p, old)) != NULL) {
        count++;
        p += old_len;
    }

    size_t new_len =
        str_len +
        count * (repl_len - old_len);

    char* result = malloc(new_len + 1);

    char* dst = result;
    const char* src = str;

    while ((p = strstr(src, old)) != NULL) {
        size_t chunk = p - src;

        memcpy(dst, src, chunk);
        dst += chunk;

        memcpy(dst, repl, repl_len);
        dst += repl_len;

        src = p + old_len;
    }

    strcpy(dst, src);

    return result;
}

int value_eq(Value a, Value b) {
    if (a.type != b.type)
        return 0;

    switch (a.type) {
        case 0: // number
            return a.num == b.num;

        case 1: // string
            return strcmp(a.s, b.s) == 0;

        case 4: // bool same as num
            return a.num == b.num;

        default:
            return 0;
    }
}

char* join(List* list, const char* sep) {
    int sep_len = strlen(sep);
    int total = 1; // '\0'

    for (int i = 0; i < list->count; i++) {
        if (list->items[i].type != 1) {
            printf("runtime error: join expects string list\n");
            exit(1);
        }

        total += strlen(list->items[i].s);

        if (i + 1 < list->count)
            total += sep_len;
    }

    char* result = malloc(total);
    result[0] = '\0';

    for (int i = 0; i < list->count; i++) {
        strcat(result, list->items[i].s);

        if (i + 1 < list->count)
            strcat(result, sep);
    }

    return result;
}

Value eval(ASTNode* node) {
    Value v = {0, 0, NULL}; if (!node) return v;
    switch (node->type) {
        case AST_NUMBER: v.type = 0; v.num = node->number; return v;
        case AST_STRING: v.type = 1; v.s = strdup(node->string); return v;
        case AST_VAR: return get_var(node->var_name);
        case AST_LET: { Value val = eval(node->let.value); set_var(node->let.name, val); return val; }
        case AST_PRINT: {
            Value val = eval(node->print.value);

            if (val.type == 0)
                printf("%g\n", val.num);

            else if (val.type == 1)
                printf("%s\n", val.s);

            else if (val.type == 2)
                printf("%s\n", val.num ? "true" : "false");

            else if (val.type == 3) {
                printf("[");

                for (int i = 0; i < val.list->count; i++) {
                    Value item = val.list->items[i];

                    if (item.type == 0)
                        printf("%g", item.num);

                    else if (item.type == 1)
                        printf("\"%s\"", item.s);

                    if (i + 1 < val.list->count)
                        printf(", ");
                }

                printf("]\n");
            }

            return val;
        }
        case AST_INPUT: {
            if (node->input.prompt) { Value p = eval(node->input.prompt); printf("%s", p.s); }
            char buf[1024]; fgets(buf, 1024, stdin); buf[strcspn(buf, "\n")] = 0;
            char* end; long res = strtol(buf, &end, 10);
            if (*end == '\0' && strlen(buf) > 0) { v.type = 0; v.num = (int)res; }
            else { v.type = 1; v.s = strdup(buf); }
            return v;
        }
        case AST_RAND: {
            Value min_val = eval(node->rand.min);
            Value max_val = eval(node->rand.max);
            v.type = 0;
            if (max_val.num <= min_val.num) v.num = min_val.num;
            else v.num = ceil((rand() % ((int)max_val.num - (int)min_val.num + 1))) + floor(min_val.num);
            return v;
        }
        case AST_BUILTIN: {
            if (node->builtin.type == VTOKEN_CLEAR) { system("cls"); return v; }
            if (node->builtin.type == VTOKEN_EXIT) { exit(0); }
            Value arg = eval(node->builtin.arg);
            switch(node->builtin.type) {
                case VTOKEN_LEN: {
                    printf("ok\n");
                    v.type = 0; 
                    if (arg.type == 1) {v.num = (int)strlen(arg.s);} 
                    break;
                }
                case VTOKEN_ABS: v.type = 0; v.num = (arg.type == 0) ? abs((int)arg.num) : 0; break;
                case VTOKEN_FABS: v.type = 0; v.num = (arg.type == 0) ? fabs(arg.num) : 0; break;
                case VTOKEN_SQRT: {
                    if (arg.type != 0) {
                        if (skip_error) {v.num = NAN;} else {printf("runtime error: sqrt of non-number\n"); exit(1);}
                    }

                    if (arg.num < 0) {
                        if (skip_error) {v.num = NAN;} else {printf("runtime error: sqrt of negative number: %g\n", arg.num); exit(1);}
                    }

                    v.num = sqrt(arg.num);
                    break;
                }
                case VTOKEN_SLEEP: if(arg.type == 0) Sleep(arg.num); break;
                case VVTOKEN_TYPE: {
                    v.type = 1;

                    if (arg.type == 0) {
                        if (arg.num == round(arg.num))
                            v.s = "int";
                        else
                            v.s = "float";
                    }
                    else if (arg.type == 1) {
                        v.s = "string";
                    }
                    else if (arg.type == 2) {
                        v.s = "bool";
                    }
                    else if (arg.type == 3) {
                        v.s = "list";
                    }
                    else {
                        v.s = "unknown";
                    }

                    return v;
                }
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

            double tol = 1e-15 * fmax(fmax(fabs(l.num), fabs(r.num)), 1.0);

            if      (node->binop.op == VTOKEN_PLUS)  v.num = l.num + r.num;
            else if (node->binop.op == VTOKEN_MINUS) v.num = l.num - r.num;
            else if (node->binop.op == VTOKEN_STAR)  v.num = l.num * r.num;
            else if (node->binop.op == VTOKEN_SLASH) {
                if (r.num == 0) {
                    if (skip_error) {v.num = NAN;} else {printf("runtime error: division by zero\n"); exit(1);}
                } else v.num = l.num / r.num;
            }
            else if (node->binop.op == VTOKEN_INTSLASH) {
                if (r.num == 0) {
                    if (skip_error) {v.num = NAN;} else {printf("runtime error: division by zero\n"); exit(1);}
                } else v.num = (int)(l.num / r.num);
            }
            else if (node->binop.op == VTOKEN_EXPO)  v.num = power(l.num, r.num);
            else if (node->binop.op == VTOKEN_CONCAT) {
                char buf[2048];

                const char* ls;
                const char* rs;

                char lnum[64];
                char rnum[64];

                if (l.type == 1)
                    ls = l.s;
                else {
                    sprintf(lnum, "%g", l.num);
                    ls = lnum;
                }

                if (r.type == 1)
                    rs = r.s;
                else {
                    sprintf(rnum, "%g", r.num);
                    rs = rnum;
                }

                snprintf(buf, sizeof(buf), "%s%s", ls, rs);

                v.type = 1;
                v.s = strdup(buf);
            }
            else if (node->binop.op == VTOKEN_MOD) {
                if (r.num == 0) {
                    if (skip_error) {v.num = NAN;} else {printf("runtime error: mod by zero\n"); exit(1);}
                } else v.num = fmod(l.num, r.num);
            }
            else if (node->binop.op == VTOKEN_MOD)   v.num = (r.num != 0) ? fmod(l.num, r.num) : 0;
            else if (node->binop.op == VTOKEN_EQ) {
                v.type = 2;

                if (l.type == 1 && r.type == 1) {
                    v.num = strcmp(l.s, r.s) == 0;
                } else {
                    v.num = fabs(l.num - r.num) < tol;
                }
            }
            else if (node->binop.op == VTOKEN_NEQ) {
                v.type = 2;

                if (l.type == 1 && r.type == 1) {
                    v.num = strcmp(l.s, r.s) != 0;
                } else {
                    v.num = fabs(l.num - r.num) >= tol;
                }
            }
            else if (node->binop.op == VTOKEN_G)     {v.type = 2; v.num = (l.num >  r.num);}
            else if (node->binop.op == VTOKEN_S)     {v.type = 2; v.num = (l.num <  r.num);}
            else if (node->binop.op == VTOKEN_GEQ)   {v.type = 2; v.num = (l.num >= r.num);}
            else if (node->binop.op == VTOKEN_SEQ)   {v.type = 2; v.num = (l.num <= r.num);}
            else if (node->binop.op == VTOKEN_AND)   {v.type = 2; v.num = (l.num && r.num);}
            else if (node->binop.op == VTOKEN_OR)    {v.type = 2; v.num = (l.num || r.num);}
            return v;
        }
        case AST_IF: {
            Value c = eval(node->if_stmt.cond);

            if (c.num) {
                return eval(node->if_stmt.body);
            } else if (node->if_stmt.else_branch) {
                return eval(node->if_stmt.else_branch);
            }

            return (Value){0};
        }
        case AST_WHILE: {
            while (1) {
                Value c = eval(node->while_stmt.cond);

                if (!c.num)
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

        case AST_BOOL: {
            v.type = 2;
            v.num = node->blean.tof;
            return v;
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
            if (strcmp(node->call.name, "startswith") == 0) {
                Value s = eval(node->call.args[0]);
                Value p = eval(node->call.args[1]);

                v.type = 2;
                v.num = starts_with(s.s, p.s);

                return v;
            }

            if (strcmp(node->call.name, "endswith") == 0) {
                Value s = eval(node->call.args[0]);
                Value p = eval(node->call.args[1]);

                v.type = 2;
                v.num = ends_with(s.s, p.s);

                return v;
            }

            if (strcmp(node->call.name, "split") == 0) {
                Value s = eval(node->call.args[0]);
                Value d = eval(node->call.args[1]);

                StringArray arr = split(s.s, d.s);

                List* list = malloc(sizeof(List));

                list->count = arr.count;
                list->items = malloc(sizeof(Value) * arr.count);

                for (int i = 0; i < arr.count; i++) {
                    list->items[i].type = 1;      // string
                    list->items[i].s = arr.items[i];
                    list->items[i].list = NULL;
                    list->items[i].num = 0;
                }

                v.type = 3;      // list
                v.list = list;

                return v;
            }

            if (strcmp(node->call.name, "pop") == 0) {
                Value listv = eval(node->call.args[0]);

                if (listv.type != 3) {
                    printf("runtime error: pop on non-list\n");
                    exit(1);
                }

                if (listv.list->count == 0) {
                    printf("runtime error: pop from empty list\n");
                    exit(1);
                }

                Value removed =
                    listv.list->items[listv.list->count - 1];

                listv.list->count--;

                return removed;
            }

            if (strcmp(node->call.name, "push") == 0) {
                Value listv = eval(node->call.args[0]);
                Value item  = eval(node->call.args[1]);

                if (listv.type != 3) {
                    printf("runtime error: push on non-list\n");
                    exit(1);
                }

                listv.list->count++;

                listv.list->items = realloc(
                    listv.list->items,
                    sizeof(Value) * listv.list->count
                );

                listv.list->items[listv.list->count - 1] = item;

                return listv;
            }

            if (strcmp(node->call.name, "contains") == 0) {
                Value arg1 = eval(node->call.args[0]);
                Value arg2 = eval(node->call.args[1]);
                if (arg1.type == 1 && arg2.type == 1) {
                    v.type = 2; // bool
                    if (strstr(arg1.s, arg2.s) != NULL) {
                        v.num = 1;
                    } else {v.num = 0;}
                    return v;
                }

                if (arg1.type == 3) {
                    v.type = 2;
                    v.num = 0;

                    for (int i = 0; i < arg1.list->count; i++) {
                        if (value_eq(arg1.list->items[i], arg2)) {
                            v.num = 1;
                            break;
                        }
                    }

                    return v;
                }
            }

            if (strcmp(node->call.name, "replace") == 0) {
                Value str  = eval(node->call.args[0]);
                Value old  = eval(node->call.args[1]);
                Value repl = eval(node->call.args[2]);

                if (str.type != 1 ||
                    old.type != 1 ||
                    repl.type != 1) {
                    printf("runtime error: replace expects 3 strings\n");
                    exit(1);
                }

                v.type = 1;
                v.s = replace_all(
                    str.s,
                    old.s,
                    repl.s
                );

                return v;
            }

            if (strcmp(node->call.name, "join") == 0) {
                Value arr = eval(node->call.args[0]);
                Value sep = eval(node->call.args[1]);

                if (arr.type != 3 || sep.type != 1) {
                    printf("runtime error: join expects (list, string)\n");
                    exit(1);
                }

                v.type = 1;
                v.s = join(arr.list, sep.s);

                return v;
            }

            if (strcmp(node->call.name, "insert") == 0) {
                Value listv = eval(node->call.args[0]);
                Value idxv  = eval(node->call.args[1]);
                Value item  = eval(node->call.args[2]);

                if (listv.type != 3) {
                    printf("runtime error: insert expects list\n");
                    exit(1);
                }

                int idx = (int)idxv.num;

                if (idx < 0 || idx > listv.list->count) {
                    printf("runtime error: insert index out of range\n");
                    exit(1);
                }

                listv.list->count++;

                listv.list->items = realloc(
                    listv.list->items,
                    sizeof(Value) * listv.list->count
                );

                for (int i = listv.list->count - 1; i > idx; i--) {
                    listv.list->items[i] =
                        listv.list->items[i - 1];
                }

                listv.list->items[idx] = item;

                return listv;
            }

            if (strcmp(node->call.name, "remove") == 0) {
                Value listv = eval(node->call.args[0]);

                Value idxv = eval(node->call.args[1]);

                if (listv.type != 3) {
                    printf("runtime error: remove expects list\n");
                    exit(1);
                }

                int idx = (int)idxv.num;

                if (idx < 0 || idx >= listv.list->count) {
                    printf("runtime error: remmove index out of range\n");
                    exit(1);
                }

                Value removed =
                    listv.list->items[idx];

                for (int i = idx;
                    i < listv.list->count - 1;
                    i++) {

                    listv.list->items[i] =
                        listv.list->items[i + 1];
                }

                listv.list->count--;

                listv.list->items = realloc(
                    listv.list->items,
                    sizeof(Value) * listv.list->count
                );

                return removed;
            }

            for (int i = 0; i < func_count; i++) {
                if (strcmp(functions[i].name, node->call.name) == 0) {

                    ASTNode* fn = functions[i].node;

                    for (int j = 0; j < fn->function.param_count; j++) {

                        if (j >= node->call.arg_count || !node->call.args) {
                            printf("runtime error: missing argument\n");
                            exit(1);
                        }

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
        case AST_LIST: {
            List* list = malloc(sizeof(List));

            list->count = node->list.count;
            list->items = malloc(sizeof(Value) * list->count);

            for (int i = 0; i < list->count; i++) {
                list->items[i] = eval(node->list.items[i]);
            }

            v.type = 3;
            v.list = list;

            return v;
        }
        case AST_INDEX: {
            Value obj = eval(node->index.object);
            Value idx = eval(node->index.index);

            if (obj.type != 3) {
                printf("runtime error: indexing non-list\n");
                exit(1);
            }

            int i = (int)idx.num;

            if (i < 0 || i >= obj.list->count) {
                printf("runtime error: index out of range\n");
                exit(1);
            }

            return obj.list->items[i];
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

    const char* filename = "main.vt";
    const char* direct_code = NULL;

    for (int i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-i") ||
            !strcmp(argv[i], "--input")) {

            if (i + 1 >= argc) {
                printf("error: missing filename\n");
                return 1;
            }

            filename = argv[++i];
        }

        else if (!strcmp(argv[i], "-c") ||
                 !strcmp(argv[i], "--code")) {

            if (i + 1 >= argc) {
                printf("error: missing code\n");
                return 1;
            }

            direct_code = argv[++i];
        }

        else if (!strcmp(argv[i], "-se") ||
                 !strcmp(argv[i], "--skiperror")) {

            skip_error = 1;
        }

        else if (!strcmp(argv[i], "-oh") ||
                 !strcmp(argv[i], "--optionhelp")) {

            printf(
                "vanta options\n"
                "\n"
                "-i,  --input <file>     run file\n"
                "-c,  --code  <code>     run code directly\n"
                "-se, --skiperror        continue after errors\n"
                "-oh, --optionhelp       show this help\n"
                "-ch, --codehelp         show language help\n"
            );

            return 0;
        }

        else if (!strcmp(argv[i], "-ch") ||
                 !strcmp(argv[i], "--codehelp")) {

            printf(
                "vanta - hobby programming language\n"
                "\n"
                "features:\n"
                "- variables\n"
                "- functions\n"
                "- if / elseif / else\n"
                "- while\n"
                "- int and float\n"
                "- boolean\n"
                "- string concat (..)\n"
                "- operators + - * / %% **\n"
                "- assign = += -= /= //= *="
                "\n"
                "builtins:\n"
                "- print\n"
                "- input\n"
                "- rand\n"
                "- len\n"
                "- abs\n"
                "- fabs\n"
                "- sqrt\n"
            );

            return 0;
        } else if (!strcmp(argv[i], "-v") ||
                   !strcmp(argv[i], "--version")) {
            printf(version);
        }

        else {
            filename = argv[i];
        }
    }

    char* code;

    if (direct_code) {
        code = strdup(direct_code);
    } else {
        code = read_file(filename);
    }

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