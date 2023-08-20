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

    bool isControl;

    Place();
    virtual ~Place();
    std::string getStr();
};

class Transition {
  protected:
  public:
    std::string name;

    bool isControl = true;
    bool isSubNet = false;

    std::string getStr();
    void init(std::string name_, bool isControl_, bool isSubNet_ = false);
};

class Arc {
  public:
    std::string st;
    std::string ed;
    std::string dir;  // "t2p" or "p2t"
    std::string name; // name当作表达式使用，加上全局编号

    bool isControl;

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
    int info();       // 输出信息
    int draw();       // 绘图，输出CPN

    Place &getPlace(const std::string &s_);
    Transition &getTransition(const std::string &s_);

  protected:
    int traverse(const rapidjson::Value *node_);    // 遍历，深搜，前后序，构建CPN

    int build_topNet();

    /* 构造中通用函数、变量 */
    int e_Unkonwn(const std::string &type_, const rapidjson::Value *node_,
                  bool pr = true);
    Transition &newTransition(const std::string &name_, const int id,
                              const bool isControl = true,
                              const bool isSubNet = false);
    Place &getPlaceByIdentifier(const std::string &id_);
    Arc &newArc(const std::string &st_, const std::string &ed,
                const std::string &dir, const std::string &name_);
    Place &newPlace(const std::string &name_, const bool isControl);
    std::string inFunction = "-global-";
    std::string lastControlPlace; // 最新的一个控制流库所/末端控制流
    std::string lastTransition;   // 最新的一个执行变迁
    std::stack<std::string> id_stk;

    /* pr_ 开头为前序遍历用到的函数 */
    int pr_selector(const std::string &type_, const rapidjson::Value *node_);
    int pr_SourceUnit(const rapidjson::Value *node_);
    int pr_PragmaDirective(const rapidjson::Value *node_);
    int pr_ContractDefinition(const rapidjson::Value *node_);
    int pr_StructuredDocumentation(const rapidjson::Value *node);
    int pr_VariableDeclaration(const rapidjson::Value *node);
    int pr_ElementaryTypeName(const rapidjson::Value *node);
    int pr_FunctionDefinition(const rapidjson::Value *node);
    int pr_Block(const rapidjson::Value *node);
    int pr_VariableDeclarationStatement(const rapidjson::Value *node);
    int pr_ExpressionStatement(const rapidjson::Value *node);
    int pr_Assignment(const rapidjson::Value *node);
    int pr_Identifier(const rapidjson::Value *node);

    /* po_ 开头为后序遍历用到的函数 */
    int po_selector(const std::string &type_, const rapidjson::Value *node_);
    int po_PragmaDirective(const rapidjson::Value *node);
    int po_StructuredDocumentation(const rapidjson::Value *node);
    int po_ElementaryTypeName(const rapidjson::Value *node);
    int po_VariableDeclaration(const rapidjson::Value *node);
    int po_VariableDeclarationStatement(const rapidjson::Value *node);
    int po_Identifier(const rapidjson::Value *node);
    int po_Assignment(const rapidjson::Value *node);
};