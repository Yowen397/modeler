#include "AST.h"

using namespace rapidjson;

AST::~AST() {}

AST::AST() {}

rapidjson::Value *AST::getRoot() { return this->root; }

std::vector<SC_VAR> &AST::getVars() { return this->vars; }

std::vector<SC_FUN> &AST::getFuns() { return this->funs; }

std::vector<SC_ENUM> &AST::getEnums() { return this->enums; }

/**
 * 从rapid json的doc中，提取出solc文件的文件名
 * 提取方式为从sourceList节点（array类型）中取第一个
 * 暂时只取第一个，默认单文件方式
*/
int AST::extractSolcFilename() { 
    auto node_sourceList = doc.FindMember("sourceList");
    if (node_sourceList==doc.MemberEnd()) {
        std::cerr << "There is no \"sourceList\" node in AST(json file)."
                  << std::endl;
        return -1;
    }
    // std::cout << kTypeNames[node_sourceList->value.GetType()] << std::endl;
    if (node_sourceList->value.GetType()!=rapidjson::kArrayType) {
        std::cerr << "\"sourceList\" node is not a array." << std::endl;
        return -1;
    }
    if (node_sourceList->value[0].IsString())
        this->filename_solc = node_sourceList->value[0].GetString();
    else {
        std::cerr << "The type of frist value in \"sourceList\" node(array) is "
                     "not string."
                  << std::endl;
        return -1;
    }
    return 0;
}


/**
 * 解析json文件，获取语法树
 * 并且获取root节点对象，存放在Value
*/
int AST::parse(std::string path_) {
    // 读取json文件中的数据
    std::ifstream ifs(path_.c_str(), std::ios::in);
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>(0));
    // 解析json数据
    if (doc.Parse(str.data()).HasParseError()) {
        std::cerr << "error in parse json file." << std::endl;
        std::cerr << "check file name [" << path_ << "]" << std::endl;
        exit(-1);
    }
    this->extractSolcFilename(); //提取solc文件的文件名
    // 至此json数据存放在doc中，下一步操作是找到语法树的根节点
    auto node_sources = doc.FindMember("sources"); // sources节点
    if ( node_sources==doc.MemberEnd()) {
        std::cerr << "There is no \"sources\" node in AST(json file)." << std::endl;
        exit(-1);
    }
    auto node_filename_solc =
        node_sources->value.FindMember(this->filename_solc.c_str()); // "源文件名"节点，例如storage.solc
    if (node_filename_solc == node_sources->value.MemberEnd()) {
        std::cerr << "There is no \"" << this->filename_solc
                  << "\" node in AST(json file)." << std::endl;
        return -1;
    }
    auto node_AST = node_filename_solc->value.FindMember("AST"); // AST节点
    if (node_AST==node_filename_solc->value.MemberEnd()) {
        std::cerr << "There is no \"AST\" node in AST(json file)." << std::endl;
        return -1;
    }
    this->root = &node_AST->value;

    /*
    if (doc.HasMember("sourceList")) {
        const rapidjson::Value &sl = doc["sourceList"];
        std::cout << "sl is array?: " << sl.IsArray() << std::endl;
        // const rapidjson::Value &mem = sl.GetArray();
        for (rapidjson::Value::ConstValueIterator it =
    sl.Begin();it!=sl.End();++it) { std::cout << (it->IsString() ?
    it->GetString() : "") << std::endl;
        }
    }
    if (doc.HasMember("contracts")) {
        const rapidjson::Value &cs = doc["contracts"];
        std::cout << "contracts Type: "
                  << (cs.GetType() == rapidjson::kObjectType) << std::endl;
        std::cout << "has member (storage.solc:Storage)?: "
                  << cs.HasMember("storage.solc:Storage") << std::endl;
        rapidjson::Value::ConstMemberIterator sss =
    cs.FindMember("storage.solc:Storage"); std::cout << sss->value.IsObject() <<
    std::endl;
    }*/
    return 0;
}

// int arrayPush(std::queue<rapidjson::Value *> &s_,
//               rapidjson::Value::ConstMemberIterator array_){
//     for (rapidjson::Value::ConstValueIterator it = array_->value.Begin();
//          it != array_->value.End(); it++) {
//         // std::cout << kTypeNames[it->GetType()] << std::endl;
//         if (it->IsObject())
//             s_.push((rapidjson::Value *)it);
//         // s_.push(&(it->GetObject()));
//     }
//     return 0;
// }

static const char *RangeStr[] = {"empty", "global", "local", "param",
                                 "r_param"};
std::string SC_VAR::getStr() {
    return type + "\t\t" + name + "\t" + RangeStr[range] + "\t\tfunction: " + fun;
}

std::string SC_FUN::getStr() {
    std::string ret = "function name: " + name + "\n";
    ret += "parameters : \n";
    for (auto p : param)
        ret += "\t" + p.getStr() + "\n";
    for (auto p : param_ret)
        ret += "\t" + p.getStr() + "\n";
    return ret + "\n";
}

std::string SC_ENUM::getStr() {
    std::string ret = name+":\t";
    for (auto &id : ids)
        ret += id + ", ";
    return ret;
}

int AST::info() {
    std::cout << "==========AST info==========\n";
    std::cout << "variables:\n";
    for (auto v : vars)
        std::cout << "\t" + v.getStr() << std::endl;
    std::cout << "enum:\n";
    for (auto e:enums)
        std::cout << e.getStr() << std::endl;
    std::cout << std::endl;
    for (auto f : funs)
        std::cout << f.getStr();
    std::cout << "============================\n";
    std::cout << std::endl;
    return 0;
}

/**
 * 遍历语法树，递归部分函数
 * @param src_node 这个参数专门为绘图使用，指出当前节点的父节点（nodeType）
*/
int AST::traverse_r(bool print_, std::ofstream &of_, rapidjson::Value *node,
                    rapidjson::Value *src_node) {
    // 先序访问，这个区域主要是绘图
    auto attr_type = node->FindMember("nodeType");
    if (attr_type!=node->MemberEnd()) {
        // 存在nodeType

        // 若存在nodeType尝试检测id
        // auto attr_id = node->FindMember("id");
        // if (attr_id!=node->MemberEnd()) {
        //     std::cout << attr_id->value.GetInt() << ",";
        //     // exit(0);
        // }

        // static int cnt = 0;
        // std::cout << ++cnt << "\t";
        // std::cout << "nodeType: " << attr_type->value.GetString() << std::endl;
        if (print_) {
            std::string label = attr_type->value.GetString();
            std::string color;
            // 输出一些节点信息
            // if (attr_type->value.GetString() == "Identifier")
            if (strcmp("Identifier", attr_type->value.GetString())==0)
                label += std::string(":") + node->FindMember("name")->value.GetString(), color = "gold";
            if (strcmp("BinaryOperation", attr_type->value.GetString())==0)
                label += std::string(":") + node->FindMember("operator")->value.GetString(), color = "blue";

            of_ << long(node) << "[label=\"" << label << "\"";
            of_ << ",color=\"" << color << "\"";
            of_ << "]\n";
            if (src_node)
                of_ << (long)src_node << "->" << (long)node << std::endl;
            src_node = node;
        }
        this->EntryOperation(attr_type->value.GetString(), node);
    }

    // 子节点继续搜索
    for (auto it = node->MemberBegin(); it != node->MemberEnd(); it++) {
        if (it->value.IsObject()) {
            traverse_r(print_, of_, &it->value, src_node);
        } else if (it->value.IsArray()) {
            // arrayPush(q, it);
            // 如果是数组，则子节点的object类型全部进入搜索
            for (rapidjson::Value::ConstValueIterator iter = it->value.Begin();
                 iter != it->value.End(); iter++) {
                // std::cout << kTypeNames[it->GetType()] << std::endl;
                if (iter->IsObject())
                    // s_.push((rapidjson::Value *)it);
                    traverse_r(print_, of_, (rapidjson::Value *)iter, src_node);
                // s_.push(&(it->GetObject()));
            }
        }
    }
    // 后序访问，
    if (attr_type!=node->MemberEnd()) {
        FunctionSelector(attr_type->value.GetString(), node);
    }

    return 0;
}

/**
 * 遍历语法树，提取关键信息，采用递归深搜
*/
int AST::traverse(bool print_) {
    std::ofstream outfile("AST.dot", std::ios::out);
    if (print_) {
        outfile << "digraph G{\n"
                << "node[shape=box]\n";
    }

    traverse_r(print_, outfile, root);

    if (print_)  
        outfile << "}\n";
    outfile.close();
    if (print_)
        system("dot -Tpng AST.dot -o AST.png");
    return 0;
}

int AST::EntryOperation(std::string str_, const rapidjson::Value *node) {
    if (str_ == "FunctionDefinition") {
        auto attr_fun_name = node->FindMember("name");
        if (attr_fun_name==node->MemberEnd()) {
            std::cerr << "AST::EntryOperation: FunctionDefinition node failed "
                         "to find attribute\"name\""
                      << std::endl;
            std::exit(-1);
        }
        // 创建变量标记，参数阶段
        this->funs.emplace_back();
        this->cur_fun = attr_fun_name->value.GetString();
        this->funs.back().name = this->cur_fun;
        this->cur_param_stage = "parameters";
    }
    if (str_ == "EventDefinition") {
        // “特殊”的函数event
        std::string e_name = node->FindMember("name")->value.GetString();
        this->funs.emplace_back();
        this->funs.back().name = e_name;
        this->funs.back().type = SC_FUN::TYPE::event;
        this->cur_param_stage = "parameters";
    }
    if (str_ == "ModifierDefinition") {
        // modifier作为特殊的函数来建模
        auto attr_name = node->FindMember("name");
        if (attr_name == node->MemberEnd()) {
            std::cerr << "AST::EntryOperation: ModifierDefinition node failed "
                         "to find attribute\"name\""
                      << std::endl;
            std::exit(-1);
        }
        this->funs.emplace_back();
        this->cur_fun = attr_name->value.GetString();
        this->funs.back().name = this->cur_fun;
        this->funs.back().type = SC_FUN::TYPE::modifier;
        this->cur_param_stage = "parameters";
    }
    if (str_ == "EnumDefinition") {
        this->enums.emplace_back();
        auto attr_name = node->FindMember("name");
        if (attr_name == node->MemberEnd()) {
            std::cerr << "AST::EntryOperation: EnumDefinition node failed "
                         "to find attribute\"name\""
                      << std::endl;
            std::exit(-1);
        }
        this->enums.back().name = attr_name->value.GetString();
    }
    return 0;
}

/**
 * 根据参数的字符串，选择应该执行的函数
 * @param str_ nodeType的字符串表示
*/
int AST::FunctionSelector(std::string str_, const rapidjson::Value *node) {
    int ret = -1;
    str_ == "SourceUnit" ? ret = po_SourceUnit(node) : 0;
    str_ == "VariableDeclaration" ? ret = po_VariableDeclaration(node) : 0;
    str_ == "ElementaryTypeName" ? ret = po_ElementaryTypeName(node) : 0;
    str_ == "FunctionDefinition" ? ret = po_FunctionDefinition(node) : 0;
    str_ == "ParameterList" ? ret = po_ParameterList(node) : 0;
    str_ == "UserDefinedTypeName" ? ret = po_UserDefinedTypeName(node) : 0;
    str_ == "EnumValue" ? ret = po_EnumValue(node) : 0;
    str_ == "Mapping" ? ret = po_Mapping(node) : 0;

    if (ret == -1)
        return e_Unknown(str_, node);
    else
        return ret;
}

int AST::e_Unknown(std::string str_, const rapidjson::Value * node) {
    // std::cerr << "Untreated node type: [" << str_ << "]" << std::endl;
    // std::cout << "---modeler exit---" << std::endl;
    return 0;
}

int AST::po_Mapping(const rapidjson::Value *node) {
    this->cur_typename = "map";
    return 0;
}

int AST::po_EnumValue(const rapidjson::Value *node) {
    auto attr_name = node->FindMember("name");
    if (attr_name == node->MemberEnd()) {
        std::cerr << "AST::po_EnumValue: EnumValue node failed to find "
                     "attribute\"name\""
                  << std::endl;
        std::exit(-1);
    }
    this->enums.back().ids.emplace_back(attr_name->value.GetString());
    return 0;
}

int AST::po_UserDefinedTypeName(const rapidjson::Value *node) {
    // 自定义类型
    auto attr_pathNode = node->FindMember("pathNode");
    if (attr_pathNode == node->MemberEnd()) {
        std::cerr << "AST::po_UserDefinedTypeName: can't find  [pathNode]"
                  << std::endl;
        exit(-1);
    }
    auto attr_name = attr_pathNode->value.FindMember("name");
    if (attr_name == node->MemberEnd()) {
        std::cerr << "AST::po_UserDefinedTypeName: can't find [name]"
                  << std::endl;
        exit(-1);
    }
    this->cur_typename = attr_name->value.GetString();
    return 0;
}

int AST::po_SourceUnit(const rapidjson::Value *node) {
    // std::cout << "postorder traversal visit node type:[SourceUnit]"
    //           << std::endl;
    return 0;
}

int AST::po_PragmaDirective(const rapidjson::Value *node) {
    return 0;
}

int AST::po_VariableDeclaration(const rapidjson::Value *node) {
    // 变量名
    auto attr_name = node->FindMember("name");
    if (attr_name==node->MemberEnd()) {
        std::cerr << "AST::po_VariableDeclaration: failed to get \"name\""
                  << std::endl;
        std::exit(-1);
    }
    this->vars.emplace_back();
    // 全局变量、局部变量
    if (this->cur_fun=="") {
        this->vars.back().name = attr_name->value.GetString();
        this->vars.back().range = SC_VAR::global;
    } else {
        this->vars.back().name = attr_name->value.GetString();
        this->vars.back().range = SC_VAR::local;
        this->vars.back().fun = this->cur_fun;
    }
    // 变量类型
    this->vars.back().type = this->cur_typename;

    return 0;
}

int AST::po_ElementaryTypeName(const rapidjson::Value*node) {
    auto attr_name = node->FindMember("name");
    if (attr_name == node->MemberEnd()) {
        std::cerr << "AST::po_ElementaryTypeName: failed to get \"name\""
                  << std::endl;
        std::exit(-1);
    }
    this->cur_typename = attr_name->value.GetString();
    return 0;
}

int AST::po_FunctionDefinition(const rapidjson::Value *node) {
    auto attr_fun_name = node->FindMember("name");
    if (attr_fun_name == node->MemberEnd()) {
        std::cerr << "AST::po_FunctionDefinition: FunctionDefinition node failed "
                     "to find attribute\"name\""
                  << std::endl;
        std::exit(-1);
    }


    // 后序遍历至此，取消当前函数标记
    this->cur_fun = "";
    return 0;
}

int AST::po_ParameterList(const rapidjson::Value *node) {
    // 统计参数数量
    auto attr_params = node->FindMember("parameters");
    if (attr_params == node->MemberEnd()) {
        std::cerr << "AST::po_ParameterList: failed to get \"parameters\" "
                  << std::endl;
        std::exit(-1);
    }
    unsigned int num = attr_params->value.Size();
    // 根据当前stage选择当前的parameters属于参数还是返回值
    std::vector<SC_VAR> &vec = this->cur_param_stage == "parameters"
                                   ? this->funs.back().param
                                   : this->funs.back().param_ret;
    for (int i = 0; i < num; i++) {
        vec.insert(vec.begin(), this->vars.back());
        this->vars.pop_back();
        // 更改类型
        vec.front().range =
            this->cur_param_stage == "parameters" ? SC_VAR::param
                                                  : SC_VAR::ret_param;
    }

    // 阶段切换
    if (this->cur_param_stage == "parameters")
        this->cur_param_stage = "return";
    else if (this->cur_param_stage == "return")
        this->cur_param_stage = "";
    return 0;
}