#include "AST.h"

using namespace rapidjson;



AST::~AST() {}

AST::AST() {}





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
        return -1;
    }
    this->extractSolcFilename(); //提取solc文件的文件名
    // 至此json数据存放在doc中，下一步操作是找到语法树的根节点
    auto node_sources = doc.FindMember("sources"); // sources节点
    if ( node_sources==doc.MemberEnd()) {
        std::cerr << "There is no \"sources\" node in AST(json file)." << std::endl;
        return -1;
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
        // static int cnt = 0;
        // std::cout << ++cnt << "\t";
        // std::cout << "nodeType: " << attr_type->value.GetString() << std::endl;
        if (print_) {
            of_ << long(node) << "[label=\"" << attr_type->value.GetString()
                << "\"]\n";
            if (src_node)
                of_ << (long)src_node << "->" << (long)node << std::endl;
            src_node = node;
        }
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

/**
 * 根据参数的字符串，选择应该执行的函数
*/
int AST::FunctionSelector(std::string str_, const rapidjson::Value *node) {
    int ret = -1;
    str_ == "SourceUnit" ? ret = e_SourceUnit(node) : 0;

    if (ret != 0)
        return e_Unknown(str_, node);
    else
        return 0;
}

int AST::e_Unknown(std::string str_, const rapidjson::Value * node) {
    std::cout << "Unknown node type: [" << str_ << "]" << std::endl;
    // std::cout << "---modeler exit---" << std::endl;
    return 0;
}

int AST::e_SourceUnit(const rapidjson::Value *node) {
    std::cout << "postorder traversal visit node type:[SourceUnit]"
              << std::endl;
    return 0;
}

int AST::e_PragmaDirective(const rapidjson::Value *node) {
    return 0;
}