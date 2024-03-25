#pragma once

#include <functional>
#include <unordered_map>
#include <set>
#include <vector>
#include "CPN.h"

class State {
  public:
    std::unordered_map<std::string, std::string> tokens;

    std::string getStr() const;
    std::size_t hash(const CPN*_cpn) const;

};

class Binding{
  public:
    std::size_t t_idx;                // 变迁下标
    std::vector<std::string> place;   // 变量绑定情况，变量库所名-token
    std::vector<std::string> token;   // 变量绑定情况，变量库所名-token
};

class MultiSet{
  public:
    std::vector<int> num;
    std::vector<std::string> token;

    std::string str();
    void sort();
    int add(const std::string& t_, const int n_ = 1);
    int sub(const std::string& t_, const int n_ = 1);
};
// class Bingdings{
//   public:
//     std::vector<Binding> value;
// };

class StateSpace
{
public:
  CPN *cpn;

  std::unordered_map<size_t, State *> states;

  StateSpace(CPN* cpn_);

  void generate(State *);

protected:
  std::vector<int> &getFireable(State *);
  std::vector<Binding> &getBinding(State *);
private:
  bool isFireable(State *s, int t);  // 旧版，被取代
  bool satisfyExp(const std::string &exp, const std::string &tokens);  // 旧版，被取代
  State *nextState(State *s, int t);  // 旧版，被取代
  void executeExp(std::string &tokens, const std::string &exp);  // 旧版，被取代
  std::string tokenString(const std::string &exp);  // 旧版，被取代
  bool tokenCheck(const std::string &exp, const std::string &token);  // 旧版，被取代
protected:
  int checkBindings(std::vector<Binding> &list,
                    const std::vector<std::vector<std::string>> &optional_list,
                    const std::vector<std::string> &place_list,
                    const std::string &trans_name);

  int satisfyExp_group(const std::vector<int> &iter_list,
                    const std::vector<std::vector<std::string>> &optional_list,
                    const std::vector<std::string> &place_list,
                    const std::string &trans_name);
  int satisfyExp_singleExp(const std::string &token, 
                    const std::string &expression);

  State* getNextState(State* cur_, Binding& b_);
  static int removeToken(std::string &all_, const std::string &t_);
  int bindVar(const std::string& p_, const int t_idx_, const std::string& t_);
  int addToken(std::string& all_, const int t_idx_, const std::string& p_);

  protected:
  /* 一些记录信息用的变量 */
  State *lastState, *cur_State;
  std::string cur_place;
  std::string cur_tran;
  std::map<std::string, std::string> consume; // 消耗的token，库所名对应token字符串

  int repeat = 0;

  public:
  State* getLastState() { return lastState; };
};

/* 初始化变量库所（已手动初始化的库所除外），局部变量、临时变量等，赋予零值 */
int init_DataPlace(CPN *cpn, State *s);

/* 将库所内token的字符串解析为多重集 */
int parse_MultiSet(MultiSet &ms, const std::string &s);