#pragma once

#include <fstream>
#include <iostream>
#include "rapidjson/document.h"



const std::string path_ast = "../solc_file/output/combined.json";

class AST {
    private:

    protected:

      rapidjson::Document doc;

    public:
      int parse(std::string f_);
};
