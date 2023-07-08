#pragma once

#include <fstream>
#include <iostream>
#include "rapidjson/document.h"



const std::string path_ast = "../solc_files/output/combined.json";

static const char *kTypeNames[] = {"Null",  "False",  "True",  "Object",
                                   "Array", "String", "Number"};

class AST {
    private:
      int extractSolcFilename();

    protected:
      rapidjson::Document doc;      // 整个json文件
      rapidjson::Value root;        // 语法树AST根节点
      std::string filename_solc;    // 源文件的文件名.solc

    public:
      int parse(std::string f_);
};
