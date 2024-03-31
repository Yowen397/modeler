#pragma once

#include "AST.h"

#include <vector>
#include <map>


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

    std::vector<int> pre;
    std::vector<int> pos;

    Place();
    virtual ~Place();
    std::string getStr();
};

class Transition {
  protected:
  public:
    std::string name;

    std::vector<int> pre;
    std::vector<int> pos;

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
  public:
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
    std::vector<SC_ENUM> &enums;

  public:
    int build();      // 构建CPN的核心函数
    int info();       // 输出信息
    int draw();       // 绘图，输出CPN

    Place &getPlace(const std::string &s_);
    Transition &getTransition(const std::string &s_);
    Transition &getTransitionByMatch(const std::string &str_);
    Place &getPlaceByIdentifier(std::string id_, bool first = false);
    Place &getPlaceByMatch(const std::string &str_);
    Arc &getArc(const std::string &st_, const std::string &ed_);

    int getIdxTransition(const std::string& s_);

protected:
    int traverse(const rapidjson::Value *node_);    // 遍历，深搜，前后序，构建CPN
    int build_entryPlace();
    int build_topNet();
    void link_();

    /* 构造中通用函数、变量 */
    int e_Unkonwn(const std::string &type_, const rapidjson::Value *node_,
                  bool pr = true);
    Transition &newTransition(const std::string &name_, const int id,
                              const bool isControl = true,
                              const bool isSubNet = false);
    Arc &newArc(const std::string &st_, const std::string &ed,
                const std::string &dir, const std::string &name_="1`()");
    Place &newPlace(const std::string &name_, const bool isControl);
    SC_FUN &getFun(const std::string &name_);
    int removePlace(const std::string &name_);
    int removeArc(const std::string &st_, const std::string &ed_);
    int mid_IfStatement(const rapidjson::Value *node);
    std::string genArcExpByPlace(const Place& p_, const std::string BaseChar_);
    std::string genArcExpById(const Place& p_, const std::string& id_, const std::string BaseChar_);
    // 一次性标记
    bool is_transfer = false;
    std::stack<std::string> memberName_stk;  // 结构成员名
    int BlockDepth = 0;
    bool need_out = false;
    // 标识符相关
    std::string inFunction = "-global-";
    std::stack<std::string> id_stk;
    std::stack<std::string> op_stk;
    // CPN相关
    std::string lastPlace;        // 最新的一个库所，末端控制流或者是最新的临时数据流
    std::string lastTransition;   // 最新的一个执行变迁
    std::string returnPlace;      // 返回值库所的名称（函数）
    std::string outPlace;         // 控制流返回库所的名称

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
    int pr_BinaryOperation(const rapidjson::Value *node);
    int pr_Literal(const rapidjson::Value *node);
    int pr_ParameterList(const rapidjson::Value *node);
    int pr_Return(const rapidjson::Value *node);
    int pr_EnumDefinition(const rapidjson::Value *node);
    int pr_EnumValue(const rapidjson::Value *node);
    int pr_UserDefinedTypeName(const rapidjson::Value *node);
    int pr_IdentifierPath(const rapidjson::Value *node);
    int pr_ModifierDefinition(const rapidjson::Value *node);
    int pr_FunctionCall(const rapidjson::Value *node);
    int pr_PlaceholderStatement(const rapidjson::Value *node);
    int pr_ErrorDefinition(const rapidjson::Value *noded);
    int pr_IfStatement(const rapidjson::Value *node);
    int pr_MemberAccess(const rapidjson::Value *node);
    int pr_RevertStatement(const rapidjson::Value *node);
    int pr_EventDefinition(const rapidjson::Value *node);
    int pr_ElementaryTypeNameExpression(const rapidjson::Value *node);
    int pr_TupleExpression(const rapidjson::Value *node);
    int pr_EmitStatement(const rapidjson::Value *node);
    int pr_ModifierInvocation(const rapidjson::Value *node);
    int pr_Mapping(const rapidjson::Value *node);

    /* po_ 开头为后序遍历用到的函数 */
    int po_selector(const std::string &type_, const rapidjson::Value *node_);
    int po_PragmaDirective(const rapidjson::Value *node);
    int po_StructuredDocumentation(const rapidjson::Value *node);
    int po_ElementaryTypeName(const rapidjson::Value *node);
    int po_VariableDeclaration(const rapidjson::Value *node);
    int po_VariableDeclarationStatement(const rapidjson::Value *node);
    int po_Identifier(const rapidjson::Value *node);
    int po_Assignment(const rapidjson::Value *node);
    int po_ExpressionStatement(const rapidjson::Value *node);
    int po_Literal(const rapidjson::Value *node);
    int po_BinaryOperation(const rapidjson::Value *node);
    int po_Block(const rapidjson::Value *node);
    int po_ParameterList(const rapidjson::Value *node);
    int po_FunctionDefinition(const rapidjson::Value *node);
    int po_Return(const rapidjson::Value *node);
    int po_ContractDefinition(const rapidjson::Value *node);
    int po_SourceUnit(const rapidjson::Value *node);
    int po_EnumValue(const rapidjson::Value *node);
    int po_EnumDefinition(const rapidjson::Value *node);
    int po_IdentifierPath(const rapidjson::Value *node);
    int po_UserDefinedTypeName(const rapidjson::Value *node);
    int po_FunctionCall(const rapidjson::Value *node);
    int po_PlaceholderStatement(const rapidjson::Value *node);
    int po_ModifierDefinition(const rapidjson::Value *node);
    int po_ErrorDefinition(const rapidjson::Value *node);
    int po_MemberAccess(const rapidjson::Value *node);
    int po_RevertStatement(const rapidjson::Value *node);
    int po_IfStatement(const rapidjson::Value *node);
    int po_EventDefinition(const rapidjson::Value *node);
    int po_ElementaryTypeNameExpression(const rapidjson::Value *node);
    int po_TupleExpression(const rapidjson::Value *node);
    int po_EmitStatement(const rapidjson::Value *node);
    int po_ModifierInvocation(const rapidjson::Value *node);
    int po_Mapping(const rapidjson::Value *node);

    /* 部分常用函数构建 */
    int preBuildFun(const std::string &f_name);
    bool fun_Require = false;
    int fun_buildRequire();
    int once_transfer(const rapidjson::Value *node);
    
    int build_User();       // 建模用户操作
};