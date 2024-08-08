#include "StateSpace.h"
#include <regex>
#include <map>

using namespace EXP;

extern std::unordered_map<std::string, std::string> var_sattisfy;  // '检查阶段'的变量绑定情况
extern std::unordered_map<std::string, std::string> var_NextState;  // '生成下一个状态阶段'的变量绑定情况

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
#define ARC_EXP_DEF(classname) \
    Result classname::operator()(Token t) const
//==========================

ARC_EXP_DEF(ControlFlow)
{
    /**
     * 控制流应该检查输入，如果满足  1`()   则可以接受
     */
    if (t.size() != 1)
        return DISSATISFIED;
    if (t[0] == "1`()")
        return SATISFIED;
    return DISSATISFIED;
}

ARC_EXP_DEF(DataConsumerAnyX)
{
    /**
     * 接受一个任意类型的Token（可能为单类型，可能为交类型）
     * 但是需要对Token重的各个成分分开存
     */
    if (t.size() != 1)
        return DISSATISFIED;

    static const std::string base_ = "x";

    if (t[0][0]=='(') {
        // 交类型
        std::string tmp = t[0].substr(1, t[0].length() - 2);

        for (int i = 1, pos = 0; pos != std::string::npos && pos < tmp.length(); i++) {
            int nxt_pos = tmp.find(',', pos);
            int len = nxt_pos - pos;
            std::string sub_t = tmp.substr(pos, len);
            pos = nxt_pos + 1;

            // 检验是否前面已经消费过这个同名变量
            std::string var = base_ + std::to_string(i);
            if (var_sattisfy.find(var)!=var_sattisfy.end()) {
                if (var_sattisfy[var]!=sub_t)
                    return DISSATISFIED;
            } else {
                var_sattisfy[var] = sub_t;
            }

            var_NextState[var] = sub_t;
        }
    } else {
        // 单类型的Token

        // 检验是否前面已经消费过这个同名变量
        std::string var = base_;
        if (var_sattisfy.find(var) != var_sattisfy.end()) {
            if (var_sattisfy[var] != t[0])
                return DISSATISFIED;
        } else {
            var_sattisfy[var] = t[0];
        }

        var_NextState[base_] = t[0];
    }
    return SATISFIED;
}

ARC_EXP_DEF(DataProducerAny) 
{
    /**
     * 需要根据终点库所类型生成对应的token，匹配可能有x1,x2或者是单个的x
    */
    if (t.size() <= 0)
        return DISSATISFIED;
    
    if (t.size() == 1) {
        if (var_NextState.find(t[0])!=var_NextState.end()) {
            return var_NextState[t[0]];
        } else {
            return DISSATISFIED;
        }
    }

    // 需要从变量空间中取出多个变量的情况
    if (t.size() > 1) {
        std::string ret;
        ret += "(";
        for (int i = 0; i < t.size(); i++) {
            if (var_NextState.find(t[i]) != var_NextState.end()) {
                ret += var_NextState[t[i]];
            } else {
                return DISSATISFIED;
            }
        }
        ret += ")";
        return ret;
    }

    return DISSATISFIED;
}

ARC_EXP_DEF(DataConsumerAnyZ) 
{
    /**
     * 接受一个任意类型的Token（可能为单类型，可能为交类型）
     * 但是需要对Token重的各个成分分开存
     */
    if (t.size() != 1)
        return DISSATISFIED;

    static const std::string base_ = "z";

    if (t[0][0]=='(') {
        // 交类型
        std::string tmp = t[0].substr(1, t[0].length() - 2);

        for (int i = 1, pos = 0; pos != std::string::npos && pos < tmp.length(); i++) {
            int nxt_pos = tmp.find(',', pos);
            int len = nxt_pos - pos;
            std::string sub_t = tmp.substr(pos, len);
            pos = nxt_pos + 1;

            // 检验是否前面已经消费过这个同名变量
            std::string var = base_ + std::to_string(i);
            if (var_sattisfy.find(var)!=var_sattisfy.end()) {
                if (var_sattisfy[var]!=sub_t)
                    return DISSATISFIED;
            } else {
                var_sattisfy[var] = sub_t;
            }

            var_NextState[var] = sub_t;
        }
    } else {
        // 单类型的Token

        // 检验是否前面已经消费过这个同名变量
        std::string var = base_;
        if (var_sattisfy.find(var) != var_sattisfy.end()) {
            if (var_sattisfy[var] != t[0])
                return DISSATISFIED;
        } else {
            var_sattisfy[var] = t[0];
        }

        var_NextState[base_] = t[0];
    }
    return SATISFIED;
}
};
