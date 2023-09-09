#pragma once

#include <functional>
#include <unordered_map>
#include <set>
#include "CPN.h"

class State {
  public:
    std::unordered_map<std::string, std::string> tokens;

    std::string getStr() const;
    std::size_t hash() const;
};

class StateSpace {
  public:
    CPN *cpn;

    std::unordered_map<size_t, State *> states;

    StateSpace(CPN *cpn_) : cpn(cpn_){};

    void generate(State *);

  protected:
    std::vector<int> &getFireable(State *);
    bool isFireable(State *s, int t);
    bool satisfyExp(const std::string &exp, const std::string &tokens);
    State *nextState(State *s, int t);
    void executeExp(std::string &tokens, const std::string &exp);
};
