#include "StateSpace.h"
#include <regex>
#include <map>

using namespace EXP;

extern std::unordered_map<std::string, std::string> var_NextState; // '生成下一个状态阶段'的变量绑定情况

std::string Expression::result()
{
    return this->str;
}

void BinExpression::parse() {
    if (!parsed)
        parsed = true;
    else
        return;
        
    static std::regex re_opcode("\\+|\\-|\\*|/|<|>|==|<=|>=|==");
    // 变量|常量,注意此处正则表达式需要先匹配变量，否则true、false会被识别为变量
    static std::regex re_oprand("[a-zA-Z]\\w*|\\d*|True|False");

    std::smatch sm;
    std::string::const_iterator iter_st = str.begin();
    std::string::const_iterator iter_ed = str.end();

    std::regex_search(iter_st, iter_ed, sm, re_oprand);
    oprand.emplace_back(sm[0]);
    iter_st = sm[0].second;
    while (true) {
        if (!std::regex_search(iter_st, iter_ed, sm, re_opcode)) 
            break;
        opcode.emplace_back(sm[0]);
        iter_st = sm[0].second;

        if (!std::regex_search(iter_st, iter_ed, sm, re_oprand))
            break;
        oprand.emplace_back(sm[0]);
        iter_st = sm[0].second;
    }

    return;
}

std::string BinExpression::result()
{
    parse();

    int p = 0, q = 1;
    std::string tmp = oprand[0];
    while (q <= oprand.size() - 1) {
        tmp = calc(tmp, opcode[p], oprand[q]);
        p++, q++;
    }

    return tmp;
}

std::string EXP::calc(const Oprand& op1, const std::string& opcode, const Oprand& op2)
{
    std::map<std::string, std::function<NUM(NUM, NUM)>> m = {
        { "==", [](const NUM a, const NUM b) -> NUM { return a == b; } },
        { "<=", [](const NUM a, const NUM b) -> NUM { return a <= b; } },
        { ">=", [](const NUM a, const NUM b) -> NUM { return a >= b; } },
        { ">", [](const NUM a, const NUM b) -> NUM { return a > b; } },
        { "<", [](const NUM a, const NUM b) -> NUM { return a < b; } },
        { "+", [](const NUM a, const NUM b) -> NUM { return a + b; } },
        { "-", [](const NUM a, const NUM b) -> NUM { return a - b; } },
        { "*", [](const NUM a, const NUM b) -> NUM { return a * b; } },
        { "/", [](const NUM a, const NUM b) -> NUM { return a / b; } },
    };

    std::string res = std::to_string(m[opcode](op1.get_value(), op2.get_value()));
    return res;
}

Oprand::Oprand(const std::string& s_)
    : str(s_)
{
    std::string s = str;
    if (var_NextState.find(str) != var_NextState.end())
        s = var_NextState[str];
    value = INT_MAX;
    value = s == "True" ? 1 : value;
    value = s == "False" ? 0 : value;
    if (std::regex_match(s, std::regex("\\d*")))
        value = std::stoi(s);
    
    if (value == INT_MAX) {
        std::cerr << RED << "Oprand::Oprand: Unknown oprand [" << str << "]" << std::endl;
        exit(-1);
    }
}

namespace ArcExp {

//======函数类的函数操作定义==
#define ARC_EXP_DEF(classname)      \
Result classname::operator()(Token t) const 
//==========================

    ARC_EXP_DEF(ControlFlow)
    {
        /**
         * 控制流应该检查输入，如果满足  ()   则可以接受
         */
        if (t.size() != 1)
            return DISSATISFIED;
        if (t[0] == "()")
            return SATISFIED;
        return DISSATISFIED;
    }
};
