#include <iostream>
#include <unistd.h>


#include "AST.h"
#include "CPN.h"
#include "StateSpace.h"
#include "common.h"

const bool print_AST = true;
std::string path_ast = "error file name";

void test_Storage(StateSpace &sp);

int main(int argc, char* argv[]){
    parse_arg(argc, argv);

    AST ast; 
    ast.parse(path_ast);
    ast.traverse(print_AST);
    ast.info();

    CPN cpn(ast);
    cpn.build();
    cpn.info();
    // cpn.draw();

    StateSpace sp(&cpn);
    test_Storage(sp);

    return 0;
}

void test_Storage(StateSpace &sp) {
    using namespace std;
    State *s = new State();
    string p_st_n = sp.cpn->getPlaceByMatch("store.start.c.").name;
    s->tokens[p_st_n] = "1`C, ";
    string p_param_n = sp.cpn->getPlaceByMatch("store.param.num").name;
    s->tokens[p_param_n] = "1`5, ";  // 初始状态
    // cout << s->getStr() << endl;

    sp.generate(s);

    return;
}