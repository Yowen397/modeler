#pragma once

#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <cstdlib>

#include <map>
#include <functional>

#include "rapidjson/document.h"

#include "CPN.h"
#include "common.h"



const std::string path_ast = "../solc_files/output/combined.json";

static const char *kTypeNames[] = {"Null",  "False",  "True",  "Object",
                                   "Array", "String", "Number"};
class AST {
  private:
    int extractSolcFilename();
    int traverse_r(bool print_, std::ofstream &of_, rapidjson::Value *node,
                   rapidjson::Value *src_node = NULL); // 遍历语法树的递归部分

  protected:
    rapidjson::Document doc;   // 整个json文件
    rapidjson::Value *root;    // 语法树AST根节点
    std::string filename_solc; // 源文件的文件名.solc

    

  public:
    AST();
    virtual ~AST();

    int parse(std::string f_); // 从json文件中读取语法树
    int traverse(bool print_); // 遍历语法树，提取一些数据

  private:
    int FunctionSelector(std::string str_, const rapidjson::Value *node);
    int e_Unknown(std::string str_, const rapidjson::Value *node);
    int e_SourceUnit(const rapidjson::Value *node);
    int e_PragmaDirective(const rapidjson::Value *node);
};
