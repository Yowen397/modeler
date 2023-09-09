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
    q.push(init_s);
    states[init_s->hash()] = init_s;

    static int state_cnt = 0;
    while (!q.empty()) {
        if (debug) {
            cout << "[State No." << state_cnt++ << " ]" << endl;
            cout << q.front()->getStr();
        }
        vector<int> &fireList = getFireable(q.front());
        if (debug) {
            cout << "fireable transition: ";
            for (auto i : fireList)
                cout << cpn->trans[i].name << ", ";
            cout << endl;
        }
        for (auto i : fireList) {
            State *next_s = nextState(q.front(), i);
            if (states.find(next_s->hash())==states.end()) {
                q.push(next_s);
                states[next_s->hash()] = next_s;
            }
        }
        q.pop();
    }
}

inline string tokenString(const string &exp) {
    if (exp.find("control.")!=string ::npos) {
        return "C";
    }
    cerr << "unknown arc expression : [" << exp << "]" << endl;
    exit(-1);
}

inline bool tokenCheck(const string &exp, const string &t);
/**
 * 执行弧表达式，修改token
 */
void StateSpace::executeExp(string &tokens, const string &exp, bool add) {
    int i = 0, j = tokens.find(", ", 0);
    // i---j之间代表一个（或多个相同的）token
    while (j != string::npos) {
        size_t k = tokens.find('`', i);
        string token = tokens.substr(k + 1, j);
        int num = atoi(tokens.substr(i, k).c_str());
        // cout << num << "##" << token << endl;
        if (tokenCheck(exp, token)) {
            // 选择碰到的第一个token执行
            if (!add) {                                                 // 消耗token
                if (num == 1) {
                    tokens = tokens.substr(0, i) + tokens.substr(j);
                    if (tokens.length() == 2)
                        tokens = "";
                    return;
                }
                else {
                    tokens = tokens.substr(0, i) + to_string(num - 1) +
                             tokens.substr(k);
                    return;
                }
            }else {                                                     // 产生token
                tokens += "1`" + tokenString(exp);
            }
        }

        i = j + 2;
        j = tokens.find(", ", i);
    }
}

/**
 * 根据当前状态和变迁，生成下一个状态
*/
State *StateSpace::nextState(State *s, int t) {
    //先复制，再修改token
    State *ret = new State();
    for (auto it: s->tokens)
        ret->tokens[it.first] = it.second;
    // 修改
    for (auto j : cpn->trans[t].pre) {
        string arc_exp =
            cpn->getArc(cpn->places[j].name, cpn->trans[t].name).name;
        
        
        auto it = ret->tokens.find(cpn->places[j].name);
        executeExp(it->second, arc_exp, false);
        if (it->second == "")
            ret->tokens.erase(it);
    }
    for (auto j : cpn->trans[t].pos) {
        string arc_exp =
            cpn->getArc(cpn->trans[t].name, cpn->places[j].name).name;

        auto it = ret->tokens.find(cpn->places[j].name);
        if (it == ret->tokens.end()) 
            ret->tokens[cpn->places[j].name] = "";
        it = ret->tokens.find(cpn->places[j].name);
        executeExp(it->second, arc_exp, true);
    }
    return ret;
}

inline bool tokenCheck(const string &exp, const string &t) {
    if (exp.find("control.")!=string ::npos) {
        if (t.find("C")!=string::npos)
            return true;
    }
    cerr << "unknown arc expression : [" << exp << "]" << endl;
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
        string token = tokens.substr(k + 1, j);
        int num = atoi(tokens.substr(i, k).c_str());
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