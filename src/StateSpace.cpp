#include "StateSpace.h"

#include <sstream>
#include <regex>

using std::cerr, std::cout, std::endl;
using namespace EXP;

extern bool debug;

inline bool isConstNum(const std::string& exp_);

std::string State::getStr(const CPN*_cpn) const {
    std::string ret;
    // for (const auto &t: tokens)
    //     ret += t.first + ": \t" + t.second + "\n";
    for (auto p : _cpn->places) {
        auto it = tokens.find(p.name);
        if (it != tokens.end())
            ret += p.name + ":" + it->second + "\n";
    }
    return ret;
}


size_t State::hash(const CPN*_cpn) const {
    static std::hash<std::string> hasher;

    std::string str;
    for (auto p : _cpn->places) {
        auto it = tokens.find(p.name);
        if (it != tokens.end())
            str += p.name + ":" + it->second + "\n";
    }
    size_t h_v = hasher(str);
    return h_v;
}

bool State::isCommonState(CPN*_cpn) const {
    // this->tokens["P.init.c"];
    const std::string pName = _cpn->getPlaceByMatch("P.init.c").name;
    auto it = this->tokens.find(pName);

    if (it != this->tokens.end()) {
        if (it->second != "")
            return true;
    }
    return false;
}

StateSpace::StateSpace(CPN* cpn_) {
    this->cpn = cpn_;
}

void StateSpace::generate(State *init_s) {
    std::queue<State *> q;
    if (states.find(init_s->hash(cpn)) == states.end())
        q.push(init_s);
    states[init_s->hash(cpn)] = init_s;

    static int state_cnt = 0;
    static int memStateCnt = 0;
    while (!q.empty()) {
        std::vector<Binding> &bindList = getBinding(q.front());
        state_cnt++;
        if (q.front()->isCommonState(cpn))
            memStateCnt++;
        if (state_cnt % 10000 == 0 && !debug)
            cout << "[State No." << state_cnt << " Processing]" << endl;
        else if (debug) {
            cout << "[State No." << state_cnt << " Processing]" << endl;
            cout << q.front()->getStr(cpn);
            std::string tmp = "  Fireable Transition: ";
            for (auto i : bindList)
                tmp += cpn->trans[i.t_idx].name + ", ";
            cout << BLUE << tmp << RESET << endl
                 << endl;
        }
        for (auto i : bindList) {
            // 生成下一个状态，并加入状态StateSpace.states
            State* next_s = getNextState(q.front(), i);
            if (next_s == nullptr) {
                delete next_s;
            } else {
                // 生成成功，则加入队列、哈希表
                q.push(next_s);
                if (next_s->isCommonState(cpn))
                    states[next_s->hash(cpn)] = next_s;
                // 维护前继和后继关系
                q.front()->subsequentNodes.emplace_back(next_s);
                next_s->predecessorNodes.emplace_back(q.front());
                lastState = next_s;  // 这一句是历史遗留问题，能跑就别动
            }
            // cout << "check point" << endl;
        }
        q.pop();
    }

    cout << "Total state num [" << state_cnt << "]" << endl;
    cout << "Total repeat num [" << repeat << "]" << endl;
    cout << "Total mem state num [" << memStateCnt << "]" << endl;
}

/**
 * 根据弧表达式生成
*/
// std::string StateSpace::tokenString(const std::string &exp) {
//     if (exp.find("1`().")!=std::string ::npos) {
//         return "C";
//     }
//     if (exp.find("replace.")!=std::string::npos) {
//         std::string token;
//         auto it = consume.find(cur_place);
//         if (it == consume.end()) {
//             cerr << "tokenString::place[" << cur_place << "] consume nothing."
//                  << endl;
//             exit(-1);
//         }
//         token = it->second;
//         return token;
//     }
//     if (exp.find("assign.") != std::string::npos && cur_tran.find("Assignment.") != std::string::npos) {
//         // assign类型只需找到read那一个不属于C类型的token即可
//         cur_State->tokens[cur_place] = "";
//         for (auto &it : consume)
//             if (it.first.find(".c.") == std::string::npos)
//                 return it.second; // 非控制库所，返回
//     }
//     if (exp.find("write.")!=std::string::npos && cur_tran.find("Return.")!=std::string::npos) {
//         // return
//         // write类型需要清空当前库所
//         for (auto &it : consume)
//             if (it.first.find(".c.") == std::string::npos)
//                 return it.second; // 非控制库所，返回
//     }
//     if (exp.find("write.")!=std::string::npos && cur_tran.find("+.") != std::string::npos) {
//         // +
//         std::string ret;
//         for (auto &it : consume)
//             if (it.first.find(".c.") == std::string::npos)
//                 ret = to_string(atoi(ret.c_str()) + atoi(it.second.c_str()));
//         return ret;
//     }
//     cerr << "tokenString::unknown arc expression : [" << exp << "], ";
//     cerr << "transition name is [" << cur_tran << "]" << endl;
//     exit(-1);
// }

/**
 * 执行弧表达式，修改token，仅处理消耗token的部分
 */
// void StateSpace::executeExp(std::string &tokens, const std::string &exp) {
//     int i = 0, j = tokens.find(", ", 0);
//     // i---j之间代表一个（或多个相同的）token
//     while (j != std::string::npos) {
//         size_t k = tokens.find('`', i);
//         std::string token = tokens.substr(k + 1, j - k - 1);
//         int num = atoi(tokens.substr(i, k - i).c_str());
//         // cout << num << "##" << token << endl;
//         if (tokenCheck(exp, token)) {
//             // 选择碰到的第一个token执行
//             // 消耗token
//             if (num == 1) {
//                 tokens = tokens.substr(0, i) + tokens.substr(j);
//                 if (tokens.length() == 2)
//                     tokens = "";
//             } else {
//                 tokens =
//                     tokens.substr(0, i) + to_string(num - 1) + tokens.substr(k);   
//             }
//             consume[cur_place] = token; // 记录消耗掉的token
//             return;
//         }
//         i = j + 2;
//         j = tokens.find(", ", i);
//     }
// }

/**
 * 根据当前状态和变迁，生成下一个状态
*/
// State *StateSpace::nextState(State *s, int t) {
//     cur_tran = cpn->trans[t].name;
//     consume.clear();
//     // 先复制，再修改token
//     State *ret = new State();
//     ret->tokens = s->tokens;
//     cur_State = ret;
//     // for (auto it : s->tokens)
//     //     ret->tokens[it.first] = it.second;
//     // 修改，消耗
//     for (auto j : cpn->trans[t].pre) {
//         std::string arc_exp =
//             cpn->getArc(cpn->places[j].name, cpn->trans[t].name).name;

//         cur_place = cpn->places[j].name;
//         auto it = ret->tokens.find(cur_place);
//         executeExp(it->second, arc_exp);
//         if (it->second == "")
//             ret->tokens.erase(it);
//     }
//     // 修改，输出
//     for (auto j : cpn->trans[t].pos) {
//         std::string arc_exp =
//             cpn->getArc(cpn->trans[t].name, cpn->places[j].name).name;

//         cur_place = cpn->places[j].name;
//         auto it = ret->tokens.find(cur_place);
//         if (it == ret->tokens.end()) 
//             ret->tokens[cpn->places[j].name] = "";
//         it = ret->tokens.find(cpn->places[j].name);
//         // executeExp(it->second, arc_exp, true);
//         if (arc_exp.find("write.")!=std::string::npos)
//             it->second = "1`" + tokenString(arc_exp) + ", ";
//         else
//             it->second += "1`" + tokenString(arc_exp) + ", ";

//     }
//     cur_place = "error:cur_place";
//     cur_tran = "error:cur_tran";
//     return ret;
// }

/**
 * 检查给定的token是否满足条件表达式exp
*/
// bool StateSpace::tokenCheck(const std::string &exp, const std::string &t) {
//     if (exp.find("1`().") != std::string ::npos) {
//         if (t.find("C") != std::string::npos)
//             return true;
//     }
//     if (exp.find("x.") != std::string::npos || exp.find("y.")!= std::string::npos) {
//         // 只是读取token，一定能读取到
//         // read-->x，读取弧变成变量绑定弧。y也是绑定变量
//         return true;
//     }
//     cerr << "tokenCheck::unknown arc expression : [" << exp << "]" << endl;
//     exit(-1);
// }

/**
 * 判断tokens满足弧表达式
*/
// bool StateSpace::satisfyExp(const std::string &exp, const std::string &tokens) {
//     int i = 0, j = tokens.find(", ", 0);
//     // i---j之间代表一个（或多个相同的）token
//     while (j != std::string::npos) {
//         size_t k = tokens.find('`', i);
//         std::string token = tokens.substr(k + 1, j - k - 1);
//         int num = atoi(tokens.substr(i, k - i).c_str());
//         // cout << num << "##" << token << endl;
//         if (tokenCheck(exp, token))
//             return true;

//         i = j + 2;
//         j = tokens.find(", ", i);
//     }
//     return false;
// }

/**
 * 判断变迁可发生
*/
// bool StateSpace::isFireable(State *s, int t) {
//     for (auto j : cpn->trans[t].pre) {
//         std::string arc_exp =
//             cpn->getArc(cpn->places[j].name, cpn->trans[t].name).name;
        
        
//         auto it = s->tokens.find(cpn->places[j].name);
//         if (it == s->tokens.end())
//             return false;  // 没有token直接不可发生
//         // cout << arc_exp << "->>";
//         // cout << it->second << endl;
//         if (!satisfyExp(arc_exp, it->second))
//             return false;
//     }
//     return true;
// }

// vector<int> &StateSpace::getFireable(State *s) {
//     static vector<int> list;
//     list.clear();

//     // 可能发生的变迁(前集库所中含有token)
//     set<int> uncheck_trans;
//     for (auto &p : s->tokens) {
//         // cout << p.first << ":" << p.second << endl;
//         auto &place = cpn->getPlace(p.first);
//         for (auto t:place.pos)
//             uncheck_trans.insert(t);
//     }

//     // 可以发生的变迁
//     for (auto i: uncheck_trans) {
//         // cout << t << ", " << cpn->trans[t].name << endl;
//         if (isFireable(s, i))
//             list.emplace_back(i);
//     }

//     return list;
// }

std::vector<Binding> &StateSpace::getBinding(State *s) {
    static std::vector<Binding> list;  // 设计为static
    list.clear();

    // 可能发生的变迁(前集【控制流】库所中含有token)
    std::set<int> uncheck_trans;
#ifndef USE_TOKENS
    for (auto &p : s->tokens) {
        // cout << p.first << ":" << p.second << endl;
        auto &place = cpn->getPlace(p.first);
        if (!place.isControl)
            continue;           // 限制为，控制流
        if (p.second == "")
            continue;           // 限制为，库所非空
        for (auto t:place.pos)
            uncheck_trans.insert(t);
    }
#else
    for (auto p : cpn->places) {
        if (!p.isControl)
            continue;
        auto it = s->tokens.find(p.name);
        if (it == s->tokens.end())
            continue;
        if (it->second == "")
            continue;
        for (auto t : p.pos)
            uncheck_trans.insert(t);
    }
#endif



    // 查找可以发生的绑定，包括变量
    // 思路：将每种可能的绑定列出来，然后查找它的可行性
    for (auto t: uncheck_trans) {
        bool check = true;
        std::vector<std::vector<std::string>> optional_list;
        std::vector<std::string> place_list;
        // 将每一个库所的变量所有种类都解析出来
        for (auto pre_place : cpn->trans[t].pre) {
            optional_list.emplace_back();
            MultiSet ms;
            parse_MultiSet(ms, s->tokens[cpn->places[pre_place].name]);
            // cout << cpn->places[pre_place].name << endl;
            for (auto t : ms.token)
                optional_list.back().emplace_back(t);
            place_list.emplace_back(cpn->places[pre_place].name);
        }
        // 将提取到的内容验证，并组成绑定
        if (optional_list.size() == place_list.size()) {
            std::vector<int> iters(place_list.size(), 0);
            checkBindings(list, optional_list, place_list, cpn->trans[t].name);
        }
    }

    return list;
}

/* 0计算成功，-1则计算失败已经到末尾 */
inline int nextIterList(std::vector<int>& I_, 
                        const std::vector<std::vector<std::string>>& O_) {
    // 执行思路：各位不同进制的加法

    I_[O_.size() - 1]++;                        // 末位+1
    for (int i = O_.size() - 1; i >= 0; i--) {  // 循环，处理进位
        if (O_[i].size() <= I_[i] && i > 0) {
            I_[i] = 0;                          // 进位
            I_[i - 1]++;
            continue;
        } else if (O_[i].size() > I_[i]) {      // 到这一位不需要继续进位
            break;
        } else if (i == 0) {
            return -1;                          // 进到最高位还需要进位
        }
    }

    return 0;
}

std::unordered_map<std::string, std::string> var_sattisfy;  // '检查阶段'的变量绑定情况

/* 尝试绑定变量与token，如果成功则返回1，失败则返回0 */
inline int try_bind(const std::string &t_, const std::string &v_, std::unordered_map<std::string, std::string>&var_map) {
    // 若不存在则直接绑定成功
    if (var_map.find(v_) == var_map.end()) {
        var_map[v_] = t_;
        return 1;
    }
    // 若存在则检查是否一致，一致也可以“绑定”成功
    if (var_map[v_] == t_)
        return 1;
    // 若存在且不一致，则视为绑定失败
    // cerr << "Variable[" << v_
    //      << "] is already bound to [" << var_map[v_]
    //      << "] and cannot be bound to [" << t_
    //      << "] again" << endl;
    return 0;
}

/**
 * 检查这个token是否满足弧表达式
 * 
 * 注意：1、此时传入参数中token已经分割出数量，此时不能处理多个token的情况
 *      2、默认库所类型是匹配弧表达式类型的，不做多余复杂检查
*/
int StateSpace::satisfyExp_singleExp(const std::string &token, 
                    const std::string &expression) {
    // 空的token直接判定为不满足
    if (token == "")
        return 0;
    // 首先去除弧表达式末尾的   '.2' 、 '.15'
    std::string exp = expression.substr(0, expression.find_last_of('.'));
    // 分类处理弧表达式
    if (exp == "1`()")                                      // 控制弧
        return token == "()" ? 1 : 0;
    if (exp.length() == 1 || exp.length() == 2)             // 单个变量绑定
        return try_bind(token, exp, var_sattisfy);
    if (exp[0] == '(' && exp[exp.length() - 1] == ')') {    // 交类型变量
        if (token[0] != '(' || token[token.length() - 1] != ')')
            return 0;
        int Lt = 1, Rt = token.find_first_of(',', Lt);
        int Lv = 1, Rv = exp.find_first_of(',', Lv);
        while (Rt != std::string::npos && Rv != std::string::npos) {
            std::string t = token.substr(Lt, Rt - Lt);
            std::string v = exp.substr(Lv, Rv - Lv);
            if (!try_bind(t, v, var_sattisfy))
                return 0;
            Lt = Rt + 1;
            Rt = token.find_first_of(',', Lt);
            Lv = Rv + 1;
            Rv = exp.find_first_of(',', Lv);
        }
        return 1;
    }
    if (exp == "True" || exp == "False")
        return exp == token ? 1 : 0;

    cerr << "StateSpace::satisfyExp_singleExp: Unrecognized arc expression [" << expression << "]" << endl;
    exit(-1);
    return -777;
}

/**
 * 检查一个组合是否满足对应的弧表达式 
 * return: 0代表不满足，1代表满足
*/
int StateSpace::satisfyExp_group(const std::vector<int> &iter_list,
                    const std::vector<std::vector<std::string>> &optional_list,
                    const std::vector<std::string> &place_list,
                    const std::string &trans_name) {
    var_sattisfy.clear();
    for (int i = 0; i < place_list.size(); i++) {
        // 如果该弧的前面的库所是空的，那么直接判定为不满足
        if (optional_list[i].size() == 0)
            return 0;
        // 逐个弧检查是否满足
        std::string token = optional_list[i][iter_list[i]];
        std::string arc_exp = cpn->getArc(place_list[i], trans_name).name;

        if (!satisfyExp_singleExp(token, arc_exp))
            return 0;
    }
    return 1;
}

/**
 * 检查token是否可以构成一个绑定，如果可以则加入list
 * 预设前提：弧表达式之间的变量不存在公用关系（例：不存在一个变迁两个弧同时使用x变量）
*/
int StateSpace::checkBindings(std::vector<Binding> &list,
                    const std::vector<std::vector<std::string>> &optional_list,
                    const std::vector<std::string> &place_list,
                    const std::string &trans_name) {
    std::vector<int> iter_list(place_list.size(), 0);
    while (iter_list[0] < place_list.size()) {
        // 验证该组合是否满足弧表达式
        if (satisfyExp_group(iter_list, optional_list, place_list, trans_name)) {
            // 如果满足，则加入list
            Binding& b = list.emplace_back();
            b.t_idx = cpn->getIdxTransition(trans_name);
            for (int i = 0; i < iter_list.size(); i++) {
                b.place.emplace_back(place_list[i]);
                b.token.emplace_back(optional_list[i][iter_list[i]]);
            }
        }

        if (nextIterList(iter_list, optional_list)==-1)
            // 所有组合尝试完毕，可以退出
            return 0;
    }
    return -1;
}

int init_DataPlace(CPN *cpn, State *s) {
    for (auto p: cpn->places) {
        if (p.isControl) 
            continue;  // 控制流库所不需要初始化
        if (s->tokens[p.name].size() != 0)
            continue;  // 已经手动初始化的库所不再重复初始化

        if (p.color.find("uint") != std::string::npos)
            s->tokens[p.name] = "1`0";
        if (p.color == "bool")
            s->tokens[p.name] = "1`False";
    }
#ifdef USE_TOKENS
    s->tokens.shrink_to_fit(cpn);
#endif
    return 0;
}

int parse_MultiSet(MultiSet &ms, const std::string &s) {
    // 已知规定，用++链接，全数字形式
    std::stringstream ss(s);
    int n;
    std::string token;
    char ch = '`';

    while (ss.peek()!=EOF) {
        ss >> n >> ch;                  // 提取数量与字符'`'
        ms.num.emplace_back(n);

        std::getline(ss, token, '+');   // 提取token名称
        ms.token.emplace_back(token);

        ss >> ch;                 // 再读取'++'
    }

    return 0;
}

std::unordered_map<std::string, std::string> var_NextState;  // '生成下一个状态阶段'的变量绑定情况

/**
 * 获取下一个状态，基于cur_和绑定b_生成
 * 如果生成成功则返回指针，如果该状态已存在则返回nullptr
*/
State *StateSpace::getNextState(State *cur_, Binding& b_) {
    State* s = new State();
    var_NextState.clear();
    // 操作顺序：复制、删除消耗掉的token、写入新增的token、删去空库所key值、检查新状态合法性
    // 复制
    s->tokens = cur_->tokens;
    // for (auto p : cur_->tokens)
    //     s->tokens[p.first] = p.second;
    // 删除消耗掉的token，并且将消耗的token与弧表达式绑定
    for (int i = 0; i < b_.place.size();i++) {
        removeToken(s->tokens[b_.place[i]], b_.token[i]);
        bindVar(b_.place[i], b_.t_idx, b_.token[i]);
    }
    // 写入新增的token
    for (int i = 0; i < cpn->trans[b_.t_idx].pos.size();i++) {
        std::string place_name = cpn->places[cpn->trans[b_.t_idx].pos[i]].name;
        addToken(s->tokens[place_name], b_.t_idx, place_name);
    }
    // 删去空库所key值
    std::vector<std::string> del;
#ifndef USE_TOKENS
    for (auto p : s->tokens)
        if (p.second == "")
            del.emplace_back(p.first);
#else
    for (auto p : cpn->places) {
        auto it = s->tokens.find(p.name);
        if (it == s->tokens.end())
            continue;
        if (it->second == "")
            del.emplace_back(it->first);
    }
#endif
    for (auto p : del)
        s->tokens.erase(p);
    // 检查新增状态的合法性
    auto H = s->hash(cpn);
    if (states.find(H) != states.end()) {
        repeat++;
        if (debug)
            cout << "Repeat state, ignored.   Total repeat [" << repeat << "]" << endl;
        return nullptr;
    }
#ifdef USE_TOKENS
    // 合法状态需要重新调整内存
    s->tokens.shrink_to_fit(cpn);
#endif

    return s;
}

// 检查是否有对应的绑定变量
inline void checkExpInVar(const std::string &e_) {
    auto it = var_NextState.find(e_);
    if (it==var_NextState.end()) {
        cerr << "StateSpace::addToken: checkExpInVar failed, unknown var["
             << e_ << "]" << endl;
        exit(-1);
    }
}

// 判断是否为二元操作, +-*/
inline bool isBinaryOp(const std::string &e_) {
    if (e_.find('+') != std::string::npos || e_.find('-') != std::string::npos 
        || e_.find('*') != std::string::npos || e_.find('/') != std::string::npos
        || e_.find('>') != std::string::npos || e_.find('<') != std::string::npos
        || e_.find("==") != std::string::npos)
        return true;
    return false;
}

// 解析二元运算表达式，并且按照运算规则得出结果，修改ms
inline void calcExp_Bin(MultiSet &ms, const std::string &exp) {
    /* 目前二元运算只接受 ‘+’ ‘-’ ‘*’ ‘/’ 这四种(已扩充)，【也仅针对数值】 */
    int i = 0;
    while (exp[i] != '+' && exp[i] != '-' && exp[i] != '*' && exp[i] != '/'
            && exp[i] != '<'&& exp[i] != '>' && exp[i] != '&'&& exp[i] != '|'
            && exp[i] != '=')
        i++;
    std::string op;
    op += exp[i];
    if (exp[i+1]=='='||exp[i+1]=='&'|| exp[i+1]=='|')
        op += exp[i + 1];

    std::string opL = exp.substr(0, i);              // 左操作数
    std::string opR = exp.substr(i + op.length());   // 右操作数
    std::string opL_s = var_NextState[opL], opR_s = var_NextState[opR];
    int opL_i = isConstNum(opL) ? atoi(opL.c_str()) : atoi(opL_s.c_str());
    int opR_i = isConstNum(opR) ? atoi(opR.c_str()) : atoi(opR_s.c_str());
    std::string res;
    if (op == "+")
        res = std::to_string(opL_i + opR_i);
    else if (op == "-")
        res = std::to_string(opL_i - opR_i);
    else if (op == "*")
        res = std::to_string(opL_i * opR_i);
    else if (op == "/")
        res = std::to_string(opL_i / opR_i);
    else if (op == "<")
        res = opL_i < opR_i ? "True" : "False";
    else if (op == ">")
        res = opL_i > opR_i ? "True" : "False";
    else if (op == "==")                            // 区分不同类型的相等判别
        if (opL_s == "True" || opL_s == "False")
            res = opL_s == opR_s ? "True" : "False";
        else if (opL_s.find("0x") != std::string::npos)
            res = opL_s == opR_s ? "True" : "False";
        else
            res = opL_i == opR_i ? "True" : "False";
    else if (op == "<=")
        res = opL_i <= opR_i ? "True" : "False";
    else if (op == ">=")
        res = opL_i >= opR_i ? "True" : "False";

    ms.add(res);
}

// 计算if语句的输出
inline void calcExp_If(MultiSet& ms, const std::string& exp)
{
    // 默认if语句的条件判断是一个二元运算判定，且有if就有else
    // 先左边变量、运算符为=，右边为常量，目前只有这种建模

    // 取出条件判定表达式
    static std::regex re_con("if .* then");
    std::smatch sm_con;
    std::regex_search(exp, sm_con, re_con);
    std::string exp_con = sm_con.str().substr(3, sm_con.length() - 3 - 5);
    // 取出true的输出集合
    static std::regex re_true("then .* else");
    std::smatch sm_true;
    std::regex_search(exp, sm_true, re_true);
    std::string exp_true = sm_true.str().substr(5, sm_true.str().length() - 5 - 5);
    // 取出false的输出集合
    static std::regex re_false("else .*");
    std::smatch sm_false;
    std::regex_search(exp, sm_false, re_false);
    std::string exp_false = sm_false.str().substr(5);

    // 计算if语句判定表达式的值
    std::shared_ptr<EXP::BinExpression> con = std::make_shared<EXP::BinExpression>(exp_con);
    std::string tmp_con = con->result();

    MultiSet tmp_ms;    

    if (tmp_con == "1")
    {
        parse_MultiSet(tmp_ms, exp_true);
        ms += tmp_ms;
    }
    else
    {
        parse_MultiSet(tmp_ms, exp_false);
        ms += tmp_ms;
    }

    return;
}

// 判断一个表达式是否为变量标识符
inline bool isVarIdentifier(const std::string& exp_) {
    if (exp_.length() > 2)  
        return false;       // 变量只接受长度为2最多，字母+数字
    if (exp_.find_first_of("xyzw") == std::string::npos)
        return false;       // 变量起始字符为xyzw中的一个
    return true;
}

// 判断一个表达式是否为常量数值
inline bool isConstNum(const std::string& exp_) {
    static std::regex re("-?\\d+");
    if (std::regex_match(exp_, re))
        return true;
    return false;

    int n = atoi(exp_.c_str());
    if (exp_ == std::to_string(n))
        return true;
    return false;
}

// 判断一个表达式是否为if语句
inline bool isIfStmt(const std::string& exp)
{
    static std::regex re("if .* then .* (else .*)?");
    if (std::regex_match(exp, re))
        return true;
    return false;
}

/**
 * 在库所（字符串all_）中新增token，根据弧（由变迁t_idx_指向库所p_）表达式新增
*/
int StateSpace::addToken(std::string& all_, const int t_idx_, const std::string& p_) {
    Arc& a = cpn->getArc(cpn->trans[t_idx_].name, p_);
    std::string exp = a.name.substr(0, a.name.find_last_of('.'));

    MultiSet ms;
    parse_MultiSet(ms, all_);

    if (isIfStmt(exp)) {
        calcExp_If(ms, exp);
        all_ = ms.str();
        return 0;
    }
    std::regex re_ctrl("1`\\(\\)");                       
    if (std::regex_match(exp, re_ctrl)) {               // 控制弧，输出控制流token
        ms.add("()");
        all_ = ms.str();
        return 0;
    }
    if (exp[0] == '(' && exp[exp.length() - 1] == ')') {// 交类型生成，（先判断交类型，避免进入二元运算）
        std::string token = addToken_tuple(exp);
        ms.add(token);
        all_ = ms.str();
        return 0;
    }
    if (isConstNum(exp)) {                              // 常量
        ms.add(exp);
        all_ = ms.str();
        return 0;
    }
    if (isVarIdentifier(exp)) {                         // 变量，直接从绑定集合找到并输出
        checkExpInVar(exp);
        ms.add(var_NextState[exp]);
        all_ = ms.str();
        return 0;
    }
    if (exp == "True" || exp == "False") {              // Bool类型直接输出
        ms.add(exp);
        all_ = ms.str();
        return 0;
    }
    if (isBinaryOp(exp)) {                              // 二元运算，根据绑定进行运算
        calcExp_Bin(ms, exp);
        all_ = ms.str();
        return 0;
    }

    cerr << "StateSpace::addToken: Unrecognized arc expression [" << exp << "]" << endl;
    exit(-1);
    return -777;
}

/**
 * 根据弧表达式和var_NextState中的情况来生成下一个token的字符串
*/
std::string StateSpace::addToken_tuple(const std::string& exp) {
    // 检查输入
    if (exp[0]!='('||exp[exp.length()-1]!=')') {
        cerr << "StateSpace::addToken_tuple: Input error, input expression is [" << exp << "]" << endl;
        exit(-1);
    }
    std::string ret = "(";
    int pos = 1;
    while (pos < exp.length() - 1) {
        std::string var = exp.substr(pos, exp.find(",", pos) - pos);
        if (isBinaryOp(var)) {
            MultiSet ms;
            calcExp_Bin(ms, var);
            // 计算过后确定只有一个token
            if (ms.num.size()!=1 || ms.num[0]!=1) {
                cerr << "StateSpace::addToken_tuple: calc multi set failed" << endl;
                exit(-1);
            }
            ret += ms.token[0] + ",";
        } else {
            if (var_NextState.find(var) == var_NextState.end()) {
                cerr << "StateSpace::addToken_tuple: Unknown var name [" << var << "]" << endl;
                exit(-1);
            }
            ret += var_NextState[var] + ",";
        }

        pos = exp.find(",", pos + 1) + 1;
    }
    ret += ")";
    return ret;
}

/**
 * 将‘生成阶段’的变量与token绑定记录
 * p_库所名、t_idx_变迁下标、t_是token的字符串
*/
int StateSpace::bindVar(const std::string& p_, const int t_idx_, const std::string& t_) {
    /*  在这个阶段不需要考虑绑定是否会失败，因为在之前生成可发
    生变迁组合的时候已经验证过一次了 */
    Arc& a = cpn->getArc(p_, cpn->trans[t_idx_].name);
    std::string exp = a.name.substr(0, a.name.find_last_of('.'));
    // 分类绑定的具体操作
    //     注意到这个函数的实现与StateSpace::satisfyExp_singleExp非常
    // 相似，但是有区别，这里的返回值虽然有处理，但是并无实际使用
    if (exp == "1`()")
        return 1;
    if (exp.length() == 1 || exp.length()==2)
        return try_bind(t_, exp, var_NextState);
    if (exp[0] == '(' && exp[exp.length() - 1] == ')') {    
        if (t_[0] != '(' || t_[t_.length() - 1] != ')') {
            cerr << "StateSpace::bindVar: Type mismatch for arc expression[" << exp << "] and token[" << t_ << "]" << endl;
            exit(-1);
        }
        int Lt = 1;
        int Rt = t_.find_first_of(',', Lt);
        int Lv = 1;
        int Rv = exp.find_first_of(',', Lv);
        while (Rt != std::string::npos && Rv != std::string::npos) {
            std::string t = t_.substr(Lt, Rt - Lt);
            std::string v = exp.substr(Lv, Rv - Lv);
            if (!try_bind(t, v, var_NextState)) {
                cerr << "StateSpace::bindVar: Try to bind var[" << v << "] and token-component[" << t << "] failed, var is [" << var_NextState[v] << "]" << endl;
                exit(-1);
            }
            Lt = Rt + 1;
            Rt = t_.find_first_of(',', Lt);
            Lv = Rv + 1;
            Rv = exp.find_first_of(',', Lv);
        }
        return 1;
    }

    return 0;
}

// 从给定的所有token中删掉指定的token，若删除失败则直接报错并退出程序
int StateSpace::removeToken(std::string &all_, const std::string &t_) {
    MultiSet ms;
    parse_MultiSet(ms, all_);

    if (ms.sub(t_) == -1) {
        cerr << "Failed to remove token [" << t_ << "] from multi-set : \n\t" << all_ << endl;
        exit(-1);
    }

    ms.sort();
    all_ = ms.str();

    return 0;
}

/* 向Multi-set中新增指定数量（n_缺省1）的某个token（参数t_） */
int MultiSet::add(const std::string& t_, const int n_) {
    // 新增一定会成功，类型不匹配问题不在此处解决
    // 该函数修改了ms的内容，所以返回之前要sort一次
    for (int i = 0; i < token.size();i++) {
        if (token[i]==t_) {
            num[i] += n_;
            this->sort();
            return 0;
        }
    }
    token.emplace_back(t_);
    num.emplace_back(n_);
    this->sort();
    return 0;
}

/* 从Multi-set中删去指定数量（n_缺省1）的某个token（参数t_），返回值为-1表示失败 */
int MultiSet::sub(const std::string& t_, const int n_) {
    int i = 0;
    bool success = false;
    for (i = 0; i < num.size(); i++) {
        if (token[i] == t_) {
            num[i] -= n_;
            if (num[i] == 0) {
                num.erase(num.begin() + i);
                token.erase(token.begin() + i);
            }
            success = true;
            break;
        }
    }
    if (success == false)
        return -1;
    this->sort();
    return 0;
}

/* 生成Multi-set对应的字符串形式的token，生成过程注意排序保持唯一性 */
std::string MultiSet::str() {
    std::string ret;
    for (int i = 0; i < num.size();i++) {
        ret += std::to_string(num[i]) + '`' + token[i];
        if (i < num.size() - 1)
            ret += "++";
    }
    return ret;
}

/* 对Multi-set中的元素进行排序（升序），冒泡排序，字符串比较 */
void MultiSet::sort() {
    for (int i = 0; i < num.size(); i++)
        for (int j = i + 1; j < num.size(); j++) {
            if (strcmp(token[i].c_str(), token[j].c_str())>0){
                int tmp_i = num[i];
                num[i] = num[j];
                num[j] = tmp_i;

                std::string tmp_s = token[i];
                token[i] = token[j];
                token[j] = tmp_s;
            }
        }
}

MultiSet& MultiSet::operator+=(const MultiSet&b)
{
    for (int i = 0; i < b.num.size();i++) 
    {
        int j;
        for (j = 0; j < this->num.size(); j++)
            if (b.token[i] == this->token[j])
                break;
        
        if (j<this->num.size())
            this->num[j] += b.num[i];
        else 
        {
            this->num.emplace_back(b.num[i]);
            this->token.emplace_back(b.token[i]);
        }
    }
    return *this;
}

/**
 * 根据库所名查找其包含的Token字符串
 * p_       库所名
 * return   一个pair指针，ret->second就是库所内的Token
 */
MarkingP *Tokens::find(const std::string& p_) const{
    static MarkingP tmp_mp; // 临时用来保存查找值的变量
    // 查找是否存在某个库所的值，基于查找速度的原则，顺序为base->V【不能基于速度】
    // 顺序：V->base，原则：基于最新、正确的值
    // 首先从V查找
    for (int i = 0; i < V.size(); i++) {
        if (V[i].first == p_){
            tmp_mp.first = V[i].first;
            tmp_mp.second = V[i].second;
            return &tmp_mp;
        }
    }
    // 其次从base查找
    auto it = base->find(p_);
    if (it!=base->end())    {
        tmp_mp.first = it->first;
        tmp_mp.second = it->second;
        return &tmp_mp;
    }
    // 都没找到
    return _ErrorM;
}

/**
 * 返回空指针，用来代指不存在的库所-token
*/
MarkingP *Tokens::end() const {
    return _ErrorM;
}

/**
 * 根据输入的字符串从当前状态中清除一个库所-token
 */
void Tokens::erase(const std::string& p_) {
    // 先尝试从MarkingALL删除
    auto it = base->find(p_);
    if (it != base->end()) {
        base->erase(it->first);
        return;
    }

    // 如果MarkingALL中没有，那么尝试从MarkingP中删除
    auto it2 = V.begin();
    while (it2->first != p_)
        it2++;
    if (it2 == V.end()) {
        cerr << "Tokens::erase: failed to locate place [" << p_ << "]" << endl;
        exit(-1);
    }
    V.erase(it2);
}

/**
 * 根据给定的MarkingP，从当前状态清除这个库所-token
*/
void Tokens::erase(MarkingP *m_) {
    // 先尝试从MarkingALL删除
    auto it = base->find(m_->first);
    if (it != base->end()) {
        base->erase(it->first);
        return;
    }

    // 如果MarkingALL中没有，那么尝试从MarkingP中删除
    auto it2 = V.begin();
    while (it2->first != m_->first)
        it2++;
    if (it2 == V.end()) {
        cerr << "Tokens::erase: failed to locate place [" << m_->first << "]" << endl;
        exit(-1);
    }
    V.erase(it2);
}

/**
 * 重载‘=’赋值
*/
Tokens &Tokens::operator=(const Tokens &src_) {
    this->base = src_.base;
    this->V = src_.V;
    return *this;
}

/**
 * 重新调整STL容器容量，
 * 实际上是调整内部的L，并且重新判断精简状态和普通状态
*/
void Tokens::shrink_to_fit(CPN *cpn_) {
    V.shrink_to_fit();
    
    std::string P_init = cpn_->getPlaceByMatch("P.init.c.").name;
    // auto mp = this->find(P_init);
    int j;
    for (j = 0; j < V.size(); j++) {
        if (V[j].first == P_init)
            break;
    }
    if (j == V.size())
        return; // 如果在V中没有找到init库所
    // 创建一个新的base，并将现有base与V合并塞入，如果重复则以V为准(修改过)
    MarkingALL *tmp_base = this->base;
    this->base = new MarkingALL;
    *this->base = *tmp_base;
    for (int i = 0; i < V.size(); i++) {
        auto it = base->find(V[i].first);
        if (it != base->end()) {
            (*base)[V[i].first] = V[i].second;  // 重复时以此覆盖
        }else {
            (*base)[V[i].first] = V[i].second;  // 不重复则直接塞入
        }
    }
    V.clear();
    V.shrink_to_fit();              // 合并后清空V
}

/**
 * 重载[]
 * 根据参数库所名，返回对应的token的字符串的引用
*/
std::string& Tokens::operator[](const std::string& p_) {
    if (!base)
        base = new MarkingALL;
    // !!!不可用思路!!! 基于速度考虑，先检查是否已有，再考虑是否插入【这个是不行的，不能修改基础的MarkingALL】

    // 直接在V中查找是否有，有则塞进去，若没有也是一次全新的引用值，本质是修改行为，应该记录在V中，最后合并时以V为准
    for (int i = 0; i < V.size(); i++) {
        if (V[i].first == p_)
            return V[i].second;
    }
    auto it = base->find(p_);
    if (it != base->end()) {
        V.emplace_back(MarkingP(it->first, it->second));
        return V[V.size() - 1].second;
    }
    // 不存在则插入空token
    V.emplace_back(MarkingP(p_, ""));
    return V[V.size() - 1].second;

}
