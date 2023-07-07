#include <fstream>
#include <iostream>
#include "rapidjson/document.h"



const std::string filename_ast = "../testset/output/combined.json";

class AST {
    private:
      

    protected:

      rapidjson::Document doc;

    public:
      void parse(std::string f_);
};
