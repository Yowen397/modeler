#include "StateSpace.h"

using namespace std;

extern bool debug;

string State::getStr() const {
    string ret;
    for (const auto &t: tokens)
        ret += t.first + ": " + t.second + "\n";
    return ret;
}

size_t State::hash() const {
    static std::hash<string> hasher;
    size_t h_v = hasher(getStr());
    return h_v;
}

void StateSpace::generate(State *init_s) {
    queue<State *> q;
    if (states.find(init_s->hash()) == states.end())
        q.push(init_s);
    states[init_s->hash()] = init_s;

    static int state_cnt = 0;
    if (debug) {
        cout << "[State No." << state_cnt++ << " ]" << endl;
        cout << q.front()->getStr() << endl;
    }
    while (!q.empty()) {
        vector<int> &fireList = getFireable(q.front());
        if (debug) {
            cout << "fireable transition: ";
            for (auto i : fireList)
                cout << cpn->trans[i].name << ", ";
            cout << endl;
        }
        for (auto i : fireList) {
            State *next_s = nextState(q.front(), i);
            if (states.find(next_s->hash()) == states.end()) {
                q.push(next_s);
                states[next_s->hash()] = next_s;
                lastState = next_s;
                if (debug) {
                    cout << "[State No." << state_cnt++ << " ]" << endl;
                    cout << q.back()->getStr() << endl;
                }
            }
        }
        q.pop();
    }
}

/**
 * 根据弧表达式生成
*/
string StateSpace::tokenString(const string &exp) {
    if (exp.find("control.")!=string ::npos) {
        return "C";
    }
    if (exp.find("replace.")!=string::npos) {
        string token;
        auto it = consume.find(cur_place);
        if (it == consume.end()) {
            cerr << "tokenString::place[" << cur_place << "] consume nothing."
                 << endl;
            exit(-1);
        }
        token = it->second;
        return token;
    }
    if (exp.find("assign.") != string::npos && cur_tran.find("Assignment.") != string::npos) {
        // assign类型只需找到read那一个不属于C类型的token即可
        cur_State->tokens[cur_place] = "";
        for (auto &it : consume)
            if (it.first.find(".c.") == string::npos)
                return it.second; // 非控制库所，返回
    }
    if (exp.find("write.")!=string::npos && cur_tran.find("Return.")!=string::npos) {
        // return
        // write类型需要清空当前库所
        for (auto &it : consume)
            if (it.first.find(".c.") == string::npos)
                return it.second; // 非控制库所，返回
    }
    if (exp.find("write.")!=string::npos && cur_tran.find("+.") != string::npos) {
        // +
        string ret;
        for (auto &it : consume)
            if (it.first.find(".c.") == string::npos)
                ret = to_string(atoi(ret.c_str()) + atoi(it.second.c_str()));
        return ret;
    }
    cerr << "tokenString::unknown arc expression : [" << exp << "], ";
    cerr << "transition name is [" << cur_tran << "]" << endl;
    exit(-1);
}

/**
 * 执行弧表达式，修改token，仅处理消耗token的部分
 */
void StateSpace::executeExp(string &tokens, const string &exp) {
    int i = 0, j = tokens.find(", ", 0);
    // i---j之间代表一个（或多个相同的）token
    while (j != string::npos) {
        size_t k = tokens.find('`', i);
        string token = tokens.substr(k + 1, j - k - 1);
        int num = atoi(tokens.substr(i, k - i).c_str());
        // cout << num << "##" << token << endl;
        if (tokenCheck(exp, token)) {
            // 选择碰到的第一个token执行
            // 消耗token
            if (num == 1) {
                tokens = tokens.substr(0, i) + tokens.substr(j);
                if (tokens.length() == 2)
                    tokens = "";
            } else {
                tokens =
                    tokens.substr(0, i) + to_string(num - 1) + tokens.substr(k);   
            }
            consume[cur_place] = token; // 记录消耗掉的token
            return;
        }
        i = j + 2;
        j = tokens.find(", ", i);
    }
}

/**
 * 根据当前状态和变迁，生成下一个状态
*/
State *StateSpace::nextState(State *s, int t) {
    cur_tran = cpn->trans[t].name;
    consume.clear();
    // 先复制，再修改token
    State *ret = new State();
    cur_State = ret;
    for (auto it : s->tokens)
        ret->tokens[it.first] = it.second;
    // 修改，消耗
    for (auto j : cpn->trans[t].pre) {
        string arc_exp =
            cpn->getArc(cpn->places[j].name, cpn->trans[t].name).name;

        cur_place = cpn->places[j].name;
        auto it = ret->tokens.find(cur_place);
        executeExp(it->second, arc_exp);
        if (it->second == "")
            ret->tokens.erase(it);
    }
    // 修改，输出
    for (auto j : cpn->trans[t].pos) {
        string arc_exp =
            cpn->getArc(cpn->trans[t].name, cpn->places[j].name).name;

        cur_place = cpn->places[j].name;
        auto it = ret->tokens.find(cur_place);
        if (it == ret->tokens.end()) 
            ret->tokens[cpn->places[j].name] = "";
        it = ret->tokens.find(cpn->places[j].name);
        // executeExp(it->second, arc_exp, true);
        if (arc_exp.find("write.")!=string::npos)
            it->second = "1`" + tokenString(arc_exp) + ", ";
        else
            it->second += "1`" + tokenString(arc_exp) + ", ";

    }
    cur_place = "error:cur_place";
    cur_tran = "error:cur_tran";
    return ret;
}

/**
 * 检查给定的token是否满足条件表达式exp
*/
bool StateSpace::tokenCheck(const string &exp, const string &t) {
    if (exp.find("control.") != string ::npos) {
        if (t.find("C") != string::npos)
            return true;
    }
    if (exp.find("read.") != string::npos) {
        // 只是读取token，一定能读取到
        return true;
    }
    cerr << "tokenCheck::unknown arc expression : [" << exp << "]" << endl;
    exit(-1);
}

/**
 * 判断tokens满足弧表达式
*/
bool StateSpace::satisfyExp(const string &exp, const string &tokens) {
    int i = 0, j = tokens.find(", ", 0);
    // i---j之间代表一个（或多个相同的）token
    while (j != string::npos) {
        size_t k = tokens.find('`', i);
        string token = tokens.substr(k + 1, j - k - 1);
        int num = atoi(tokens.substr(i, k - i).c_str());
        // cout << num << "##" << token << endl;
        if (tokenCheck(exp, token))
            return true;

        i = j + 2;
        j = tokens.find(", ", i);
    }
    return false;
}

/**
 * 判断变迁可发生
*/
bool StateSpace::isFireable(State *s, int t) {
    for (auto j : cpn->trans[t].pre) {
        string arc_exp =
            cpn->getArc(cpn->places[j].name, cpn->trans[t].name).name;
        
        
        auto it = s->tokens.find(cpn->places[j].name);
        if (it == s->tokens.end())
            return false;  // 没有token直接不可发生
        // cout << arc_exp << "->>";
        // cout << it->second << endl;
        if (!satisfyExp(arc_exp, it->second))
            return false;
    }
    return true;
}

vector<int> &StateSpace::getFireable(State *s) {
    static vector<int> list;
    list.clear();

    // 可能发生的变迁(前集库所中含有token)
    set<int> uncheck_trans;
    for (auto &p : s->tokens) {
        // cout << p.first << ":" << p.second << endl;
        auto &place = cpn->getPlace(p.first);
        for (auto t:place.pos)
            uncheck_trans.insert(t);
    }

    // 可以发生的变迁
    for (auto i: uncheck_trans) {
        // cout << t << ", " << cpn->trans[t].name << endl;
        if (isFireable(s, i))
            list.emplace_back(i);
    }

    return list;
}