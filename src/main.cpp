#include <iostream>
#include <unistd.h>


#include "AST.h"

int main(int argc, char* argv[]){

    AST ast;
    ast.parse(path_ast);
    ast.traverse();

    return 0;
}
