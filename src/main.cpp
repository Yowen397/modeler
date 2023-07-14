#include <iostream>
#include <unistd.h>


#include "AST.h"

const bool print_AST = true;

int main(int argc, char* argv[]){

    AST ast;
    ast.parse(path_ast);
    ast.traverse(print_AST);

    return 0;
}
