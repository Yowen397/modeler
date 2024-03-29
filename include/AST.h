#pragma once

#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <cstdlib>

#include <map>
#include <functional>

#include "rapidjson/document.h"

// #include "CPN.h"
#include "common.h"





static const char *kTypeNames[] = {"Null",  "False",  "True",  "Object",
                                   "Array", "String", "Number"};

class SC_VAR {
  public:
    enum RANGE { empty, global, local, param, ret_param };

    std::string name;
    std::string type;
    RANGE range = empty;
    std::string fun = "";     // 若不是全局变量，则fun说明其归属函数

    std::string getStr();
};

class SC_FUN {
  public:
    enum TYPE { function, error, modifier, event, internal };

    std::string name;
    TYPE type = function;
    std::vector<SC_VAR> param;
    std::vector<SC_VAR> param_ret;

    std::string getStr();
};

class SC_ENUM {
  public:
    std::string name;
    std::vector<std::string> ids;

    std::string getStr();
};

class AST {

  protected:
    rapidjson::Document doc;   // 整个json文件
    rapidjson::Value *root;    // 语法树AST根节点
    std::string filename_solc; // 源文件的文件名.solc

    std::vector<SC_VAR> vars;
    std::vector<SC_FUN> funs;
    std::vector<SC_ENUM> enums;

  public:
    AST();
    virtual ~AST();

    int parse(std::string f_);  // 从json文件中读取语法树
    int traverse(bool print_);  // 遍历语法树，提取一些数据
    int info();                 // 输出提取到的信息

    rapidjson::Value *getRoot();
    std::vector<SC_VAR> &getVars();
    std::vector<SC_FUN> &getFuns();
    std::vector<SC_ENUM> &getEnums();

  private:
    int extractSolcFilename();
    int traverse_r(bool print_, std::ofstream &of_, rapidjson::Value *node,
                   rapidjson::Value *src_node = NULL); // 遍历语法树的递归部分
  private:
    /* 以下变量用于搜索语法树过程中暂存，部分信息 */
    std::string cur_fun = "";
    std::stack<std::string> cur_typename;
    std::string cur_param_stage = ""; // ""->"parameters"->"return"，三阶段

    int EntryOperation(std::string str_, const rapidjson::Value *node);
    int FunctionSelector(std::string str_, const rapidjson::Value *node);
    int e_Unknown(std::string str_, const rapidjson::Value *node);
    int po_SourceUnit(const rapidjson::Value *node);
    int po_PragmaDirective(const rapidjson::Value *node);
    int po_VariableDeclaration(const rapidjson::Value *node);
    int po_ElementaryTypeName(const rapidjson::Value *node);
    int po_FunctionDefinition(const rapidjson::Value *node);
    int po_ParameterList(const rapidjson::Value *node);
    int po_UserDefinedTypeName(const rapidjson::Value *node);
    int po_EnumValue(const rapidjson::Value *node);
    int po_Mapping(const rapidjson::Value *node);
};
