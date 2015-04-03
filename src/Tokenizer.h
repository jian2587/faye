#ifndef __tokenizer__
#define __tokenizer__

#include <list>
#include <string>
using namespace std;

enum TokenType {
    INVALID = 0,
    IDENTIFIER,
    INT_LIT,
    FLOAT_LIT,
    BOOL_LIT,
    STRING_LIT,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LCURLY,
    RCURLY,
    COMMA,
    COLON,
    SEMICOLON,
    DOT,
    IF,
    ELIF,
    ELSE,
    WHILE,
    FOREACH,
    IN,
    FOR,
    RETURN,
    ISTYPE,
    FUNC,
    INT,
    FLOAT,
    STRING,
    BOOL,
    LIST,
    DICT,
    ANY,
    NIL,
    ASSIGN,
    ADD,
    MINUS,
    TIMES,
    DIVIDE,
    MODULO,
    AND_BIT,
    OR_BIT,
    XOR_BIT,
    NOT_BIT,
    AND,
    OR,
    NOT,
    LT,
    GT,
    LE,
    GE,
    EQ,
    NE,
    ENDOFFILE,
    MAX_TOKEN_TYPES
};

typedef struct Token {
    TokenType token_type;
    union {
        long long   i;
        double      f;
        bool        b;
        string *    s;
        string *    id;
    } literal;

    Token ();
    virtual ~Token ();
} Token;

class TokenAllocator {
public:
    TokenAllocator ();
    Token * newToken (TokenType t);
    virtual ~TokenAllocator ();

    static const int TOKEN_BUCKET_SIZE = 1024;
private:
    void allocNewTokenBucket ();

    list<Token *>   token_buckets;
    int             num_free_tokens;
};

class Tokenizer {
public:
    Tokenizer (const string &src_file_path);
    virtual ~Tokenizer ();
    bool    init ();
    Token * getNextToken ();
private:
    void    read ();
    Token * makeToken (TokenType t, int col_add);
    Token * parseAlphaToken ();
    Token * parseNumericToken ();
    Token * parseStringToken ();
    Token * parseSymbolToken ();
    Token * attemptParseKeywordToken (const char *s, int len);

    string          src_file_path;
    ifstream        *src_file;
    string          current_line;
    int             current_col;
    bool            eof;
    TokenAllocator  token_allocator;
};

#endif

