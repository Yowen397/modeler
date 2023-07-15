#pragma once

#include "AST.h"

#include <vector>


/**
 * 注意：此目录下的CPN并不是用来进行验证计算的CPN，只是用来存储的CPN，后续使用的验证计算器
 * 所需要的CPN从这里输出
 */

class Place {
  protected:
    std::string name;
    std::string color;

    std::string init_tokens;
};

class Transition {
  protected:
    std::string name;
};

class Arc {
  protected:
    std::string st;
    std::string ed;
};

class CPN {
  protected:
    std::vector<Place> places;
    std::vector<Transition> trans;
    std::vector<Arc> arcs;

  public:
    CPN(AST &ast_);
    virtual ~CPN();

  private:
    rapidjson::Value *root;
    std::vector<SC_VAR> &vars;
    std::vector<SC_FUN> &funs;

  public:
    // int traverse();
};