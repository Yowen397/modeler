#include <iostream>
#include <unistd.h>


#include "AST.h"
#include "CPN.h"
#include "StateSpace.h"
#include "common.h"

const bool print_AST = true;
bool debug = true;
std::string path_ast = "error file name";
std::string path_ini = "error file name";
std::unordered_map<std::string, std::string> ini_info;
std::vector<Timer> timer;

void test4ini(StateSpace& sp);
void test_Storage(StateSpace& sp);
void test_Purchase(StateSpace& sp);
void test_Timelock(StateSpace& sp);
void test_SafeMath(StateSpace& sp);
void test_integer_overflow_multitx_onefunc_feasible(StateSpace& sp);

int main(int argc, char* argv[]){
    timer.emplace_back("program begin");
    
    parse_arg(argc, argv);
    parse_ini(ini_info, path_ini);

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
    test4ini(sp);
    // if (path_ast.find("storage") != std::string::npos)
    //     test_Storage(sp);
    // // test_Storage(sp);
    // if (path_ast.find("Purchase") != std::string::npos)
    //     test_Purchase(sp);
    // if (path_ast.find("Timelock") != std::string::npos)
    //     test_Timelock(sp);
    // if (path_ast.find("SafeMath") != std::string::npos)
    //     test_SafeMath(sp);
    // if (path_ast.find("integer_overflow_multitx_onefunc_feasible")!=std::string::npos)
    //     test_integer_overflow_multitx_onefunc_feasible(sp);

    timer.emplace_back("state space done");

    std::cout << YELLOW << "AST Filename: [" << path_ast << "]" << RESET << std::endl;
    std::cout << YELLOW << "ini Filename: [" << path_ini << "]" << RESET << std::endl;
    Timer::outputTime(timer);
    VmPeak();

    return 0;
}

void test4ini(StateSpace& sp)
{
    State* s = new State();
    for (auto kv : ini_info)
        s->tokens[sp.cpn->getPlaceByMatch(kv.first).name] = kv.second;

    init_DataPlace(sp.cpn, s);
    std::cout << s->getStr(sp.cpn) << std::endl;

    sp.generate(s);

    return;
}

void test_Storage(StateSpace &sp) {
    using namespace std;
    State *s = new State();
    // 初始状态
    s->tokens[sp.cpn->getPlaceByMatch("P.init.c").name] = "1`()";
    s->tokens[sp.cpn->getPlaceByMatch("global.this").name] = "1`0";
    s->tokens[sp.cpn->getPlaceByMatch("global.msg").name] = "1`(0x0000,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("global.ALLUSERS").name] = "1`(0x000A,100,)";

    s->tokens[sp.cpn->getPlaceByMatch("retrieve.pcall").name] = "2`(3,4,0x000A,0,)++2`(7,8,0x000A,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("store.pcall").name] = "2`(5,0x000A,0,)++1`(9,0x000A,0,)";
    // s->tokens[sp.cpn->getPlaceByMatch("retrieve.pcall").name] = "2`(3,4,0x000A,0,)++1`(7,8,0x000A,0,)++9`(13,17,0x000A,0,)";
    // s->tokens[sp.cpn->getPlaceByMatch("store.pcall").name] = "2`(5,0x000A,0,)++2`(9,0x000A,0,)++8`(19,0x000A,0,)";
    // 变量初值
    init_DataPlace(sp.cpn, s);
    cout << s->getStr(sp.cpn) << endl;
    sp.generate(s);

    return;
}

void test_Purchase(StateSpace &sp) {
    State* s = new State();
    // 初始状态
    s->tokens[sp.cpn->getPlaceByMatch("P.init.c").name] = "1`()";
    s->tokens[sp.cpn->getPlaceByMatch("global.value").name] = "1`10";
    s->tokens[sp.cpn->getPlaceByMatch("global.seller").name] = "1`0x00AA";
    s->tokens[sp.cpn->getPlaceByMatch("global.buyer").name] = "1`0x00BB";
    s->tokens[sp.cpn->getPlaceByMatch("global.state").name] = "1`0";
    s->tokens[sp.cpn->getPlaceByMatch("global.this").name] = "1`20";
    s->tokens[sp.cpn->getPlaceByMatch("global.msg").name] = "1`(0x0000,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("global.ALLUSERS").name] = "1`(0x00AA,100,)++1`(0x00BB,200,)";

    s->tokens[sp.cpn->getPlaceByMatch("abort.pcall").name] = "1`(0x00AA,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("confirmPurchase.pcall").name] = "1`(0x00BB,20,)";
    s->tokens[sp.cpn->getPlaceByMatch("confirmReceived.pcall").name] = "1`(0x00BB,00,)";
    s->tokens[sp.cpn->getPlaceByMatch("refundSeller.pcall").name] = "1`(0x00AA,00,)";
    // s->tokens[sp.cpn->getPlaceByMatch("abort.pcall").name] = "2`(0x00AA,0,)";
    // s->tokens[sp.cpn->getPlaceByMatch("confirmPurchase.pcall").name] = "2`(0x00BB,20,)";
    // s->tokens[sp.cpn->getPlaceByMatch("confirmReceived.pcall").name] = "2`(0x00BB,00,)";
    // s->tokens[sp.cpn->getPlaceByMatch("refundSeller.pcall").name] = "2`(0x00AA,00,)";

    init_DataPlace(sp.cpn, s);
    std::cout << s->getStr(sp.cpn) << std::endl;
    sp.generate(s);

    return;
}

void test_Timelock(StateSpace &sp) {
    State* s = new State();
    // 初始状态

    init_DataPlace(sp.cpn, s);
    std::cout << s->getStr(sp.cpn) << std::endl;
    sp.generate(s);
    
    return;
}

void test_SafeMath(StateSpace &sp) {
    State* s = new State();
    // 初始状态
    s->tokens[sp.cpn->getPlaceByMatch("P.init.c").name] = "1`()";
    s->tokens[sp.cpn->getPlaceByMatch("global.this").name] = "1`20";
    s->tokens[sp.cpn->getPlaceByMatch("global.msg").name] = "1`(0x0000,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("global.ALLUSERS").name] = "1`(0x000A,100,)";

    s->tokens[sp.cpn->getPlaceByMatch("add.pcall").name] = "3`(23,29,0x000A,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("sub.pcall").name] = "1`(10,7,0x000A,0,)";
    // s->tokens[sp.cpn->getPlaceByMatch("add.pcall").name] = "1`(23,29,0x000A,0,)++4`(1,5,0x000A,0,)++10`(7,123,0x000A,0,)";
    // s->tokens[sp.cpn->getPlaceByMatch("sub.pcall").name] = "1`(7,10,0x000A,0,)++10`(3,2,0x000A,0,)++7`(56,55,0x000A,0,)";

    init_DataPlace(sp.cpn, s);
    std::cout << s->getStr(sp.cpn) << std::endl;
    sp.generate(s);
    
    return;
}

void test_integer_overflow_multitx_onefunc_feasible(StateSpace& sp)
{
    State* s = new State();
    // std::shared_ptr<State> s = std::make_shared<State>();
    s->tokens[sp.cpn->getPlaceByMatch("P.init.c").name] = "1`()";
    s->tokens[sp.cpn->getPlaceByMatch("global.this").name] = "1`20";
    s->tokens[sp.cpn->getPlaceByMatch("global.msg").name] = "1`(0x0000,0,)";
    s->tokens[sp.cpn->getPlaceByMatch("global.ALLUSERS").name] = "1`(0x000A,100,)";

    s->tokens[sp.cpn->getPlaceByMatch("run.pcall").name] = "1`(5,0x000A,0,)++1`(3,0x000A,0,)";

    init_DataPlace(sp.cpn, s);
    std::cout << s->getStr(sp.cpn) << std::endl;
    sp.generate(s);

    return;
}