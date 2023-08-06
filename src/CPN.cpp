#include "CPN.h"

using namespace rapidjson;
using namespace std;

bool debug = true;

Place::Place() {}

Place::~Place() {}

bool Transition::getSubNet() { return isSubNet; }

void Transition::setSubNet(bool v_) { this->isSubNet = v_; }

CPN::CPN(AST &ast_) : vars(ast_.getVars()), funs(ast_.getFuns()) {
    root = ast_.getRoot();
}

CPN::~CPN() {}

std::string Place::getStr() { return "[" + color + "\t] " + name; }

std::string Transition::getStr() {
    return name + (getSubNet() ? "(subnet)" : "(normal)");
}

std::string Arc::getStr() { return "[" + dir + "] " + st + "-->>" + ed; }

/**
 * 输出CPN的信息
*/
int CPN::info() {
    cout << "==========CPN info==========" << endl;
    cout << "---------- place  ----------" << endl;
    for (auto &p : places) {
        cout << p.getStr() << endl;
    }
    cout << "----------transtion----------" << endl;
    for (auto &t : trans) {
        cout << t.getStr() << endl;
    }
    cout << "----------  arc   ----------" << endl;
    for (auto &a : arcs) {
        cout << a.getStr() << endl;
    }
    cout << "============================" << endl << endl;
    return 0;
}

/**
 * 构建CPN的顶层网络
*/
int CPN::build_topNet() {
    // 1.a 变量库所：全局变量、局部变量
    for (auto &v : vars) {
        places.emplace_back();
        auto &p = places.back();
        if (v.range == SC_VAR::RANGE::global)
            p.name = "global." + v.name;
        else
            p.name = v.fun + "." + v.name;
        p.color = v.type; // CPN的库所颜色就是变量类型
    }
    // 1.b 变量库所：函数参数、函数返回值
    for (auto &f : funs) {
        for (auto &v : f.param) {
            places.emplace_back();
            auto &p = places.back();
            p.name = f.name + ".param." + v.name;
            p.color = v.type;
        }
        for (auto &v : f.param_ret) {
            places.emplace_back();
            auto &p = places.back();
            p.name = f.name + ".ret." + v.name;
            p.color = v.type;
        }
    }

    // 2 函数变迁
    for (const auto &f : funs) {
        trans.emplace_back();
        auto &t = trans.back();
        t.name = f.name;
        t.setSubNet(true);
    }

    return 0;
}

/**
 * 构建CPN的核心函数
*/
int CPN::build() {

    // 1.构建顶层网络结构，函数作为变迁不展开
    build_topNet();

    // 2.深搜
    traverse(root);

    return 0;
}

/**
 * 该函数为第二次遍历语法树
 * 采用深度优先搜索
 * 前序、后序都对节点进行解析处理
 * @return int 状态返回值，0为正常返回，其它为异常
*/
int CPN::traverse(const rapidjson::Value *node_) {
    auto attr_nodeType = node_->FindMember("nodeType");
    string nodeType =
        (attr_nodeType != node_->MemberEnd() ? attr_nodeType->value.GetString()
                                             : "");

    if (nodeType != "")
        pr_selector(nodeType, node_);

    // 子节点继续搜索
    for (auto it = node_->MemberBegin(); it != node_->MemberEnd(); it++) {
        if (it->value.IsObject()) {
            traverse(&it->value);
        } else if (it->value.IsArray()) {
            // 如果是数组，则子节点的object类型全部进入搜索
            for (rapidjson::Value::ConstValueIterator iter = it->value.Begin();
                 iter != it->value.End(); iter++) {
                if (iter->IsObject())
                    traverse((const rapidjson::Value *)iter);
            }
        }
    }

    if (nodeType != "")
        po_selector(nodeType, node_);

    return 0;
}

/**
 * @param pr bool型变量，用于判断是否是前序遍历中进入该函数，若为后序遍历
 *              进入，则该变量应该为false
*/
int CPN::e_Unkonwn(const std::string &type_, const rapidjson::Value *node_,
                   bool pr)
{
    cerr << "Untreated node type: [" << type_ << "]";
    cerr << string(" in ") + (pr ? "[pre order]" : "[post order]") << endl;

    cout << "---modeler exit---" << endl;
    exit(-1);

    return 0;
}

/**
 * pre 前序遍历
*/
int CPN::pr_selector(const std::string &type_, const rapidjson::Value *node) {
    if (debug)
        cout << "PRE--->>>:\t" << type_ << endl;
    int check = 0;
    // 如果字符串匹配，则执行对应函数，并且将check修改为1
    type_ == "SourceUnit" ? pr_SourceUnit(node), check = 1 : 0;
    type_ == "PragmaDirective" ? pr_PragmaDirective(node), check = 1 : 0;
    type_ == "ContractDefinition" ? pr_ContractDefinition(node), check = 1 : 0;
    type_ == "StructuredDocumentation" ? pr_StructuredDocumentation(node), check = 1 : 0;
    type_ == "VariableDeclaration" ? pr_VariableDeclaration(node), check = 1 : 0;
    type_ == "ElementaryTypeName" ? pr_ElementaryTypeName(node), check = 1 : 0;
    type_ == "FunctionDefinition" ? pr_FunctionDefinition(node), check = 1 : 0;
    type_ == "Block" ? pr_Block(node), check = 1 : 0;
    type_ == "VariableDeclarationStatement"?pr_VariableDeclarationStatement(node), check = 1 : 0;


    if (!check)
        return e_Unkonwn(type_, node);
    else
        return 0;
    return 0;
}

/**
 * post 后序遍历
*/
int CPN::po_selector(const std::string &type_, const rapidjson::Value *node) {
    if (debug)
        cout << "POST<<<---:\t" << type_ << endl;
    int check = 0;
    // 如果字符串匹配，则执行对应函数，并且将check修改为1
    // type_ == "SourceUnit" ? po_SourceUnit(node), check = 1 : 0;
    type_ == "PragmaDirective" ? po_PragmaDirective(node), check = 1 : 0;
    type_ == "StructuredDocumentation" ? po_StructuredDocumentation(node), check = 1 : 0;
    type_ == "ElementaryTypeName" ? po_ElementaryTypeName(node), check = 1 : 0;
    type_ == "VariableDeclaration" ? po_VariableDeclaration(node), check = 1 : 0;
    type_ == "VariableDeclarationStatement" ? po_VariableDeclarationStatement(node), check = 1 : 0;

    if (!check)
        return e_Unkonwn(type_, node, false);
    else
        return 0;
    return 0;
}

int CPN::po_VariableDeclarationStatement(const Value *node) { return 0; }

int CPN::pr_VariableDeclarationStatement(const Value *node) {
    // 变量声明，但是变量已经在0层网络中构建对应库所
    return 0;
}

int CPN::pr_Block(const Value *node) {
    // 语句块
    return 0;
}

int CPN::pr_FunctionDefinition(const Value *node) {
    // 进入函数定义区域
    auto attr_name = node->FindMember("name");
    if (attr_name==node->MemberEnd()) {
        cerr << "FunctionDefinition node can't find member [name]" << endl;
        exit(-1);
    }
    inFunction = attr_name->value.GetString();
    if (debug)
        cout << "entry function: " << attr_name->value.GetString() << endl;
    return 0;
}

int CPN::po_VariableDeclaration(const Value *node) {
    // 由于变量库所的构建已经在构建0层网的过程中完成，该步骤不需要重复构建
    if (debug) {
        auto attr_name = node->FindMember("name");
        cout << attr_name->value.GetString() << "\tin function : " << inFunction << endl;
    }
    return 0;
}

int CPN::po_ElementaryTypeName(const Value *node) {
    // 变量的基础类型
    // 该节点一般为叶子节点
    if (debug) {
        auto attr_name = node->FindMember("name");
        cout << attr_name->value.GetString() << endl;
    }
    return 0;
}

int CPN::pr_ElementaryTypeName(const Value *node) {
    // 基础的变量类型
    return 0;
}

int CPN::pr_VariableDeclaration(const Value *node) {
    // 变量定义相关
    return 0;
}

int CPN::po_StructuredDocumentation(const Value *node) {
    // 合约前注释文档，进入时有debug输出，退出不需要操作
    return 0;
}

int CPN::pr_StructuredDocumentation(const Value *node) {
    // 合约前注释文档
    if (debug) {
        auto attr_text = node->FindMember("text");
        cout << attr_text->value.GetString() << endl;
    }
    return 0;
}

int CPN::pr_ContractDefinition(const Value *node) {
    // 合约定义函数
    return 0;
}

int CPN::po_PragmaDirective(const Value *node) {
    // 代码编译信息节点，退出
    // 不需要任何操作
    return 0;
}

int CPN::pr_PragmaDirective(const Value *node_) {
    // 代码编译信息节点
    if (debug) {
        auto attr_literals = node_->FindMember("literals");
        if (attr_literals == node_->MemberEnd()) {
            cerr << "no literals node in this node[PragmaDirective]" << endl;
            exit(-1);
        }
        for (const auto &it : attr_literals->value.GetArray())
            cout << it.GetString() << ' '; 
        cout << endl;
        return 0;
    }
    return 0;
}

int CPN::pr_SourceUnit(const Value *node_) {
    // 最根节点
    return 0;
}