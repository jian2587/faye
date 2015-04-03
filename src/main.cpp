#include <iostream>
#include <string>
#include "parser.h"
using namespace std;

void
displayHelp (const char *argv0)
{
    cout << "Usage: " << argv0 << " <filename>" << endl;
}

int
main (int argc, char **argv)
{
    if (argc < 2) {
        displayHelp (argv[0]);
        exit (0);
    }
    string src_file (argv[1]);
    Parser parser (src_file);
    parser.init ();
    parser.parse ();
    // Parse src_file; get AST
    // execute AST
}

