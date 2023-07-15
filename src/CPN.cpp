#include "CPN.h"

using namespace rapidjson;
using namespace std;

CPN::CPN(AST &ast_) : vars(ast_.getVars()), funs(ast_.getFuns()) {
    root = ast_.getRoot();
}

CPN::~CPN() {}


