#include "AST.h"
#include "common.h"

using namespace rapidjson;

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
    this->root = node_AST->value;

    /*
    if (doc.HasMember("sourceList")) {
        const rapidjson::Value &sl = doc["sourceList"];
        std::cout << "sl is array?: " << sl.IsArray() << std::endl;
        // const rapidjson::Value &mem = sl.GetArray();
        for (rapidjson::Value::ConstValueIterator it = sl.Begin();it!=sl.End();++it) {
            std::cout << (it->IsString() ? it->GetString() : "") << std::endl;
        }
    }
    if (doc.HasMember("contracts")) {
        const rapidjson::Value &cs = doc["contracts"];
        std::cout << "contracts Type: "
                  << (cs.GetType() == rapidjson::kObjectType) << std::endl;
        std::cout << "has member (storage.solc:Storage)?: "
                  << cs.HasMember("storage.solc:Storage") << std::endl;
        rapidjson::Value::ConstMemberIterator sss = cs.FindMember("storage.solc:Storage");
        std::cout << sss->value.IsObject() << std::endl;
    }*/
    return 0;
}