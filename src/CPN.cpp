#include "CPN.h"

using namespace rapidjson;
using namespace std;

bool debug = true;

Place::Place() {}

Place::~Place() {}

CPN::CPN(AST &ast_) : vars(ast_.getVars()), funs(ast_.getFuns()) {
    root = ast_.getRoot();
}

CPN::~CPN() {}

std::string Place::getStr() { return "[" + color + "\t] " + name; }

std::string Transition::getStr() {
    return name + (isSubNet ? "(subnet)" : "(normal)");
}

void Transition::init(std::string name_, bool isControl_, bool isSubNet_) {
    this->name = name_;
    this->isControl = isControl_;
    this->isSubNet = isSubNet_;
}

std::string Arc::getStr() { return "[" + dir + "] " + st + "\t-->>\t" + ed; }

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
 * 在字符串前后添加   \"
*/
inline string to_quotation(const string &str_) {
    string ret = "\"";

    return ret + str_ + "\"";
}

int CPN::draw() {
    ofstream outfile("CPN.dot", ios::out);
    outfile << "digraph G{" << endl;
    outfile << "node[shape=box]" << endl; // 先输出变迁

    for (const auto &t : trans) {
        outfile << to_quotation(t.name) << "[label=\"" << t.name << "\"";
        outfile << (t.isControl ? ",color=gold" : "");
        outfile << "]" << endl;
    }

    outfile << "node[shape=circle]" << endl;
    for (const auto &p : places) {
        outfile << to_quotation(p.name) << "[label=\"" << p.name << "\"";
        outfile << (p.isControl ? ",color=gold" : "");
        outfile << "]" << endl;
    }

    for (const auto &a : arcs) {
        outfile << to_quotation(a.st) << "->" << to_quotation(a.ed) << "[";
        if (a.isControl)
            outfile << "color=gold";
        else 
            outfile << "label=\"" << a.name << "\"";
        outfile << "]" << endl;
    }

    outfile << "}" << endl;
    outfile.close();
    if (debug)
        cout << "CALL dot TO GENERATE .png FILE..." << endl;
    system("dot -Tpng CPN.dot -o CPN.png");
    return 0;
}

Place &CPN::getPlace(const string &s_) {
    for (auto &it : places) {
        if (it.name == s_)
            return it;
    }
    cerr << "\ninexistent place name [" << s_ << "]" << endl;
    exit(-1);
}

Transition &CPN::getTransition(const std::string &s_) {
    for (auto &it : trans) {
        if (it.name == s_)
            return it;
    }
    cerr << "\ninexistent transition name [" << s_ << "]" << endl;
    exit(-1);
}

/**
 * 构建CPN的顶层网络
*/
int CPN::build_topNet() {
    // 1.a 变量库所：全局变量、局部变量
    for (auto &v : vars) {
        places.emplace_back();
        auto &p = places.back();
        p.isControl = false;
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
            p.isControl = false;
            p.name = f.name + ".param." + v.name;
            p.color = v.type;
        }
        for (auto &v : f.param_ret) {
            places.emplace_back();
            auto &p = places.back();
            p.isControl = false;
            p.name = f.name + ".ret." + v.name;
            p.color = v.type;
        }
    }

    // 2 函数变迁
    for (const auto &f : funs) {
        trans.emplace_back();
        auto &t = trans.back();
        t.name = f.name + ".f";
        t.isSubNet = true;
        t.isControl = true;
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



Place &CPN::getPlaceByIdentifier(const string &id_) {
    size_t pos;
    // 从所有库所中查找，先查找函数内作用域的(排除全局变量)
    for (auto &p : places) {
        // 排除控制库所
        if (p.name.find(".c.") != string::npos)
            continue;
        // 排除全局变量
        if (p.name.find("global.") != string::npos)
            continue;
        // 变量名匹配
        if ((pos = p.name.find(id_)) != string::npos &&
            pos + id_.size() == p.name.size() && p.name[pos - 1] == '.')
            return p;
    }
    // 全局变量
    for (auto &p : places) {
        if (p.name.find(".c.") != string::npos)
            continue;
        if ((pos = p.name.find(id_)) != string::npos &&
            pos + id_.size() == p.name.size() && p.name[pos - 1] == '.')
            return p;
    }
    // 正常来说不会执行此句，仅写出来取消警告
    static Place error_place;
    error_place.name = "ERROR";
    // cerr << "can't find place by id [" << id_ << "]" << endl;
    // exit(-1);
    return error_place;
}

Transition &CPN::newTransition(const string &name_, const int id,
                               const bool isControl_, const bool isSubNet_) {
    Transition t;
    t.name = name_ + "." + to_string(id);
    t.isControl = isControl_;
    t.isSubNet = isSubNet_;

    trans.emplace_back(t);
    lastTransition = t.name;
    return trans.back();
}

Arc& CPN::newArc(const string &st_, const string &ed_, const string &dir_,
                 const string &name_) {
    Arc a;
    a.st = st_;
    a.ed = ed_;
    a.dir = dir_;

    static int cnt = 0;
    if (name_ != "control") {
        a.name = name_ + "." + to_string(cnt++);
        a.isControl = false;
    } else {
        a.name = "control." + to_string(cnt++);
        a.isControl = true;
    }

    arcs.emplace_back(a);
    return arcs.back();
}

/**
 * 新建一个库所
 * 若isControl为true，则会自动在库所名称后追加  .c.cnt
*/
Place &CPN::newPlace(const string &name_, const bool isControl_) {
    Place p;
    p.name = name_;

    p.isControl = isControl_;
    if (p.isControl) {
        p.name += ".c";

        static int cnt = 0;
        p.name += "." + to_string(cnt++);
    }

    places.emplace_back(p);
    lastPlace = p.name;
    return places.back();
}

/**
 * 通过函数名获取函数的引用 (SC_FUN)
*/
SC_FUN &CPN::getFun(const string &name_) {
    for (auto &f: funs) {
        if (f.name == name_)
            return f;
    }
    static SC_FUN ef;
    ef.name = "ERROR_FUN";
    return ef;
}

/**
 * 删除库所，连带相关的Arc
*/
int CPN::removePlace(const string &name_) {
    //
    auto it = places.begin();
    while (it != places.end()) {
        if (it->name != name_) {
            it++;
            continue;
        }
        // 找到要移除的库所，首先移除相关的arc
        auto it_a = arcs.begin();
        while (it_a != arcs.end()) {
            if (it_a->st == name_ || it_a->ed == name_)
                it_a = arcs.erase(it_a);
            else
                it_a++;
        }
        places.erase(it);
        return 0;
    }
    if (it == places.end()) {
        cerr << "place [" << name_ << "] doesn't exist, remove failed..."
             << endl;
        exit(-1);
    }
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
    type_ == "VariableDeclarationStatement" ? pr_VariableDeclarationStatement(node), check = 1 : 0;
    type_ == "ExpressionStatement" ? pr_ExpressionStatement(node), check = 1 : 0;
    type_ == "Assignment" ? pr_Assignment(node), check = 1 : 0;
    type_ == "Identifier" ? pr_Identifier(node), check = 1 : 0;
    type_ == "BinaryOperation" ? pr_BinaryOperation(node), check = 1 : 0;
    type_ == "Literal" ? pr_Literal(node), check = 1 : 0;
    type_ == "ParameterList" ? pr_ParameterList(node), check = 1 : 0;
    type_ == "Return" ? pr_Return(node), check = 1 : 0;

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
    type_ == "Identifier" ? po_Identifier(node), check = 1 : 0;
    type_ == "Assignment" ? po_Assignment(node), check = 1 : 0;
    type_ == "ExpressionStatement" ? po_ExpressionStatement(node), check = 1 : 0;
    type_ == "Literal" ? po_Literal(node), check = 1 : 0;
    type_ == "BinaryOperation" ? po_BinaryOperation(node), check = 1 : 0;
    type_ == "Block" ? po_Block(node), check = 1 : 0;
    type_ == "ParameterList" ? po_ParameterList(node), check = 1 : 0;
    type_ == "FunctionDefinition" ? po_FunctionDefinition(node), check = 1 : 0;
    type_ == "Return" ? po_Return(node), check = 1 : 0;
    type_ == "ContractDefinition" ? po_ContractDefinition(node), check = 1 : 0;
    type_ == "SourceUnit" ? po_SourceUnit(node), check = 1 : 0;

    if (!check)
        return e_Unkonwn(type_, node, false);
    else
        return 0;
    return 0;
}

int CPN::po_SourceUnit(const Value *node) {
    //
    return 0;
}

int CPN::po_ContractDefinition(const Value *node) {
    //
    return 0;
}

int CPN::po_Return(const Value *node) {
    // 返回语句最终在id_stk中必然包含一个变量（可能临时变量）

    // 执行变迁
    auto attr_id = node->FindMember("id");
    newTransition("Return", attr_id->value.GetInt());
    newArc(lastPlace, lastTransition, "p2t");

    // 返回值来源
    Place &p = getPlaceByIdentifier(id_stk.top());
    id_stk.pop();
    newArc(p.name, lastTransition, "p2t", "read");

    // 找到返回值库所，目前只处理单值返回的，返回值库所命名唯一
    newArc(lastTransition, returnPlace, "t2p", "write");

    // 控制流
    newArc(lastTransition, outPlace, "t2p");

    return 0;
}

int CPN::pr_Return(const Value *node) {
    //
    
    return 0;
}

int CPN::po_FunctionDefinition(const Value *node) {
    // 退出函数构建，如果退出时最后一句不是return，则需引导控制流回到退出点
    // 若函数最后一句就是return，则直接返回
    if (getTransition(lastTransition).name.find("Return") != string::npos)
        return 0;

    // 否则引导控制流退出
    newArc(lastTransition, outPlace, "t2p");    
    // 并且删除无效的控制库所
    removePlace(lastPlace);
    return 0;
}

int CPN::po_ParameterList(const Value *node) {
    // 参数列表，0层构建时已经处理
    return 0;
}

int CPN::pr_ParameterList(const Value *node) {
    // 参数列表，0层构建时已经处理
    return 0;
}

int CPN::po_Block(const Value *node) {
    // 暂时不处理
    return 0;
}

int CPN::po_BinaryOperation(const Value *node) {
    // 二元运算的处理，暂且为先加
    // 先生成一个变迁
    auto attr_id = node->FindMember("id");
    newTransition(op_stk.top(), attr_id->value.GetInt());
    op_stk.pop();
    newArc(lastPlace, lastTransition, "p2t");
    // 左值和右值存放在id_stk
    // 右值
    Place &right = getPlaceByIdentifier(id_stk.top());
    id_stk.pop();
    if (right.name=="ERROR"){
        if (debug)
            cout << "right value is Literal"; // 找不到，说明是常量
    }
    else{
        newArc(right.name, lastTransition, "p2t", "read");
        newArc(lastTransition, right.name, "t2p", "replace");
    }
    // 左值
    Place &left = getPlaceByIdentifier(id_stk.top());
    id_stk.pop();
    if (left.name=="ERROR"){
        if (debug)
            cout << "left value is Literal"; // 找不到，说明是常量
    }
    else{
        newArc(left.name, lastTransition, "p2t", "read");
        newArc(lastTransition, left.name, "t2p", "replace");
    }
    // 结果库所（tmp）
    newPlace(inFunction + ".tmp." + to_string(attr_id->value.GetInt()), false);
    newArc(lastTransition, lastPlace, "t2p", "write");
    id_stk.push("tmp." + to_string(attr_id->value.GetInt()));
    // 填充控制库所
    newPlace("BinaryOperation", true);
    newArc(lastTransition, lastPlace, "t2p");

    // draw();
    return 0;
}

int CPN::po_Literal(const Value *node) {
    // 常量值
    auto attr_value = node->FindMember("value");
    if (attr_value==node->MemberEnd()) {
        cerr << "Literal can't find member [value]" << endl;
        exit(-1);
    }
    id_stk.push(attr_value->value.GetString());
    return 0;
}

int CPN::pr_Literal(const Value *node) {
    // 常量值的出现形式？
    return 0;
}

int CPN::pr_BinaryOperation(const Value *node) {
    // 在二元运算进入节点不需要考虑处理，记录操作符即可
    auto attr_operator = node->FindMember("operator");
    if (attr_operator == node->MemberEnd()) {
        cerr << "BinaryOperation can't find member [operator]" << endl;
        exit(-1);
    }
    op_stk.push(attr_operator->value.GetString());
    return 0;
}

int CPN::po_ExpressionStatement(const Value *node) {
    // 表达式语句认为在赋值中已经处理

    // id_stk需要清空
    while (!id_stk.empty())
        id_stk.pop();
    return 0;
}

int CPN::po_Assignment(const Value *node) {
    // 当前处理方案，认为Assignment结束时应按序处理所有的Identifier，在id_stk中
    // 需要找到Assignment左值，left hand side，定为处理的终止点
    auto node_leftHandSide = node->FindMember("leftHandSide");
    if (node_leftHandSide==node->MemberEnd()) {
        cerr << "Assignment node can't find member [leftHandSide]" << endl;
        exit(-1);
    }
    auto attr_name = node_leftHandSide->value.FindMember("name");
    if (attr_name==node_leftHandSide->value.MemberEnd()) {
        cerr << "leftHandSide node can't find member [name]" << endl;
        exit(-1);
    }
    auto attr_id = node->FindMember("id"); // 确定nodeType节点一定包含id
    // 新建一个变迁，并连接
    // Transition t;
    // t.name = attr_name->value.GetString() + string(".") +
    //          to_string(attr_id->value.GetInt());
    // trans.emplace_back(t);
    // lastTransition = t.name;
    Transition &t =
        newTransition("Assignment", attr_id->value.GetInt(), true, false);
    newArc(lastPlace, t.name, "p2t", "control");
    // t.init()
    // 从栈顶取元素处理
    while (id_stk.size() && id_stk.top()!=attr_name->value.GetString()) {
        string id = id_stk.top();
        id_stk.pop();
        Place &p = getPlaceByIdentifier(id);
        // 建立弧连接，操作类型为read
        newArc(p.name, t.name, "p2t", "read");
        // 读完之后要返回
        newArc(t.name, p.name, "t2p", "replace");
    }
    // 最后处理表达式左值
    if (id_stk.empty()||(id_stk.top()!=attr_name->value.GetString())) {
        cerr << "stack top is not assignment left hand side." << endl;
        exit(-1);
    }
    Place &p_reslut = getPlaceByIdentifier(id_stk.top());
    id_stk.pop();
    newArc(t.name, p_reslut.name, "t2p", "assign");

    // 填充控制库所
    Place &p_c = newPlace("Assignment", true);
    newArc(t.name, lastPlace, "t2p", "control");

    if (debug) {
        cout << "Assignment expression left hand side variable is : "
             << attr_name->value.GetString() << endl;
    }

    
    return 0;
}

int CPN::po_Identifier(const Value *node) {
    auto attr_name = node->FindMember("name");
    if (attr_name == node->MemberEnd()) {
        cerr << "Identifier node can't find member [name]" << endl;
        exit(-1);
    }
    id_stk.push(attr_name->value.GetString());

    if (debug)
        cout << "Identifier: " << attr_name->value.GetString() << endl;
    return 0;
}

int CPN::pr_Identifier(const Value *node) {
    return 0;
}

int CPN::pr_Assignment(const Value *node) {
    // 赋值语句
    return 0;
}

int CPN::pr_ExpressionStatement(const Value *node) {
    // 表达式
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

    // 构建入口控制流库所，以及对应的弧
    Place &p = newPlace(inFunction + ".in", true);
    newArc(inFunction + ".f", p.name, "t2p", "control");
 
    // 构建出口库所，该库所当前不需要弧连接
    outPlace = newPlace(inFunction + ".out", true).name;

    // cout << "DEBUG::arcs st[" << getTransition(a.st).name << "]" << endl;
    // cout << "DEBUG::arcs ed[" << getPlace(a.ed).name << "]" << endl;

    lastPlace = p.name;
    lastTransition = inFunction + ".f";
    // 暂时只处理单个返回值(若有返回值)
    if (getFun(inFunction).param_ret.size())
        returnPlace =
            inFunction + ".ret." + getFun(inFunction).param_ret[0].name;

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