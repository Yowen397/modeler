#include "CPN.h"

using namespace rapidjson;
using namespace std;

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
 * pre 前序遍历
*/
int CPN::pr_selector(const std::string &type_, const rapidjson::Value *node_) {
    cout << "pre->>:\t" << type_ << endl;
    return 0;
}

/**
 * post 后序遍历
*/
int CPN::po_selector(const std::string &type_, const rapidjson::Value *node_) {
    cout << "post<<-:\t" << type_ << endl;
    return 0;
}