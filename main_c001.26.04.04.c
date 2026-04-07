#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <math.h>
#include <time.h> // 🔥 Thêm để dùng time()

// Tương thích giữa các trình biên dịch (GCC/MSVC)
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
    VTOKEN_IDENTIFIER, VTOKEN_PLUS, VTOKEN_MINUS, VTOKEN_STAR, VTOKEN_COMMA,
    VTOKEN_SLASH, VTOKEN_LPAREN, VTOKEN_RPAREN, VTOKEN_NEWLINE,
    VTOKEN_PRINTF, VTOKEN_INPUT, VTOKEN_STRING, VTOKEN_RAND,
    VTOKEN_LEN, VTOKEN_CLEAR, VTOKEN_ABS, VTOKEN_EXIT,
    VTOKEN_SQRT, VTOKEN_SLEEP, VVTOKEN_TYPE, VTOKEN_UPPER, VTOKEN_LOWER
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
        skip_whitespace(l); // Xóa khoảng trắng sau khi xuống dòng
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
        // Cho phép ký tự chữ, số, dấu gạch dưới và các byte UTF-8 (> 127)
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
        else t.type = VTOKEN_IDENTIFIER;
        t.value = id; return t;
    }

    if (l->current == '"') { t.type = VTOKEN_STRING; t.value = read_string(l); return t; }

    switch (l->current) {
        case '+': t.type = VTOKEN_PLUS; break;
        case '-': t.type = VTOKEN_MINUS; break;
        case '*': t.type = VTOKEN_STAR; break;
        case '/': t.type = VTOKEN_SLASH; break;
        case ',': t.type = VTOKEN_COMMA; break;
        case '=': t.type = VTOKEN_ASSIGN; break;
        case '(': t.type = VTOKEN_LPAREN; break;
        case ')': t.type = VTOKEN_RPAREN; break;
        case '\n': t.type = VTOKEN_NEWLINE; break;
        default: error_alert(l, "unexpected character '%c'", l->current);
    }
    advance(l); return t;
}

/* ===================== AST ===================== */

typedef enum {
    AST_NUMBER, AST_BINOP, AST_VAR, AST_LET, AST_PRINT, AST_INPUT, 
    AST_STRING, AST_BUILTIN, AST_RAND
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

    // 🔥 HỖ TRỢ SỐ ÂM (Unary Minus)
    if (t.type == VTOKEN_MINUS) {
        parser_advance(p); // Bỏ dấu '-'
        ASTNode* operand = parse_factor(p); // Lấy con số hoặc biểu thức phía sau
        
        // Biến -x thành phép tính (0 - x)
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
        ASTNode* n = new_node(AST_VAR);
        n->var_name = t.value;
        parser_advance(p);
        return n;
    }

    // Xử lý built-ins (printf, input, rand, etc.)
    if (t.type >= VTOKEN_PRINTF && t.type <= VTOKEN_LOWER) {
        VVTOKENType type = t.type;

        if (type == VTOKEN_RAND) {
            parser_advance(p); // skip 'rand'
            parser_advance(p); // skip '('
            ASTNode* min_n = parse_expr(p);
            parser_advance(p); // skip ','
            ASTNode* max_n = parse_expr(p);
            parser_advance(p); // skip ')'
            ASTNode* n = new_node(AST_RAND);
            n->rand.min = min_n; n->rand.max = max_n;
            return n;
        }

        parser_advance(p);
        ASTNode* arg = NULL;
        if (p->current.type == VTOKEN_LPAREN) {
            parser_advance(p);
            if (p->current.type != VTOKEN_RPAREN) arg = parse_expr(p);
            parser_advance(p); // skip ')'
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
        parser_advance(p); // skip ')'
        return n;
    }

    error_alert(&p->lexer, "syntax error at '%s'", t.value ? t.value : "EOF");
    return NULL;
}

ASTNode* parse_term(Parser* p) {
    ASTNode* node = parse_factor(p);
    while (p->current.type == VTOKEN_STAR || p->current.type == VTOKEN_SLASH) {
        VVTOKENType op = p->current.type; parser_advance(p);
        ASTNode* right = parse_factor(p);
        ASTNode* n = new_node(AST_BINOP); n->binop.left = node; n->binop.op = op; n->binop.right = right;
        node = n;
    }
    return node;
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

ASTNode* parse_statement(Parser* p) {
    skip_newlines(p);
    if (p->current.type == VTOKEN_LET) {
        parser_advance(p);
        const char* name = p->current.value;
        parser_advance(p);
        if (p->current.type != VTOKEN_ASSIGN) error_alert(&p->lexer, "expected '=' after %s", name);
        parser_advance(p);
        ASTNode* val = parse_expr(p);
        ASTNode* n = new_node(AST_LET); n->let.name = name; n->let.value = val;
        return n;
    }
    return parse_expr(p);
}

/* ===================== EVALUATE ===================== */

typedef struct { int type; int i; char* s; } Value;
typedef struct { char* name; Value value; } Variable;
Variable vars[1000]; int var_count = 0;

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
    printf("Runtime Error: Undefined variable '%s'\n", name); exit(1);
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
            if (node->binop.op == VTOKEN_PLUS) v.i = l.i + r.i;
            else if (node->binop.op == VTOKEN_MINUS) v.i = l.i - r.i;
            else if (node->binop.op == VTOKEN_STAR) v.i = l.i * r.i;
            else if (node->binop.op == VTOKEN_SLASH) v.i = (r.i != 0) ? l.i / r.i : 0;
            return v;
        }
    }
    return v;
}

/* ===================== FILE HELPER ===================== */

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) { printf("Error: Could not open file %s\n", filename); exit(1); }
    fseek(f, 0, SEEK_END); long size = ftell(f); rewind(f);
    char* buf = malloc(size + 1);
    fread(buf, 1, size, f); buf[size] = '\0';
    fclose(f); return buf;
}

/* ===================== MAIN ===================== */

int main() {
    srand(time(NULL));
    set_unicode();
    char* code = read_file("test.vt");

    Parser p;
    lexer_init(&p.lexer, code);
    parser_advance(&p);

    while (p.current.type != VTOKEN_EOF) {
        skip_newlines(&p);
        if (p.current.type == VTOKEN_EOF) break;
        ASTNode* stmt = parse_statement(&p);
        eval(stmt);
        skip_newlines(&p);
    }

    free(code);
    return 0;
}