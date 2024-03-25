#include <iostream>
#include <unistd.h>


#include "AST.h"
#include "CPN.h"
#include "StateSpace.h"
#include "common.h"

const bool print_AST = true;
std::string path_ast = "error file name";
std::vector<Timer> timer;

void test_Storage(StateSpace &sp);
void test_Storage2(StateSpace &sp);

int main(int argc, char* argv[]){
    timer.emplace_back("program begin");
    
    parse_arg(argc, argv);
    
    timer.emplace_back("before parse ast");
    
    AST ast;
    ast.parse(path_ast);
    ast.traverse(print_AST);
    ast.info();

    timer.emplace_back("ast done, next build cpn");

    CPN cpn(ast);
    cpn.build();
    cpn.info();
    cpn.draw();

    timer.emplace_back("cpn done, next state space");

    StateSpace sp(&cpn);
    test_Storage2(sp);
    // test_Storage(sp);

    timer.emplace_back("state space done");
    
    
    Timer::outputTime(timer);
    VmPeak();

    return 0;
}

void test_Storage2(StateSpace &sp) {
    using namespace std;
    State *s = new State();
    // 初始状态
    s->tokens[sp.cpn->getPlaceByMatch("P.init.c").name] = "1`()";
    s->tokens[sp.cpn->getPlaceByMatch("retrieve.pcall").name] = "2`(3,4,)++1`(7,8,)";
    s->tokens[sp.cpn->getPlaceByMatch("store.pcall").name] = "2`(5,)++1`(9,)";
    // 变量初值
    init_DataPlace(sp.cpn, s);
    cout << s->getStr() << endl;
    sp.generate(s);

    return;
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

    State *s2 = sp.getLastState();
    string p_st_n2 = sp.cpn->getPlaceByMatch("retrieve.start.c.").name;
    s2->tokens[p_st_n2] = "1`C, ";
    string p_param_p1 = sp.cpn->getPlaceByMatch("retrieve.param.p1").name;
    string p_param_p2 = sp.cpn->getPlaceByMatch("retrieve.param.p2").name;
    s2->tokens[p_param_p1] = "1`21, ";
    s2->tokens[p_param_p2] = "1`22, ";
    sp.generate(s2);

    s = sp.getLastState();
    s->tokens[p_st_n] = "1`C, ";
    s->tokens[p_param_n] = "1`7, ";  // 初始状态
    sp.generate(s);

    s2 = sp.getLastState();
    s2->tokens[p_st_n2] = "1`C, ";
    s2->tokens[p_param_p1] = "1`23, ";
    s2->tokens[p_param_p2] = "1`24, ";
    sp.generate(s2);

    s2 = sp.getLastState();
    s2->tokens[p_st_n2] = "1`C, ";
    s2->tokens[p_param_p1] = "1`25, ";
    s2->tokens[p_param_p2] = "1`26, ";
    sp.generate(s2);

    s = sp.getLastState();
    s->tokens[p_st_n] = "1`C, ";
    s->tokens[p_param_n] = "1`9, ";  // 初始状态
    sp.generate(s);

    // for (int i = 1; i <= 3000 ; i++) {
    //     if (i%2==1) {
    //         s2 = sp.getLastState();
    //         s2->tokens[p_st_n2] = "1`C, ";
    //         s2->tokens[p_param_p1] = "1`3, ";
    //         s2->tokens[p_param_p2] = "1`4, ";
    //         sp.generate(s2);
    //     }
    //     else {
    //         s = sp.getLastState();
    //         s->tokens[p_st_n] = "1`C, ";
    //         s->tokens[p_param_n] = "1`5, ";
    //         sp.generate(s);
    //     }
    // }

    return;
}