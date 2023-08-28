#include <iostream>
#include <unistd.h>


#include "AST.h"
#include "CPN.h"
#include "common.h"

const bool print_AST = true;
std::string path_ast = "error file name";


int main(int argc, char* argv[]){
    parse_arg(argc, argv);

    AST ast; 
    ast.parse(path_ast);
    ast.traverse(print_AST);
    ast.info();

    CPN cpn(ast);
    // CPN cpn;
    cpn.build();
    cpn.info();
    cpn.draw();

    return 0;
}