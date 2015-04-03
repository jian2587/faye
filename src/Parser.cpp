#include <iostream>
#include <fstream>
#include <string>
#include "Parser.h"

using namespace std;
using namespace production;

Rule rule1(1, U(IF), U(1));

Parser::Parser (const string& src_file_path)
    : tokenizer (src_file_path)
{
    this->src_file_path = src_file_path;
}

bool
Parser::init ()
{
    return this->tokenizer.init ();
}

void
Parser::parse ()
{
    Token *tkn;
    cout << "Parsing " << src_file_path << endl;
    while ((tkn = tokenizer.getNextToken ()) != NULL) {
        if (tkn->token_type == ENDOFFILE) {
            break;
        }
        cout << tkn->token_type;
        if (tkn->token_type == IDENTIFIER) {
            cout << " " << *tkn->literal.id;
        }
        cout << endl;
    }
}

Parser::~Parser ()
{

}

Rule::Rule (int _id, const U& u0)
    : id (_id)
{
    refs.push_back (u0);
}

Rule::Rule (int _id, const U& u0, const U& u1)
{
    Rule (_id, u0);
    refs.push_back (u1);
}

Rule::Rule (int _id, const U& u0, const U& u1, const U& u2)
{
    Rule (_id, u0, u1);
    refs.push_back (u2);
}

Rule::Rule (int _id, const U& u0, const U& u1, const U& u2, const U& u3)
{
    Rule (_id, u0, u1, u2);
    refs.push_back (u3);
}
