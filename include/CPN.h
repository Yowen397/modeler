#pragma once

#include "AST.h"

#include <vector>


/**
 * 注意：此目录下的CPN并不是用来进行验证计算的CPN，只是用来存储的CPN，后续使用的验证计算器
 * 所需要的CPN从这里输出
 */

class Place {
  protected:

  public:
    std::string name;
    std::string color;
    std::string init_tokens;

    Place();
    virtual ~Place();
    std::string getStr();
};

class Transition {
  protected:
    bool isSubNet = false;
  public:
    std::string name;


    bool getSubNet();
    void setSubNet(bool v_);
    std::string getStr();
};

class Arc {
  public:
    std::string st;
    std::string ed;
    std::string dir;  // "t2p" or "p2t"

    std::string getStr();
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
    int build();      // 构建CPN的核心函数
    int info();

  protected:
    int traverse(const rapidjson::Value *node_);    // 遍历，深搜，前后序，构建CPN

    int build_topNet();

    /* 构造中通用函数、变量 */
    int e_Unkonwn(const std::string &type_, const rapidjson::Value *node_,
                  bool pr = true);

    /* pr_ 开头为前序遍历用到的函数 */
    int pr_selector(const std::string &type_, const rapidjson::Value *node_);
    int pr_SourceUnit(const rapidjson::Value *node_);
    int pr_PragmaDirective(const rapidjson::Value *node_);
    int pr_ContractDefinition(const rapidjson::Value *node_);
    int pr_StructuredDocumentation(const rapidjson::Value *node);
    int pr_VariableDeclaration(const rapidjson::Value *node);
    int pr_ElementaryTypeName(const rapidjson::Value *node);

    /* po_ 开头为后序遍历用到的函数 */
    int po_selector(const std::string &type_, const rapidjson::Value *node_);
    int po_PragmaDirective(const rapidjson::Value *node);
    int po_StructuredDocumentation(const rapidjson::Value *node);
    int po_ElementaryTypeName(const rapidjson::Value *node);
    int po_VariableDeclaration(const rapidjson::Value *node);
};