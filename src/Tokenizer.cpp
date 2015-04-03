#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "Tokenizer.h"

using namespace std;

Tokenizer::Tokenizer (const string& src_file_path)
{
    this->src_file_path = src_file_path;
    this->src_file = new ifstream ();
    this->current_line.clear ();
    this->current_col = 1;
    this->eof = false;
}

bool
Tokenizer::init ()
{
    this->src_file->open (this->src_file_path.c_str ());
    return this->src_file->is_open ();
}

Tokenizer::~Tokenizer ()
{
    delete this->src_file;
}

void
Tokenizer::read ()
{
    if (this->current_col >= this->current_line.length ()) {
        if (this->src_file->eof ()) {
            this->eof = true;
        }
        else {
            getline (*(this->src_file), this->current_line);
            this->current_col = 0;
        }
    }
}

Token *
Tokenizer::makeToken (TokenType t, int col_add)
{
    Token *tkn = this->token_allocator.newToken (t);
    this->current_col += col_add;
    return tkn;
}

static int
escapeChar (const char *s, char *c)
{
    if (*s == '\\') {
        s++;
        switch (*s) {
            case '\0':
                // ERROR: escape character not specified
                exit (1);
            case 'n':
                *c = '\n';
                return 2;
            case 't':
                *c = '\t';
                return 2;
            case '\\':
                *c = '\\';
                return 2;
            case '"':
                *c = '\"';
                return 2;
            case '\'':
                *c = '\'';
                return 2;
            case '0':
                *c = '\0';
                return 2;
            default:
            // ERROR: unhandled symbol
                exit (1);
        }
    }
    else if (*s == '\0') {
        // ERROR: end of string
        exit (1);
    }
    *c = *s;
    return 1;
}

static int
computeEscapedLength (const char *s)
{
    if (*s != '"' && *s != '\'') {
        return 0;
    }
    char c;
    const char *begin = s;
    int len = 0;
    s++;
    while (*s != *begin) {
        int unescapedLen = escapeChar (s, &c);
        s += unescapedLen;
        len++;
    }
    return len;
}

Token *
Tokenizer::parseStringToken ()
{
    const char *s = this->current_line.c_str ();
    int begin = this->current_col;
    int i = begin;

    if (s[i] != '"' && s[i] != '\'') {
        return NULL;
    }
    /* compute string literal's length so we know
     * how much memory to allocate
     */
    size_t escapedLength = (size_t)computeEscapedLength (s + i);
    char *strng_lit = new char[escapedLength + 1];
    char c;
    
    i++;
    int j = 0;
    /* Now we escape the string and store it in strng_lit.
     */
    while (s[i] != s[begin]) {
        int unescapedLen = escapeChar (s + i, &c);
        strng_lit[j++] = c;
        i += unescapedLen;
    }
    strng_lit[escapedLength] = '\0';

    /* Now we make the token with the string we parsed.
     */
    Token *tkn = makeToken (STRING_LIT, i - this->current_col);
    tkn->literal.s = new string (strng_lit, escapedLength);

    delete [] strng_lit;

    return tkn;
}

typedef struct {
    TokenType       token_type;
    const char *    keyword;
    int             len;
} KeywordToken;

#define NUM_KEYWORDS 18
KeywordToken keyword_tokens[NUM_KEYWORDS] = {
    {IF,        "if",       2},
    {ELIF,      "elif",     4},
    {ELSE,      "else",     4},
    {WHILE,     "while",    5},
    {FOREACH,   "foreach",  7},
    {FOR,       "for",      3},
    {IN,        "in",       2},
    {RETURN,    "return",   6},
    {ISTYPE,    "istype",   6},
    {FUNC,      "func",     4},
    {INT,       "int",      3},
    {FLOAT,     "float",    5},
    {STRING,    "string",   6},
    {BOOL,      "bool",     4},
    {LIST,      "list",     4},
    {DICT,      "dict",     4},
    {ANY,       "any",      3},
    {NIL,       "nil",      3}
};

Token *
Tokenizer::attemptParseKeywordToken (const char *s, int len)
{
    for (int i = 0; i < NUM_KEYWORDS; ++i) {
        if (len == keyword_tokens[i].len
         && 0 == strncmp (s, keyword_tokens[i].keyword, (size_t)len)) {
            return makeToken (keyword_tokens[i].token_type, len);
        }
    }
    return NULL;
}

Token *
Tokenizer::parseAlphaToken ()
{
    const char *s   = this->current_line.c_str ();
    int begin       = this->current_col;
    int len         = this->current_line.length ();
    int i           = begin;

    while (i < len) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            i++;
            continue;
        }
        if (s[i] >= 'a' && s[i] <= 'z') {
            i++;
            continue;
        }
        if (i > this->current_col
        && s[i] >= '0'
        && s[i] <= '9') {
            i++;
            continue;
        }
        if (s[i] == '_') {
            i++;
            continue;
        }
        break;
    }

    Token *tkn = attemptParseKeywordToken (s + begin, i - begin);
    if (NULL != tkn) {
        return tkn;
    }

    tkn = makeToken (IDENTIFIER, i - begin);
    tkn->literal.id = new string (s + begin, (size_t)(i - begin));

    return tkn;
}

static long long
alphaToLongLong (const char *s, int len)
{
    long long val = 0;
    for (int i = 0; i < len; ++i, ++s) {
        val *= 10;
        val += (*s - '0');
    }
    return val;
}

static double
alphaToDouble (const char *s, int len)
{
    long long llval = 0;
    int i;
    for (int i = 0; i < len; ++i, ++s) {
        if ('.' == *s) {
            break;
        }
        llval *= 10;
        llval += (*s - '0');
    }
    long long lldec = 0;
    long long denom = 1;
    for (++i, ++s; i < len; ++i, ++s) {
        lldec *= 10;
        denom *= 10;
        lldec += (*s - '0');
    }
    // TODO handle EXP, e.g. 1e10, 1.3e-12
    return (double)llval + ((double)lldec / (double)denom);
}

Token *
Tokenizer::parseNumericToken ()
{
    const char *s       = this->current_line.c_str ();
    int begin           = this->current_col;
    int len             = this->current_line.length ();
    int i               = begin;
    bool seen_dec_pt    = false;

    while (i < len) {
        if (s[i] >= '0' && s[i] <= '9') {
            i++;
            continue;
        }
        if (s[i] == '.') {
            if (!seen_dec_pt) {
                i++;
                seen_dec_pt = true;
                continue;
            }
            else {
                // ERROR: multiple decimal points
                exit (1);
            }
        }
        break;
    }
    // TODO handle exponents
    Token *tkn;
    if (seen_dec_pt) {
        tkn = makeToken (FLOAT_LIT, i - begin);
        tkn->literal.f = alphaToDouble (s + begin, i - begin);
    }
    else {
        tkn = makeToken (INT_LIT, i - begin);
        tkn->literal.i = alphaToLongLong (s + begin, i - begin);
    }
    return tkn;
}

typedef struct {
    TokenType       token_type;
    const char *    symbol;
    int             len;
} SymbolToken;

#define NUM_SYMBOLS 28

SymbolToken symbolTokens[NUM_SYMBOLS] = {
    {AND,           "&&",   2},
    {OR,            "||",   2},
    {LE,            "<=",   2},
    {GE,            ">=",   2},
    {EQ,            "==",   2},
    {NE,            "!=",   2},
    {LPAREN,        "(",    1},
    {RPAREN,        ")",    1},
    {LBRACKET,      "[",    1},
    {RBRACKET,      "]",    1},
    {LCURLY,        "{",    1},
    {RCURLY,        "}",    1},
    {COMMA,         ",",    1},
    {COLON,         ":",    1},
    {SEMICOLON,     ";",    1},
    {DOT,           ".",    1},
    {ASSIGN,        "=",    1},
    {ADD,           "+",    1},
    {MINUS,         "-",    1},
    {TIMES,         "*",    1},
    {DIVIDE,        "/",    1},
    {MODULO,        "%",    1},
    {AND_BIT,       "&",    1},
    {OR_BIT,        "|",    1},
    {XOR_BIT,       "^",    1},
    {NOT_BIT,       "!",    1},
    {LT,            "<",    1},
    {GT,            ">",    1}
};

Token *
Tokenizer::parseSymbolToken ()
{
    const char *s = this->current_line.c_str ();
    int begin = this->current_col;
    int len = this->current_line.length ();
    
    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        if (begin + symbolTokens[i].len - 1 < len) {
            if (0 == strncmp (s + begin,
                              symbolTokens[i].symbol,
                              symbolTokens[i].len)) {
                return makeToken (symbolTokens[i].token_type,
                                  symbolTokens[i].len);
            }
        }
    }
    return NULL;
}

Token *
Tokenizer::getNextToken ()
{
    const char * s = NULL;
    while (true) {
        this->read ();
        if (this->eof) {
            return makeToken (ENDOFFILE, 0);
        }
        s = this->current_line.c_str ();
        // skip space
        while (s[this->current_col] == ' '
            || s[this->current_col] == '\t') {
            this->current_col++;
        }
        // skip comment
        if (s[this->current_col] == '#') {
            this->current_col = this->current_line.length ();
            continue;
        }
        break;
    }

    /* Test for first character of token:
     * [_A-Za-z]
     * [0-9]
     * +-*%/&|^!~=()[]{},.<>"'
     * ==,!=,<=,>=,&&,||
     */
    int i   = this->current_col;
    int len = this->current_line.length ();
    if ((s[i] >= 'A' && s[i] <= 'Z')
     || (s[i] >= 'a' && s[i] <= 'z')
     || s[i] == '_') {
        return parseAlphaToken ();
    }
    else if (s[i] >= '0' && s[i] <= '9') {
        return parseNumericToken ();
    }
    else {
        return parseSymbolToken ();
    }
}

TokenAllocator::TokenAllocator ()
    : num_free_tokens (0)
{

}

void
TokenAllocator::allocNewTokenBucket ()
{
    Token *bucket = new (nothrow) Token[TOKEN_BUCKET_SIZE];
    if (NULL == bucket) {
        // ERROR: memory allocation failure
        exit (1);
    }
    token_buckets.push_back (bucket);
    num_free_tokens = TOKEN_BUCKET_SIZE;
}

Token *
TokenAllocator::newToken (TokenType t)
{
    if (0 == num_free_tokens) {
        allocNewTokenBucket ();
    }
    Token *bucket = token_buckets.back ();
    Token *tkn = bucket + (TOKEN_BUCKET_SIZE - (num_free_tokens--));
    tkn->token_type = t;
    return tkn;
}

TokenAllocator::~TokenAllocator ()
{
    for (list<Token *>::reverse_iterator it = token_buckets.rbegin ();
         it != token_buckets.rend ();
         ++it) {
        delete [] *it;
    }
    num_free_tokens = 0;
}

Token::Token ()
    : token_type (INVALID)
{

}

Token::~Token ()
{
    if (token_type == STRING_LIT) {
        delete literal.s;
    }
    else if (token_type == IDENTIFIER) {
        delete literal.id;
    }
}

