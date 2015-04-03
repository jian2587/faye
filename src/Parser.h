#ifndef __parser__
#define __parser__

#include <list>
#include <string>
#include "Tokenizer.h"

using namespace std;

class Parser {
public:
    Parser (const string& src_file_path);
    virtual ~Parser ();
    void parse ();
    bool init ();
private:
    string      src_file_path;
    Tokenizer   tokenizer;
};

#endif

namespace production {
    typedef struct Unit {
        union {
            TokenType   token_type;
            int         rule_id;
        };
        bool            is_rule;
        Unit (TokenType t) : token_type (t),
                             is_rule (false) {}
        Unit (int rid) : rule_id (rid),
                         is_rule (true) {}
    } U;

    typedef struct Rule {
        int     id;
        list<U> refs;

        Rule (int _id, const U& u0);
        Rule (int _id, const U& u0, const U& u1);
        Rule (int _id, const U& u0, const U& u1, const U& u2);
        Rule (int _id, const U& u0, const U& u1, const U& u2, const U& u3);
    } Rule;
}
