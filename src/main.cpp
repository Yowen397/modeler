#include <iostream>
#include <unistd.h>


#include "AST.h"
#include "CPN.h"

const bool print_AST = true;

int main(int argc, char* argv[]){

    AST ast;
    ast.parse(path_ast);
    ast.traverse(print_AST);
    ast.info();

    CPN cpn(ast);
    // CPN cpn;

    return 0;
}
